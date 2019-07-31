
// -*- mode: C++ -*-
/*
 *  Copyright (c) 2019, GITAI Inc.
 *  All rights reserved.
 * encoder.cpp
 * Author: Yuki Furuta <me@furushchev.ru>
 */

#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER

#include <atomic>
#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libavutil/intreadwrite.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

void error(const std::string &s)
{
  std::cerr << "\033[1;31mError: " << s << "\033[0m" << std::endl;
  throw std::runtime_error(s);
}

void error(const std::ostringstream &s)
{
  error(s.str());
}

int dispatch_output_packet(void* opaque, uint8_t* buffer, int buffer_size)
{
  std::cout << "dispatch_output_packet: " << buffer_size << std::endl;
}

int main(int argc, char** argv)
{

  if (argc != 2)
  {
    error("encoder filename");
    return 1;
  }

  // load image
  const std::string filename = argv[1];
  const auto image = cv::imread(filename);
  const auto width = image.cols;
  const auto height = image.rows;

  /////////////////////////////////////////////////////////////
  // alloc
  /////////////////////////////////////////////////////////////
  // image streamer
  const int output_width_ = width;
  const int output_height_ = height;
  const bool invert_ = false;

  // libav
  AVOutputFormat* output_format_ = nullptr;
  AVFormatContext* format_context_ = nullptr;
  AVCodec* codec_ = nullptr;
  AVCodecContext* codec_context_ = nullptr;
  AVStream* video_stream_ = nullptr;
  AVFrame* frame_ = nullptr;
  struct SwsContext* sws_context_ = nullptr;
  AVDictionary* opt_ = nullptr;
  const std::string format_name_ = "mp4";
  const std::string codec_name_ = "libx264";
  const int bitrate_ = 100000;
  const int qmin_ = 10;
  const int qmax_ = 42;
  const int gop_ = 250;

  // h264
  const std::string preset_ = "ultrafast";

  /////////////////////////////////////////////////////////////
  // initialize
  /////////////////////////////////////////////////////////////
  // libav
  format_context_ = avformat_alloc_context();
  if (!format_context_) error("avformat_alloc_context");

  output_format_ = av_guess_format(format_name_.c_str(), NULL, NULL);
  if (!output_format_) error("av_guess_format");


  format_context_->oformat = output_format_;

  const size_t io_buffer_size = 3 * 1024;    // 3M seen elsewhere and adjudged good
  unsigned char *io_buffer_ = new unsigned char[io_buffer_size];
  AVIOContext* io_ctx = avio_alloc_context(
      io_buffer_, io_buffer_size, AVIO_FLAG_WRITE, NULL, NULL, dispatch_output_packet, NULL);
  if (!io_ctx) error("avio_alloc_context");

  io_ctx->seekable = 0;
  format_context_->pb = io_ctx;
  output_format_->flags |= AVFMT_FLAG_CUSTOM_IO;
  output_format_->flags |= AVFMT_NOFILE;

  if (codec_name_.empty()) // use default codec if none specified
    codec_ = avcodec_find_encoder(output_format_->video_codec);
  else
    codec_ = avcodec_find_encoder_by_name(codec_name_.c_str());
  if (!codec_) error("avcodec_find_encoder");

  codec_context_ = avcodec_alloc_context3(codec_);
  if (!codec_context_) error("avcodec_alloc_context3");

  codec_context_->codec_id = codec_->id;
  codec_context_->codec_type = codec_->type;
  codec_context_->bit_rate = bitrate_;

  codec_context_->width = output_width_;
  codec_context_->height = output_height_;
  codec_context_->delay = 0;

  codec_context_->time_base.num = 1;
  codec_context_->time_base.den = 1;
  codec_context_->gop_size = gop_;
  codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;
  codec_context_->max_b_frames = 0;

  // Quality settings
  codec_context_->qmin = qmin_;
  codec_context_->qmax = qmax_;

  // Initialize encoder (h264)
  av_opt_set(codec_context_->priv_data, "preset", preset_.c_str(), 0);
  av_opt_set(codec_context_->priv_data, "tune", "zerolatency", 0);
  av_opt_set_int(codec_context_->priv_data, "crf", 20, 0);
  av_opt_set_int(codec_context_->priv_data, "bufsize", 100, 0);
  av_opt_set_int(codec_context_->priv_data, "keyint", 30, 0);
  av_opt_set_int(codec_context_->priv_data, "g", 1, 0);
  if (!strcmp(format_context_->oformat->name, "mp4"))
    av_dict_set(&opt_, "movflags", "+frag_keyframe+empty_moov+faststart", 0);

  // libav
  if (format_context_->oformat->flags & AVFMT_GLOBALHEADER)
    codec_context_->flags |= CODEC_FLAG_GLOBAL_HEADER;

  video_stream_ = avformat_new_stream(format_context_, codec_);
  if (!video_stream_) error("avformat_new_stream");
  video_stream_->time_base.num = 1;
  video_stream_->time_base.den = 1000;
  if (avcodec_parameters_from_context(video_stream_->codecpar, codec_context_) < 0) error("avcodec_parameters_from_context");

  if (avcodec_open2(codec_context_, codec_, NULL) < 0) error("avcodec_open2");
  frame_ = av_frame_alloc();
  av_image_alloc(frame_->data, frame_->linesize, output_width_, output_height_,
                 codec_context_->pix_fmt, 1);

  frame_->width = output_width_;
  frame_->height = output_height_;
  frame_->format = codec_context_->pix_fmt;
  output_format_->flags |= AVFMT_NOFILE;

  std::vector<uint8_t> header_buffer;
  std::size_t header_size;
  uint8_t *header_raw_buffer;
  // define meta data
  av_dict_set(&format_context_->metadata, "author", "gitai", 0);
  av_dict_set(&format_context_->metadata, "title", "gitai", 0);

  if (avformat_write_header(format_context_, &opt_) < 0) error("avformat_write_header");

  /////////////////////////////////////////////////////////////
  // send image
  /////////////////////////////////////////////////////////////

  AVPixelFormat input_coding_format = AV_PIX_FMT_BGR24;

  AVFrame *raw_frame = av_frame_alloc();
  av_image_fill_arrays(raw_frame->data, raw_frame->linesize,
                       image.data, input_coding_format, output_width_, output_height_, 1);

  // Convert from opencv to libav
  if (!sws_context_)
  {
    static int sws_flags = SWS_BICUBIC;
    sws_context_ = sws_getContext(output_width_, output_height_, input_coding_format, output_width_, output_height_,
                                  codec_context_->pix_fmt, sws_flags, NULL, NULL, NULL);
    if (!sws_context_)
    {
      throw std::runtime_error("Could not initialize the conversion context");
    }
  }

  int ret = sws_scale(sws_context_,
          (const uint8_t * const *)raw_frame->data, raw_frame->linesize, 0,
          output_height_, frame_->data, frame_->linesize);

  av_frame_free(&raw_frame);

  // Encode the frame
  AVPacket pkt;
  int got_packet = 0;
  av_init_packet(&pkt);

  pkt.data = NULL; // packet data will be allocated by the encoder
  pkt.size = 0;
  if (avcodec_send_frame(codec_context_, frame_) < 0)
  {
    throw std::runtime_error("Error encoding video frame");
  }
  if (avcodec_receive_packet(codec_context_, &pkt) < 0)
  {
    throw std::runtime_error("Error retrieving encoded packet");
  }

  if (pkt.size > 0)
  {
    std::size_t size;
    uint8_t *output_buf;

    // double seconds = (time - first_image_timestamp_).toSec();
    double seconds = 1.0 / 25.0;
    // Encode video at 1/0.95 to minimize delay
    pkt.pts = (int64_t)(seconds / av_q2d(video_stream_->time_base) * 0.95);
    if (pkt.pts <= 0)
      pkt.pts = 1;
    pkt.dts = AV_NOPTS_VALUE;

    if (pkt.flags&AV_PKT_FLAG_KEY)
      pkt.flags |= AV_PKT_FLAG_KEY;

    pkt.stream_index = video_stream_->index;

    if (av_write_frame(format_context_, &pkt))
    {
      throw std::runtime_error("Error when writing frame");
    }
  }

  avcodec_close(codec_context_);
  avcodec_free_context(&codec_context_);
  av_frame_free(&frame_);
  delete io_buffer_;
  av_packet_unref(&pkt);
  av_free(format_context_->pb);
  avformat_free_context(format_context_);
  sws_freeContext(sws_context_);

  return 0;
}
