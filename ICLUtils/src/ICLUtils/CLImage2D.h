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
#ifdef HAVE_OPENCL
#pragma once

#include <string>
#include <ICLUtils/CLException.h>

/** \cond */
namespace cl {
    class Image2D;
    class Context;
    class CommandQueue;
}
/** \endcond */

namespace icl {
    namespace utils {

        /// Wrapper for an OpenCL Buffer
        /** Valid CLBuffer instances can only be created by a CLProgram instance.
         @see CLProgram for more details */
        class CLImage2D {
            struct Impl; //!< internal hidden implementation type
            Impl *impl;//!< internal implemetation

            /// private constructor (buffer can only be created by CLProgram instances)
            CLImage2D(cl::Context& context, cl::CommandQueue &cmdQueue,
                    const string &accessMode, const size_t width, const size_t height,
//               const core::depth imgDepth,
//               const icl::core::format imgFormat,

                    const void *src=NULL) throw (CLBufferException);

            /// provides access to the underlying cl-buffer
            cl::Image2D getImage2D();
            const cl::Image2D getImage2D() const;

        public:
            friend class CLProgram;//!< for tight integration with CLProgram instances
            friend class CLKernel;//!< for tight integration with CLKernel instances

            /// default constructor (creates null instance)
            CLImage2D();

            /// copy constructor (always performs shallow copy)
            CLImage2D(const CLImage2D& other);

            /// assignment operator (always performs a shallow copy)
            CLImage2D& operator=(const CLImage2D& other);

            /// destructor
            ~CLImage2D();

            /// reads buffer from graphics memory into given destination pointer
            void read(void *dst, unsigned regionWidth, unsigned regionHeight,
                    unsigned originX = 0, unsigned originY = 0,
                    bool block = true) throw (CLBufferException);

            /// writes source data into the graphics memory
            void write(const void *src, unsigned regionWidth, unsigned regionHeight,
                    unsigned originX = 0, unsigned originY = 0, bool block = true) throw (CLBufferException);

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
