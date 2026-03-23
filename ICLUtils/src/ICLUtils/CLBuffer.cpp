/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/CLBuffer.cpp                     **
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

#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/Macros.h>

#include <ICLUtils/CLIncludes.h>

#include <iostream>
#include <sstream>

namespace icl {
  namespace utils {

    struct CLBuffer::Impl {
      cl_mem buffer;
      cl_command_queue cmdQueue;

      static cl_mem_flags stringToMemFlags(const std::string &accessMode) {
        switch(accessMode.length()){
          case 1:
            if(accessMode[0] == 'w') return CL_MEM_WRITE_ONLY;
            if(accessMode[0] == 'r') return CL_MEM_READ_ONLY;
          case 2:
            if( (accessMode[0] == 'r' && accessMode[1] == 'w') ||
                (accessMode[0] == 'w' && accessMode[1] == 'r') ){
              return CL_MEM_READ_WRITE;
            }
          default:
            throw CLBufferException("undefined access-mode '"+accessMode+"' (allowed are 'r', 'w' and 'rw')");
            return CL_MEM_READ_WRITE;
        }
      }

      Impl() : buffer(nullptr), cmdQueue(nullptr) {}

      ~Impl() {
        if (buffer) clReleaseMemObject(buffer);
        if (cmdQueue) clReleaseCommandQueue(cmdQueue);
      }

      Impl(Impl& other) : buffer(other.buffer), cmdQueue(other.cmdQueue) {
        if (buffer) clRetainMemObject(buffer);
        if (cmdQueue) clRetainCommandQueue(cmdQueue);
      }

      Impl(cl_context context, cl_command_queue cmdQueue,
           const std::string &accessMode, size_t size, const void *src = nullptr)
        : buffer(nullptr), cmdQueue(cmdQueue)
      {
        if (this->cmdQueue) clRetainCommandQueue(this->cmdQueue);

        cl_mem_flags memFlags = stringToMemFlags(accessMode);
        if (src) {
          memFlags = memFlags | CL_MEM_COPY_HOST_PTR;
        }

        cl_int err = CL_SUCCESS;
        buffer = clCreateBuffer(context, memFlags, size,
                                const_cast<void*>(src), &err);
        if (err != CL_SUCCESS) {
          throw CLBufferException(CLException::getMessage(err, "clCreateBuffer"));
        }
      }

      void copy(cl_mem dst, int len, int src_offset = 0, int dst_offset = 0) {
        cl_int err = clEnqueueCopyBuffer(cmdQueue, buffer, dst,
                                         src_offset, dst_offset, len,
                                         0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
          throw CLBufferException(CLException::getMessage(err, "clEnqueueCopyBuffer"));
        }
      }

      void read(void *dst, int len, int offset = 0, bool block = true) {
        cl_bool blocking = block ? CL_TRUE : CL_FALSE;
        cl_int err = clEnqueueReadBuffer(cmdQueue, buffer, blocking,
                                         offset, len, dst,
                                         0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
          throw CLBufferException(CLException::getMessage(err, "clEnqueueReadBuffer"));
        }
      }

      void write(const void *src, int len, int offset = 0, bool block = true) {
        cl_bool blocking = block ? CL_TRUE : CL_FALSE;
        cl_int err = clEnqueueWriteBuffer(cmdQueue, buffer, blocking,
                                          offset, len, src,
                                          0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
          throw CLBufferException(CLException::getMessage(err, "clEnqueueWriteBuffer"));
        }
      }

    };

    CLBuffer::CLBuffer(cl_context context, cl_command_queue cmdQueue,
                       const std::string &accessMode, size_t size, const void *src)
      : CLMemory(CLMemory::Buffer) {
      impl = new Impl(context, cmdQueue, accessMode, size, src);
      setDimensions(size,1,1);
      m_byte_depth = 1;
    }

    CLBuffer::CLBuffer(cl_context context, cl_command_queue cmdQueue,
                       const std::string &accessMode, size_t length,
                       size_t byteDepth, const void *src)
      : CLMemory(CLMemory::Buffer) {
      impl = new Impl(context, cmdQueue, accessMode, byteDepth*length, src);
      setDimensions(length,1,1);
      m_byte_depth = byteDepth;
    }

    CLBuffer::CLBuffer()
      : CLMemory(CLMemory::Buffer) {
      impl = new Impl();
    }

    CLBuffer::CLBuffer(const CLBuffer& other)
      : CLMemory(other) {
      impl = new Impl(*(other.impl));
    }

    CLBuffer& CLBuffer::operator=(CLBuffer const& other) {
      if (this != &other) {
        CLMemory::operator=(other);
        // Release old resources
        if (impl->buffer) clReleaseMemObject(impl->buffer);
        if (impl->cmdQueue) clReleaseCommandQueue(impl->cmdQueue);
        // Copy and retain new resources
        impl->buffer = other.impl->buffer;
        impl->cmdQueue = other.impl->cmdQueue;
        if (impl->buffer) clRetainMemObject(impl->buffer);
        if (impl->cmdQueue) clRetainCommandQueue(impl->cmdQueue);
      }
      return *this;
    }

    CLBuffer::~CLBuffer() {
      delete impl;
    }

    void CLBuffer::copy(CLBuffer &dst, int len, int src_offset, int dst_offset) {
      impl->copy(dst.getBuffer(), len, src_offset, dst_offset);
    }

    void CLBuffer::read(void *dst, int len, int offset, bool block) {
      impl->read(dst, len, offset, block);
    }

    void CLBuffer::write(const void *src, int len, int offset, bool block) {
      impl->write(src, len, offset, block);
    }

    cl_mem CLBuffer::getBuffer() {
      return impl->buffer;
    }

    cl_mem CLBuffer::getBuffer() const {
      return impl->buffer;
    }
  }
}
#endif
