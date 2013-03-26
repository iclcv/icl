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
#ifdef HAVE_OPENCL
#include <iostream>
#include <sstream>
#include <ICLUtils/CLBuffer.h>

namespace icl {
	namespace utils {

		struct CLBuffer::Impl {
			cl::Buffer buffer;
			cl::CommandQueue cmdQueue;
			static cl_mem_flags stringToMemFlags(const string &accessMode)
			throw (CLBufferException) {
				if (accessMode.find("w") != string::npos) {
					if (accessMode.find("r") != string::npos) {
						return CL_MEM_READ_WRITE;
					} else {
						return CL_MEM_WRITE_ONLY;
					}

				} else if (accessMode.find("r") != string::npos) {
					return CL_MEM_READ_ONLY;
				}
				string errorMsg = "unknown access mode ";
				errorMsg = errorMsg.append(accessMode);
				throw CLBufferException(errorMsg);
			}
			Impl(){}
			~Impl() {
			}
			Impl(Impl& other):cmdQueue(other.cmdQueue){
				buffer = other.buffer;
			}
			Impl(cl::Context &context, cl::CommandQueue &cmdQueue,
					const string &accessMode, size_t size, void *src = 0)
			throw (CLBufferException):cmdQueue(cmdQueue) {
				cl_mem_flags memFlags = stringToMemFlags(accessMode);
				if (src) {
					memFlags = memFlags | CL_MEM_COPY_HOST_PTR;
				}

				try {
					buffer = cl::Buffer(context, memFlags, size, src);
				} catch (cl::Error& error) {
					throw CLBufferException(CLException::getMessage(error.err(), error.what()));
				}

			}
			void read(void *dst, int len, int offset = 0, bool block = true)
			throw (CLBufferException) {
				cl_bool blocking;
				if (block)
				blocking = CL_TRUE;
				else
				blocking = CL_FALSE;
				try {
					cmdQueue.enqueueReadBuffer(buffer, blocking, offset, len, dst);
				} catch (cl::Error& error) {
					throw CLBufferException(CLException::getMessage(error.err(), error.what()));
				}
			}

			void write(void *src, int len, int offset = 0, bool block = true)
			throw (CLBufferException) {
				cl_bool blocking;
				if (block)
				blocking = CL_TRUE;
				else
				blocking = CL_FALSE;
				try {
					cmdQueue.enqueueWriteBuffer(buffer, blocking, offset, len, src);
				} catch (cl::Error& error) {
					throw CLBufferException(CLException::getMessage(error.err(), error.what()));
				}
			}

		};

		CLBuffer::CLBuffer(cl::Context &context, cl::CommandQueue &cmdQueue,
				const string &accessMode, size_t size, void *src)
		throw (CLBufferException) {
			impl = new Impl(context, cmdQueue, accessMode, size, src);

		}

		CLBuffer::CLBuffer(){
			impl = new Impl();
		}
		CLBuffer::CLBuffer(const CLBuffer& other){
		  impl = new Impl(*(other.impl));
		}

		CLBuffer const& CLBuffer::operator=(CLBuffer const& other){
			impl->cmdQueue = other.impl->cmdQueue;
			impl->buffer = other.impl->buffer;
		  return *this;
		}

		CLBuffer::~CLBuffer() {
			delete impl;
		}

		void CLBuffer::read(void *dst, int len, int offset, bool block)
		throw (CLBufferException) {
			impl->read(dst, len, offset, block);

		}

		void CLBuffer::write(void *src, int len, int offset, bool block)
		throw (CLBufferException) {
			impl->write(src, len, offset, block);
		}

		cl::Buffer CLBuffer::getBuffer() {
			return impl->buffer;
		}

		const cl::Buffer CLBuffer::getBuffer() const {
			return impl->buffer;
		}
	}
}
#endif
