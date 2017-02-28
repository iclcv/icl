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
#include <ICLUtils/StringUtils.h>

#include <ICLMath/RansacFitter.h>
#include <ICLGeom/CoplanarPointPoseEstimator.h>

namespace icl{
  namespace geom{
    
    using namespace math;
    using namespace utils;
    
    struct RansacBasedPoseEstimator::Data{
      // int iterations;
      //int minPoints;
      //float maxError;
      //float minPointsForGoodModel;
      Camera camera;
      CoplanarPointPoseEstimator pe;
      std::vector<utils::Point32f> lastConsensusSet;
    };

#if 0
    static icl64f err_coplanar_verbose(const std::vector<float> &m, const std::vector<float> &p, const Camera &cam){
      std::cout << "model: [" 
                << m[0] << ", "
                << m[1] << ", "
                << m[2] << ", "
                << m[3] << ", "
                << m[4] << ", "
                << m[5] << "]" << std::endl;

      Mat T = create_hom_4x4<float>(m[0],m[1],m[2],m[3],m[4],m[5]);
      SHOW(T);

      SHOW(T * Vec(p[2],p[3],0,1));
      
      Point32f q = cam.project(T * Vec(p[2], p[3], 0, 1));
      
      SHOW(q);
      
      icl64f err = sqr(p[0] - q[0]) + sqr(p[1] - q[1]);
      
      return err;
    }
#endif

    icl64f RansacBasedPoseEstimator::err_coplanar(const std::vector<float> &m, const std::vector<float> &p){
      Mat T = create_hom_4x4<float>(m[0],m[1],m[2],m[3],m[4],m[5]);
      Point32f q = m_data->camera.project(T * Vec(p[2], p[3], 0, 1));
      
      icl64f err = sqr(p[0] - q[0]) + sqr(p[1] - q[1]);
      
      return err;
    }

#if 0
    static icl64f mean_error(const std::vector<std::vector<float> > &pts, const std::vector<float> &m,
                             Function<icl64f,const std::vector<float> &, const std::vector<float> &> err_coplanar,
                             const Camera &cam){
      //std::cout << "mean error ";
      icl64f err = 0;
      for(size_t i=0;i<pts.size();++i){
        err += err_coplanar(m,pts[i]);
        //if(str(err) == "nan" || str(err) == "-nan") {
        //  err_coplanar_verbose(m,pts[i], cam);
        //  break;
        //}
      }
      //std::cout << "mean error end ";
      return err / pts.size();

    }
#endif

    std::vector<float> RansacBasedPoseEstimator::fit_coplanar(const std::vector<std::vector<float> > &pts){
      //DEBUG_LOG("------------- fitting on " << pts.size() << " point");
      //std::cout << "Points:: [";
      std::vector<Point32f> curr(pts.size()),templ(pts.size());
      for(size_t i=0;i<pts.size();++i){
        curr[i] = Point32f(pts[i][0], pts[i][1]);
        templ[i] = Point32f(pts[i][2], pts[i][3]);
        
        //std::cout << curr[i] << "->" << templ[i] << (i < pts.size()-1 ? ",": "]\n");
      }


      
      Mat T = m_data->pe.getPose(pts.size(), templ.data(), curr.data(), m_data->camera);
      //SHOW(T);
      Vec3 e = extract_euler_angles(T);
      std::vector<float> p(6);
      p[0] =  e[0];
      p[1] =  e[1];
      p[2] =  e[2];
      p[3] = T(3,0);
      p[4] = T(3,1);
      p[5] = T(3,2);
      /*
      std::cout << "model: [" 
                << p[0] << ", "
                << p[1] << ", "
                << p[2] << ", "
                << p[3] << ", "
                << p[4] << ", "
                << p[5] << "] " << std::endl;
      std::cout << "(Err: " 
                << mean_error(pts, p, function(this,&RansacBasedPoseEstimator::err_coplanar), m_data->camera) << ")"<< std::endl;
          */
      return p;
    }
    
    
    
    RansacBasedPoseEstimator::RansacBasedPoseEstimator(const geom::Camera &camera, 
                                                         int iterations, 
                                                         int minPoints,
                                                         float maxErr,
                                                       float minPointsForGoodModel,
                                                       bool storeLastConsensusSet){
      m_data = new Data;
      addProperty("iterations","range","[1,1000000]:1",str(iterations));
      addProperty("min points","range","[1,1000000]:1",str(minPoints));
      addProperty("max error","range","[0,10e30]",str(maxErr));
      addProperty("min points for good model", "range", "[0,10000000]:1",str(minPointsForGoodModel));
      addProperty("debug output","flag","",false);
      addProperty("store last consensus set","flag","",storeLastConsensusSet);
      
      addChildConfigurable(&m_data->pe,"pose estimator");

      setPropertyValue("pose estimator.algorithm","HomographyBasedOnly");
      m_data->camera = camera;
    }

    void RansacBasedPoseEstimator::setStoreLastConsensusSet(bool on){
      setPropertyValue("store last consensus set",on);
    }

    std::vector<utils::Point32f>  RansacBasedPoseEstimator::getLastConsensusSet() throw (utils::ICLException){
      bool hasSet = getPropertyValue("store last consensus set");
      if(!hasSet) throw utils::ICLException("RansacBasedPoseEstimator::getLastConsensusSet() even though "
                                            "'store last consensus set' property was not set to 'true'");
      return m_data->lastConsensusSet;
    }
    

    RansacBasedPoseEstimator::~RansacBasedPoseEstimator(){
      delete m_data;
    }
      
    void RansacBasedPoseEstimator::setIterations(int iterations){
      setPropertyValue("iterations",iterations);
    }
    
    void RansacBasedPoseEstimator::setMinPoints(int minPoints){
      setPropertyValue("min points",minPoints);
    }
    
    void RansacBasedPoseEstimator::setMaxError(float maxError){
      setPropertyValue("max error",maxError);
    }
    
    void RansacBasedPoseEstimator::setMinPointsForGoodModel(float f){
      setPropertyValue("min points for good model", f);
    }
    
    RansacBasedPoseEstimator::Result 
    RansacBasedPoseEstimator::fit(const std::vector<Point32f> &templ,
                                   const std::vector<Point32f> &curr){
      
      int iterations = getPropertyValue("iterations");
      int maxError = getPropertyValue("max error");
      int minPoints = getPropertyValue("min points");
      int minPointsForGoodModel = getPropertyValue("min points for good model");
      bool dbg = getPropertyValue("debug output");
      


      RansacFitter<> ransac(minPoints, iterations, 
                            function(this,&RansacBasedPoseEstimator::fit_coplanar), 
                            function(this,&RansacBasedPoseEstimator::err_coplanar),
                            maxError, minPointsForGoodModel);
      RansacFitter<>::DataSet data(curr.size(), std::vector<float>(4,0));
      for(size_t i=0;i<curr.size();++i){
        data[i][0] = curr[i].x;
        data[i][1] = curr[i].y;
        data[i][2] = templ[i].x;
        data[i][3] = templ[i].y;
        //if(dbg){
          //DEBUG_LOG("associating curr: " << curr[i] << " --> " << templ[i]);
        //}
      }

      if(dbg){
        DEBUG_LOG("using these parameters:");
        SHOW(minPoints);
        SHOW(iterations);
        SHOW(maxError);
        SHOW(minPointsForGoodModel);
        SHOW(curr.size());
        SHOW(data.size());
      }


      const RansacFitter<>::Result &res = ransac.fit(data);

      if(getPropertyValue("store last consensus set")){
        m_data->lastConsensusSet.resize(res.consensusSet.size());
        for(size_t i=0;i<m_data->lastConsensusSet.size();++i){
          m_data->lastConsensusSet[i] = Point32f(res.consensusSet[i][0], res.consensusSet[i][1]);
        }
      }

      if(res.found()){
        const std::vector<float> &m = res.model;
        Mat T = create_hom_4x4<float>(m[0],m[1],m[2],m[3],m[4],m[5]);

        if(dbg){
          DEBUG_LOG("result was found: \n" << T << "\n");
        }


        Result r =  { T, true, (float)res.error };
        return r;
      }else{
        if(dbg){
          DEBUG_LOG("result was not found");
        }
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


