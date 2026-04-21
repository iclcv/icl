// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/V4L2LoopBackOutput.h>
#include <icl/core/CCFunctions.h>
#include <icl/utils/StringUtils.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

namespace icl::io {
    struct V4L2LoopBackOutput::Data{
      std::vector<icl8u> m_out;
      std::string m_device;
      int m_handle;
      utils::Size m_deviceSize;
      core::format m_deviceFormat;
      v4l2_capability caps;
      v4l2_format fmt;
      Data():m_handle(-1){}

      ~Data(){
        if(m_handle > 0){
          close(m_handle);
        }
      }
      void init(const std::string &device){
        if(m_handle > 0){
          close(m_handle);
        }
        m_device = device;

        m_handle = open(device.c_str(), O_RDWR);
        if(m_handle < 0) throw utils::ICLException("could not open device " + device);

        memset(&caps, 0, sizeof(caps));
        memset(&fmt, 0, sizeof(fmt));

        int r = ioctl(m_handle, VIDIOC_QUERYCAP, &caps);
        if(r == -1) throw utils::ICLException("could not query device capabilities");
      }
      void ensureDeviceSizeAndFormat(const utils::Size &s, const core::format f){
        if(m_deviceSize != s || m_deviceFormat != f){
          m_deviceSize = s;
          m_deviceFormat = f;
          fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
          fmt.fmt.pix.width = s.width;
          fmt.fmt.pix.height = s.height;
          fmt.fmt.pix.pixelformat =  (f == core::formatGray
                                      ? V4L2_PIX_FMT_GREY
                                      : V4L2_PIX_FMT_RGB24);

          fmt.fmt.pix.sizeimage = fmt.fmt.pix.width * fmt.fmt.pix.height  * 3;
          fmt.fmt.pix.field = V4L2_FIELD_NONE;
          fmt.fmt.pix.bytesperline = fmt.fmt.pix.width * 3;
          fmt.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

          int r = ioctl(m_handle, VIDIOC_S_FMT, &fmt);
          if(r == -1) throw utils::ICLException("could set query video format");

          v4l2_format getFmt = fmt;

          r = ioctl(m_handle, VIDIOC_G_FMT, &getFmt);
          if(r == -1) throw utils::ICLException("could not query video format");

          //show_fmt(getFmt);
#define CHECK(X)                                                        \
          if(fmt.fmt.pix.X != getFmt.fmt.pix.X) {                       \
            std::cout << "V4L2LoopBackOutput: error "              \
            "setting up device parameter " << #X << std::endl;          \
          }
          CHECK(width);
          CHECK(height);
          CHECK(pixelformat);
          CHECK(sizeimage);
          CHECK(field);
          CHECK(bytesperline);
          CHECK(colorspace);
        }
      }
      void send(const core::Image &image){
        using namespace utils;
        using namespace core;

        core::format f = image->getFormat();
        ICLASSERT_THROW(m_handle > 0, ICLException("V4L2LoopBackOutput::send: device not initialized"));
        ICLASSERT_THROW(image, ICLException("V4L2LoopBackOutput::send: image was null"));
        ICLASSERT_THROW(image->getDim(), ICLException("V4L2LoopBackOutput::send: image dimension is null"));
        ICLASSERT_THROW(f == formatRGB || f == formatGray, ICLException("V4L2LoopBackOutput::send: image must be in gray or RGB format"));
        ICLASSERT_THROW(image->getDepth() == depth8u, ICLException("V4L2LoopBackOutput::send: image must have depth8u"));


        ensureDeviceSizeAndFormat(image->getSize(), f);


        int r = 0;
        if(f == formatRGB){
          m_out.resize(image->getDim()*(f == core::formatGray ? 1 : 3));
          planarToInterleaved(image->as8u(), m_out.data());
          r = write(m_handle, m_out.data(), m_out.size());
        }else{
          r = write(m_handle, image->getDataPtr(0), image->getDim());
        }

        if(r < 0){
          DEBUG_LOG("V4L2LoopBackOutput::send: could not write data to " << m_device);
        }
      }
    };

    V4L2LoopBackOutput::V4L2LoopBackOutput(const std::string &device):
      m_data(new Data){

      init(device);
    }

    void V4L2LoopBackOutput::init(const std::string &device){
      if(device.length() == 1 || device.length() == 2){
        int id = utils::parse<int>(device);
        if(id >= 0 && id <= 99){
          m_data->init("/dev/video"+utils::str(id));
        }else{
          throw utils::ICLException("V4L2LoopBackOutput:init("
                                    +device+
                                    ") unable to parse given device index");
        }
      }else{
        m_data->init(device);
      }
    }

    V4L2LoopBackOutput::V4L2LoopBackOutput():
      m_data(new Data){
    }


    V4L2LoopBackOutput::~V4L2LoopBackOutput(){
      delete m_data;
    }

    void send(const core::Image &image){
      m_data->send(image);
    }
  }