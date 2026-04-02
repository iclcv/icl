// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLMarkers/MultiCamFiducial.h>
#include <ICLMarkers/MultiCamFiducialImpl.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::cv;
using namespace icl::geom;

namespace icl::markers {
    int MultiCamFiducial::getCamsFound() const{
      ICLASSERT_THROW(!isNull(), ICLException(str(__FUNCTION__)+": this is null"));
      return impl->numFound;
    }

    Fiducial MultiCamFiducial::operator[](int idx){
      ICLASSERT_THROW(!isNull(), ICLException(str(__FUNCTION__)+": this is null"));
      return impl->fids[idx];
    }
    const Fiducial MultiCamFiducial::operator[](int idx) const{
      ICLASSERT_THROW(!isNull(), ICLException(str(__FUNCTION__)+": this is null"));
      return impl->fids[idx];
    }

    Camera &MultiCamFiducial::getCamera(int idx){
      ICLASSERT_THROW(!isNull(), ICLException(str(__FUNCTION__)+": this is null"));
      return *impl->cams[idx];
    }

    const Camera &MultiCamFiducial::getCamera(int idx) const{
      ICLASSERT_THROW(!isNull(), ICLException(str(__FUNCTION__)+": this is null"));
      return *impl->cams[idx];
    }


    int MultiCamFiducial::getID() const{
      ICLASSERT_THROW(!isNull(), ICLException(str(__FUNCTION__)+": this is null"));
      return impl->id;
    }

    const FixedColVector<float,3> &MultiCamFiducial::getCenter3D() const{
      ICLASSERT_THROW(!isNull(), ICLException(str(__FUNCTION__)+": this is null"));
      return impl->estimateCenter3D();
    }
    const FixedColVector<float,3> &MultiCamFiducial::getOrientation3D() const{
      ICLASSERT_THROW(!isNull(), ICLException(str(__FUNCTION__)+": this is null"));
      return impl->estimateOrientation3D();
    }
    const Mat &MultiCamFiducial::getPose3D() const{
      ICLASSERT_THROW(!isNull(), ICLException(str(__FUNCTION__)+": this is null"));
      return impl->estimatePose3D();
    }


  } // namespace icl::markers