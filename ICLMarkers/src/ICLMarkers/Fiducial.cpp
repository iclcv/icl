// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLMarkers/Fiducial.h>
#include <ICLMarkers/FiducialImpl.h>
#include <ICLMarkers/FiducialDetectorPlugin.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::geom;
using namespace icl::cv;

namespace icl::markers {
    const Fiducial::FeatureSet Fiducial::AllFeatures(std::string(static_cast<int>(Fiducial::FeatureCount),'1'));

    int Fiducial::getID() const{
      return impl->id;
    }

    std::string Fiducial::getName() const{
      return impl->parent->getName(impl);
    }

    bool Fiducial::supports(Feature f) const{
      return impl->supported[static_cast<int>(f)];
    }

    FiducialDetectorPlugin *Fiducial::getDetector(){
      return impl->parent;
    }
    const FiducialDetectorPlugin *Fiducial::getDetector() const{
      return impl->parent;
    }

    const ImageRegion Fiducial::getImageRegion() const{
      return impl->imageRegion;
    }

  #define FORWARD_CALL_TO_IMPL(T,DIM,X)                \
    const T &Fiducial::get##X##DIM##D() const{         \
      if(impl->computed[static_cast<int>(Fiducial::X##DIM##D)]){    \
        return impl->info##DIM##D->info##X;            \
      }else{                                           \
        T &x = impl->ensure##DIM##D()->info##X;        \
        impl->parent->get##X##DIM##D(x,*impl);         \
        impl->computed.set(static_cast<int>(Fiducial::X##DIM##D));  \
        return x;                                      \
      }                                                \
    }


    FORWARD_CALL_TO_IMPL(Point32f,2,Center)
    FORWARD_CALL_TO_IMPL(Vec,3,Center)
    FORWARD_CALL_TO_IMPL(float,2,Rotation)
    FORWARD_CALL_TO_IMPL(Vec,3,Rotation)
    FORWARD_CALL_TO_IMPL(Mat,3,Pose)
    FORWARD_CALL_TO_IMPL(std::vector<Fiducial::KeyPoint>,2,KeyPoints)
    FORWARD_CALL_TO_IMPL(std::vector<Point32f>,2,Corners)





  } // namespace icl::markers