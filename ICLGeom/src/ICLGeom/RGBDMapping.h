// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/Camera.h>
#include <ICLUtils/Array2D.h>

namespace icl{
  namespace geom{


    /// Utility class for RGBDMapping
    /** For the mapping, two Camera instances are needed. The mapping computes
        the source color image positions for a given depth (x,y,d)-depth image pixel
    */
    class ICLGeom_API RGBDMapping{
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
