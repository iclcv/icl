/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/RGBDImageSceneObject.cpp                   **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Andre Ückermann                   **
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

#include <ICLGeom/RGBDImageSceneObject.h>

namespace icl{
  
  
  // static inline float sprod3(const Vec &a, const Vec &b){ 
  //  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
  // }
  
  //static inline float norm3(const Vec &a){
  //  return ::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
  //}
  
  inline float RGBDImageSceneObject::getDepthNorm(const Vec &dir, const Vec &centerDir){
    return sprod3(dir,centerDir)/(norm3(dir)*norm3(centerDir));
  }
  
  void RGBDImageSceneObject::init(const Size &size,
                                  const RGBDMapping &mapping,
                                  const Camera &cam){
    m_size = size;
    m_mapping = mapping;
    
    addProperty("use openmp","flag","",false);
    
    const int dim = size.getDim();
      
    m_vertices.resize(dim,Vec(0,0,0,1));
    m_vertexColors.resize(dim, GeomColor(0.0,0.4,1,1));
    m_viewRaysAndNorms.resize(dim);
    
    setLockingEnabled(true);
    setVisible(Primitive::vertex,true);
    setPointSize(3);
    setPointSmoothingEnabled(false);
    
    Array2D<ViewRay> viewRays = cam.getAllViewRays();
    m_viewRayOffset = viewRays(0,0).offset;
    
    
    for(int y=0;y<size.height;++y){
      for(int x=0;x<size.width;++x){
        const int idx = x + size.width * y;
        const Vec &d = viewRays[idx].direction;
        Vec &n = m_viewRaysAndNorms[idx];
        
        n[0] = d[0];
        n[1] = d[1];
        n[2] = d[2];
        n[3] = 1.0/getDepthNorm(d,viewRays(319,239).direction);
      }
    }
  }
    
  Camera RGBDImageSceneObject::get_default_kinect_camera(const Size &size) { 
    return Camera(); 
  }
    

  RGBDMapping RGBDImageSceneObject::get_default_kinect_rgbd_mapping(const Size &size) { 
    RGBDMapping m;
    if(size != Size::VGA){
      ERROR_LOG("right now, only mappings for size::VGA are available");
      (Mat&)m = Mat::id();
      return m;
    }
    (Mat&)m = Mat(-22701,240.874,35.7513,-397184,
                  7.69983, -22436.1, 258.607, -1.41846e+06,
                  0, 0, 0, 0,
                  -0.0563062, 0.596686, 0.800497, -25860.9);
    return m;
  }
  
  
    
  RGBDImageSceneObject::RGBDImageSceneObject(const Size &size){
    init(size, get_default_kinect_rgbd_mapping(size), get_default_kinect_camera(size) );
  }
  
  RGBDImageSceneObject::RGBDImageSceneObject(const Size &size, const RGBDMapping &mapping,
                                             const Camera &cam){
    init(size,mapping,cam);
  }
  
  const RGBDMapping &RGBDImageSceneObject::getMapping() const{ 
    return m_mapping; 
  }
  
  void RGBDImageSceneObject::setMapping(const RGBDMapping &mapping) { 
    m_mapping = mapping; 
  }
  
  const Size &RGBDImageSceneObject::getSize() const { 
    return m_size;
  }
  
  const std::vector<Vec> &RGBDImageSceneObject::getViewRaysAndNorms() const { 
    return  m_viewRaysAndNorms;
  }
  
  void RGBDImageSceneObject::update(const Img32f &depthImage,
                                    const Img8u *rgbImage){
    
    lock();
    const Channel32f d = depthImage[0];
    const Rect imageRect(Point::null,m_size);
    
    Time now = Time::now();
    const bool useOpenMP = getPropertyValue("use openmp");
    
    // chache some local variables
    const RGBDMapping M = m_mapping;
    const int W = m_size.width;
    const int H = m_size.height;
    const Vec off = m_viewRayOffset;
    if(rgbImage){
      const Channel8u r = (*rgbImage)[0], g = (*rgbImage)[1], b = (*rgbImage)[2];
      for(int y=0;y<H;++y){
#pragma omp parallel if(useOpenMP)
        {
#pragma omp for
          for(int x=0;x<W;++x){
            const int idx = x + W * y;
            const Vec &dir = m_viewRaysAndNorms[idx];
            const float depthValue = d[idx] * dir[3];
            
            Vec &v = m_vertices[idx];
            v[0] = off[0] + depthValue * dir[0];
            v[1] = off[1] + depthValue * dir[1];
            v[2] = off[2] + depthValue * dir[2];
            
            Point p = M.apply(x,y,depthValue);
            
            GeomColor &c = m_vertexColors[idx];        
            if(imageRect.contains(p.x, p.y)){
              static const float FACTOR = 1.0/255;
              c[0] = r(p.x,p.y) * FACTOR;
              c[1] = g(p.x,p.y) * FACTOR;
              c[2] = g(p.x,p.y) * FACTOR;
              c[3] = 1;
            }else{
              c[3] = 0;
            }
          }
        }
      }
    }else{ // no rgb image -> creation of the pointcloud only
      const int DIM = W*H;
#pragma omp parallel if(useOpenMP)
      {
#pragma omp for
        for(int i=0;i<DIM;++i){
          const Vec &dir = m_viewRaysAndNorms[i];
          const float depthValue = d[i] * dir[3];
          Vec &v = m_vertices[i];
          v[0] = off[0] + depthValue * dir[0];
          v[1] = off[1] + depthValue * dir[1];
          v[2] = off[2] + depthValue * dir[2];
        }
      }
    }
    SHOW( (Time::now()-now).toMilliSecondsDouble() );
    unlock();
  }

} // end namespace icl


