/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLGeom/src/Scene.cpp                                  **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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
*********************************************************************/

#include <ICLGeom/Scene.h>

#ifdef HAVE_QT
#include <ICLQt/DrawWidget.h>
#include <ICLQt/GLTextureMapBaseImage.h>
#endif

#include <ICLQuick/Quick.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/StringUtils.h>

#ifdef HAVE_OPENGL

#ifdef SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#endif

#include <set>

namespace icl{

  struct CameraObject : public SceneObject{
    Scene *scene;
    int cameraIndex;
    std::vector<Vec> verticesPushed;
    
    CameraObject(Scene *parent, int cameraIndex, float camSize):
      SceneObject("cuboid",FixedColVector<float,6>(0,0,-5*camSize,6*camSize,5*camSize,10*camSize).data()),
      scene(parent),cameraIndex(cameraIndex){
      
      float &S = camSize;
      
      static float xs[8] = {-1,1,2,2,1,-1,-2,-2};
      static float ys[8] = {2,2,1,-1,-2,-2,-1,1};
      for(int i=0;i<8;++i){
        addVertex(Vec(S*xs[i],S*ys[i],S*3,1));
      }
      for(int i=0;i<8;++i){
        addVertex(Vec(S*xs[i]*0.6,S*ys[i]*0.6,0,1));
        addLine(i+8,i+16);
        if(i)addQuad(i+8,i+16,i+15,i+7);
        else addQuad(8,16,23,15);
      }
      setColor(Primitive::line,GeomColor(0,0,0,255));
      setColor(Primitive::quad,GeomColor(255,0,0,255));
      
      addVertex(Vec(0,0,-0.1,1)); // center: 24
      addVertex(Vec(S*5,0,-0.1,1)); // x-axis: 25
      addVertex(Vec(0,S*5,-0.1,1)); // y-axis: 26
      
      addLine(24,25,GeomColor(255,0,0,255));
      addLine(24,26,GeomColor(0,255,0,255));
      
      setVisible(Primitive::vertex,false);
      setVisible(Primitive::line,true);

      const std::string &name = parent->getCamera(cameraIndex).getName();
      if(name != ""){
        addVertex(Vec(3.1, 1.5, -0.1 ,1/S)*S);  // 27
        addVertex(Vec(3.1, 1.5, -9.8 ,1/S)*S);  // 28
        addVertex(Vec(3.1,-1.5, -9.8 ,1/S)*S);  // 29
        addVertex(Vec(3.1,-1.5, -0.1 ,1/S)*S);  // 30

        ImgQ image(Size(200,30),3);
        std::fill(image.begin(0),image.end(0),255);
        color(255,255,255,255);
        fontsize(25);
        text(image,2,-10,name);
        addTexture(27,28,29,30,cvt8u(image));
      }

      verticesPushed=m_vertices;
    }
    
    virtual void prepareForRendering() {
      const Camera &cam = scene->getCamera(cameraIndex);
      Mat T = cam.getCSTransformationMatrix().inv();
      for(unsigned int i=0;i<m_vertices.size();++i){
        m_vertices[i] = T*verticesPushed[i]; 
      }
    }
  };
#ifdef HAVE_QT
  struct Scene::GLCallback : public ICLDrawWidget3D::GLCallback{
    int cameraIndex;
    Scene *parent;
    GLCallback(int cameraIndex,Scene *parent):
      cameraIndex(cameraIndex),parent(parent){}
    virtual void draw(){}
    virtual void drawSpecial(ICLDrawWidget3D *widget){
      parent->render(cameraIndex, widget);
    }
  };


  
  struct  Scene::SceneMouseHandler : public MouseHandler{
    int cameraIndex;
    Scene *parent;
    Point32f anchor;
    Camera camSave;
    float speed;
    SceneMouseHandler(int cameraIndex,Scene *parent, float maxSceneDim):
      cameraIndex(cameraIndex),parent(parent),speed(maxSceneDim){
    }
    
    virtual void process(const MouseEvent &evt){
      Camera &camera = parent->getCamera(cameraIndex);
      
      if(evt.isPressEvent()){
        anchor = evt.getRelPos();
        camSave = camera;
      }
      if(evt.isDragEvent()){
        Point32f delta = evt.getRelPos()-anchor;
        camera = camSave;   
        if(evt.isLeft() && evt.isRight()){
          Vec v = camSave.getUp()*delta.y + camSave.getHoriz()*delta.x;
          v[3] = 0;
          camera.translate( v * speed);
        }else if(evt.isLeft()){
          // rotate norm about up (by delta.x)
          Vec norm = camera.getNorm();
          Vec up = camera.getUp();
          norm = rotate_vector(up, delta.x, norm);

          // rotate norm and up about horz (by delta.y)
          norm = rotate_vector(camera.getHoriz(), delta.y, norm);
          up = rotate_vector(camera.getHoriz(), delta.y, up);

          camera.setNorm(norm);
          camera.setUp(up);

        }else if(evt.isMiddle()){
          // rotate norm about up (by delta.x)
          Vec norm = camera.getNorm();
          Vec up = camera.getUp();
          Vec horz = camera.getHoriz();

          Vec upSave = up;
          Vec horzSave = camera.getHoriz();


          norm = rotate_vector(up, delta.x, norm);
          
          // rotate norm and up about horz (by delta.y)
          norm = rotate_vector(camera.getHoriz(), delta.y, norm);
          up = rotate_vector(camera.getHoriz(), delta.y, up);
          
          camera.setNorm(norm);
          camera.setUp(up);
          
          Vec pos = camera.getPosition();
          //TODO what are the correct axis?
          pos = rotate_vector(upSave,delta.x,pos);
          pos = rotate_vector(horzSave,delta.y,pos);
          
          camera.setPosition(pos);
        }else if(evt.isRight()){
          camera.translate(camSave.getNorm()*(-delta.y*speed));
          
          // rotate up about norm (by delta.x)
          Vec norm = camera.getNorm();
          Vec up = camera.getUp();
          up = rotate_vector(norm, -3.14*delta.x, up);
          camera.setUp(up);
        }
      }
    }
    void setSpeed(float maxSceneDim){
      speed = maxSceneDim;
    }
  };
#endif
  
  struct Scene::RenderPlugin{
    virtual void color(const GeomColor &c)=0;
    virtual void line(float x1, float y1, float x2, float y2, float width)=0;
    virtual void point(float x, float y, float size)=0;
    virtual void triangle(float x1, float y1, float x2, 
                          float y2, float x3, float y3)=0;
    
    virtual void quad(float x1, float y1, float x2, float y2, 
                      float x3, float y3, float x4, float y4)=0;

  };
  
  struct ImgQRenderPlugin : public Scene::RenderPlugin{
    ImgQ &image;
    ImgQRenderPlugin(ImgQ &image):image(image){}
    virtual void color(const GeomColor &c){
      icl::color(c[0],c[1],c[2],c[3]);
      icl::fill(c[0],c[1],c[2],c[3]);
    }
    virtual void line(float x1, float y1, float x2, float y2, float width){
      (void)width; // linewidth is not yet supported!
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

#ifdef HAVE_QT
  struct ICLDrawWidgetRenderPlugin : public Scene::RenderPlugin{
    ICLDrawWidget &w;
    ICLDrawWidgetRenderPlugin(ICLDrawWidget &w):w(w){}
    virtual void color(const GeomColor &c){
      w.color(c[0],c[1],c[2],c[3]);
      w.fill(c[0],c[1],c[2],c[3]);
    }
    virtual void line(float x1, float y1, float x2, float y2, float width){
      w.linewidth(width);
      w.line(x1,y1,x2,y2);
    }
    virtual void point(float x, float y, float size){
      w.pointsize(size);
      w.point(x,y);
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
#endif  
  
  Scene::Scene():m_lightSimulationEnabled(true),m_drawCamerasEnabled(true){}
  Scene::~Scene(){}
  Scene::Scene(const Scene &scene){
    *this = scene;
  }
  Scene &Scene::operator=(const Scene &scene){
    clear();
    m_cameras = scene.m_cameras;
    m_projections = scene.m_projections;
    m_objects.resize(scene.m_objects.size());
    for(unsigned int i=0;i<m_objects.size();++i){
      m_objects[i] = scene.m_objects[i]->copy();
    }
#ifdef HAVE_QT
    m_mouseHandlers.resize(scene.m_mouseHandlers.size());
    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      m_mouseHandlers[i] = new SceneMouseHandler(scene.m_mouseHandlers[i]->cameraIndex,this,scene.m_mouseHandlers[i]->speed);
    }

    
    m_glCallbacks.resize(scene.m_glCallbacks.size());
    for(unsigned int i=0;i<m_glCallbacks.size();++i){
      m_glCallbacks[i] = new GLCallback(scene.m_glCallbacks[i]->cameraIndex,this);
    }
#endif

    m_lightSimulationEnabled = scene.m_lightSimulationEnabled;
    m_drawCamerasEnabled = scene.m_drawCamerasEnabled;
    return *this;
  }
  
  void Scene::addCamera(const Camera &cam, float visSize){
    m_cameras.push_back(cam);
    m_cameraObjects.push_back(new CameraObject(this,m_cameraObjects.size(), visSize));
  }
  void Scene::removeCamera(int index){
    ICLASSERT_RETURN(index > 0 && index <(int) m_cameras.size());
    m_cameras.erase(m_cameras.begin()+index);
    delete m_cameraObjects[index];
    m_cameraObjects.erase(m_cameraObjects.begin()+index);
  }
  Camera &Scene::getCamera(int camIndex){
    return m_cameras[camIndex];
  }
  const Camera &Scene::getCamera(int camIndex) const{
    return m_cameras[camIndex];
  }
  
  void Scene::render(Img32f &image, int camIndex){
    ImgQRenderPlugin p(image);
    render(p,camIndex);
  }
#ifdef HAVE_QT
  void Scene::render(ICLDrawWidget &widget, int camIndex){
    ICLDrawWidgetRenderPlugin p(widget);
    render(p,camIndex);
  }
#endif
  
  void Scene::addObject(SceneObject *object){
    m_objects.push_back(object);
  }
  void Scene::removeObject(int idx, bool deleteObject){
    ICLASSERT_RETURN(idx >= 0 && idx < (int)m_objects.size());
    SceneObject *p = m_objects[idx];
    if(deleteObject) delete p;
    m_objects.erase(m_objects.begin()+idx);
  }
  void Scene::removeObjects(int startIndex, int endIndex, bool deleteObjects){
    if(endIndex < 0) endIndex = m_objects.size();
    ICLASSERT_RETURN(startIndex >= 0 && startIndex < (int)m_objects.size());
    ICLASSERT_RETURN(endIndex >= 0 && endIndex <= (int)m_objects.size());
    ICLASSERT_RETURN(endIndex > startIndex);
    
    int pos = startIndex;
    while(startIndex++ < endIndex){
      removeObject(pos,deleteObjects);
    }
  }
  
  void Scene::clear(bool camerasToo){
    for(unsigned int i=0;i<m_objects.size();++i){
      delete m_objects[i];
    }

    m_objects.clear();
    m_projections.clear();
    
    if(camerasToo){
      m_cameras.clear();
    }      
#ifdef HAVE_QT
    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      delete m_mouseHandlers[i];
    }
    m_mouseHandlers.clear();
    
    for(unsigned int i=0;i<m_glCallbacks.size();++i){
      delete m_glCallbacks[i];
    }
    m_glCallbacks.clear();
#endif

  }

  
  static void update_z(Primitive &p,const std::vector<Vec> &ps){
    switch(p.type){
      case Primitive::line: 
        p.z = (ps[p.a][2]+ps[p.b][2])/2.0; 
        break;
      case Primitive::triangle: 
        p.z = (ps[p.a][2]+ps[p.b][2]+ps[p.c][2])/3.0; 
        break;
      case Primitive::quad: 
      case Primitive::texture:
        p.z = (ps[p.a][2]+ps[p.b][2]+ps[p.c][2]+ps[p.d][2])/4.0; 
        break;
      default:
        p.z = 0;
        break;
    }
  }
  
  static GeomColor adapt_color_by_light_simulation(const Vec &a, const Vec &b, const Vec &c,
                                                   GeomColor col){
    Vec d = normalize3(cross(a-c,b-c));
    
    float fr = 0.3+fabs(d[0])*0.7;
    float fg = 0.3+fabs(d[1])*0.7;
    float fb = 0.3+fabs(d[2])*0.7;
    col[0]*=fr;
    col[1]*=fg;
    col[2]*=fb;
    return col;
  }

  namespace{
    struct ProjectedSceneObject{
      ProjectedSceneObject(SceneObject *obj=0, std::vector<Vec>* ps=0):
        obj(obj),projections(ps){}
      SceneObject *obj;
      std::vector<Vec> *projections;
      
      // NOT reverse ordering
      bool operator<(const ProjectedSceneObject &other) const{
        return obj->getZ() < other.obj->getZ();
      }
    };
  }

  void Scene::render(RenderPlugin &renderer, int camIndex){
    Mutex::Locker l(this);
    ICLASSERT_RETURN(camIndex >= 0 && camIndex < (int)m_cameras.size());

    std::vector<SceneObject*> allObjects(m_objects);
    if(m_drawCamerasEnabled){
      for(unsigned int i=0;i<m_cameraObjects.size();++i){
        if((int)i == camIndex) continue;
        allObjects.push_back(m_cameraObjects[i]);
      }
    }
    
    if((int)m_projections.size() <= camIndex){
      m_projections.resize(camIndex+1);
    }
    m_projections[camIndex].resize(allObjects.size());
       
    for(unsigned int i=0;i<allObjects.size();++i){
      SceneObject *o = allObjects[i];
      o->lock();
      o->prepareForRendering();

      m_cameras[camIndex].projectGL(o->getVertices(),m_projections[camIndex][i]);
      std::vector<Primitive> &ps = o->getPrimitives();
      for(unsigned int j=0;j<ps.size();++j){
        update_z(ps[j],m_projections[camIndex][i]);
      }
      o->updateZFromPrimitives();
      std::sort(ps.begin(),ps.end());
    }
    

    // ok, i guess there's some bug in std::sort or something ...
    /*
        std::vector<ProjectedSceneObject> sorted(allObjects.size());
        for(unsigned int i=0;i<sorted.size();++i){
        sorted[i] = ProjectedSceneObject(allObjects[i],&m_projections[camIndex][i]);
        }
        
        DEBUG_LOG("starting to sort ..");
        std::sort(sorted.begin(),sorted.end());
        DEBUG_LOG("done");
    */
    
    /// fallback using a std::multiset
    std::multiset<ProjectedSceneObject> sortedSet;
    for(unsigned int i=0;i<allObjects.size();++i){
      sortedSet.insert(ProjectedSceneObject(allObjects[i],&m_projections[camIndex][i]));
    }
    std::vector<ProjectedSceneObject> sorted(sortedSet.begin(),sortedSet.end()); 
    
    for(unsigned int i=0;i<sorted.size();++i){  
      SceneObject *o = sorted[i].obj;
      std::vector<Primitive> &primitives = o->getPrimitives();
      std::vector<Vec> &ps = *sorted[i].projections;
      std::vector<Vec> &vx = o->m_vertices;
      for(unsigned int j=0;j<primitives.size();++j){

        Primitive &p  = primitives[j];

        if(!o->isVisible(p.type)) continue;      
        
        if(!m_lightSimulationEnabled){
          renderer.color(p.color);
        }
        switch(p.type){
          case Primitive::line:
            if(m_lightSimulationEnabled){
              renderer.color(p.color);
            }
            renderer.line(ps[p.a][0],ps[p.a][1],ps[p.b][0],ps[p.b][1],o->m_lineWidth);
            break;
          case Primitive::triangle:{

            Vec &a = vx[p.a];
            Vec &b = vx[p.b];
            Vec &c = vx[p.c];
            
            if(m_lightSimulationEnabled){
              renderer.color(adapt_color_by_light_simulation(a,b,c,p.color));
            }

            renderer.triangle(ps[p.a][0],ps[p.a][1],ps[p.b][0],ps[p.b][1],ps[p.c][0],ps[p.c][1]);
            break;
          }case Primitive::quad:{

            Vec &a = vx[p.a];
            Vec &b = vx[p.b];
            Vec &c = vx[p.c];
            //Vec &d = vx[p.c];

            if(m_lightSimulationEnabled){
              renderer.color(adapt_color_by_light_simulation(a,b,c,p.color));
            }

            renderer.quad(ps[p.a][0],ps[p.a][1],ps[p.b][0],ps[p.b][1],
                          ps[p.c][0],ps[p.c][1],ps[p.d][0],ps[p.d][1]);
            break;
           }case Primitive::texture:{
              // not yet supported
           }
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
      o->unlock();
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
#ifdef HAVE_QT
#ifdef HAVE_OPENGL
  void Scene::render(int camIndex, ICLDrawWidget3D *widget){

    Mutex::Locker l(this);
    ICLASSERT_RETURN(camIndex >= 0 && camIndex < (int)m_cameras.size());

    Rect currentImageRect = widget->getImageRect(true);
    Size currentImageSize = widget->getImageSize(true);
    Size widgetSize = widget->getSize();

    Camera cam = m_cameras[camIndex];
    cam.getRenderParams().viewport = currentImageRect;

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(GLMatrix(cam.getCSTransformationMatrixGL()));
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(GLMatrix(cam.getProjectionMatrixGL()));

    if (widget->getFitMode() == ICLWidget::fmZoom) {
      // transforms y in case of having zoom activated
      float dy = (currentImageRect.height-widgetSize.height);
      glViewport(currentImageRect.x,-dy-currentImageRect.y,currentImageRect.width,currentImageRect.height);
    } else {
      glViewport(currentImageRect.x,currentImageRect.y,currentImageRect.width,currentImageRect.height);
    }
    GLboolean on[4] = {0,0,0,0};
    GLenum flags[4] = {GL_DEPTH_TEST,GL_LINE_SMOOTH,GL_POINT_SMOOTH,GL_POLYGON_SMOOTH};
    for(int i=0;i<4;++i){
      glGetBooleanv(flags[i],on+i);
      glEnable(flags[i]);
    }

    std::vector<SceneObject*> allObjects(m_objects);
    if(m_drawCamerasEnabled){
      for(unsigned int i=0;i<m_cameraObjects.size();++i){
        if((int)i == camIndex) continue;
        allObjects.push_back(m_cameraObjects[i]);
      }
    }

    for(unsigned int i=0;i<allObjects.size();++i){
      SceneObject *o = allObjects[i];
      
      glPointSize(o->m_pointSize);
      glLineWidth(o->m_lineWidth);

      o->prepareForRendering();
      std::vector<Vec> &ps = o->m_vertices;
      for(unsigned int j=0;j<o->m_primitives.size();++j){
        Primitive &p = o->m_primitives[j];
        if(!o->isVisible(p.type)) continue; 
        glColor4fv(((p.color)/255.0).begin());
        switch(p.type){
          case Primitive::line:
            glBegin(GL_LINES);
            if(o->m_lineColorsFromVertices) glColor3fv((o->m_vertexColors[p.a]/255).data());
            glVertex3fv(ps[p.a].data());              
            if(o->m_lineColorsFromVertices) glColor3fv((o->m_vertexColors[p.b]/255).data());
            glVertex3fv(ps[p.b].data());
            glEnd();

            break;
          case Primitive::triangle:{
            glBegin(GL_TRIANGLES);
            Vec &a = ps[p.a];
            Vec &b = ps[p.b];
            Vec &c = ps[p.c];

            glNormal3fv(normalize(cross(a-c,b-c)).data());

            if(o->m_triangleColorsFromVertices) glColor3fv((o->m_vertexColors[p.a]/255).data());
            glVertex3fv(a.data());              
            if(o->m_triangleColorsFromVertices) glColor3fv((o->m_vertexColors[p.b]/255).data());
            glVertex3fv(b.data());
            if(o->m_triangleColorsFromVertices) glColor3fv((o->m_vertexColors[p.c]/255).data());
            glVertex3fv(c.data());
            glEnd();
            break;
          }case Primitive::quad:{

            glBegin(GL_QUADS);
            Vec &a = ps[p.a];
            Vec &b = ps[p.b];
            Vec &c = ps[p.c];
            Vec &d = ps[p.d];
            
            glNormal3fv(normalize(cross(d-c,b-c)).data());

            if(o->m_quadColorsFromVertices) glColor3fv((o->m_vertexColors[p.a]/255).data());
            glVertex3fv(a.data());              
            if(o->m_quadColorsFromVertices) glColor3fv((o->m_vertexColors[p.b]/255).data());
            glVertex3fv(b.data());
            if(o->m_quadColorsFromVertices) glColor3fv((o->m_vertexColors[p.c]/255).data());
            glVertex3fv(c.data());
            if(o->m_quadColorsFromVertices) glColor3fv((o->m_vertexColors[p.d]/255).data());
            glVertex3fv(d.data());
            glEnd();
            break;
           }case Primitive::texture:{
              glColor4f(1,1,1,1);
              Vec &a = ps[p.a];
              Vec &b = ps[p.b];
              Vec &c = ps[p.c];
              Vec &d = ps[p.d];
              // left hand normal ?!
              glNormal3fv(normalize(cross(b-c,d-c)).data());
              GLTextureMapBaseImage tim(&p.tex);
              tim.drawTo3D(a.begin(),b.begin(),d.begin());
              break;
          }
          default:
            ERROR_LOG("unsupported primitive type");
        }
      }
      glBegin(GL_POINTS);
      if(o->isVisible(Primitive::vertex)){
        for(unsigned int j=0;j<ps.size();++j){
          glColor3fv(((o->m_vertexColors[j])/255.0).data());
          glVertex3fv(ps[j].data());
        }
      }
      glEnd();

    }

    for(int i=0;i<4;++i){
      if(!on[i]){
        glDisable(flags[i]);
      }
    }

  }



  MouseHandler *Scene::getMouseHandler(int camIndex){
    ICLASSERT_RETURN_VAL(camIndex >= 0 && camIndex < (int)m_cameras.size(),0);
    // search for already exsiting mouse handler for given camera
    
    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      if(m_mouseHandlers[i]->cameraIndex == camIndex){
        m_mouseHandlers[i]->setSpeed(getMaxSceneDim());
        return m_mouseHandlers[i];
      }
    }
    m_mouseHandlers.push_back(new SceneMouseHandler(camIndex,this, getMaxSceneDim()));
    return m_mouseHandlers.back();
  }

  ICLDrawWidget3D::GLCallback *Scene::getGLCallback(int camIndex){
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

#endif // QT
#endif // GL

  void Scene::setLightSimulationEnabled(bool enabled){
    m_lightSimulationEnabled = enabled;
  }
  bool Scene::getLightSimulationEnabled() const{
    return m_lightSimulationEnabled;
  }

  void Scene::setDrawCamerasEnabled(bool enabled){
    m_drawCamerasEnabled = enabled;
  }
  bool Scene::getDrawCamerasEnabled() const{
    return m_drawCamerasEnabled;
  }
  
  
  float Scene::getMaxSceneDim() const{
    Range32f rangeXYZ[3]={Range32f::limits(),Range32f::limits(),Range32f::limits()};
    for(int i=0;i<3;++i){
      std::swap(rangeXYZ[i].minVal,rangeXYZ[i].maxVal);
    }
    for(unsigned int i=0;i<m_objects.size();++i){
      SceneObject *o = m_objects[i];
      std::vector<Vec> &ps = o->m_vertices;
      for(unsigned int j=0;j<ps.size();++j){
        for(int k=0;k<3;++k){
          if(ps[j][k] < rangeXYZ[k].minVal) rangeXYZ[k].minVal = ps[j][k];
          if(ps[j][k] > rangeXYZ[k].maxVal) rangeXYZ[k].maxVal = ps[j][k];
        }
      }
    }
    return iclMax(iclMax(rangeXYZ[1].getLength(),rangeXYZ[2].getLength()),rangeXYZ[0].getLength());
  }


  
}

