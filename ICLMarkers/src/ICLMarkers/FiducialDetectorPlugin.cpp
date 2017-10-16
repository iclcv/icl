/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialDetectorPlugin.cpp   **
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

#include <ICLMarkers/FiducialDetectorPlugin.h>
#include <ICLUtils/Range.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::geom;


namespace icl{
  namespace markers{
    FiducialDetectorPlugin::FiducialDetectorPlugin():camera(0){
      addChildConfigurable(&poseEst,"pose");
    }

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
        int n = (int)kps.size();
        std::vector<Point32f> ps(2*n);
        for(int i=0;i<n;++i){
          ps[i] = kps[i].markerPos;
          ps[n+i] = kps[i].imagePos;
        }
        dst = poseEst.getPose(n, ps.data(), ps.data()+n, *camera);
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

    std::vector<int> FiducialDetectorPlugin::parse_list_str(const Any &s){
      if(!s.length()) throw ICLException("FiducialDetectorPlugin::parse_list_str: got empty string");
      std::vector<int> back;
      switch(s[0]){
        case '{':
          back = parseVecStr<int>(s.substr(1,s.length()-2),",");
          break;
        case '[':{
          Range32s r = s;
          for(int i=r.minVal;i<=r.maxVal;++i){
            back.push_back(i);
          }
          break;
        }
        default:
          back.push_back(s.as<int>());
      }
      return back;
    }

    struct FiducialDetectorPlugin_VIRTUAL : public FiducialDetectorPlugin{
      virtual void getFeatures(Fiducial::FeatureSet&){}
      virtual void detect(std::vector<FiducialImpl*> &, const Img8u &){}
      virtual void addOrRemoveMarkers(bool, const Any &, const ParamList &){}
    };

    REGISTER_CONFIGURABLE_DEFAULT(FiducialDetectorPlugin_VIRTUAL);
  } // namespace markers
}
