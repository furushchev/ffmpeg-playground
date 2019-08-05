
// -*- mode: C++ -*-
/*
 *  Copyright (c) 2019, GITAI Inc.
 *  All rights reserved.
 * decoder_h264_x264.cpp
 * Author: Yuki Furuta <me@furushchev.ru>
 */

#include <atomic>
#include <iostream>
#include <memory>
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

int main(int argc, char** argv)
{
  AVCodec* codec_ = nullptr;

  const std::string codec_name_ = "libx264";

  if (codec_name_.empty()) // use default codec if none specified
    codec_ = avcodec_find_encoder(output_format_->video_codec);
  else
    codec_ = avcodec_find_encoder_by_name(codec_name_.c_str());
  if (!codec_) error("avcodec_find_encoder");

  return 0;
}
