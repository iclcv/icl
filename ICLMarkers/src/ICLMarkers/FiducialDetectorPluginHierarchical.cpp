/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialDetectorPluginHierar **
**          chical.cpp                                             **
** Module : ICLMarkers                                             **
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

#include <ICLMarkers/FiducialDetectorPluginHierarchical.h>
#include <ICLCV/RegionDetector.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::cv;

namespace icl{
  namespace markers{
    struct FiducialDetectorPluginHierarchical::Data{
      RegionDetector rd;
      std::vector<ImageRegion> results;
    };


    FiducialDetectorPluginHierarchical::FiducialDetectorPluginHierarchical():data(new Data){
      data->rd.setCreateGraph(true);
      data->rd.setConstraints(10,1000000,0,255);
      data->rd.deactivateProperty("create region graph");
      addChildConfigurable(&data->rd,"regions");
    }

    FiducialDetectorPluginHierarchical::~FiducialDetectorPluginHierarchical(){
      delete data;
    }

    void FiducialDetectorPluginHierarchical::detect(std::vector<FiducialImpl*> &dst, const Img8u &image){
      detect(dst,data->rd.detect(&image));
    }


  } // namespace markers
}
