/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MultiCamFiducialImpl.cpp     **
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

#include <ICLMarkers/MultiCamFiducialImpl.h>
#include <ICLGeom/PoseEstimator.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::geom;

namespace icl{
  namespace markers{
    MultiCamFiducialImpl::MultiCamFiducialImpl(){}
    MultiCamFiducialImpl::MultiCamFiducialImpl(int id,
                                               const std::vector<Fiducial> &fids,
                                               const std::vector<Camera*> cams):id(id),numFound(0),
                                                                                fids(fids),cams(cams),
                                                                                haveCenter(false),
                                                                                haveOrientation(false),
                                                                                havePose(false){}
    
    void MultiCamFiducialImpl::init(int id){
      this->numFound = 0;
      this->id = id;
      this->fids.clear();
      this->cams.clear();
      this->haveCenter = this->haveOrientation = this->havePose = false;
    }
      
    const FixedColVector<float,3> &MultiCamFiducialImpl::estimateCenter3D(){
      if(haveCenter) return center;
      if(numFound == 1){
        center = fids[0].getCenter3D().part<0,0,1,3>();
      }else{
        std::vector<Point32f> centerPoints(numFound);
        for(int i=0;i<numFound;++i){
          centerPoints[i] = fids[i].getCenter2D();
        }
        center = Camera::estimate_3D(cams,centerPoints).part<0,0,1,3>();
      }
      haveCenter = true;
      return center;
    }    
      
    const Mat &MultiCamFiducialImpl::estimatePose3D(){
      if(havePose) return pose;
        
      if(numFound == 1){
        pose = fids[0].getPose3D();
      }else{        
        int n = fids[0].getKeyPoints2D().size();
        // assumtion keypoints are ordered and identical ...
          
        DynMatrix<float> W(n,3),O(n,3);      
        for(int i=0;i<n;++i){
          std::vector<Point32f> pI(numFound);
          for(int j=0;j<numFound; ++j){
            pI[j] = fids[j].getKeyPoints2D()[i].imagePos;
          }
          Vec pW = Camera::estimate_3D(cams,pI);
          std::copy(pW.begin(),pW.begin()+3,W.col_begin(i));
            
          Point32f p = fids[0].getKeyPoints2D()[i].markerPos;
          O(i,0) = p.x;
          O(i,1) = p.y;
          O(i,2) = 0;
        }
        try{
          pose = PoseEstimator::map(O,W);
            
          /// invert y and z axis!
          for(int x=1;x<3;++x){
            for(int y=0;y<3;++y){
              pose(x,y) *= -1;
            }
          }
        }catch(ICLException &e){
          // pose estimation did not converge while trying to find eigenvectors ...
          pose = fids[0].getPose3D();
        }
      }
      havePose = true;
  
      center = FixedColVector<float,3>(pose(3,0), pose(3,1), pose(3,2));
      haveCenter = true;
        
      return pose;
    }
      
    const FixedColVector<float,3> &MultiCamFiducialImpl::estimateOrientation3D(){
      if(haveOrientation) return orientation;
      estimatePose3D();
      orientation = extract_euler_angles(pose);
      haveOrientation = true;
      return orientation;
    }
  
  } // namespace markers
}
