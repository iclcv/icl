/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/RGBDMapping.h                          **
** Module : ICLGeom                                                **
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

#ifndef ICL_RGBD_MAPPING_H
#define ICL_RGBD_MAPPING_H

#include <ICLGeom/Camera.h>
#include <ICLUtils/ConfigFile.h>

namespace icl{
  
  //// RGB to Depth image mapping e.g. for Kinect Cameras
  /** The mappoing algorithm using a projective camera transform to 
      implement the mapping between a depth camera and a common 2D 
      (color) camera. For the creation of the mapping one needs a 
      set of 3D reference points in the depth image where x and y are
      pixel positions while z is the depth cameras depth value that
      corresponts to the (x,y) position. In addition corresponding
      reference points in the RGB image are needed. The 3D reference
      points must be scattered in 3D (i.e. they must not be coplanar).
      It is strongly recommended to much more corresponding points.
      
      \section KIN Kinect
      For the Kinect RGBD Mapping a dedicated calibration application
      called icl-kinect-rgbd-calib is supported. Here, the (x,y) positions
      of reference points are obtained by conducting fiducial detection on
      the infrared image.
  */
  class RGBDMapping : public Mat{
    public:
    
    /// creates an ID mapping
    RGBDMapping():Mat(Mat::id()){}

    /// creates a mapping using the given set of landmark points for calibration
    /** @param depthImagePointsAndDepths the i-th component Vi contains the depth image
               reference point position (Vi[0]=x, Vi[1]=y), and the corresponding normalized
               depth values [in mm] at Vi[2]. Vi[3] must be 1. 
        @param rgbImagePoints the i-th component is the corresponding landmark calibration point
               in the rgb image.
    */
    inline RGBDMapping(const std::vector<Vec> &depthImagePointsAndDepths,
                       const std::vector<Point32f> &rgbImagePoints){
      Camera cam = Camera::calibrate(depthImagePointsAndDepths, rgbImagePoints);
      (Mat&)(*this) = cam.getProjectionMatrix()*cam.getCSTransformationMatrix();
    }
    
    /// loads a mapping from given xml filename
    inline RGBDMapping(const std::string &filename){
      ConfigFile f(filename);
      Mat m = f["config.rgbd-mapping"];
      (Mat&)(*this) = m;
    }

    /// applies the mapping
    /** @param x depth image x coordinate
        @param y depth image y coordinate
        @param depthValue the depth images depth value at position (x,y) (in mm) 
        @return rgb-image position
    */
    inline Point apply(int x, int y, float depthValue) const {
      const float phInv = 1.0/ ((*this)(0,3) * x + (*this)(1,3) * y + (*this)(2,3) * depthValue + (*this)(3,3));
      const int px = phInv * ( (*this)(0,0) * x + (*this)(1,0) * y + (*this)(2,0) * depthValue + (*this)(3,0) );
      const int py = phInv * ( (*this)(0,1) * x + (*this)(1,1) * y + (*this)(2,1) * depthValue + (*this)(3,1) );
      return Point(px,py);
    }
    
    /// saves the mapping using a given xml filename
    void save(const std::string &filename) const{
      ConfigFile f;
      f["config.rgbd-mapping"] = (const Mat&)(*this);
      f.save(filename);
    }
    
    
    
  };
  
}

#endif
