/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/FiducialDetectorPlugin.cpp              **
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
#include <ICLMarkers/FiducialDetectorPlugin.h>

namespace icl{
  void FiducialDetectorPlugin::getCenter2D(Point32f &dst, FiducialImpl &impl){
    throw ICLException("FiducialDetectorPlugin::getCenter2D not supported for this type");
  }
  void FiducialDetectorPlugin::getRotation2D(float &dst, FiducialImpl &impl){
    throw ICLException("FiducialDetectorPlugin::getRotation2D not supported for this type");
  }
  void FiducialDetectorPlugin::getCorners2D(std::vector<Point32f> &dst, FiducialImpl &impl){
    throw ICLException("FiducialDetectorPlugin::getCorners2D not supported for this type");
  }
  void FiducialDetectorPlugin::getKeyPoints2D(std::vector<Fiducial::KeyPoint> &dst, FiducialImpl &impl){
    throw ICLException("FiducialDetectorPlugin::getKeyPoints2D not supported for this type");
  }
  void FiducialDetectorPlugin::getCenter3D(Vec &dst, FiducialImpl &impl){
    dst = Fiducial(&impl).getPose3D().part<3,0,1,4>();
  }
  void FiducialDetectorPlugin::getRotation3D(Vec &dst, FiducialImpl &impl){
    dst = extract_euler_angles( Fiducial(&impl).getPose3D() ).resize<1,4>(1);
  }
  void FiducialDetectorPlugin::getPose3D(Mat &dst, FiducialImpl &impl){
    if(!camera) throw ICLException("FiducialDetectorPlugin::getPose3D:"
                                   " no camera was given to parent FiducialDetector!");
    try{
      const std::vector<Fiducial::KeyPoint> & kps = Fiducial(&impl).getKeyPoints2D();
      Point32f ps[8];
      for(int i=0;i<4;++i){
        ps[i] = kps[i].markerPos;
        ps[4+i] = kps[i].imagePos;
      }
      dst = poseEst.getPose(4, ps, ps+4, *camera);
      for(int x=1;x<3;++x){
        for(int y=0;y<3;++y){
          dst(x,y) *= -1;
        }
      }
    }catch(ICLException &){
      throw ICLException("FiducialDetectorPlugin:getPose3D unable to estimate 3D pose with less than 4 key-points");
    }
  }


  std::string FiducialDetectorPlugin::getName(const FiducialImpl *impl){
    return str(impl->id);
  }
}
