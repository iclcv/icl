/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/Fiducial.cpp                            **
** Module : ICLMarkers                                             **
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
#include <ICLMarkers/Fiducial.h>
#include <ICLMarkers/FiducialImpl.h>
#include <ICLMarkers/FiducialDetectorPlugin.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::geom;
using namespace icl::cv;

namespace icl{
  namespace markers{
  
    const Fiducial::FeatureSet Fiducial::AllFeatures(std::string((int)Fiducial::FeatureCount,'1'));
    
    int Fiducial::getID() const{
      return impl->id;
    }
    
    std::string Fiducial::getName() const{
      return impl->parent->getName(impl);
    }
  
    bool Fiducial::supports(Feature f) const{
      return impl->supported[(int)f];
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
      if(impl->computed[(int)Fiducial::X##DIM##D]){    \
        return impl->info##DIM##D->info##X;            \
      }else{                                           \
        T &x = impl->ensure##DIM##D()->info##X;        \
        impl->parent->get##X##DIM##D(x,*impl);         \
        impl->computed.set((int)Fiducial::X##DIM##D);  \
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
  
  
  
  
    
  } // namespace markers
}
