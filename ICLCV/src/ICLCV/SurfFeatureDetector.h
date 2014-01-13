/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/SurfFeatureDetector.h                  **
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


#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/ImgBase.h>
#include <ICLCV/SurfFeature.h>
#include <string>
#include <vector>

namespace icl{
  namespace cv{
    
      /// ICL's *New* Generic Surf Feature detection class
      /** Internally, the class either uses the opensurf-based implementation
          or an OpenCL-based implementation based on the clsurf library.
          The OpenSurf backend needs OpenCV, while the clsurf-backend
          builds on mandatory OpenCL support.
      */
      class ICL_CV_API SurfFeatureDetector {
        struct Data;  //!< hidden implementation
        Data *m_data; //!< hidden data pointer

        public:
    
        /// Constructor with given SURF detection parameters
        /** plugin can be either "opensurf" or "clsurf" or "best",
            which will prefer "clsurf" if possible */
        SurfFeatureDetector(int octaves=5, int intervals=4, int sampleStep=2, 
                            float threshold = 0.00005f, 
                            const std::string &plugin="best");

        ~SurfFeatureDetector();
    
        /// detects SURF features in given image
        const std::vector<SurfFeature> &detect(const core::ImgBase *image);

        /// detects SURF features in given image and stores them as internal reference features
        /** Please note, that the reference image is internally stored (by deep copy)
            When parameters are changed and the back-end must be re-initialized, the
            copy of the reference image is used to compute a new reference feature set
            based on the new parameters. */
        void setReferenceImage(const core::ImgBase *image);
        
        /// returns the featuers internally stored as reference 
        const std::vector<SurfFeature> &getReferenceFeatures() const ;
        
        /// detections SURF features and matches them against the given reference features
        const std::vector<SurfMatch> &match(const core::ImgBase *image, float significance=0.65);
        
        void setOctaves(int octaves);
        
        void setIntervals(int intervals);
        
        void setSampleStep(int sampleStep);
        
        void setThreshold(float threshold);
      };

  }
}
