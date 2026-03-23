/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLUtils/src/ICLUtils/CLImage2D.cpp                    **
 ** Module : ICLUtils                                               **
 ** Authors: Viktor Losing                                          **
 **                                                                 **
 **                                                                 **
 ** GNU LESSER GENERAL PUBLIC LICENSE                               **
 ** This file may be used under the terms of the GNU Lesser General **
 ** Public License version 3.0 as published by the                  **
 **                                                                 **
 ** Free Software Foundation and appearing in the file LICENSE.GPL  **
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
#ifdef ICL_HAVE_OPENCL

#include <ICLUtils/CLImage2D.h>
#include <ICLUtils/Macros.h>

#include <ICLUtils/CLIncludes.h>

#include <iostream>
#include <sstream>

#include <set>
#include <map>

namespace icl {
  namespace utils {

    struct CLImage2D::Impl {
      size_t width;
      size_t height;
      cl_mem image2D;
      cl_command_queue cmdQueue;
      std::map<uint32_t, std::set<uint32_t>> supported_channel_orders;

      static cl_mem_flags stringToMemFlags(const std::string &accessMode) {
        switch (accessMode.length()) {
          case 1:
            if (accessMode[0] == 'w') return CL_MEM_WRITE_ONLY;
            if (accessMode[0] == 'r') return CL_MEM_READ_ONLY;
            // fall through
          case 2:
            if ((accessMode[0] == 'r' && accessMode[1] == 'w') ||
                (accessMode[0] == 'w' && accessMode[1] == 'r')) {
              return CL_MEM_READ_WRITE;
            }
            // fall through
          default:
            throw CLBufferException("undefined access-mode '" + accessMode +
                                    "' (allowed are 'r', 'w' and 'rw')");
            return CL_MEM_READ_WRITE;
        }
      }

      static cl_channel_order parseChannelOrder(const int num_channels) {
        cl_channel_order order = CL_R;
        switch (num_channels) {
          case 1: order = CL_R; break;
          case 2: order = CL_RA; break;
          case 3: order = CL_RGB; break;
          case 4: order = CL_RGBA; break;
          default: {
            std::stringstream sstream;
            sstream << num_channels;
            throw CLBufferException("unsupported number of channels: " + sstream.str());
          }
        }
        return order;
      }

      bool checkSupportedImageFormat(cl_channel_order order, cl_channel_type type) {
        if (supported_channel_orders.count(order)) {
          return supported_channel_orders[order].count(type);
        }
        return false;
      }

      Impl() : width(0), height(0), image2D(nullptr), cmdQueue(nullptr) {}

      ~Impl() {
        if (image2D) clReleaseMemObject(image2D);
        if (cmdQueue) clReleaseCommandQueue(cmdQueue);
      }

      Impl(const Impl &other)
        : width(other.width), height(other.height),
          image2D(other.image2D), cmdQueue(other.cmdQueue),
          supported_channel_orders(other.supported_channel_orders) {
        if (image2D) clRetainMemObject(image2D);
        if (cmdQueue) clRetainCommandQueue(cmdQueue);
      }

      Impl &operator=(const Impl &other) {
        if (this == &other) return *this;
        // Release old handles
        if (image2D) clReleaseMemObject(image2D);
        if (cmdQueue) clReleaseCommandQueue(cmdQueue);
        // Copy and retain new handles
        width = other.width;
        height = other.height;
        image2D = other.image2D;
        cmdQueue = other.cmdQueue;
        supported_channel_orders = other.supported_channel_orders;
        if (image2D) clRetainMemObject(image2D);
        if (cmdQueue) clRetainCommandQueue(cmdQueue);
        return *this;
      }

      Impl(cl_context context, cl_command_queue cmdQueue,
           const std::string &accessMode, const size_t width, const size_t height,
           int depth, int num_channel, const void *src = nullptr,
           const std::map<uint32_t, std::set<uint32_t>>
               &supported_formats = std::map<uint32_t, std::set<uint32_t>>())
        : width(width), height(height), image2D(nullptr),
          cmdQueue(cmdQueue), supported_channel_orders(supported_formats) {

        if (this->cmdQueue) clRetainCommandQueue(this->cmdQueue);

        cl_mem_flags memFlags = stringToMemFlags(accessMode);
        if (src) {
          memFlags = memFlags | CL_MEM_COPY_HOST_PTR;
        }

        cl_channel_type channelType;
        switch (depth) {
          case 0: channelType = CL_UNSIGNED_INT8; break;
          case 1: channelType = CL_SIGNED_INT16; break;
          case 2: channelType = CL_SIGNED_INT32; break;
          case 3: channelType = CL_FLOAT; break;
          default:
            throw CLBufferException("unknown depth value");
        }

        cl_channel_order order = parseChannelOrder(num_channel);
        if (!checkSupportedImageFormat(order, channelType)) {
          std::stringstream sstream;
          sstream << "channel: " << num_channel << ", depth-type: " << depth;
          throw CLBufferException("No such image type is supported: " + sstream.str());
        }

        cl_image_format fmt;
        fmt.image_channel_order = order;
        fmt.image_channel_data_type = channelType;

        cl_image_desc desc = {};
        desc.image_type = CL_MEM_OBJECT_IMAGE2D;
        desc.image_width = width;
        desc.image_height = height;

        cl_int err = CL_SUCCESS;
        image2D = clCreateImage(context, memFlags, &fmt, &desc,
                                const_cast<void *>(src), &err);
        if (err != CL_SUCCESS) {
          throw CLBufferException(CLException::getMessage(err, "clCreateImage"));
        }
      }

      void regionToArrays(const utils::Rect &region,
                           size_t origin[3], size_t rgn[3]) {
        origin[0] = region.x;
        origin[1] = region.y;
        origin[2] = 0;

        if (region == Rect::null) {
          rgn[0] = width;
          rgn[1] = height;
        } else {
          rgn[0] = region.width;
          rgn[1] = region.height;
        }
        rgn[2] = 1;
      }

      void read(void *dst, const utils::Rect &region = utils::Rect::null,
                bool block = true) {
        cl_bool blocking = block ? CL_TRUE : CL_FALSE;
        size_t origin[3];
        size_t rgn[3];
        regionToArrays(region, origin, rgn);

        cl_int err = clEnqueueReadImage(cmdQueue, image2D, blocking,
                                        origin, rgn, 0, 0, dst,
                                        0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
          throw CLBufferException(CLException::getMessage(err, "clEnqueueReadImage"));
        }
      }

      void write(void *src, const utils::Rect &region = utils::Rect::null,
                 bool block = true) {
        cl_bool blocking = block ? CL_TRUE : CL_FALSE;
        size_t origin[3];
        size_t rgn[3];
        regionToArrays(region, origin, rgn);

        cl_int err = clEnqueueWriteImage(cmdQueue, image2D, blocking,
                                         origin, rgn, 0, 0, src,
                                         0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
          throw CLBufferException(CLException::getMessage(err, "clEnqueueWriteImage"));
        }
      }

      static const icl32s iclDepthToByteDepth(int icl_depth) {
        switch (icl_depth) {
          case 0: return 1;
          case 1: return 2;
          case 2: return 4;
          case 3: return 4;
          case 4: return 8;
          default: return 1;
        }
      }
    };

    CLImage2D::CLImage2D(cl_context context, cl_command_queue cmdQueue,
                         const std::string &accessMode, const size_t width,
                         const size_t height, int depth, int num_channel,
                         const void *src,
                         const std::map<uint32_t, std::set<uint32_t>> &supported_formats)
      : CLMemory(CLMemory::Image2D) {
      impl = new Impl(context, cmdQueue, accessMode, width, height,
                      depth, num_channel, src, supported_formats);
      setDimensions(width, height, num_channel);
      m_byte_depth = Impl::iclDepthToByteDepth(depth);
    }

    CLImage2D::CLImage2D()
      : CLMemory(CLMemory::Image2D) {
      impl = new Impl();
    }

    CLImage2D::CLImage2D(const CLImage2D &other)
      : CLMemory(other) {
      impl = new Impl(*(other.impl));
    }

    CLImage2D &CLImage2D::operator=(CLImage2D const &other) {
      CLMemory::operator=(other);
      *impl = *(other.impl);
      return *this;
    }

    CLImage2D::~CLImage2D() {
      delete impl;
    }

    void CLImage2D::read(void *dst, const utils::Rect &region, bool block) {
      impl->read(dst, region, block);
    }

    void CLImage2D::write(const void *src, const utils::Rect &region, bool block) {
      impl->write(const_cast<void *>(src), region, block);
    }

    cl_mem CLImage2D::getImage2D() {
      return impl->image2D;
    }

    cl_mem CLImage2D::getImage2D() const {
      return impl->image2D;
    }
  }
}
#endif
