// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

// Requires FFmpeg 5.0+ (send/receive encoding API)

#include <ICLIO/LibAVVideoWriter.h>
#include <ICLUtils/File.h>
#include <ICLCore/CCFunctions.h>

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
#define INPUT_FORMAT AV_PIX_FMT_RGB24
#define STREAM_FORMAT AV_PIX_FMT_YUV420P

  struct LibAVVideoWriter::Data {

    AVFormatContext *formatCtx = nullptr;
    const AVCodec *codec = nullptr;
    AVCodecContext *codecCtx = nullptr;
    AVStream *stream = nullptr;
    AVFrame *frame = nullptr;       // encoded format (YUV420P)
    AVFrame *rgbFrame = nullptr;    // input format (RGB24)
    AVPacket *pkt = nullptr;
    SwsContext *swsCtx = nullptr;
    int64_t nextPts = 0;
    int swsWidth = 0, swsHeight = 0;
    std::string filename;
    double fps;
    Size frameSize;

    Data(const std::string &filename, const std::string &fourcc,
         double fps, const Size &frame_size)
      : filename(filename), fps(fps), frameSize(frame_size)
    {
      if(File(filename).exists()){
        throw ICLException("file already exists");
      }

      // Allocate output format context (auto-detect format from filename)
      int ret = avformat_alloc_output_context2(&formatCtx, nullptr,
                  nullptr, filename.c_str());
      if(ret < 0 || !formatCtx){
        throw ICLException("Could not create output context");
      }

      // Find encoder
      codec = avcodec_find_encoder(formatCtx->oformat->video_codec);
      if(!codec){
        throw ICLException("Codec not found for format");
      }

      // Create stream
      stream = avformat_new_stream(formatCtx, codec);
      if(!stream){
        throw ICLException("Could not allocate stream");
      }

      // Allocate codec context
      codecCtx = avcodec_alloc_context3(codec);
      if(!codecCtx){
        throw ICLException("Could not allocate codec context");
      }

      // Configure codec
      codecCtx->width = frameSize.width;
      codecCtx->height = frameSize.height;
      codecCtx->pix_fmt = STREAM_FORMAT;
      codecCtx->time_base = AVRational{1, static_cast<int>(std::round(fps))};
      stream->time_base = codecCtx->time_base;
      codecCtx->gop_size = 12;

      if(codecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO){
        codecCtx->max_b_frames = 2;
      }

      if(formatCtx->oformat->flags & AVFMT_GLOBALHEADER){
        codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
      }

      // Open codec
      ret = avcodec_open2(codecCtx, codec, nullptr);
      if(ret < 0){
        throw ICLException("Could not open codec");
      }

      // Copy codec parameters to stream
      ret = avcodec_parameters_from_context(stream->codecpar, codecCtx);
      if(ret < 0){
        throw ICLException("Could not copy codec parameters");
      }

      // Allocate frames
      frame = av_frame_alloc();
      if(!frame) throw ICLException("Could not allocate video frame");
      frame->format = codecCtx->pix_fmt;
      frame->width = codecCtx->width;
      frame->height = codecCtx->height;
      ret = av_frame_get_buffer(frame, 0);
      if(ret < 0) throw ICLException("Could not allocate frame buffer");

      // Allocate packet
      pkt = av_packet_alloc();
      if(!pkt) throw ICLException("Could not allocate packet");

      // Open output file
      if(!(formatCtx->oformat->flags & AVFMT_NOFILE)){
        ret = avio_open(&formatCtx->pb, filename.c_str(), AVIO_FLAG_WRITE);
        if(ret < 0) throw ICLException("Could not open output file");
      }

      // Write header
      ret = avformat_write_header(formatCtx, nullptr);
      if(ret < 0) throw ICLException("Error writing stream header");
    }

    ~Data(){
      // Flush encoder
      if(codecCtx){
        avcodec_send_frame(codecCtx, nullptr);
        while(avcodec_receive_packet(codecCtx, pkt) == 0){
          av_packet_rescale_ts(pkt, codecCtx->time_base, stream->time_base);
          pkt->stream_index = stream->index;
          av_interleaved_write_frame(formatCtx, pkt);
          av_packet_unref(pkt);
        }
      }

      if(formatCtx) av_write_trailer(formatCtx);

      av_frame_free(&frame);
      av_frame_free(&rgbFrame);
      av_packet_free(&pkt);
      if(swsCtx) sws_freeContext(swsCtx);
      if(codecCtx) avcodec_free_context(&codecCtx);
      if(formatCtx){
        if(!(formatCtx->oformat->flags & AVFMT_NOFILE)){
          avio_closep(&formatCtx->pb);
        }
        avformat_free_context(formatCtx);
      }
    }

    void fillRgbFrame(const ImgBase *src){
      int w = src->getSize().width;
      int h = src->getSize().height;

      // Allocate or resize RGB frame
      if(!rgbFrame || rgbFrame->width != w || rgbFrame->height != h){
        av_frame_free(&rgbFrame);
        rgbFrame = av_frame_alloc();
        rgbFrame->format = INPUT_FORMAT;
        rgbFrame->width = w;
        rgbFrame->height = h;
        if(av_frame_get_buffer(rgbFrame, 0) < 0){
          throw ICLException("Could not allocate RGB frame buffer");
        }
      }
      av_frame_make_writable(rgbFrame);

      // Convert ICL planar image to interleaved RGB
      switch(src->getDepth()){
        case depth8u:
          planarToInterleaved(src->as8u(), rgbFrame->data[0], rgbFrame->linesize[0]);
          break;
        case depth16s:
          planarToInterleaved(src->as16s(), rgbFrame->data[0], rgbFrame->linesize[0]);
          break;
        case depth32s:
          planarToInterleaved(src->as32s(), rgbFrame->data[0], rgbFrame->linesize[0]);
          break;
        case depth32f:
          planarToInterleaved(src->as32f(), rgbFrame->data[0], rgbFrame->linesize[0]);
          break;
        case depth64f:
          planarToInterleaved(src->as64f(), rgbFrame->data[0], rgbFrame->linesize[0]);
          break;
        default:
          planarToInterleaved(src->as8u(), rgbFrame->data[0], rgbFrame->linesize[0]);
          break;
      }
    }

    void writeFrame(const ImgBase *src){
      int sw = src->getSize().width;
      int sh = src->getSize().height;

      fillRgbFrame(src);

      // Scale/convert to codec format if needed
      if(codecCtx->pix_fmt != INPUT_FORMAT ||
         codecCtx->width != sw || codecCtx->height != sh)
      {
        if(!swsCtx || swsWidth != sw || swsHeight != sh){
          if(swsCtx) sws_freeContext(swsCtx);
          swsCtx = sws_getContext(sw, sh, INPUT_FORMAT,
                                  codecCtx->width, codecCtx->height,
                                  codecCtx->pix_fmt,
                                  SWS_BICUBIC, nullptr, nullptr, nullptr);
          if(!swsCtx) throw ICLException("Cannot create conversion context");
          swsWidth = sw;
          swsHeight = sh;
        }
        av_frame_make_writable(frame);
        sws_scale(swsCtx, rgbFrame->data, rgbFrame->linesize,
                  0, sh, frame->data, frame->linesize);
      } else {
        // Direct copy when formats match (unlikely with YUV420P output)
        av_frame_make_writable(frame);
        av_frame_copy(frame, rgbFrame);
      }

      frame->pts = nextPts++;

      // Send frame to encoder
      int ret = avcodec_send_frame(codecCtx, frame);
      if(ret < 0) throw ICLException("Error sending frame to encoder");

      // Receive and write all available packets
      while(ret >= 0){
        ret = avcodec_receive_packet(codecCtx, pkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        if(ret < 0) throw ICLException("Error receiving packet from encoder");

        av_packet_rescale_ts(pkt, codecCtx->time_base, stream->time_base);
        pkt->stream_index = stream->index;

        ret = av_interleaved_write_frame(formatCtx, pkt);
        av_packet_unref(pkt);
        if(ret < 0) throw ICLException("Error writing video frame");
      }
    }
  };

  LibAVVideoWriter::LibAVVideoWriter(const std::string &filename, const std::string &fourcc,
                                     double fps, Size frame_size)
    : m_data(new Data(filename, fourcc, fps, frame_size))
  {
  }

  LibAVVideoWriter::~LibAVVideoWriter(){
    delete m_data;
  }

  void LibAVVideoWriter::send(const Image &image){
    m_data->writeFrame(image.ptr());
  }

  LibAVVideoWriter &LibAVVideoWriter::operator<<(const ImgBase *image){
    m_data->writeFrame(image);
    return *this;
  }

  } // namespace icl::io
