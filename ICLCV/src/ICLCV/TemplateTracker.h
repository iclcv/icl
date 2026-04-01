// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Eckard Riedenklau, Christof Elbrechter

#pragma once

#include <ICLUtils/Configurable.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Point32f.h>
#include <ICLCore/Img.h>
namespace icl{
  namespace cv{


    /// Utility class vor viewbased template tracking
    /** TODO: add more documentation! */
    class ICLCV_API TemplateTracker : public utils::Configurable{
      struct Data; //!< internal data storage
      Data *data;  //!< internal data pointer

      public:
      TemplateTracker(const TemplateTracker&) = delete;
      TemplateTracker& operator=(const TemplateTracker&) = delete;

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
      void setRotationLUT(const std::vector<std::shared_ptr<core::Img8u> > &lut);

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
