/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLUtils/src/ICLUtils/CLImage2D.h                      **
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
#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/CLException.h>
#include <icl/utils/Rect.h>
#include <icl/utils/CLMemory.h>
#include <string>

#include <set>
#include <map>
#include <stdint.h>

#include <icl/utils/CLIncludes.h>

namespace icl::utils {
        /// Wrapper for an OpenCL Image2D
        /** Valid CLImage2D instances can only be created by a CLProgram instance.
         @see CLProgram for more details */
	  class ICLUtils_API CLImage2D : public CLMemory {
            struct Impl;
            Impl *impl;

			CLImage2D(cl_context context, cl_command_queue cmdQueue,
					const std::string &accessMode, const size_t width, const size_t height,
					int depth, int num_channel, const void *src=nullptr, std::map< uint32_t, std::set<uint32_t> > const
					  &supported_formats = std::map< uint32_t, std::set<uint32_t> >());

            cl_mem getImage2D();
			cl_mem getImage2D() const;

		public:

            friend class CLProgram;//!< for tight integration with CLProgram instances
            friend class CLKernel;//!< for tight integration with CLKernel instances
			friend class CLDeviceContext;//!< for tight integration with CLDeviceContext instances

            /// default constructor (creates null instance)
			CLImage2D();

            /// copy constructor (always performs shallow copy)
            CLImage2D(const CLImage2D& other);

            /// assignment operator (always performs a shallow copy)
            CLImage2D& operator=(const CLImage2D& other);

            /// destructor
            ~CLImage2D();

            /// reads image from graphics memory into given destination pointer
            /** region defines the accessed image area. When no region is provided
             *  the complete image is addressed(initial width, height and (0, 0) as origin */
            void read(void *dst, const utils::Rect &region=utils::Rect::null,  bool block = true);

            /// writes source data into the graphics memory
            /** region defines the accessed image area. When no region is provided
             *  the complete image is addressed(initial width, height and (0, 0) as origin */
            void write(const void *src,const utils::Rect &region=utils::Rect::null,
                    bool block = true);

            /// checks whether image is null
            bool isNull() const {
                return !impl;
            }

            /// checks whether image is not null
            operator bool() const {
                return impl;
            }

			icl32s getWidth() const { return m_dimensions[0]; }
			icl32s getHeight() const { return m_dimensions[1]; }
			icl32s getChannelSize() const { return m_dimensions[0]*m_dimensions[1]; }
			icl32s getNumChannels() const { return m_dimensions[2]; }

        };
    }
#endif
