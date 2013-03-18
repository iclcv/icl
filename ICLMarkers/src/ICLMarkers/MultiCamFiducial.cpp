/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MultiCamFiducial.cpp         **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLMarkers/MultiCamFiducial.h>
#include <ICLMarkers/MultiCamFiducialImpl.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::cv;
using namespace icl::geom;

namespace icl{
  namespace markers{
    
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
   
  
  } // namespace markers
}
