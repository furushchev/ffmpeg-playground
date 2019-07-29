
// -*- mode: C++ -*-
/*
 *  Copyright (c) 2019, GITAI Inc.
 *  All rights reserved.
 * encoder.cpp
 * Author: Yuki Furuta <me@furushchev.ru>
 */

#include <atomic>
#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>
#include <libavutil/hwcontext.h>
}


int main(int argc, char** argv)
{

  if (argc != 2)
  {
    std::cerr << "encoder filename" << std::endl;
    return 1;
  }

  // load image
  std::string filename = argv[1];
  const auto image = cv::imread(filename);
  const auto width = image.cols;
  const auto height = image.rows;
  const uint8_t *data_ptr = &image.data[0];
  const int step = static_cast<const int>(width * 3);

  AVBufferRef *m_hw_device_ctx;
  std::atomic<bool> m_initialized;
  AVCodecContext *m_enc_ctx;
  SwsContext *m_sws_ctx;
  AVFrame *m_frame;
  AVCodec *codec = NULL;

  auto err = av_hwdevice_ctx_create(&m_hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, NULL, NULL, 0);
  if (err < 0)
  {
    std::cerr << "failed av_hwdevice_ctx_create" << std::endl;
    return 1;
  }

  if (!(codec = avcodec_find_encoder_by_name("h264_vaapi")))
  {
    std::cerr <<  "Failed to find h264_vaapi encoder" << std::endl;
    return 1;
  }

  if (!(m_enc_ctx = avcodec_alloc_context3(codec)))
  {
    std::cerr << "No memory allocated for codec" << std::endl;
    return 1;
  }

  m_enc_ctx->pix_fmt = AV_PIX_FMT_VAAPI;
  m_enc_ctx->width = width;
  m_enc_ctx->height = height;
  m_enc_ctx->time_base = (AVRational){ 1, 25 };
  m_enc_ctx->framerate = (AVRational){ 25, 1 };
  m_enc_ctx->gop_size = 10;
  m_enc_ctx->max_b_frames = 1;
  m_enc_ctx->bit_rate = 512000; // TODO: parameterize
  m_enc_ctx->qmax = 20; // TODO: parameterize

  AVBufferRef *hw_frames_ref;
  AVHWFramesContext *frames_ctx = NULL;
  if (!(hw_frames_ref = av_hwframe_ctx_alloc(m_hw_device_ctx))) {
    std::cerr << "Failed to create VAAPI frame context." << std::endl;
    return 1;
  }
  frames_ctx = (AVHWFramesContext *)(hw_frames_ref->data);
  frames_ctx->format    = AV_PIX_FMT_VAAPI;
  frames_ctx->sw_format = m_enc_ctx->sw_pix_fmt;
  frames_ctx->width     = width;
  frames_ctx->height    = height;
  frames_ctx->initial_pool_size = 20;
  if ((err = av_hwframe_ctx_init(hw_frames_ref)) < 0) {
    std::cerr << "Failed to initialize VAAPI frame context." << std::endl;
    av_buffer_unref(&hw_frames_ref);
    return 1;
  }
  m_enc_ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
  if (!m_enc_ctx->hw_frames_ctx)
  {
    std::cerr << "Failed to alloc memory for hw_frames_ctx" << std::endl;
    return 1;
  }
  av_buffer_unref(&hw_frames_ref);

  // if ((err = set_hwframe_ctx(m_enc_ctx, m_hw_device_ctx)) < 0) {
  //   std::cerr << "Failed to set hwframe context" << std::endl;
  //   return 1;
  // }

  if ((err = avcodec_open2(m_enc_ctx, codec, NULL)) < 0)
  {
    std::cerr << "Failed to open codec" << std::endl;
    return 1;
  }
  m_sws_ctx = sws_getCachedContext(m_sws_ctx,
                                   width, height, AV_PIX_FMT_BGR24,
                                   m_enc_ctx->width, m_enc_ctx->height, m_enc_ctx->pix_fmt,
                                   SWS_FAST_BILINEAR, NULL, NULL, NULL);

  AVPacket packet;

  sws_scale(m_sws_ctx, &data_ptr, &step, 0, height, m_frame->data, m_frame->linesize);

  int result = 0;

  av_init_packet(&packet);

  result = avcodec_send_frame(m_enc_ctx, m_frame);
  if (result < 0)
  {
    std::cerr << "Failed to send frame data to encoder" << std::endl;
    return 1;
  }
  result = avcodec_receive_packet(m_enc_ctx, &packet);
  if (result < 0)
  {
    std::cerr << "Failed to get packet from encoder" << std::endl;
    return 1;
  }

  if (packet.size > 0)
  {
    const std::vector<uint8_t> packet_vec(packet.data, packet.data + packet.size);
    cv::Mat encoded(packet_vec);
    cv::Mat decoded = cv::imdecode(encoded, 1);
    cv::imshow("decoded", decoded);
    cv::waitKey(1000000000);
    av_packet_unref(&packet);
  }
  else
  {
    std::cerr << "empty packet received" << std::endl;
  }

  if (m_enc_ctx) avcodec_free_context(&m_enc_ctx);
  if (m_sws_ctx) sws_freeContext(m_sws_ctx);
  if (m_frame) av_frame_free(&m_frame);
  if (m_hw_device_ctx) av_buffer_unref(&m_hw_device_ctx);

  return 0;
}
