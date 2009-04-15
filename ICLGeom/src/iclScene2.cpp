#include <iclScene2.h>
#include <iclDrawWidget.h>
#include <iclImg.h>
#include <iclQuick.h>

#include <GL/gl.h>
#include <GL/glu.h>

namespace icl{

  struct Scene2::GLCallback : public ICLDrawWidget3D::GLCallback{
    int cameraIndex;
    Scene2 *parent;
    GLCallback(int cameraIndex,Scene2 *parent):
      cameraIndex(cameraIndex),parent(parent){}
    virtual void draw(){
      parent->render(cameraIndex);
    }
  };

  struct  Scene2::SceneMouseHandler : public MouseHandler{
    int cameraIndex;
    Scene2 *parent;
    Point32f anchor;
    Camera camSave;
    SceneMouseHandler(int cameraIndex,Scene2 *parent):
      cameraIndex(cameraIndex),parent(parent){}
    
    virtual void process(const MouseEvent &evt){
      Camera &camera = parent->getCamera(cameraIndex);
      
      if(evt.isPressEvent()){
        anchor = evt.getRelPos();
        camSave = camera;
      }
      if(evt.isDragEvent()){
        Point32f delta = evt.getRelPos()-anchor;
        camera = camSave;      
        if(evt.isLeft()){
          camera.transform(create_hom_4x4<float>(delta.x,delta.y,0));
        }else if(evt.isMiddle()){
          Vec v = camSave.getUp()*delta.y + camSave.getHorz()*delta.x;
          v[3] = 0;
          camera.translate( v*3);
        }else if(evt.isRight()){
          camera.translate(camSave.getNorm()*(-delta.y*10));
        }
      }
    }
  };

  
  struct Scene2::RenderPlugin{
    virtual void color(GeomColor &c)=0;
    virtual void line(float x1, float y1, float x2, float y2)=0;
    virtual void point(float x, float y, float size)=0;
    virtual void triangle(float x1, float y1, float x2, 
                          float y2, float x3, float y3)=0;    
    virtual void quad(float x1, float y1, float x2, float y2, 
                      float x3, float y3, float x4, float y4)=0;
  };
  
  struct ImgQRenderPlugin : public Scene2::RenderPlugin{
    ImgQ &image;
    ImgQRenderPlugin(ImgQ &image):image(image){}
    virtual void color(GeomColor &c){
      icl::color(c[0],c[1],c[2],c[3]);
      icl::fill(c[0],c[1],c[2],c[3]);
    }
    virtual void line(float x1, float y1, float x2, float y2){
      icl::line(image,x1,y1,x2,y2);
    }
    virtual void point(float x, float y, float size){
      icl::circle(image,x,y,size);
    }
    virtual void triangle(float x1, float y1, float x2, 
                          float y2, float x3, float y3){
      icl::triangle(image,x1,y1,x2,y2,x3,y3);
    }
    virtual void quad(float x1, float y1, float x2, float y2, 
                      float x3, float y3, float x4, float y4){
      icl::triangle(image,x1,y1,x2,y2,x3,y3);
      icl::triangle(image,x3,y3,x4,y4,x1,y1);
    }
  };
  struct ICLDrawWidgetRenderPlugin : public Scene2::RenderPlugin{
    ICLDrawWidget &w;
    ICLDrawWidgetRenderPlugin(ICLDrawWidget &w):w(w){}
    virtual void color(GeomColor &c){
      w.color(c[0],c[1],c[2],c[3]);
      w.fill(c[0],c[1],c[2],c[3]);
    }
    virtual void line(float x1, float y1, float x2, float y2){
      w.line(x1,y1,x2,y2);
    }
    virtual void point(float x, float y, float size){
      w.ellipse(x-size/2,y-size/2,size,size);
    }
    virtual void triangle(float x1, float y1, float x2, 
                          float y2, float x3, float y3){
      w.triangle(x1,y1,x2,y2,x3,y3);
    }
    virtual void quad(float x1, float y1, float x2, float y2, 
                      float x3, float y3, float x4, float y4){
      w.quad(x1,y1,x2,y2,x3,y3,x4,y4);
    }
  };
  
  
  Scene2::Scene2(){}
  Scene2::~Scene2(){}
  Scene2::Scene2(const Scene2 &scene){
    *this = scene;
  }
  Scene2 &Scene2::operator=(const Scene2 &scene){
    clear();
    m_cameras = scene.m_cameras;
    m_projections = scene.m_projections;
    m_objects.resize(scene.m_objects.size());
    for(unsigned int i=0;i<m_objects.size();++i){
      m_objects[i] = scene.m_objects[i]->copy();
    }
    m_mouseHandlers.resize(scene.m_mouseHandlers.size());
    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      m_mouseHandlers[i] = new SceneMouseHandler(scene.m_mouseHandlers[i]->cameraIndex,this);
    }
    
    m_glCallbacks.resize(scene.m_glCallbacks.size());
    for(unsigned int i=0;i<m_glCallbacks.size();++i){
      m_glCallbacks[i] = new GLCallback(scene.m_glCallbacks[i]->cameraIndex,this);
    }
    
  }
  
  void Scene2::addCamera(const Camera &cam){
    m_cameras.push_back(cam);
  }
  void Scene2::removeCamera(int index){
    ICLASSERT_RETURN(index > 0 && index < m_cameras.size());
    m_cameras.erase(m_cameras.begin()+index);
  }
  Camera &Scene2::getCamera(int camIndex){
    return m_cameras[camIndex];
  }
  const Camera &Scene2::getCamera(int camIndex) const{
    return m_cameras[camIndex];
  }
  
  void Scene2::render(Img32f &image, int camIndex){
    ImgQRenderPlugin p(image);
    render(p,camIndex);
  }
  void Scene2::render(ICLDrawWidget &widget, int camIndex){
    ICLDrawWidgetRenderPlugin p(widget);
    render(p,camIndex);
  }
  
  void Scene2::addObject(Object2 *object){
    m_objects.push_back(object);
  }
  void Scene2::removeObject(int idx, bool deleteObject){
    Object2 *p = m_objects[idx];
    if(deleteObject) delete p;
    m_objects.erase(m_objects.begin()+idx);
  }
  
  void Scene2::clear(bool camerasToo){
    for(unsigned int i=0;i<m_objects.size();++i){
      delete m_objects[i];
    }

    m_objects.clear();
    m_projections.clear();
    
    if(camerasToo){
      m_cameras.clear();
    }      

    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      delete m_mouseHandlers[i];
    }
    m_mouseHandlers.clear();
    
    for(unsigned int i=0;i<m_glCallbacks.size();++i){
      delete m_glCallbacks[i];
    }
    m_glCallbacks.clear();

  }

  
  static void update_z(Primitive &p,const std::vector<Vec> &ps){
    switch(p.type){
      case Primitive::line: 
        p.z = (ps[p.a][3]+ps[p.b][3])/2.0; 
        break;
      case Primitive::triangle: 
        p.z = (ps[p.a][3]+ps[p.b][3]+ps[p.c][3])/3.0; 
        break;
      case Primitive::quad: 
        p.z = (ps[p.a][3]+ps[p.b][3]+ps[p.c][3]+ps[p.d][3])/4.0; 
        break;
      default:
        p.z = 0;
        break;
    }
  }

  namespace{
    struct ProjectedObject2{
      ProjectedObject2(Object2 *obj=0, std::vector<Vec>* ps=0):
    

        obj(obj),projections(ps){}
      Object2 *obj;
      std::vector<Vec> *projections;
      bool operator<(const ProjectedObject2 &other) const{
        obj->getZ() < other.obj->getZ();
      }
    };
  }

  void Scene2::render(RenderPlugin &renderer, int camIndex){
    Mutex::Locker l(this);
    ICLASSERT_RETURN(camIndex >= 0 && camIndex < (int)m_cameras.size());
    
    if((int)m_projections.size() <= camIndex){
      m_projections.resize(camIndex+1);
    }
    m_projections[camIndex].resize(m_objects.size());
       
    for(unsigned int i=0;i<m_objects.size();++i){
      Object2 *o = m_objects[i];
      m_cameras[camIndex].project(o->getVertices(),m_projections[camIndex][i]);
      std::vector<Primitive> &ps = o->getPrimitives();
      for(unsigned int j=0;j<ps.size();++j){
        update_z(ps[i],m_projections[camIndex][i]);
      }
      o->updateZFromPrimitives();
      std::sort(ps.begin(),ps.end());
    }
    
    
    std::vector<ProjectedObject2> sorted(m_objects.size());
    for(unsigned int i=0;i<sorted.size();++i){
      sorted[i] = ProjectedObject2(m_objects[i],&m_projections[camIndex][i]);
    }
    std::sort(sorted.begin(),sorted.end());
      
    for(unsigned int i=0;i<sorted.size();++i){  
      Object2 *o = sorted[i].obj;
      std::vector<Primitive> &primitives = o->getPrimitives();
      std::vector<Vec> &ps = *sorted[i].projections;
      for(unsigned int j=0;j<primitives.size();++j){

        Primitive &p  = primitives[j];

        if(!o->isVisible(p.type)) continue;      
        
        renderer.color(p.color);
        switch(p.type){
          case Primitive::line:
            renderer.line(ps[p.a][0],ps[p.a][1],ps[p.b][0],ps[p.b][1]);
            break;
          case Primitive::triangle:
            renderer.triangle(ps[p.a][0],ps[p.a][1],ps[p.b][0],ps[p.b][1],ps[p.c][0],ps[p.c][1]);
            break;
          case Primitive::quad:
            renderer.quad(ps[p.a][0],ps[p.a][1],ps[p.b][0],ps[p.b][1],
                          ps[p.c][0],ps[p.c][1],ps[p.d][0],ps[p.d][1]);
            break;
          default:
            ERROR_LOG("unsupported primitive type");
        }
      }
      if(o->isVisible(Primitive::vertex)){
        for(unsigned int j=0;j<ps.size();++j){
          renderer.color(o->m_vertexColors[j]);
          renderer.point(ps[j][0],ps[j][1],3);
        }
      }
    }
  }
  
  namespace {
    struct GLMatrix{
      FixedMatrix<float,4,4> mt;
      GLMatrix(const Mat &m):mt(m.transp()){}
      
      operator const float*() const{
        return mt.data();
      }
      
    };
  }
  
  void Scene2::render(int camIndex){
    Mutex::Locker l(this);
    ICLASSERT_RETURN(camIndex >= 0 && camIndex < (int)m_cameras.size());
    Camera &cam = m_cameras[camIndex];

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Vec vc = cam.getPos()+cam.getNorm();
    gluLookAt(cam.getPos()[0],cam.getPos()[1],cam.getPos()[2],
              vc[0],vc[1],vc[2],
              cam.getUp()[0],cam.getUp()[1],cam.getUp()[2]);
    // glLoadMatrixf(GLMatrix(cam.getCoordinateSystemTransformationMatrix()));
    // DEBUG_LOG("modelview matrix:\n" << GLMatrix(cam.getCoordinateSystemTransformationMatrix()).mt);
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(GLMatrix(cam.getProjectionMatrix()));

    GLfloat viewport[4];
    glGetFloatv(GL_VIEWPORT,viewport);
    float ar = viewport[2]/viewport[3];
    if(ar>1){
      glScalef(1.0/ar,1,1);
    }else{
      glScalef(1.0,ar,1);
    }
    
    glEnable(GL_DEPTH_TEST);

    //Rect v = cam.getViewPort();
    //glViewport(v.x,v.y,v.width,v.height);
    
    for(unsigned int i=0;i<m_objects.size();++i){
      Object2 *o = m_objects[i];
      std::vector<Vec> &ps = o->m_vertices;
      for(unsigned int j=0;j<o->m_primitives.size();++j){
        Primitive &p = o->m_primitives[j];
        if(!o->isVisible(p.type)) continue; 
        glColor4fv(((p.color)/255.0).begin());
        switch(p.type){
          case Primitive::line:
            glBegin(GL_LINES);
            glVertex3fv(ps[p.a].data());              
            glVertex3fv(ps[p.b].data());
            glEnd();

            break;
          case Primitive::triangle:
            glBegin(GL_TRIANGLES);
            glVertex3fv(ps[p.a].data());              
            glVertex3fv(ps[p.b].data());
            glVertex3fv(ps[p.c].data());
            glEnd();
            break;
          case Primitive::quad:
            glBegin(GL_QUADS);
            glVertex3fv(ps[p.a].data());              
            glVertex3fv(ps[p.b].data());
            glVertex3fv(ps[p.c].data());
            glVertex3fv(ps[p.d].data());
            glEnd();
            break;
          default:
            ERROR_LOG("unsupported primitive type");
        }
      }
      //glPointSize(3);
      glBegin(GL_POINTS);
      if(o->isVisible(Primitive::vertex)){
        for(unsigned int j=0;j<ps.size();++j){
          glColor3fv(((o->m_vertexColors[j])/255.0).data());
          glVertex3fv(ps[j].data());
        }
      }
      glEnd();
      //glPointSize(1);
    }
    
    
  }



  MouseHandler *Scene2::getMouseHandler(int camIndex){
    ICLASSERT_RETURN_VAL(camIndex >= 0 && camIndex < (int)m_cameras.size(),0);
    // search for already exsiting mouse handler for given camera
    
    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      if(m_mouseHandlers[i]->cameraIndex == camIndex){
        return m_mouseHandlers[i];
      }
    }
    m_mouseHandlers.push_back(new SceneMouseHandler(camIndex,this));
    return m_mouseHandlers.back();
  }

  ICLDrawWidget3D::GLCallback *Scene2::getGLCallback(int camIndex){
    ICLASSERT_RETURN_VAL(camIndex >= 0 && camIndex < (int)m_cameras.size(),0);

    // search for already exsiting mouse handler for given camera
    for(unsigned int i=0;i<m_glCallbacks.size();++i){
      if(m_glCallbacks[i]->cameraIndex == camIndex){
        return m_glCallbacks[i];
      }
    }
    m_glCallbacks.push_back(new GLCallback(camIndex,this));
    return m_glCallbacks.back();
  }



  
}

