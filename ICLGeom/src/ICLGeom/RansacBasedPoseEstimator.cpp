/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RansacBasedPoseEstimator.cpp       **
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

#include <ICLGeom/RansacBasedPoseEstimator.h>

#include <ICLUtils/Uncopyable.h>
#include <ICLMath/RansacFitter.h>
#include <ICLGeom/CoplanarPointPoseEstimator.h>

namespace icl{
  namespace geom{
    
    using namespace math;
    using namespace utils;
    
    struct RansacBasedPoseEstimator::Data{
      int iterations;
      int minPoints;
      float maxError;
      float minPointsForGoodModel;
      Camera camera;
      CoplanarPointPoseEstimator pe;
    };

    std::vector<float> RansacBasedPoseEstimator::fit_coplanar(const std::vector<std::vector<float> > &pts){
      std::vector<Point32f> curr(pts.size()),templ(pts.size());
      for(size_t i=0;i<pts.size();++i){
        curr[i] = Point32f(pts[i][0], pts[i][1]);
        templ[i] = Point32f(pts[i][2], pts[i][3]);
      }
      Mat T = m_data->pe.getPose(pts.size(), templ.data(), curr.data(), m_data->camera);
      Vec3 e = extract_euler_angles(T);
      std::vector<float> p(6);
      p[0] =  e[0];
      p[1] =  e[1];
      p[2] =  e[2];
      p[3] = T(3,0);
      p[4] = T(3,1);
      p[5] = T(3,2);
      return p;
    }
    
    icl64f RansacBasedPoseEstimator::err_coplanar(const std::vector<float> &m, const std::vector<float> &p){
      Mat T = create_hom_4x4<float>(m[0],m[1],m[2],m[3],m[4],m[5]);
      Point32f q = m_data->camera.project(T * Vec(p[2], p[3], 0, 1));
      return sqr(p[0] - q[0]) + sqr(p[1] - q[1]);
    }
    
    
    RansacBasedPoseEstimator::RansacBasedPoseEstimator(const geom::Camera &camera, 
                                                         int iterations, 
                                                         int minPoints,
                                                         float maxErr,
                                                         float minPointsForGoodModel){
      m_data = new Data;
      setIterations(iterations);
      setMinPoints(minPoints);
      setMaxError(maxErr);
      setMinPointsForGoodModel(minPointsForGoodModel);
      m_data->camera = camera;
    }
    
    RansacBasedPoseEstimator::~RansacBasedPoseEstimator(){
      delete m_data;
    }
      
    void RansacBasedPoseEstimator::setIterations(int iterations){
      m_data->iterations = iterations;
    }
    
    void RansacBasedPoseEstimator::setMinPoints(int minPoints){
      m_data->minPoints = minPoints;
    }
    
    void RansacBasedPoseEstimator::setMaxError(float maxError){
      m_data->maxError = maxError;
    }
    
    void RansacBasedPoseEstimator::setMinPointsForGoodModel(float f){
      m_data->minPointsForGoodModel = f;
    }
    
    RansacBasedPoseEstimator::Result 
    RansacBasedPoseEstimator::fit(const std::vector<Point32f> &templ,
                                   const std::vector<Point32f> &curr){
      
      RansacFitter<> ransac(m_data->minPoints, m_data->iterations, 
                            function(this,&RansacBasedPoseEstimator::fit_coplanar), 
                            function(this,&RansacBasedPoseEstimator::err_coplanar),
                            m_data->maxError, m_data->minPointsForGoodModel);
      RansacFitter<>::DataSet data(curr.size(), std::vector<float>(4));
      for(size_t i=0;i<curr.size();++i){
        data[i][0] = curr[i].x;
        data[i][1] = curr[i].y;
        data[i][2] = templ[i].x;
        data[i][3] = templ[i].y;
      }
      const RansacFitter<>::Result &res = ransac.fit(data);

      if(res.found()){
        const std::vector<float> &m = res.model;
        Mat T = create_hom_4x4<float>(m[0],m[1],m[2],m[3],m[4],m[5]);
        Result r =  { T, true, (float)res.error };
        return r;
      }
      Result r =  { Mat::id(), false, float(-1) };
      return r;
    }
    
    RansacBasedPoseEstimator::Result 
    RansacBasedPoseEstimator::fit(const std::vector<Vec> &modelPoints, 
                                             const std::vector<Point32f> &imagePoints){
      throw ICLException("RansacBasedPoseEstimator::fit is not yet implemented for non-planar targets");
      Result r =  { Mat::id(), false, float(-1) };
      return r;
    }

  }
}


