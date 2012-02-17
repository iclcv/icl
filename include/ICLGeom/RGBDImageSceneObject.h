/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/RGBDImageSceneObject.h                 **
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

#ifndef ICL_RGBD_IMAGE_SCENE_OBJECT_H
#define ICL_RGBD_IMAGE_SCENE_OBJECT_H

#include <ICLGeom/Camera.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/RGBDMapping.h>

namespace icl{
  
  
  class RGBDImageSceneObject : public SceneObject, public Configurable{
    /// optimized 3D scalar product
    static inline float sprod3(const Vec &a, const Vec &b){ 
      return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
    }
    
    /// optimized 3D norm
    static inline float norm3(const Vec &a){
      return ::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    }
    
    protected:
    /// corresponging depth image size
    Size m_size;
    
    /// each view-ray directions and depth normalization factors
    /** v = [ dir-x, dir-y, dir-z, norm-factor ] */
    std::vector<Vec> m_viewRaysAndNorms;
    
    /// Camera center
    Vec m_viewRayOffset;
    
    /// mapping, that maps RGB to depth values
    RGBDMapping m_mapping;
    
    /// computes the normalization factor for two given viewrays
    /** if the depth values, a camera provide are given wrt to the distance to the 
        camera's sensor plane, using view-rays demands to correct these values
        because, the view-ray lengths need the distances to the sensor center.
        
        In 3D space, the nomalization factor is exactly the cosine between the
        center view-ray direction an the actual view ray direction        
    */
    inline float getDepthNorm(const Vec &dir, const Vec &centerDir){
      return sprod3(dir,centerDir)/(norm3(dir)*norm3(centerDir));
    }

    void init(const Size &size,
              const RGBDMapping &mapping,
              const Camera &cam){
      m_size = size;
      m_mapping = mapping;
      
      addProperty("use openmp","flag","",true);
      
      const int dim = size.getDim();
      
      m_vertices.resize(dim,Vec(0,0,0,1));
      m_vertexColors.resize(dim);
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
    public:
    
    /// returns a default camera model for VGA and QVGA kinect cameras
    /** If the default camera model does not work well for your application, you
        can calibrate your kinect device using the icl-cam-calib-2 tool */
    static Camera get_default_kinect_camera(const Size &size) { return Camera(); }
    
    /// returns a default rgb to depth mapping for VGA and QVGA kinect cameras
    /** if the default mapping does not work for your kinect device, you can 
        create a new mapping using the icl-kinect-rgbd-calib tool */
    static RGBDMapping get_default_kinect_rgbd_mapping(const Size &size) { 
      RGBDMapping m;
      (Mat&)m = Mat(-22701,240.874,35.7513,-397184,
                    7.69983, -22436.1, 258.607, -1.41846e+06,
                    0, 0, 0, 0,
                    -0.0563062, 0.596686, 0.800497, -25860.9);
      return m;
    }
    
    RGBDImageSceneObject(const Size &size = Size::VGA){
      init(size, get_default_kinect_rgbd_mapping(size), get_default_kinect_camera(size) );
    }
    
    RGBDImageSceneObject(const Size &size, const RGBDMapping &mapping,
                         const Camera &cam){
      init(size,mapping,cam);
    }
      
    const RGBDMapping &getMapping() const{ return m_mapping; }
    void setMapping(const RGBDMapping &mapping) { m_mapping = mapping; }
    const Size &getSize() const { return m_size; }
    const std::vector<Vec> &getViewRaysAndNorms() const { return  m_viewRaysAndNorms; }

    void update(const Img32f &depthImage,
                const Img8u &rgbImage){
      
      lock();
      const Channel32f d = depthImage[0];
      const Channel8u r = rgbImage[0], g = rgbImage[1], b = rgbImage[2];
      
      static const Rect imageRect(Point::null,m_size);
      
      Time now = Time::now();
      
      const bool useOpenMP = getPropertyValue("use openmp");
      
      // cache this!
      const RGBDMapping M = m_mapping;
      const int W = m_size.width;
      const int H = m_size.height;
      
      
      
      for(int y=0;y<H;++y){
        // too much page misses if we add parallellism
#pragma omp parallel num_threads(4) if(useOpenMP)
        
        {
#pragma omp for
          for(int x=0;x<W;++x){
            const int idx = x + W * y;
            const Vec &dir = m_viewRaysAndNorms[idx];
            const float depthValue = d[idx] * dir[3];
            
            Vec &v = m_vertices[idx];
            v[0] = m_viewRayOffset[0] + depthValue * dir[0];
            v[1] = m_viewRayOffset[1] + depthValue * dir[1];
            v[2] = m_viewRayOffset[2] + depthValue * dir[2];
            
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
      SHOW( (Time::now()-now).toMilliSecondsDouble() );
      unlock();
      
    }
  };
}

#endif
