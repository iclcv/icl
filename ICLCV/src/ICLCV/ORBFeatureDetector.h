/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/ORBFeatureDetector.h                   **
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

    class ORBFeatureDetector : public utils::Configurable{
      struct Data;
      Data *m_data;

      public:
      ORBFeatureDetector();

      ~ORBFeatureDetector();

      struct FeatureSetClass : public utils::Uncopyable{
        struct Impl;
        Impl *impl;
        FeatureSetClass();
        ~FeatureSetClass();
        utils::VisualizationDescription vis() const;
      };

      typedef utils::SmartPtr<FeatureSetClass> FeatureSet;


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
