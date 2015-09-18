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

#define __CL_ENABLE_EXCEPTIONS
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
            cl::Image2D image2D;
			cl::CommandQueue cmdQueue;
			std::map< uint,set<uint> > supported_channel_orders;

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

			static cl_channel_order parseChannelOrder(const int num_channels) {
				cl_channel_order order = CL_R;
				switch(num_channels) {
					case(1): order = CL_R; break;
					case(2): order = CL_RA; break;
					case(3): order = CL_RGB; break;
					case(4): order = CL_RGBA; break;
					default: {
						std::stringstream sstream;
						sstream << num_channels;
						throw CLBufferException("unsupported number of channels: "+sstream.str());
					}
				}
				return order;
			}

			bool checkSupportedImageFormat(cl_channel_order order, cl_channel_type type) {
				  if (supported_channel_orders.count(order)) {
					  std::set<cl_channel_type> &types = supported_channel_orders[order];
					  return (bool)types.count(type);
				  }
				  return false;
			}

            Impl() {}
			~Impl() {}

            Impl(Impl& other):cmdQueue(other.cmdQueue) {
                width = other.width;
                height = other.height;
                image2D = other.image2D;
				supported_channel_orders = other.supported_channel_orders;
			}

            Impl(cl::Context &context, cl::CommandQueue &cmdQueue,
                    const string &accessMode, const size_t width, const size_t height,
					int depth, int num_channel, const void *src = 0, const std::map<uint, std::set<uint> >
					&supported_formats = std::map< uint,std::set<uint> >()) throw (CLBufferException)
				:cmdQueue(cmdQueue), supported_channel_orders(supported_formats) {

                cl_mem_flags memFlags = stringToMemFlags(accessMode);
                if (src) {
                    memFlags = memFlags | CL_MEM_COPY_HOST_PTR;
				}
                this->width = width;
                this->height = height;
                try {
                    cl_channel_type channelType;
                    switch(depth) {
                        case 0:
                        channelType = CL_UNSIGNED_INT8;
                        break;
                        case 1:
                        channelType = CL_SIGNED_INT16;
                        break;
                        case 2:
                        channelType = CL_SIGNED_INT32;
                        break;
                        case 3:
                        channelType = CL_FLOAT;
                        break;
                        default:
                        throw CLBufferException("unknown depth value");
                    }
					cl_channel_order order = parseChannelOrder(num_channel);
					if (!checkSupportedImageFormat(order,channelType)) {
						std::stringstream sstream;
						sstream << "channel: " << num_channel << ", depth-type: "  << depth;
						throw CLBufferException("No such image type is supported: "+sstream.str());
					}
					image2D = cl::Image2D(context, memFlags, cl::ImageFormat(order, channelType), width, height, 0, (void*) src);
                } catch (cl::Error& error) {
                    throw CLBufferException(CLException::getMessage(error.err(), error.what()));
                }

            }

            void regionToCLTypes(const utils::Rect &region, cl::size_t<3> &clOrigin,
                    cl::size_t<3> &clRegion) {
                clOrigin[0] = region.x;
                clOrigin[1] = region.y;
                clOrigin[2] = 0;

                if (region == Rect::null) {
                    clRegion[0] = width;
                    clRegion[1] = height;
                } else {
                    clRegion[0] = region.width;
                    clRegion[1] = region.height;
                }
                clRegion[2] = 1;
            }

            void read(void *dst, const utils::Rect &region=utils::Rect::null, bool block = true)
            throw (CLBufferException) {
                cl_bool blocking;
                if (block)
                blocking = CL_TRUE;
                else
                blocking = CL_FALSE;
                try {
                    cl::size_t<3> clOrigin;
                    cl::size_t<3> clRegion;
                    regionToCLTypes(region, clOrigin, clRegion);
                    cmdQueue.enqueueReadImage(image2D, blocking, clOrigin, clRegion, 0, 0, dst);
                } catch (cl::Error& error) {
                    throw CLBufferException(CLException::getMessage(error.err(), error.what()));
                }
            }

            void write(void *src, const utils::Rect &region=utils::Rect::null,
                    bool block = true)
            throw (CLBufferException) {
                cl_bool blocking;
                if (block)
                blocking = CL_TRUE;
                else
                blocking = CL_FALSE;
                try {
                    cl::size_t<3> clOrigin;
                    cl::size_t<3> clRegion;
                    regionToCLTypes(region, clOrigin, clRegion);
                    cmdQueue.enqueueWriteImage(image2D, blocking, clOrigin, clRegion, 0, 0, src);
                } catch (cl::Error& error) {
                    throw CLBufferException(CLException::getMessage(error.err(), error.what()));
                }
            }

        };

		CLImage2D::CLImage2D(cl::Context &context, cl::CommandQueue &cmdQueue,
				const string &accessMode, const size_t width, const size_t height,
				int depth, int num_channel, const void *src,
				const std::map<uint, std::set<uint> > &supported_formats)
        throw (CLBufferException) {
            impl = new Impl(context, cmdQueue, accessMode, width, height,
					depth, num_channel, src, supported_formats);

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
            impl->width = other.impl->width;
            impl->height = other.impl->height;
			impl->supported_channel_orders = other.impl->supported_channel_orders;
            return *this;
        }

        CLImage2D::~CLImage2D() {
            delete impl;
        }

        void CLImage2D::read(void *dst, const utils::Rect &region, bool block)
        throw (CLBufferException) {
            impl->read(dst, region, block);

        }

        void CLImage2D::write(const void *src, const utils::Rect &region,
                bool block)
        throw (CLBufferException) {
            impl->write((void *)src, region, block);
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
