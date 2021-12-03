/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/CLSurfLib.h                            **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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

/****************************************************************************\
 * Copyright (c) 2011, Advanced Micro Devices, Inc.                           *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions         *
 * are met:                                                                   *
 *                                                                            *
 * Redistributions of source code must retain the above copyright notice,     *
 * this list of conditions and the following disclaimer.                      *
 *                                                                            *
 * Redistributions in binary form must reproduce the above copyright notice,  *
 * this list of conditions and the following disclaimer in the documentation  *
 * and/or other materials provided with the distribution.                     *
 *                                                                            *
 * Neither the name of the copyright holder nor the names of its contributors *
 * may be used to endorse or promote products derived from this software      *
 * without specific prior written permission.                                 *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED  *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 * If you use the software (in whole or in part), you shall adhere to all     *
 * applicable U.S., European, and other export laws, including but not        *
 * limited to the U.S. Export Administration Regulations ('EAR'), (15 C.F.R.  *
 * Sections 730 through 774), and E.U. Council Regulation (EC) No 1334/2000   *
 * of 22 June 2000.  Further, pursuant to Section 740.6 of the EAR, you       *
 * hereby certify that, except pursuant to a license granted by the United    *
 * States Department of Commerce Bureau of Industry and Security or as        *
 * otherwise permitted pursuant to a License Exception under the U.S. Export  *
 * Administration Regulations ("EAR"), you will not (1) export, re-export or  *
 * release to a national of a country in Country Groups D:1, E:1 or E:2 any   *
 * restricted technology, software, or source code you receive hereunder,     *
 * or (2) export to Country Groups D:1, E:1 or E:2 the direct product of such *
 * technology or software, if such foreign produced direct product is subject *
 * to national security controls as identified on the Commerce Control List   *
 * (currently found in Supplement 1 to Part 774 of EAR).  For the most current*
 * Country Group listings, or for additional information about the EAR or     *
 * your obligations under those regulations, please refer to the U.S. Bureau  *
 * of Industry and Security's website at http://www.bis.doc.gov/.             *
 \****************************************************************************/


#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLCV/SurfFeature.h>
#include <vector>
#ifdef ICL_HAVE_OPENCL
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/CLKernel.h>
#include <ICLUtils/CLBuffer.h>
#endif

namespace icl{

  namespace cv{
    namespace clsurf{

      /// again, we use the generic Ipoint here!
      typedef SurfFeature Ipoint;

      /// used typedef for vector of interest points
      typedef std::vector<Ipoint> IpVec;


      /// OpenCL-based Surf Feature detector implmentation (by AMD)
      /** Copyright (c) 2011, Advanced Micro Devices, Inc.
          (see license text)
      */
      class Surf {
        struct Data;  //!< hidden implementation
        Data *m_data; //!< hidden data pointer
        void createKernels();

        public:

        /// initializer (OpenCL initialization is performed internally)
        ICLCV_API Surf(int initialPoints, int i_height, int i_width,  int octaves,
             int intervals, int sample_step, float threshold);

        ICLCV_API ~Surf();

        /// our own ICL-based detection method
        ICLCV_API const IpVec &detect(const core::ImgBase *image);

        private:

        icl::utils::CLProgram program;
        icl::utils::CLKernel createDescrtptorsKernel;
        icl::utils::CLKernel getOrientationStep1Kernel;
        icl::utils::CLKernel getOrientationStep2Kernel;
        icl::utils::CLKernel hessian_detKernel;
        icl::utils::CLKernel scanKernel;
        icl::utils::CLKernel scan4Kernel;
        icl::utils::CLKernel scanImageKernel;
        icl::utils::CLKernel transposeKernel;
        icl::utils::CLKernel transposeImageKernel;
        icl::utils::CLKernel nearestNeighborKernel;
        icl::utils::CLKernel non_max_supressionKernel;
        icl::utils::CLKernel normalizeDescriptorsKernel;
        /// Compute the integral image
        void computeIntegralImage(const icl::core::Img32f &source);

        /// Create the SURF descriptors
        void createDescriptors(int i_width, int i_height);

        /// Calculate Orientation for each Ipoint
        void getOrientations(int i_width, int i_height);

        /// Rellocate OpenCL buffers if the number of ipoints is too high
        void reallocateIptBuffers();

        /// Resets the object state so that SURF can be run on a new frame
        void reset();

        /// Copy the descriptors from the GPU to the host
        const IpVec &retrieveDescriptors();

        /// Run the main SURF loop
        void run(const icl::core::Img32f &image, bool upright);

      };

    }
  }
}
