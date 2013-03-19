/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RGBDMapping.h                      **
** Module : ICLGeom                                                **
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


#pragma once

#include <ICLGeom/Camera.h>
#include <ICLUtils/Array2D.h>

namespace icl{
  namespace geom{

    
    /// Utility class for RGBDMapping
    /** For the mapping, two Camera instances are needed. The mapping computes
        the source color image positions for a given depth (x,y,d)-depth image pixel
    */
    class RGBDMapping{
      protected:
      Mat colorCamMatrix;        //!< color camera matrix
      utils::Array2D<Vec> depthCamRays; //!< depth camera view rays
      Vec depthCamPos;           //!< depth camera offset

      /// internally used utility function
      static inline utils::Point map_rgbd(const Mat &M, const Vec &v){
        const float phInv = 1.0/ ( M(0,3) * v[0] + M(1,3) * v[1] + M(2,3) * v[2] + M(3,3) );
        const int px = phInv * ( M(0,0) * v[0] + M(1,0) * v[1] + M(2,0) * v[2] + M(3,0) );
        const int py = phInv * ( M(0,1) * v[0] + M(1,1) * v[1] + M(2,1) * v[2] + M(3,1) );
        return utils::Point(px,py);
      }

      public:
      /// empty constructor (no initialization)
      RGBDMapping(){}

      /// create RGBDMapping from given color camera, and depth camera parameters
      RGBDMapping(const Camera &colorCam, const utils::Array2D<Vec> &depthCamRays, 
                  const Vec &depthCamPos):
        colorCamMatrix(colorCam.getProjectionMatrix() * colorCam.getCSTransformationMatrix()),
        depthCamRays(depthCamRays){}
        
      /// create RGBDMapping from given color camera, and depth camera 
      RGBDMapping(const Camera &colorCam, const Camera &depthCamera):
        colorCamMatrix(colorCam.getProjectionMatrix() * colorCam.getCSTransformationMatrix()),
        depthCamRays(depthCamera.getResolution()){

          utils::Array2D<ViewRay> rays = depthCamera.getAllViewRays();
          for(int i=0;i<rays.getDim();++i){
            depthCamRays[i] = rays[i].direction;
          }
          
          depthCamPos = depthCamera.getPosition();
      }
        
      /// applies the mapping  
      utils::Point apply(const utils::Point &p, float dMM) const{
        Vec pW = depthCamPos + depthCamRays(p.x,p.y) * dMM;
        return map_rgbd(colorCamMatrix,pW);
      }

      /// applies the mapping  
      utils::Point operator()(const utils::Point &p, float dMM) const {
        return apply(p,dMM);
      }
      
      /// detaches the viewrays from other instances
      /** The internal depthCamRay-Array2D might be shallow copied.
          This method ensured that *this becomes independent */
      void detach(){
        depthCamRays.detach();
      }
    };
  }
}
