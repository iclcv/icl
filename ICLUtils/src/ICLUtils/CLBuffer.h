/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/CLBuffer.h                       **
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
#ifdef HAVE_OPENCL
#pragma once
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <string.h>
#include <ICLUtils/CLException.h>
#include <ICLUtils/Uncopyable.h>
using namespace std;
namespace icl {
	namespace utils {
	/// Wrapper for an OpenCL Buffer
	    /**
	      Can only be created via CLProgram.
	    */
		class CLBuffer : public Uncopyable {
		public:
			struct Impl;
		private:
			Impl *impl;
			CLBuffer(cl::Context& context, cl::CommandQueue &cmdQueue, const string &accessMode, size_t size, void *src=NULL) throw (CLBufferException);
			cl::Buffer* getBuffer();
			const cl::Buffer* getBuffer() const;
			CLBuffer(const CLBuffer &other);
		public:
			~CLBuffer();
			friend class CLProgram;
			friend class CLKernel;

			void read(void *dst, int len, int offset = 0, bool block = true) throw (CLBufferException);

			void write(void *src, int len, int offset = 0, bool block = true) throw (CLBufferException);

			bool isNull() const {
				return !impl;
			}
			operator bool() const {
				return impl;
			}
		};
	}
}
#endif
