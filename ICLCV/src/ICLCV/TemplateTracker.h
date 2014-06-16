/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/TemplateTracker.h                      **
** Module : ICLCV                                                  **
** Authors: Eckard Riedenklau, Christof Elbrechter                 **
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

#include <ICLUtils/Configurable.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>
namespace icl{
  namespace cv{
  
  
    /// Utility class vor viewbased template tracking
    /** TODO: add more documentation! */
    class ICLCV_API TemplateTracker : public utils::Configurable, public utils::Uncopyable{
      struct Data; //!< internal data storage
      Data *data;  //!< internal data pointer
      
      public:
      /// Result class that describes a tracking result
      struct Result{
        /// Constructor with given parameters
        inline Result(const utils::Point32f &pos=utils::Point32f(-1,-1), 
                      float angle=0, float proximityValue=0,
                      const core::Img8u *matchedTemplateImage=0):
          pos(pos),angle(angle),proximityValue(proximityValue),
          matchedTemplateImage(matchedTemplateImage){}
        utils::Point32f pos; //!< image position
        float angle;  //!< pattern orientation
        float proximityValue; //!< match quality
        /// internally assotiate (and rotatated) tempalte
        const core::Img8u *matchedTemplateImage;  
      };
  
      /// Constructor with given parameters
      /** TODO: describe parameters and methology */
      TemplateTracker(const core::Img8u *templateImage=0,
                      float rotationStepSizeDegree=1.0,
                      int positionTrackingRangePix=100, 
                      float rotationTrackingRangeDegree=45,
                      int coarseSteps=10,int fineSteps=1,
                      const Result &initialResult=Result());
  
      /// Desctructor
      ~TemplateTracker();
      
      
      /// utility method that shows the template rotation lookup table
      void showRotationLUT() const;
      
      /// sets a new set or rotated template images
      void setRotationLUT(const std::vector<utils::SmartPtr<core::Img8u> > &lut);
      
      /// sets a new template image, that is internally rotated
      /** internally 360/rotationStepSizeDegree images are sampled using
          image rotation. Please note that at some loations the image
          edges are cut. Therefore, the template should only be located
          within the inner center circle of the template's image rectangle */
      void setTemplateImage(const core::Img8u &templateImage, 
                            float rotationStepSizeDegree=1.0);
      
      /// actual track method
      Result track(const core::Img8u &image, const Result *initialResult=0,
                   std::vector<Result> *allResults=0);
  
    };
  
  } // namespace cv
}

