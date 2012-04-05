/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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
#include <omp.h>
#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif


namespace icl{
  static inline float compute_depth_norm(const Vec &dir, const Vec &centerDir){
    return sprod3(dir,centerDir)/(norm3(dir)*norm3(centerDir));
  }
  

  struct RGBDImageSceneObject::Data{
    FixedMatrix<float,3,4> Qi;
    Size size;
    std::vector<Vec3> viewRayDirs;
    std::vector<float> correctionFactors;
    
    Vec viewRayOffset;
    RGBDMapping mapping;
    RGBDImageSceneObject *parent;
    Img32f lastCorrectedDepthImage;
    RGBDImageSceneObject::MappingMode mode;
    
    /// internal initialization method
    void init(const Size &size,
              const RGBDMapping &mapping,
              const Camera &cam,
              MappingMode mode,
              RGBDImageSceneObject *parent){
      this->parent = parent;
      this->size = size;
      this->mapping = mapping;
      this->mode = mode;
      lastCorrectedDepthImage = Img32f(size,1);
      Qi = cam.getQMatrix().pinv(true);
    
      const int dim = size.getDim();
    
      parent->m_vertices.resize(dim,Vec(0,0,0,1));
      parent->m_vertexColors.resize(dim, GeomColor(0.0,0.4,1,1));
      parent->setLockingEnabled(true);
      parent->setVisible(Primitive::vertex,true);
      parent->setPointSize(3);
      parent->setPointSmoothingEnabled(false);

      viewRayDirs.resize(dim);
      correctionFactors.resize(dim);
      Array2D<ViewRay> viewRays = cam.getAllViewRays();
      viewRayOffset = viewRays(0,0).offset;
    
      for(int y=0;y<size.height;++y){
        for(int x=0;x<size.width;++x){
          const int idx = x + size.width * y;
          const Vec &d = viewRays[idx].direction;
          Vec3 &n = viewRayDirs[idx];
        
          n[0] = d[0];
          n[1] = d[1];
          n[2] = d[2];
          correctionFactors[idx] = 1.0/compute_depth_norm(d,viewRays(319,239).direction);
        }
      }

      parent->addProperty("openmp threads","range","[1,16]:1",1,0,
                          "number of threads to use if openmp is available");
      parent->addProperty("update time","info","","inf",0,
                          "benchmarking result");
      parent->addProperty("rendering.point size","range","[1:10]:1",3,0,
                          "point size for rendering (only used of the property\n"
                          "rendering.mode is set to points)");
      parent->addProperty("rendering.mode","menu","points,triangles","triangles",0,
                          "Mode for rendering:\n"
                          "points:    In this case, the scene object is rendered\n"
                          "           in the default manner. It's vertices are\n"
                          "           rendered as colored points\n"
                          "triangles: In this, case the SceneObjects vertices are not\n"
                          "           rendered, but a custom triangle based rendering\n"
                          "           method is called");
      parent->addProperty("rendering.max Dz","range","[10,100]:1",15,0,
                          "(For triangle rendering only)\n"
                          "This value defines the max depth difference for\n"
                          "for triangles. Triangles, whose corners depth\n"
                          "difference is larger than this value are not rendered.");
      parent->addProperty("rendering.max depth","range","[300,10000]:1",5000,0,
                          "(For triangle rendering only)\n"
                          "Triangles that have a corner that is further away than\n"
                          "this value (in mm) are not drawn");
      parent->addProperty("rendering.min depth","range","[1,5000]:1",500,0,
                          "(For triangle rendering only)\n"
                          "Triangles that have a corner that is closer to the camera\n"
                          "than this value (in mm) are not drawn");

      parent->registerCallback(function(this,&Data::property_callback));
    }
    
    void property_callback(const Configurable::Property &p){
      const std::string &n = p.name;
      if(n == "update time") return;
      if(n == "rendering.point size") parent->setPointSize(parent->getPropertyValue(n));
    }
  };

  /** This could be useful on the graphics card (however there's still a bug in this!)
      static inline Vec3 cast_ray(const FixedMatrix<icl32f,3,4> &Qi, float x, float y, const Vec &p){
      const float dirX = Qi(0,0) * x + Qi(1,0) * y + Qi(2,0);
      const float dirY = Qi(0,1) * x + Qi(1,1) * y + Qi(2,1);
      const float dirZ = Qi(0,2) * x + Qi(1,2) * y + Qi(2,2);
      const float dirH = Qi(0,3) * x + Qi(1,3) * y + Qi(2,3);
      
      const float sign = dirH > 0 ? -1 : 1;
      
      const float dirX2 = p[0] - dirX/dirH;
      const float dirY2 = p[1] - dirY/dirH;
      const float dirZ2 = p[2] - dirZ/dirH;
      
      const float len = ::sqrt( dirX2*dirX2 + dirY2*dirY2 + dirZ2*dirZ2);
      
      return Vec3(sign * dirX/len, sign * dirY/len, sign * dirZ/len);
      }
  */
  
    
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
  
  
    
  RGBDImageSceneObject::RGBDImageSceneObject(const Size &size,MappingMode mode):m_data(new Data){
   
    m_data->init(size, get_default_kinect_rgbd_mapping(size), 
                 get_default_kinect_camera(size), mode,this);
  }
  
  RGBDImageSceneObject::RGBDImageSceneObject(const Size &size, const RGBDMapping &mapping,
                                             const Camera &cam, MappingMode mode):m_data(new Data){
    m_data->init(size,mapping,cam, mode,this);
  }
  
  RGBDImageSceneObject::~RGBDImageSceneObject(){
    delete m_data;
  }
  
  const RGBDMapping &RGBDImageSceneObject::getMapping() const{ 
    return m_data->mapping; 
  }
  
  void RGBDImageSceneObject::setMapping(const RGBDMapping &mapping) { 
    m_data->mapping = mapping; 
  }
  
  const Size &RGBDImageSceneObject::getSize() const { 
    return m_data->size;
  }
  
  const std::vector<Vec3> &RGBDImageSceneObject::getViewRayDirs() const { 
    return  m_data->viewRayDirs;
  }
  
  const std::vector<float> &RGBDImageSceneObject::getCorrectionFactors() const {
    return m_data->correctionFactors;
  }
  const Img32f &RGBDImageSceneObject::getCorrectedDepthImage() const{
    return m_data->lastCorrectedDepthImage;
  }
  
  void RGBDImageSceneObject::update(const Img32f &depthImage,
                                    const Img8u *rgbImage){

    const int W = m_data->size.width;
    const int H = m_data->size.height;
    const int DIM = W*H;
    const Rect imageRect(Point::null,m_data->size);

    lock();

    { 
#ifdef HAVE_IPP
      ippsMul_32f(depthImage.begin(0),  m_data->correctionFactors.data(), m_data->lastCorrectedDepthImage.begin(0), DIM);
#else
      std::transform(depthImage.begin(0), depthImage.end(0), m_data->correctionFactors.begin(),
                     m_data->lastCorrectedDepthImage.begin(0), std::multiplies<float>());
#endif
    }
    
    
    const icl32f *d = m_data->lastCorrectedDepthImage.begin(0);

    
    Time now = Time::now();
    const int ompThreads = getPropertyValue("openmp threads");
    omp_set_num_threads(ompThreads);
    
    // chache some local variables
    const RGBDMapping M = m_data->mapping;
    const Vec off = m_data->viewRayOffset;
    
    const FixedMatrix<float,3,4> Qi = m_data->Qi;

    const bool xyzMapping = m_data->mode == XYZ_WORLD;
    
    if(rgbImage){
      const Channel8u r = (*rgbImage)[0], g = (*rgbImage)[1], b = (*rgbImage)[2];
      for(int y=0;y<H;++y){
#pragma omp parallel
        {
#pragma omp for
          for(int x=0;x<W;++x){
            const int idx = x + W * y;
            
            const Vec3 &dir = m_data->viewRayDirs[idx];
            const float depthValue = d[idx];// * dir[3];
            
            Vec &v = m_vertices[idx];
            v[0] = off[0] + depthValue * dir[0];
            v[1] = off[1] + depthValue * dir[1];
            v[2] = off[2] + depthValue * dir[2];
            
            /// todo: move this if out of this loop!
            Point p = xyzMapping ? M.apply(v[0],v[1],v[2]) : M.apply(x,y,depthValue); 
            
            GeomColor &c = m_vertexColors[idx];        
            if(imageRect.contains(p.x, p.y)){
              static const float FACTOR = 1.0/255;
              c[0] = r(p.x,p.y) * FACTOR;
              c[1] = g(p.x,p.y) * FACTOR;
              c[2] = b(p.x,p.y) * FACTOR;
              c[3] = 1;
            }else{
              c[3] = 0;
            }
          }
        }
      }
    }else{ // no rgb image -> creation of the pointcloud only
      const int DIM = W*H;
#pragma omp parallel
      {
#pragma omp for
        for(int i=0;i<DIM;++i){
          const Vec3 &dir = m_data->viewRayDirs[i];
          const float depthValue = d[i];// * dir[3];
          Vec &v = m_vertices[i];
          v[0] = off[0] + depthValue * dir[0];
          v[1] = off[1] + depthValue * dir[1];
          v[2] = off[2] + depthValue * dir[2];
        }
      }
    }
    const float dtMS = (Time::now()-now).toMilliSecondsDouble();
    setPropertyValue("update time", str(0.01*(int)(dtMS*100)) + "ms" );
    unlock();
  }

  Array2D<Vec> RGBDImageSceneObject::getPoints() { 
    return Array2D<Vec>(m_data->size, m_vertices.data(), false); 
  }
  
  const Array2D<Vec> RGBDImageSceneObject::getPoints() const { 
    return const_cast<RGBDImageSceneObject*>(this)->getPoints(); 
  }
  
  
  Array2D<GeomColor> RGBDImageSceneObject::getColors() { 
    return Array2D<GeomColor>(m_data->size, m_vertexColors.data(), false); 
  }
  
  
  const Array2D<GeomColor> RGBDImageSceneObject::getColors() const { 
    return const_cast<RGBDImageSceneObject*>(this)->getColors(); 
  }

  template<class T>
  void map_image(const Img<T> &src, Img<T> &dst, const RGBDMapping M, const float *D, 
                 const Array2D<Vec> &ps, RGBDImageSceneObject::MappingMode mode){
    const int W = src.getWidth(), H = src.getHeight();
    const Rect imageRect(0,0,W,H);
    const int C = src.getChannels();
    for(int c=0;c<C;++c){
      const Channel<T> s = src[c];
      Channel<T> d = dst[c];
      if(mode == RGBDImageSceneObject::XYZ_WORLD){
        for(int y=0;y<H;++y){
          for(int x=0;x<W;++x,++D){
            const Vec &pW = ps(x,y);
            Point p = M.apply(pW[0],pW[1],pW[2]);
            d(x,y) = imageRect.contains(p.x,p.y) ? s(p.x,p.y) : 0;
          }
        }
      }else{
        for(int y=0;y<H;++y){
          for(int x=0;x<W;++x,++D){
            Point p = M.apply(x,y,(*D));
            d(x,y) = imageRect.contains(p.x,p.y) ? s(p.x,p.y) : 0;
          }
        }
      }
    }
  }

  void RGBDImageSceneObject::prepareForRendering(){
    if(getPropertyValue("rendering.mode") == "points"){
      setVisible(Primitive::vertex,true);
    }else{
      setVisible(Primitive::vertex,false);
    }
  }
  
  static inline bool relative_distance(float a, float b, float c, float maxDZ){
    return sqr(a-b)+sqr(b-c)+sqr(a-c) < maxDZ;
  }

  void RGBDImageSceneObject::customRender(){
    if(getPropertyValue("rendering.mode") == "points") return;
    const int maxDZ = 3 * sqr(getPropertyValue("rendering.max Dz").as<int>());
    const int maxDepth = getPropertyValue("rendering.max depth");
    const int minDepth = getPropertyValue("rendering.min depth");
                              


    // TODO dont draw if max dz is too large, use triangle strip

    const int W1 = m_data->size.width, H1 = m_data->size.height -1;
    glBegin(GL_TRIANGLES);
    const Channel32f d = m_data->lastCorrectedDepthImage[0];
    const Array2D<GeomColor> colors = getColors();
    const Array2D<Vec> points = getPoints();
    for(int y=0;y<H1;++y){
      for(int x=0;x<W1;++x){
        if( (d(x,y) < minDepth) || (d(x,y) > maxDepth) ) continue;
        
        if( relative_distance( d(x,y), d(x+1,y), d(x,y+1), maxDZ )) {
          glColor3fv(&colors(x,y)[0]);
          glVertex3fv(&points(x,y)[0]);
          
          glColor3fv(&colors(x+1,y)[0]);
          glVertex3fv(&points(x+1,y)[0]);
          
          glColor3fv(&colors(x,y+1)[0]);
          glVertex3fv(&points(x,y+1)[0]);
        }
        
        if(relative_distance( d(x+1,y+1), d(x+1,y), d(x,y+1), maxDZ) ){
          glColor3fv(&colors(x+1,y+1)[0]);
          glVertex3fv(&points(x+1,y+1)[0]);
          
          glColor3fv(&colors(x,y+1)[0]);
          glVertex3fv(&points(x,y+1)[0]);
          
          glColor3fv(&colors(x+1,y)[0]);
          glVertex3fv(&points(x+1,y)[0]);
        }
      }
    }
    
    glEnd();
  }

  void RGBDImageSceneObject::mapImage(const ImgBase *src, ImgBase **dst){
    if(!src) throw ICLException(str(__FUNCTION__)+": given source image was null");
    if(!dst) throw ICLException(str(__FUNCTION__)+": given destination image was null");
    ensureCompatible(dst,src->getDepth(),src->getSize(),src->getChannels());
    (*dst)->setTime(src->getTime());
    switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
      case depth##D: map_image(*src->as##D(), *(*dst)->as##D(),         \
                               m_data->mapping,                         \
                               m_data->lastCorrectedDepthImage.begin(0), \
                               getPoints(), m_data->mode);           \
        break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
  }

  REGISTER_CONFIGURABLE_DEFAULT(RGBDImageSceneObject);
} // end namespace icl


