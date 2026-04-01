// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Viktor Losing, Christof Elbrechter

#ifdef ICL_HAVE_OPENCL
#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/CLException.h>
#include <ICLUtils/CLMemory.h>
#include <ICLUtils/CLIncludes.h>
#include <string>

namespace icl {
  namespace utils {

    /// Wrapper for an OpenCL Buffer
    /** Valid CLBuffer instances can only be created by a CLProgram instance.
        @see CLProgram for more details */
	class ICLUtils_API CLBuffer : public CLMemory {
      struct Impl;
      Impl *impl;

      CLBuffer(cl_context context, cl_command_queue cmdQueue,
               const std::string &accessMode, size_t size, const void *src=nullptr);

	  CLBuffer(cl_context context, cl_command_queue cmdQueue, const std::string &accessMode,
			   size_t length, size_t byte_depth, const void *src=nullptr);

      cl_mem getBuffer();
      cl_mem getBuffer() const;

    public:
      friend class CLProgram; //!< for tight integration with CLProgram instances
      friend class CLKernel;  //!< for tight integration with CLKernel instances
	  friend class CLDeviceContext;//!< for tight integration with CLDeviceContext instances

      /// default constructor (creates null instance)
      CLBuffer();

      /// copy constructor (always performs shallow copy)
      CLBuffer(const CLBuffer& other);

      /// assignment operator (always performs a shallow copy)
      CLBuffer& operator=(const CLBuffer& other);

      /// destructor
      ~CLBuffer();

      /// copies the content of this buffer into the given buffer
      void copy(CLBuffer &dst, int len, int src_offset = 0, int dst_offset = 0);

      /// reads buffer from graphics memory into given destination pointer
      void read(void *dst, int len, int offset = 0, bool block = true);

      /// writes source data into the graphics memory
      void write(const void *src, int len, int offset = 0, bool block = true);

      /// checks whether buffer is null
      bool isNull() const {
        return !impl;
      }

      /// checks whether buffer is not null
      operator bool() const {
        return impl;
      }

    };
  }
}
#endif
