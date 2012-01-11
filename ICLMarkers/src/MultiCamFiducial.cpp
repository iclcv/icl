/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/MultiCamFiducial.cpp                    **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLMarkers/MultiCamFiducial.h>
#include <ICLMarkers/MultiCamFiducialImpl.h>

namespace icl{
  
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
 

}
