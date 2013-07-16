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

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <iostream>
#include <sstream>
#include <ICLUtils/CLImage2D.h>
#include <ICLUtils/Macros.h>

namespace icl {
    namespace utils {

        struct CLImage2D::Impl {
            cl::Image2D image2D;
            cl::CommandQueue cmdQueue;

            static cl_mem_flags stringToMemFlags(const string &accessMode)
            throw (CLBufferException) {
                switch(accessMode.length()) {
                    case 1:
                    if(accessMode[0] == 'w') return CL_MEM_WRITE_ONLY;
                    if(accessMode[0] == 'r') return CL_MEM_READ_ONLY;
                    case 2:
                    if( (accessMode[0] == 'r' && accessMode[1] == 'w') ||
                            (accessMode[0] == 'w' && accessMode[1] == 'r') ) {
                        return CL_MEM_READ_WRITE;
                    }
                    default:
                    throw CLBufferException("undefined access-mode '"+accessMode+"' (allowed are 'r', 'w' and 'rw')");
                    return CL_MEM_READ_WRITE;
                }
            }

//      static ImageFormat getImageFormat(const icl::core::depth imgDepth, const icl::core::format imgFormat){
//          return ImageFormat();
//      }

            Impl() {}
            ~Impl() {
            }

            Impl(Impl& other):cmdQueue(other.cmdQueue) {
                image2D = other.image2D;
            }

            Impl(cl::Context &context, cl::CommandQueue &cmdQueue,
                    const string &accessMode, const size_t width, const size_t height,
//           const icl::core::depth imgDepth,
//           const icl::core::format imgFormat,
                    const void *src = 0)
            throw (CLBufferException):cmdQueue(cmdQueue) {
                cl_mem_flags memFlags = stringToMemFlags(accessMode);
                if (src) {
                    memFlags = memFlags | CL_MEM_COPY_HOST_PTR;
                }

                try {
                    image2D = cl::Image2D(context, memFlags, cl::ImageFormat(CL_R, CL_UNSIGNED_INT32), width, height, 0, (void*) src);
                } catch (cl::Error& error) {
                    throw CLBufferException(CLException::getMessage(error.err(), error.what()));
                }

            }

            void read(void *dst,unsigned regionWidth, unsigned regionHeight,
                    unsigned originX, unsigned originY, bool block = true)
            throw (CLBufferException) {
                cl_bool blocking;
                if (block)
                blocking = CL_TRUE;
                else
                blocking = CL_FALSE;
                try {
                    cl::size_t<3> origin;
                    cl::size_t<3> region;
                    origin.push_back(originX);
                    origin.push_back(originY);
                    origin.push_back(0);
                    region.push_back(regionWidth);
                    region.push_back(regionHeight);
                    region.push_back(1);
                    cmdQueue.enqueueReadImage(image2D, blocking, origin, region, 0, 0, dst);
                } catch (cl::Error& error) {
                    throw CLBufferException(CLException::getMessage(error.err(), error.what()));
                }
            }

            void write(void *src, unsigned regionWidth, unsigned regionHeight,
                    unsigned originX, unsigned originY, bool block = true)
            throw (CLBufferException) {
                cl_bool blocking;
                if (block)
                blocking = CL_TRUE;
                else
                blocking = CL_FALSE;
                try {
                    cl::size_t<3> origin;
                    cl::size_t<3> region;
                    origin.push_back(originX);
                    origin.push_back(originY);
                    origin.push_back(0);
                    region.push_back(regionWidth);
                    region.push_back(regionHeight);
                    region.push_back(1);
                    cmdQueue.enqueueWriteImage(image2D, blocking, origin, region, 0, 0, src);
                } catch (cl::Error& error) {
                    throw CLBufferException(CLException::getMessage(error.err(), error.what()));
                }
            }

        };

        CLImage2D::CLImage2D(cl::Context &context, cl::CommandQueue &cmdQueue,
                const string &accessMode, const size_t width, const size_t height,
//                       const icl::core::depth imgDepth,
//                       const icl::core::format imgFormat,
                const void *src)
        throw (CLBufferException) {
            impl = new Impl(context, cmdQueue, accessMode, width, height,
//              imgDepth,
//              imgFormat,
                    src);

        }

        CLImage2D::CLImage2D() {
            impl = new Impl();
        }
        CLImage2D::CLImage2D(const CLImage2D& other) {
            impl = new Impl(*(other.impl));
        }

        CLImage2D& CLImage2D::operator=(CLImage2D const& other) {
            impl->cmdQueue = other.impl->cmdQueue;
            impl->image2D = other.impl->image2D;
            return *this;
        }

        CLImage2D::~CLImage2D() {
            delete impl;
        }

        void CLImage2D::read(void *dst, unsigned regionWidth, unsigned regionHeight,
                unsigned originX, unsigned originY, bool block)
        throw (CLBufferException) {
            impl->read(dst, regionWidth, regionHeight, originX, originY, block);

        }

        void CLImage2D::write(const void *src, unsigned regionWidth, unsigned regionHeight,
                unsigned originX, unsigned originY, bool block)
        throw (CLBufferException) {
            impl->write((void *)src, regionWidth, regionHeight, originX, originY, block);
        }

        cl::Image2D CLImage2D::getImage2D() {
            return impl->image2D;
        }

        const cl::Image2D CLImage2D::getImage2D() const {
            return impl->image2D;
        }
    }
}
#endif
