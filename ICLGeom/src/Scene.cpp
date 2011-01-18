/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/Scene.cpp                                  **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Erik Weitnauer, Daniel Dornbusch  **
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

#include <ICLGeom/Scene.h>
#include <ICLGeom/CoordinateFrameSceneObject.h>
#ifdef HAVE_QT
#include <ICLQt/DrawWidget.h>
#include <ICLQt/GLTextureMapBaseImage.h>
#endif

#include <ICLQuick/Quick.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/StringUtils.h>

#ifdef HAVE_OPENGL

#ifdef ICL_SYSTEM_APPLE
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
    std::vector<Vec> origVertices;
    float S;
    bool haveName;
    Mutex mutex;
    std::string lastName;

    CameraObject(Scene *parent, int cameraIndex, float camSize):
      scene(parent),cameraIndex(cameraIndex){

      S = camSize*50;

      addVertex(Vec(0,0,0,1),geom_white());
      addVertex(Vec(S,0,0,1),geom_red());
      addVertex(Vec(0,S,0,1),geom_green());
      for(int i=0;i<4;++i){ // indices 3,4,5 and 6
        /// these depend on the camera's parameters
        addVertex(Vec(0,0,0,1),geom_blue());
      }
      
      addLine(0,1,geom_red());
      addLine(0,2,geom_green());
     
      for(int i=0;i<4;++i){
        addLine(0,i+3,geom_white());
      }
      
      addLine(3,4,geom_white());
      addLine(3,5,geom_white());
      addLine(5,6,geom_white());
      addLine(4,6,geom_white());
      
      addTriangle(0,3,4,geom_blue(100));
      addTriangle(0,4,6,geom_blue(100));
      addTriangle(0,6,5,geom_blue(100));
      addTriangle(0,5,3,geom_blue(100));


      for(int i=0;i<4;++i){ // 7,8,9,10
        addVertex(Vec(0,0,0,1),geom_invisible());
      }

      addTexture(7,8,10,9,Img8u(Size(10,10),4));

      origVertices = m_vertices;
    }

    virtual void prepareForRendering() {
      const Camera &cam = scene->getCamera(cameraIndex);

      Mat T = cam.getCSTransformationMatrix().inv();
      int w = cam.getRenderParams().viewport.width;
      int h = cam.getRenderParams().viewport.height;

      PlaneEquation p(T*Vec(0,0,S,1),T*Vec(0,0,1,1)-cam.getPosition());
      m_vertices[3] = cam.getViewRay(Point32f(w-1,0)).getIntersection(p);
      m_vertices[4] = cam.getViewRay(Point32f(0,0)).getIntersection(p);
      m_vertices[5] = cam.getViewRay(Point32f(w-1,h-1)).getIntersection(p);
      m_vertices[6] = cam.getViewRay(Point32f(0,h-1)).getIntersection(p);

      std::string name = cam.getName();
      
      if(name != lastName){
        if(name != ""){
          lock();
          m_primitives.back().tex = Primitive::create_text_texture(name,GeomColor(255,255,255,255),30);
          unlock();
        }else{
          m_primitives.back().tex.fill((icl8u)0);
        }
        lastName = name;
      }
      
      if(lastName != ""){
        float h = S/5;
        float w = name.length()*h*0.6;
        origVertices[7] = Vec(0,0,0,1);
        origVertices[8] = Vec(w,0,0,1);
        origVertices[9] = Vec(0,h,0,1);
        origVertices[10] = Vec(w,h,0,1);
      }
      
      for(unsigned int i=0;i<3;++i){
        m_vertices[i] = T * origVertices[i];
      }
      for(unsigned int i=7;i<11;++i){
        m_vertices[i] = T * origVertices[i];
      }
    }

    virtual void lock(){ mutex.lock(); }
    virtual void unlock(){ mutex.unlock(); }

  };


#ifdef HAVE_QT
#ifdef HAVE_OPENGL
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
#endif
#endif

 
  Scene::Scene():m_drawCamerasEnabled(true),
                 m_drawCoordinateFrameEnabled(false),
                 m_lightingEnabled(true){
    m_coordinateFrameObject = SmartPtr<SceneObject>(new CoordinateFrameSceneObject(100,5));
    m_lights[0] = SmartPtr<SceneLight>(new SceneLight(0));
  }
  Scene::~Scene(){
    
  }
  Scene::Scene(const Scene &scene){
    *this = scene;
  }
  Scene &Scene::operator=(const Scene &scene){
    m_cameras = scene.m_cameras;
    m_objects.resize(scene.m_objects.size());
    for(unsigned int i=0;i<m_objects.size();++i){
      m_objects[i] = scene.m_objects[i]->copy();
    }
#ifdef HAVE_QT
#ifdef HAVE_OPENGL
    m_mouseHandlers.resize(scene.m_mouseHandlers.size());
    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      m_mouseHandlers[i] = SmartPtr<SceneMouseHandler>(new SceneMouseHandler( *(scene.m_mouseHandlers[i].get()) ));
      m_mouseHandlers[i]->setParentScene( this );
    }

    m_glCallbacks.resize(scene.m_glCallbacks.size());
    for(unsigned int i=0;i<m_glCallbacks.size();++i){
      m_glCallbacks[i] = SmartPtr<GLCallback>(new GLCallback(scene.m_glCallbacks[i]->cameraIndex,this));
    }
#endif
#endif

    m_drawCamerasEnabled = scene.m_drawCamerasEnabled;
    m_drawCoordinateFrameEnabled = scene.m_drawCoordinateFrameEnabled;
    m_coordinateFrameObject = scene.m_coordinateFrameObject->copy();
    
    for(unsigned int i=0;i<8;++i){
      if(scene.m_lights[i]){
        m_lights[i] = SmartPtr<SceneLight>(new SceneLight(*scene.m_lights[i]));
        if(m_lights[i]->anchor == SceneLight::ObjectAnchor){
          m_lights[i]->setAnchor(getObject(scene.findPath(m_lights[i]->objectAnchor)));
        }
      }else{
        m_lights[i] = SmartPtr<SceneLight>();
      }
    }
    
    
    return *this;
  }

  void Scene::addCamera(const Camera &cam, float visSize){
    m_cameras.push_back(cam);
    m_cameraObjects.push_back(new CameraObject(this,m_cameraObjects.size(), visSize));
  }
  void Scene::removeCamera(int index){
    ICLASSERT_RETURN(index > 0 && index <(int) m_cameras.size());
    m_cameras.erase(m_cameras.begin()+index);
    m_cameraObjects.erase(m_cameraObjects.begin()+index);
  }
  Camera &Scene::getCamera(int camIndex){
    return m_cameras[camIndex];
  }
  const Camera &Scene::getCamera(int camIndex) const{
    return m_cameras[camIndex];
  }

  void Scene::addObject(SceneObject *object, bool passOwnerShip){
    m_objects.push_back(SmartPtr<SceneObject>(object,passOwnerShip));
  }
  void Scene::removeObject(int idx){
    ICLASSERT_RETURN(idx >= 0 && idx < (int)m_objects.size());
    m_objects.erase(m_objects.begin()+idx);
  }
  void Scene::removeObjects(int startIndex, int endIndex){
    if(endIndex < 0) endIndex = m_objects.size();
    ICLASSERT_RETURN(startIndex >= 0 && startIndex < (int)m_objects.size());
    ICLASSERT_RETURN(endIndex >= 0 && endIndex <= (int)m_objects.size());
    ICLASSERT_RETURN(endIndex > startIndex);

    int pos = startIndex;
    while(startIndex++ < endIndex){
      removeObject(pos);
    }
  }

  void Scene::clear(bool camerasToo){
    m_objects.clear();

    if(camerasToo){
      m_cameras.clear();
    }
#ifdef HAVE_QT
#ifdef HAVE_OPENGL
    m_mouseHandlers.clear();
    m_glCallbacks.clear();
#endif
#endif

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

  void Scene::renderSceneObjectRecursive(SceneObject *o){
    if(o->getSmoothShading()){
      glShadeModel(GL_SMOOTH);
    }else{
      glShadeModel(GL_FLAT);
    }
    
    glPointSize(o->m_pointSize);
    glLineWidth(o->m_lineWidth);
    
    // *new* we use openGL's matrix stack to draw the scene graph!
    // this is much more efficient than trasforming all vertices in
    // software. 
    // Nontheless, prepareForRenderingAndTransform is still used for
    // the non-opengl based redering pipeline ...
    o->prepareForRendering();

    o->lock();
    
    const std::vector<Vec> &ps = o->m_vertices;
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    const Mat &T = o->getTransformation(true);
    glMultMatrixf(T.transp().data());
    

    for(unsigned int j=0;j<o->m_primitives.size();++j){
      Primitive &p = o->m_primitives[j];
      if(!o->isVisible(p.type)) continue;
      glColor4fv(((p.color)/255.0).begin());
      switch(p.type){
        case Primitive::line:
          glBegin(GL_LINES);

          if(o->m_normalMode == SceneObject::NormalsPerFace){
            glNormal3fv(o->m_normals[j].data());
          }

          if(o->m_normalMode == SceneObject::NormalsPerVertex) glNormal3fv(o->m_normals[p.a].data());
          if(o->m_lineColorsFromVertices) glColor3fv((o->m_vertexColors[p.a]/255).data());
          glVertex3fv(ps[p.a].data());
          if(o->m_normalMode == SceneObject::NormalsPerVertex) glNormal3fv(o->m_normals[p.b].data());
          if(o->m_lineColorsFromVertices) glColor3fv((o->m_vertexColors[p.b]/255).data());
          glVertex3fv(ps[p.b].data());
          glEnd();
          
          break;
        case Primitive::triangle:{
          glBegin(GL_TRIANGLES);
          const Vec &a = ps[p.a];
          const Vec &b = ps[p.b];
          const Vec &c = ps[p.c];
          
          if(o->m_normalMode == SceneObject::AutoNormals){
            glNormal3fv(normalize(cross(a-c,b-c)).data());
          }else if(o->m_normalMode == SceneObject::NormalsPerFace){
            glNormal3fv(o->m_normals[j].data());
          }
          
          if(o->m_normalMode == SceneObject::NormalsPerVertex) glNormal3fv(o->m_normals[p.a].data());
          if(o->m_triangleColorsFromVertices) glColor3fv((o->m_vertexColors[p.a]/255).data());
          glVertex3fv(a.data());
          if(o->m_normalMode == SceneObject::NormalsPerVertex) glNormal3fv(o->m_normals[p.b].data());
          if(o->m_triangleColorsFromVertices) glColor3fv((o->m_vertexColors[p.b]/255).data());
          glVertex3fv(b.data());
          if(o->m_normalMode == SceneObject::NormalsPerVertex) glNormal3fv(o->m_normals[p.c].data());
          if(o->m_triangleColorsFromVertices) glColor3fv((o->m_vertexColors[p.c]/255).data());
          glVertex3fv(c.data());
          glEnd();
          break;
        }case Primitive::quad:{
           glBegin(GL_QUADS);
           const Vec &a = ps[p.a];
           const Vec &b = ps[p.b];
           const Vec &c = ps[p.c];
           const Vec &d = ps[p.d];
           
           if(o->m_normalMode == SceneObject::AutoNormals){
             glNormal3fv(normalize(cross(d-c,b-c)).data());
           }else if(o->m_normalMode == SceneObject::NormalsPerFace){
             glNormal3fv(o->m_normals[j].data());
           }
           
           if(o->m_normalMode == SceneObject::NormalsPerVertex) glNormal3fv(o->m_normals[p.a].data());
           if(o->m_quadColorsFromVertices) glColor3fv((o->m_vertexColors[p.a]/255).data());
           glVertex3fv(a.data());
           if(o->m_normalMode == SceneObject::NormalsPerVertex) glNormal3fv(o->m_normals[p.b].data());
           if(o->m_quadColorsFromVertices) glColor3fv((o->m_vertexColors[p.b]/255).data());
           glVertex3fv(b.data());
           if(o->m_normalMode == SceneObject::NormalsPerVertex) glNormal3fv(o->m_normals[p.c].data());
           if(o->m_quadColorsFromVertices) glColor3fv((o->m_vertexColors[p.c]/255).data());
           glVertex3fv(c.data());
           if(o->m_normalMode == SceneObject::NormalsPerVertex) glNormal3fv(o->m_normals[p.d].data());
           if(o->m_quadColorsFromVertices) glColor3fv((o->m_vertexColors[p.d]/255).data());
           glVertex3fv(d.data());
           glEnd();
           
           break;
         }
        case Primitive::polygon:{
          glBegin(GL_POLYGON);
          if(o->m_normalMode == SceneObject::AutoNormals){
            // currently there are not autonormals for polygonal primitives
            //glNormal3fv(normalize(cross(d-c,b-c)).data());
          }else if(o->m_normalMode == SceneObject::NormalsPerFace){
            glNormal3fv(o->m_normals[j].data());
          }

          for(unsigned int k=0;k<p.polyData.size();++k){
            const Vec &v = ps[p.polyData[k]];
            // how to generate a normal here
            // glNormal3fv(normalize(cross(d-c,b-c)).data());
            if(o->m_normalMode == SceneObject::NormalsPerVertex) glNormal3fv(o->m_normals[p.polyData[k]].data());
            if(o->m_polyColorsFromVertices) glColor3fv((o->m_vertexColors[p.polyData[k]]/255).data());
            glVertex3fv(v.data());
          }
          glEnd();
          break;
        }
        case Primitive::texture:{
          glColor4f(1,1,1,1);
          const Vec &a = ps[p.a];
          const Vec &b = ps[p.b];
          const Vec &c = ps[p.c];
          const Vec &d = ps[p.d];
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
    
    
    for(unsigned int i=0;i<o->m_children.size();++i){
      renderSceneObjectRecursive(o->m_children[i].get());
    }
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    o->unlock();
  }

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

    if(m_lightingEnabled){
      glEnable(GL_LIGHTING);
      for(int i=0;i<8;++i){
        if(m_lights[i]) {
          m_lights[i]->setupGL(*this,getCamera(camIndex));
        }else{
          glDisable(GL_LIGHT0+i);
        }
      }
    }else{
      glDisable(GL_LIGHTING);
    }

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

    std::vector<SmartPtr<SceneObject> > allObjects(m_objects);
    if(m_drawCamerasEnabled){
      for(unsigned int i=0;i<m_cameraObjects.size();++i){
        if((int)i == camIndex) continue;
        allObjects.push_back(m_cameraObjects[i]);
      }
    }
    if(m_drawCoordinateFrameEnabled){
      allObjects.push_back(m_coordinateFrameObject);
    }

    for(unsigned int i=0;i<allObjects.size();++i){
      renderSceneObjectRecursive(allObjects[i].get());
    }

    for(int i=0;i<4;++i){
      if(!on[i]){
        glDisable(flags[i]);
      }
    }

  }

  MouseHandler *Scene::getMouseHandler(int camIndex){
    // check input
    ICLASSERT_RETURN_VAL(camIndex >= 0 && camIndex < (int)m_cameras.size(),0);

    // Search for already exsiting mouse handler for given camera.
    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      if(m_mouseHandlers[i]->getCameraIndex() == camIndex){
        return m_mouseHandlers[i].get();
      }
    }

    // No mouse handler found for camera. Create a new one.
    SceneMouseHandler* newSceneMouseHandler = new SceneMouseHandler(camIndex,this);
    ICLASSERT(newSceneMouseHandler!=0);
    newSceneMouseHandler->setSensitivities(getMaxSceneDim());
    m_mouseHandlers.push_back(newSceneMouseHandler);

    // return mouse handler
    return newSceneMouseHandler;
  }

  void Scene::setMouseHandler(SceneMouseHandler* sceneMouseHandler, int camIndex){
    // check input
    ICLASSERT_RETURN(camIndex >= 0 && camIndex < (int)m_cameras.size());
    ICLASSERT_RETURN(sceneMouseHandler);

    // Search for existing mouse handler and replace it.
    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      if(m_mouseHandlers[i]->getCameraIndex() == camIndex){
        // assign new mouse handler
        sceneMouseHandler->setParentScene(this);
        m_mouseHandlers[i] = sceneMouseHandler;
        return;
      }
    }

    // Camera did not have a mouse handler. Add new one.
    sceneMouseHandler->setParentScene(this);
    m_mouseHandlers.push_back(sceneMouseHandler);
  }

  ICLDrawWidget3D::GLCallback *Scene::getGLCallback(int camIndex){
    ICLASSERT_RETURN_VAL(camIndex >= 0 && camIndex < (int)m_cameras.size(),0);

    // search for already exsiting mouse handler for given camera
    for(unsigned int i=0;i<m_glCallbacks.size();++i){
      if(m_glCallbacks[i]->cameraIndex == camIndex){
        return m_glCallbacks[i].get();
      }
    }
    m_glCallbacks.push_back(new GLCallback(camIndex,this));
    return m_glCallbacks.back().get();
  }

#endif // QT
#endif // GL

  void Scene::setDrawCamerasEnabled(bool enabled){
    m_drawCamerasEnabled = enabled;
  }

  void Scene::setDrawCoordinateFrameEnabled(bool enabled, float axisLength, float axisThickness){
    m_drawCoordinateFrameEnabled = enabled;
    ((CoordinateFrameSceneObject*)m_coordinateFrameObject.get())->setParams(axisLength,axisThickness);
  }

  bool Scene::getDrawCamerasEnabled() const{
    return m_drawCamerasEnabled;
  }

  void Scene::extendMaxSceneDimRecursive(float &minX, float &maxX, 
                                         float &minY, float &maxY, 
                                         float &minZ, float &maxZ,
                                         SceneObject *o) const{
    std::vector<Vec> &ps = o->m_vertices;
    for(unsigned int j=0;j<ps.size();++j){
      if(ps[j][0] < minX) minX = ps[j][0];
      if(ps[j][0] > maxX) maxX = ps[j][0];
      if(ps[j][1] < minY) minY = ps[j][1];
      if(ps[j][1] > maxY) maxY = ps[j][1];
      if(ps[j][2] < minZ) minZ = ps[j][2];
      if(ps[j][2] > maxZ) maxZ = ps[j][2];
    }
    
    for(unsigned int i=0;i<o->m_children.size();++i){
      extendMaxSceneDimRecursive(minX,maxX,minY,maxY,minZ,maxZ,o->m_children[i].get());
    }
   
  }



  float Scene::getMaxSceneDim() const{
    Range32f rangeXYZ[3]={Range32f::limits(),Range32f::limits(),Range32f::limits()};
    for(int i=0;i<3;++i){
      std::swap(rangeXYZ[i].minVal,rangeXYZ[i].maxVal);
    }
    for(unsigned int i=0;i<m_objects.size();++i){
      extendMaxSceneDimRecursive(rangeXYZ[0].minVal,rangeXYZ[0].maxVal, 
                                 rangeXYZ[1].minVal,rangeXYZ[1].maxVal, 
                                 rangeXYZ[2].minVal,rangeXYZ[2].maxVal,
                                 const_cast<SceneObject*>(m_objects[i].get()));
    }
    if(m_drawCamerasEnabled){
      for(unsigned i=0;i<m_cameraObjects.size();++i){
        extendMaxSceneDimRecursive(rangeXYZ[0].minVal,rangeXYZ[0].maxVal, 
                                   rangeXYZ[1].minVal,rangeXYZ[1].maxVal, 
                                   rangeXYZ[2].minVal,rangeXYZ[2].maxVal,
                                   const_cast<SceneObject*>(m_cameraObjects[i].get()));
      }
    }
    if(m_drawCoordinateFrameEnabled){
      extendMaxSceneDimRecursive(rangeXYZ[0].minVal,rangeXYZ[0].maxVal, 
                                 rangeXYZ[1].minVal,rangeXYZ[1].maxVal, 
                                 rangeXYZ[2].minVal,rangeXYZ[2].maxVal,
                                 const_cast<SceneObject*>(m_coordinateFrameObject.get()));
    }

    return iclMax(iclMax(rangeXYZ[1].getLength(),rangeXYZ[2].getLength()),rangeXYZ[0].getLength());
  }

  SceneLight &Scene::getLight(int index) throw (ICLException){
    if(index < 0 || index > 7) throw ICLException("invalid light index");
    if(!m_lights[index]){
      m_lights[index] = SmartPtr<SceneLight>(new SceneLight(index));
    }
    return *m_lights[index];
  }

  const SceneLight &Scene::getLight(int index) const throw (ICLException){
    return const_cast<Scene*>(this)->getLight(index);
  }

    
  void Scene::setLightingEnabled(bool flag){
    m_lightingEnabled = flag;
  }

  inline SceneObject *Scene::getObject(int index) throw (ICLException){
    if(index < 0 || index >= (int)m_objects.size()) throw ICLException("Scene::getObject: invalid index");
    return m_objects[index].get();
  }

  inline const SceneObject *Scene::getObject(int index) const throw (ICLException){
    return const_cast<Scene*>(this)->getObject(index);
  }
  
  SceneObject *find_object_recursive(SceneObject *o, int idx, const std::vector<int> &indices){
    if(idx == (int)indices.size()-1) return o->getChild(indices[idx]);
    else return find_object_recursive(o->getChild(indices[idx]),idx+1,indices);
  }  
  
  SceneObject *Scene::getObject(const std::vector<int> recursiveIndices) throw (ICLException){
    if(!recursiveIndices.size()) throw ICLException("Scene::getObject: recursiveIndices's size was 0");
    if(recursiveIndices.size() == 1) return getObject(recursiveIndices.front());
    SceneObject *found = 0;
    try{
      found = find_object_recursive(getObject(recursiveIndices.front()),1, recursiveIndices);
    }catch(...){
      throw ICLException("Scene::getObject: recursive object index was invalid (object not found)");
    }
    return found;
  }
  
  SceneObject *Scene::getObject(const std::vector<int> recursiveIndices) const throw (ICLException){
    return const_cast<Scene*>(this)->getObject(recursiveIndices);
  }
  

  bool find_path_recursive(const SceneObject *o, std::vector<int> &path, const SceneObject *x){
    for(int i=0;i<o->getChildCount();++i){
      path.push_back(i);
      if(o->getChild(i) == x || find_path_recursive(o->getChild(i),path,x)) return true;
      else path.pop_back();
    }
    return false;
  }

  std::vector<int> Scene::findPath(const SceneObject *o) const throw (ICLException){
    std::vector<int> path;
    for(unsigned int i=0;i<m_objects.size();++i){
      path.push_back(i); 
      if(m_objects[i].get() == o) {
        return path;
      }else{
        if(find_path_recursive(m_objects[i].get(),path,o)){
          return path;
        }else{
          path.pop_back();
        }
      }
    }
    throw ICLException("Scene::findPath: given object not found!");
    return path;
  }



}

