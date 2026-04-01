// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#ifndef ICL_HAVE_OPENCV_FEATURES_2D
#warning "This header must not be included without ICL_HAVE_OPENCV_FEATURES_2D defined"
#endif

#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Configurable.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/VisualizationDescription.h>

namespace icl{
  namespace cv{

    class ICLCV_API ORBFeatureDetector : public utils::Configurable{
      struct Data;
      Data *m_data;

      public:
      ORBFeatureDetector();

      ~ORBFeatureDetector();

      struct ICLCV_API FeatureSetClass {
        FeatureSetClass(const FeatureSetClass&) = delete;
        FeatureSetClass& operator=(const FeatureSetClass&) = delete;
        struct Impl;
        Impl *impl;
        FeatureSetClass();
        ~FeatureSetClass();
        utils::VisualizationDescription vis() const;
      };

      using FeatureSet = std::shared_ptr<FeatureSetClass>;


      struct Match{
        utils::Point32f a,b;
        float distance;
      };

      FeatureSet detect(const core::Img8u &image);

      const core::ImgBase *getIntermediateImage(const std::string &id);

      std::vector<Match> match(const FeatureSet &a, const FeatureSet &b);
    };
  }
}
