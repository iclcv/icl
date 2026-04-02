// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLMarkers/FiducialDetectorPluginHierarchical.h>
#include <ICLCV/RegionDetector.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::cv;

namespace icl::markers {
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


  } // namespace icl::markers