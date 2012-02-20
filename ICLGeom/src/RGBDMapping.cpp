/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/RGBDMapping.cpp                            **
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

#include <ICLGeom/RGBDMapping.h>
#include <ICLGeom/Camera.h>
#include <ICLUtils/ConfigFile.h>

namespace icl{
  
  RGBDMapping::RGBDMapping():Mat(Mat::id()){}

  RGBDMapping::RGBDMapping(const std::vector<Vec> &depthImagePointsAndDepths,
                       const std::vector<Point32f> &rgbImagePoints){
    Camera cam = Camera::calibrate(depthImagePointsAndDepths, rgbImagePoints);
    (Mat&)(*this) = cam.getProjectionMatrix()*cam.getCSTransformationMatrix();
  }
  
  RGBDMapping::RGBDMapping(const std::string &filename){
    ConfigFile f(filename);
    Mat m = f["config.rgbd-mapping"];
    (Mat&)(*this) = m;
  }
  
  RGBDMapping::RGBDMapping(const Mat &M){
    *this = M;
  }
    
  void RGBDMapping::save(const std::string &filename) const{
    ConfigFile f;
    f["config.rgbd-mapping"] = (const Mat&)(*this);
    f.save(filename);
  }
  
  RGBDMapping &RGBDMapping::operator=(const Mat &M){
    (Mat&)*this = M;
    return *this;
  }
  
}

