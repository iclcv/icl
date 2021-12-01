/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/LibAVVideoWriter.cpp                   **
** Module : ICLIO                                                  **
** Authors: Matthias Esau                                          **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLIO/LibAVVideoWriter.h>
#include <ICLUtils/File.h>
#include <ICLCore/CCFunctions.h>

extern "C"
{
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavresample/avresample.h>
#include <libswscale/swscale.h>
#include <libavcodec/version.h>
}

//#if LIBAVCODEC_VERSION_MAJOR <= 54

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{


    struct LibAVVideoWriter::Data{
      Data(const std::string &filename, const std::string &fourcc, double fps, const Size &frame_size):
        video_st(),filename(filename),fps(fps),frame_size(frame_size){

        DEBUG_LOG("fps:" << fps);
        if(File(filename).exists()){
          throw ICLException("file already exists");
        }
        avformat_alloc_output_context2(&oc, nullptr, fourcc.empty() ? nullptr : fourcc.c_str(), filename.c_str());
        if (!oc) throw ICLException("Memory error");
        AVOutputFormat *fmt = oc->oformat;
        add_video_stream(&video_st, oc, fmt->video_codec);
        open_video(oc, &video_st);
        av_dump_format(oc, 0, filename.c_str(), 1);
        if (!(fmt->flags & AVFMT_NOFILE)) {
            if (avio_open(&oc->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
                throw ICLException("Could not open file");
            }
        }
        if (avformat_write_header(oc, nullptr) < 0)
          throw ICLException("Error writing video stream header");
      }

      ~Data(){
        av_write_trailer(oc);
        close_stream(oc, &video_st);
        if (!(oc->oformat->flags & AVFMT_NOFILE))avio_close(oc->pb);
        avformat_free_context(oc);
      }
      struct OutputStream {
        AVStream *st;
        AVCodecContext *enc;
        int64_t next_pts;
        AVFrame *frame;
        AVFrame *tmp_frame;
        float t, tincr, tincr2;
        struct SwsContext *sws_ctx;
        int sws_ctx_width, sws_ctx_height;
      };

      OutputStream video_st;
      std::string filename;
      AVFormatContext *oc;
      double fps;
      utils::Size frame_size;
      void add_video_stream(OutputStream *ost, AVFormatContext *oc, enum AVCodecID codec_id);
      AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);
      void open_video(AVFormatContext *oc, OutputStream *ost);
      void close_stream(AVFormatContext *oc, OutputStream *ost);
      void fill_rgb_image(const core::ImgBase *src, AVFrame **pict);
      AVFrame *get_video_frame(const core::ImgBase *src, OutputStream *ost);
      int write_video_frame(const core::ImgBase *src, AVFormatContext *oc, OutputStream *ost);
    };

#define INPUT_FORMAT AV_PIX_FMT_RGB24
#define STREAM_FORMAT AV_PIX_FMT_YUV420P
    AVFrame *LibAVVideoWriter::Data::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
    {
        AVFrame *picture;
#if LIBAVCODEC_VERSION_MAJOR > 54
        picture = av_frame_alloc();
#else
        picture = avcodec_alloc_frame();
#endif
        if (!picture)
            return 0;

        picture->format = pix_fmt;
        picture->width  = width;
        picture->height = height;

        /* allocate the buffers for the frame data */
#if LIBAVCODEC_VERSION_MAJOR > 54
        if (av_frame_get_buffer(picture, 32) < 0) throw ICLException("Could not allocate frame data");
#else
        int size = avpicture_get_size(pix_fmt, width, height);
        uint8_t *picture_buf = (uint8_t*) av_malloc(size);
        if (!picture_buf) {
	    throw ICLException("Could not allocate frame data");
        }
        avpicture_fill((AVPicture *)picture, picture_buf,
                       pix_fmt, width, height);
#endif

        return picture;
    }

    void LibAVVideoWriter::Data::open_video(AVFormatContext *oc, OutputStream *ost)
    {
        AVCodecContext *c = ost->enc;

        /* open the codec */
        if (avcodec_open2(c, 0, 0) < 0) throw ICLException("could not open codec");

        /* Allocate the encoded raw picture. */
        ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
        if (!ost->frame) throw ICLException("Could not allocate picture");

        /* If the output format is not YUV420P, then a temporary YUV420P
         * picture is needed too. It is then converted to the required
         * output format. */
        ost->tmp_frame = 0;
    }


    void LibAVVideoWriter::Data::add_video_stream(OutputStream *ost, AVFormatContext *oc,
                                 enum AVCodecID codec_id)
    {
        AVCodecContext *c;
        AVCodec *codec;

        /* find the video encoder */
        codec = avcodec_find_encoder(codec_id);
        if (!codec) throw ICLException("codec not found");

        ost->st = avformat_new_stream(oc, nullptr);
        if (!ost->st) throw ICLException("could not allocate stream");

        c = avcodec_alloc_context3(codec);
        if (!c) throw ICLException("could not allocate encoding context");
        ost->enc = c;

        /* Put sample parameters. */
        //c->bit_rate = 400000;
        /* Resolution must be a multiple of two. */
        c->width    = frame_size.width;
        c->height   = frame_size.height;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
        ost->st->time_base = (AVRational){ 10, (int)(fps*10.) };
        std::cout<<"fps:"<<fps<<std::endl;
        c->time_base       = ost->st->time_base;

        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = STREAM_FORMAT;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }
        /* Some formats want stream headers to be separate. */
        if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        #if LIBAVCODEC_VERSION_MAJOR > 56
            c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        #else
            c->flags |= CODEC_FLAG_GLOBAL_HEADER;
        #endif
    }

    void LibAVVideoWriter::Data::close_stream(AVFormatContext *oc, OutputStream *ost)
    {


        avcodec_close(ost->enc);

#if LIBAVCODEC_VERSION_MAJOR > 54

        av_frame_free(&ost->frame);
        av_frame_free(&ost->tmp_frame);
#else
        WARNING_LOG("warning: proper memory deallocation is skipped to avoid seg-fault! please fix!");
        //av_free(&ost->frame->data[0]);  if we do this, feeing the frame crashes!
        //av_free(&ost->frame);
        //av_free(&ost->tmp_frame->data[0]);
        //av_free(&ost->tmp_frame);
#endif
        sws_freeContext(ost->sws_ctx);
    }

    void LibAVVideoWriter::Data::fill_rgb_image(const ImgBase *src, AVFrame **pict)
    {
      if(!*pict) {
        *pict = alloc_picture(INPUT_FORMAT,src->getSize().width,src->getSize().height);
      } else {
        if((*pict)->width != src->getSize().width || (*pict)->height != src->getSize().height) {
#if LIBAVCODEC_VERSION_MAJOR > 54
          av_frame_free(pict);
#else
          av_free((*pict)->data[0]);
          av_free(*pict);
#endif
          *pict = alloc_picture(INPUT_FORMAT,src->getSize().width,src->getSize().height);
        }
      }
#if LIBAVCODEC_VERSION_MAJOR > 54
      av_frame_make_writable(*pict);
#endif
      depth d = src->getDepth();
      switch(d) {
        case depth16s:
          core::planarToInterleaved(src->as16s(),(*pict)->data[0],(*pict)->linesize[0]);
        case depth32f:
          core::planarToInterleaved(src->as32f(),(*pict)->data[0],(*pict)->linesize[0]);
        case depth32s:
          core::planarToInterleaved(src->as32s(),(*pict)->data[0],(*pict)->linesize[0]);
        case depth64f:
          core::planarToInterleaved(src->as64f(),(*pict)->data[0],(*pict)->linesize[0]);
        default:
          core::planarToInterleaved(src->as8u(),(*pict)->data[0],(*pict)->linesize[0]);
      }
    }

    AVFrame *LibAVVideoWriter::Data::get_video_frame(const ImgBase *src, OutputStream *ost)
    {
        AVCodecContext *c = ost->enc;
        int sw = src->getSize().width;
        int sh = src->getSize().height;
        //check if format or size of the input and output do not match
        if (c->pix_fmt != INPUT_FORMAT || c->width != sw || c->height != sh) {
            //check if the scale context needs to be updated
            if (!ost->sws_ctx || ost->sws_ctx_width != sw || ost->sws_ctx_height != sh) {
                //update context
                if(ost->sws_ctx)sws_freeContext(ost->sws_ctx);
                ost->sws_ctx = sws_getContext(sw, sh,
                                              INPUT_FORMAT,
                                              c->width, c->height,
                                              c->pix_fmt,
                                              SWS_BICUBIC, 0, 0, 0);
                if (!ost->sws_ctx) throw ICLException("Cannot initialize the conversion context");
            }
            fill_rgb_image(src,&(ost->tmp_frame));
            sws_scale(ost->sws_ctx, ost->tmp_frame->data, ost->tmp_frame->linesize,
                      0, ost->tmp_frame->height, ost->frame->data, ost->frame->linesize);
        } else {
            fill_rgb_image(src,&(ost->frame));
        }

        ost->frame->pts = ost->next_pts++;

        return ost->frame;
    }

    int LibAVVideoWriter::Data::write_video_frame(const ImgBase *src, AVFormatContext *oc, OutputStream *ost)
    {
        int ret;
        AVCodecContext *c;
        AVFrame *frame;
        int got_packet = 0;

        c = ost->enc;

        frame = get_video_frame(src,ost);

        AVPacket pkt = { 0 };
        av_init_packet(&pkt);

        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
        if (ret < 0) throw ICLException("Error encoding a video frame");

        if (got_packet) {
#if LIBAVCODEC_VERSION_MAJOR > 54
            av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);
#endif
            pkt.stream_index = ost->st->index;

            /* Write the compressed frame to the media file. */
            ret = av_interleaved_write_frame(oc, &pkt);
        }
        if (ret != 0) throw ICLException("Error while writing video frame");
        return (frame || got_packet) ? 0 : 1;
    }

    LibAVVideoWriter::LibAVVideoWriter(const std::string &filename, const std::string &fourcc,
                                       double fps, Size frame_size):
      m_data(new Data(filename, fourcc, fps, frame_size)){
    }

    LibAVVideoWriter::~LibAVVideoWriter(){
      delete m_data;
    }

    void LibAVVideoWriter::send(const ImgBase *image){
       m_data->write_video_frame(image, m_data->oc, &m_data->video_st);
    }


    LibAVVideoWriter &LibAVVideoWriter::operator<<(const ImgBase *image){
      m_data->write_video_frame(image, m_data->oc, &m_data->video_st);
      return *this;
    }
  } // namespace io

}
//#endif
