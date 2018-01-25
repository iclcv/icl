/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/Scene.cpp                          **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Erik Weitnauer, Daniel Dornbusch, **
**          Matthias Esau                                          **
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

#include <ICLGeom/Scene.h>
#include <ICLGeom/CoordinateFrameSceneObject.h>
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLGeom/PointCloudCreator.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/File.h>
#include <ICLUtils/PluginRegister.h>

#ifdef ICL_HAVE_QT
#include <ICLQt/Quick.h>
#include <ICLQt/QImageConverter.h>
#include <ICLQt/DrawWidget.h>
#include <ICLQt/GLImg.h>
#include <ICLQt/GUI.h>
#include <ICLQt/ContainerGUIComponents.h>
#include <ICLQt/IconFactory.h>
#include <ICLQt/Application.h>
#include <ICLGeom/ShaderUtil.h>
#include <QtOpenGL/QGLPixelBuffer>
#include <QtOpenGL/QGLContext>
#include <ICLCore/CCFunctions.h>
#endif

#include <set>
#include <vector>
#include <map>


using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl{
  namespace geom{

    static bool creatingDisplayList = false;

    struct CameraObject : public SceneObject{
      Scene *scene;
      int index;
      std::vector<Vec> origVertices;
      float S;
      bool haveName;
      bool isShadowCam;
      Mutex mutex;
      std::string lastName;
      Img8u nameTexture;
      CameraObject(Scene *parent, int cameraIndex, const float camSize, bool isShadowCam = false):
        scene(parent),index(cameraIndex), S(camSize*50), isShadowCam(isShadowCam){

        nameTexture = Img8u(Size(1,1),4);

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

        addTexture(7,8,10,9,&nameTexture);

        origVertices = m_vertices;
      }

      virtual void prepareForRendering() {
        const Camera &cam = isShadowCam? *scene->getLight(index).getShadowCam() : scene->getCamera(index);
        int w = cam.getRenderParams().viewport.width;
        int h = cam.getRenderParams().viewport.height;

        Mat T = cam.getInvCSTransformationMatrix();
        // this has some severe issue in some configurations? Why?
        /** Idea: Yes, we could rotate stuff, but that wont help either!
            CE in oktober '15:
            I hope there is no fundamental issue with the camera stuff????

            ok, I found out that setting a light's norm does not update its up-vector
            which can of course lead to any issues!
            */
        try{
          PlaneEquation p(T*Vec(0,0,S,1), T*Vec(0,0,1,0));
          const Point32f ps[4] = { Point32f(w-1,0), Point32f(0,0), Point32f(w-1,h-1), Point32f(0,h-1) };
          std::vector<ViewRay> vs = cam.getViewRays(std::vector<Point32f>(ps,ps+4));

          for(int i=0;i<4;++i){
            m_vertices[i+3] = vs[i].getIntersection(p);
          }

        }catch(ICLException &e){
          WARNING_LOG("error visualizing camera: " << e.what());
        }

        std::string name = cam.getName();

        if(name != lastName){
          lock();
          delete m_primitives.back();
          m_primitives.pop_back();
          addTextTexture(7,8,10,9,name.length() ? name : str("camera"));
          unlock();
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

    struct icl::geom::Scene::RenderSettings {
      bool useImprovedShading;
      bool lightingEnabled;
      //0 = default, 1 = force off, 2 = force on
      int lineSmoothing;
      //0 = default, 1 = force off, 2 = force on
      int pointSmoothing;
      //0 = default, 1 = force off, 2 = force on
      int polygonSmoothing;
      bool wireframe;
      float shadowBias;
    };

    struct icl::geom::Scene::FBOData {
      struct Glints {
        bool created;
        /// GLuint pointing to the shadowmap texture
        GLuint shadowTexture;
        /// GLuint pointing to the shadowmap FBO
        GLuint shadowFBO;
        Glints():created(false) {
        }
      };
      uint shadow_size;
      uint num_shadows;
      Glints glints;
      FBOData():shadow_size(0),num_shadows(0) {}
      public:
      void freeShadowFBO() {
        if(glints.created) {
          glDeleteFramebuffersEXT(1, &glints.shadowFBO);
          glDeleteTextures(1, &glints.shadowTexture);
          glints.created = false;
        }
        shadow_size = 0;
        num_shadows = 0;
      }

      void createShadowFBO() {
        if(glints.created)freeShadowFBO();
        glints.created = true;
          GLenum FBOstatus;

        //create the shadow texture if simple filtering
          glGenTextures(1, &glints.shadowTexture);
          glBindTexture(GL_TEXTURE_2D, glints.shadowTexture);

          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

          glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
          glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

          glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_size * num_shadows, shadow_size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
          glBindTexture(GL_TEXTURE_2D, 0);

          // create a framebuffer object
          GLint prev_framebuffer;
          glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prev_framebuffer);
          glGenFramebuffers(1, &glints.shadowFBO);
          glBindFramebuffer(GL_FRAMEBUFFER_EXT, glints.shadowFBO);

          glDrawBuffer(GL_NONE);
          glReadBuffer(GL_NONE);

          glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, glints.shadowTexture, 0);

          // check FBO status
          FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
          if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
              throw ICLException("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO");

          // switch back to previous framebuffer
          glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, prev_framebuffer);
      }

      void setShadowFBO(uint size = 512, uint shadows = 1) {
        if(size != shadow_size || num_shadows != shadows) freeShadowFBO();
        shadow_size = size;
        num_shadows = shadows;
      }
    };

  #ifdef ICL_HAVE_QT
    struct Scene::GLCallback : public ICLDrawWidget3D::GLCallback{
      int cameraIndex;
      Scene *parent;
      GUI *gui;
      bool needLink;

      void performLink(ICLDrawWidget *widget){

        typedef std::map<Scene*,std::vector<ICLDrawWidget*> > LinkMap;
        static LinkMap current_links;
        static Mutex mutex;

        Mutex::Locker lock(mutex);
        LinkMap::iterator it = current_links.find(parent);
        if(it != current_links.end()){
          std::vector<ICLDrawWidget*> &ws = it->second;
          if(std::find(ws.begin(),ws.end(), widget) != ws.end()){
            // link between scene and widget is already established!
            return;
          }else{
            ws.push_back(widget);
          }
        }else{
          current_links[parent].push_back(widget);
        }

        const std::string save = parent->getConfigurableID();
        const std::string id = "scene"+str(parent)+str(utils::Time::now().toMicroSeconds());

        parent->setConfigurableID(id);

        if(!gui){
          // create GUI
          gui = new VBox();
          *gui << Prop(id) << Create();
        }

        static ImgQ icon = cvt(IconFactory::create_image("scene-props"));

        widget->addSpecialButton("mousehandler"+str(this),&icon,
                                 utils::function(gui,&GUI::switchVisibility),
                                 "3D scene properties  ");
        parent->setConfigurableID(save);
      }

      GLCallback(int cameraIndex,Scene *parent):
        cameraIndex(cameraIndex),parent(parent),gui(0), needLink(false){}

      virtual void draw(ICLDrawWidget3D *widget){
        if(needLink){
          performLink(widget);
          needLink = false;
        }
        parent->renderScene(cameraIndex, widget);
      }

      Color bgfunc(){
        GeomColor c = parent->getBackgroundColor();
        return Color(c[0],c[1],c[2]);
      }

      virtual void link(ICLDrawWidget3D *widget){
        needLink = true;
        widget->setBackgroundColorSource(utils::function(this,&Scene::GLCallback::bgfunc));
      }

      virtual void unlink(ICLDrawWidget3D *widget){
        TODO_LOG("implement unlink");
        widget->setBackgroundColorSource(ICLWidget::BGColorSource());
        //widget->removeSpecialButton("mousehandler"+str(this));
      }

    };
  #endif


    Scene::Scene():Lockable(true),m_fps(10){

      #ifdef ICL_HAVE_QT
      m_renderSettings = new RenderSettings();
      m_fboData = new FBOData();
      #endif

      m_lights[0] = SmartPtr<SceneLight>(new SceneLight(this,0));
      m_shadowCameraObjects[0] = SmartPtr<SceneObject>(new CameraObject(this,0,1,true));
      m_globalAmbientLight = FixedColVector<int,4>(255,255,255,20);
      m_backgroundColor = GeomColor(0,0,0,255);
      for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 3; j++) {
          m_previousLightState[i][j] = false;
        }
      }

      for(int i = 0; i < ShaderUtil::COUNT; i++) {
        m_shaders[i] = 0;
      }

      m_cursor = SceneObject::sphere(0,0,0,1,10,10);
      m_cursor->setColor(Primitive::all,GeomColor(255,0,255,255),true);
      m_cursor->addChild(new CoordinateFrameSceneObject(5,0.5),true);

      addProperty("visualize cameras","flag","",false);
      addProperty("visualize cursor","flag","",false);
      addProperty("visualize world frame","flag","",false);
      addProperty("visualize object frames","flag","",false);
      addProperty("visualize lights","flag","",false);
      addProperty("enable lighting","flag","",true);
      addProperty("object frame size","float","[0,100000000]",100);
      addProperty("world frame size","float","[0,100000000]",100);
      addProperty("light object size","float","[0,100000000]",30);
      addProperty("background color","color","",Color(0,0,0));
      addProperty("line smoothing","menu","default,force off,force on","default");
      addProperty("point smoothing","menu","default,force off,force on","default");
      addProperty("polygon smoothing","menu","default,force off,force on","default");
      addProperty("wireframe","flag","",false);
      addProperty("shadows.use improved shading","flag","",false);
      addProperty("shadows.cull object front for shadows","flag","",true);
      addProperty("shadows.resolution","menu","64,256,512,1024,2048",512);
      addProperty("shadows.bias","float","[-100,100]",1.0);
      addProperty("info.FPS","info","",0);
      addProperty("info.Objects in the Scene","info","",0);
      addProperty("info.Primitives in the Scene","info","",0);
      addProperty("info.Vertices in the Scene","info","",0);
      addProperty("point cloud grabber cam","range:spinbox","[0,10000]",0);
      addProperty("grab depth feature","flag","",false);
    }
    Scene::~Scene(){
  #ifdef ICL_HAVE_QT
      for(unsigned int i = 0; i < ShaderUtil::COUNT; i++) {
        delete m_shaders[i];
      }
      m_fboData->freeShadowFBO();
      freeAllPBuffers();
      delete m_renderSettings;
      delete m_fboData;
  #endif
    }
    Scene::Scene(const Scene &scene) : Lockable(true){
      *this = scene;
    }

    Scene &Scene::operator=(const Scene &scene){
      m_cameras = scene.m_cameras;
      m_objects.resize(scene.m_objects.size());
      m_backgroundColor = scene.m_backgroundColor;
      for(unsigned int i=0;i<m_objects.size();++i){
        m_objects[i] = scene.m_objects[i]->copy();
      }
  #ifdef ICL_HAVE_QT
      m_mouseHandlers.resize(scene.m_mouseHandlers.size());
      for(unsigned int i=0;i<m_mouseHandlers.size();++i){
        m_mouseHandlers[i] = SmartPtr<SceneMouseHandler>(new SceneMouseHandler( *(scene.m_mouseHandlers[i].get()) ));
        m_mouseHandlers[i]->setParentScene( this );
      }

      m_glCallbacks.resize(scene.m_glCallbacks.size());
      for(unsigned int i=0;i<m_glCallbacks.size();++i){
        m_glCallbacks[i] = SmartPtr<GLCallback>(new GLCallback(scene.m_glCallbacks[i]->cameraIndex,this));
      }
      freeAllPBuffers();

  #endif

      for(unsigned int i=0;i<8;++i){
        if(scene.m_lights[i]){
          m_lights[i] = SmartPtr<SceneLight>(new SceneLight(*scene.m_lights[i]));
          if(m_lights[i]->anchor == SceneLight::ObjectAnchor){
            m_lights[i]->setAnchor(getObject(scene.findPath(m_lights[i]->objectAnchor)));
          }
        }else{
          m_lights[i] = SmartPtr<SceneLight>();
        }
        m_shadowCameraObjects[i] = SmartPtr<SceneObject>(new CameraObject(this,i,1,true));
      }

      if(scene.m_bounds){
        setBounds(scene.m_bounds.get()[0].minVal,
                  scene.m_bounds.get()[0].maxVal,
                  scene.m_bounds.get()[1].minVal,
                  scene.m_bounds.get()[1].maxVal,
                  scene.m_bounds.get()[2].minVal,
                  scene.m_bounds.get()[2].maxVal);
      }else{
        m_bounds = 0;
      }
      m_globalAmbientLight = scene.m_globalAmbientLight;

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
      /**
          TODO: cameras objects with higher indices must be adapted
      */
    }
    Camera &Scene::getCamera(int camIndex){
      return m_cameras[camIndex];
    }
    const Camera &Scene::getCamera(int camIndex) const{
      return m_cameras[camIndex];
    }

    std::vector<Camera*> Scene::getAllCameras(int firstIndex, int num){
      std::vector<Camera*> cams;
      for(int i=firstIndex,j=0; ( i<getCameraCount() ) && ( (num<0) || (j<num) ); ++i,++j){
        cams.push_back(&m_cameras[i]);
      }
      return cams;
    }

    void Scene::addObject(SceneObject *object, bool passOwnerShip){
      m_objects.push_back(SmartPtr<SceneObject>(object,passOwnerShip));
    }

    void Scene::removeObject(int idx){
      ICLASSERT_RETURN(idx >= 0 && idx < (int)m_objects.size());
      m_objects.erase(m_objects.begin()+idx);
    }

    namespace{ struct is_obj{
      const SceneObject *o;
      is_obj(const SceneObject *o):o(o){}
      bool operator()(const SmartPtr<SceneObject> &p) const{
        return p.get() == o;
      }
    }; } // ending anonymos namespace

    void Scene::removeObject(const SceneObject *obj){
      std::vector<SmartPtr<SceneObject> >::iterator it = std::find_if(m_objects.begin(),m_objects.end(),is_obj(obj));
      if(it == m_objects.end()){
        WARNING_LOG("unable to remove given object " << (void*) obj << " from scene: Object not found!");
      }
      m_objects.erase(it);
    }

    void Scene::setGlobalAmbientLight(const GeomColor &color){
      std::copy(color.begin(),color.end(), m_globalAmbientLight.begin());
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
      // #ifdef ICL_HAVE_QT
      // m_mouseHandlers.clear();
      // m_glCallbacks.clear();
      // #endif

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
  #ifdef ICL_HAVE_QT
    void Scene::renderSceneObjectRecursive(ShaderUtil* util, SceneObject *o, int camID) const{
      if(!creatingDisplayList){
        if(o->m_createDisplayListNextTime == 1){
          createDisplayList(o);
          o->m_createDisplayListNextTime = 0;
          return;
        }else if(o->m_createDisplayListNextTime == 2){
          freeDisplayList(o);
          o->m_createDisplayListNextTime = 0;
        }else if(o->m_displayListHandle){
          glCallLists(1,GL_UNSIGNED_INT,o->m_displayListHandle);
          return;
        }
      }

      if(o->getSmoothShading()){
        glShadeModel(GL_SMOOTH);
      }else{
        glShadeModel(GL_FLAT);
      }

      if(m_renderSettings->lineSmoothing == 1) {
        glDisable(GL_LINE_SMOOTH);
      } else if(m_renderSettings->lineSmoothing == 2) {
        glEnable(GL_LINE_SMOOTH);
      }else {
        if(o->m_lineSmoothingEnabled){
          glEnable(GL_LINE_SMOOTH);
        }else{
          glDisable(GL_LINE_SMOOTH);
        }
      }

      if(m_renderSettings->pointSmoothing == 1) {
        glDisable(GL_POINT_SMOOTH);
      } else if(m_renderSettings->pointSmoothing == 2) {
        glEnable(GL_POINT_SMOOTH);
      }else {
        if(o->m_pointSmoothingEnabled){
          glEnable(GL_POINT_SMOOTH);
        }else{
          glDisable(GL_POINT_SMOOTH);
        }
      }

      if(m_renderSettings->polygonSmoothing == 1) {
        glDisable(GL_POLYGON_SMOOTH);
      } else if(m_renderSettings->polygonSmoothing == 2) {
        glEnable(GL_POLYGON_SMOOTH);
      }else {
        if(o->m_polygonSmoothingEnabled){
          glEnable(GL_POLYGON_SMOOTH);
        }else{
          glDisable(GL_POLYGON_SMOOTH);
        }
      }


      if(o->m_depthTestEnabled){
        glEnable(GL_DEPTH_TEST);
      }else{
        glDisable(GL_DEPTH_TEST);
      }


      //check if the custom shader has been set
      bool useCustomShader = o->getFragmentShader();

      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, o->m_specularReflectance.data());

      float shininess[] = { (float)o->m_shininess };
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

      glPointSize(o->m_pointSize);
      glLineWidth(o->m_lineWidth);

      o->prepareForRendering();

      o->lock();

      const std::vector<Vec> &ps = o->m_vertices;
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      const Mat &T = o->getTransformation(true);
      glMultMatrixf(T.transp().data());



      if(o->isVisible() && !o->isInvisibleForCamera(camID) && !o->m_useCustomRender){

        o->complexCustomRender(util);
        if(o->m_primitives.size()){
          const Primitive::RenderContext ctx = { o->m_vertices, o->m_normals, o->m_vertexColors,
                                                 o->m_sharedTextures,
                                                 o->m_lineColorsFromVertices,
                                                 o->m_triangleColorsFromVertices,
                                                 o->m_quadColorsFromVertices,
                                                 o->m_polyColorsFromVertices, o };

          for(unsigned int j=0;j<o->m_primitives.size();++j){
            Primitive *p = o->m_primitives[j];
            if(o->isVisible(p->type)){
              //use the texture shader if the primitive is a texture or text
              if(useCustomShader) {
                o->getFragmentShader()->activate();
              } else if(m_renderSettings->useImprovedShading) {
                util->activateShader(p->type,o->getReceiveShadowsEnabled());
                p->render(ctx);
              } else {
                p->render(ctx);
              }
            }
          }
        }


        if(o->isVisible(Primitive::vertex)){
          GLboolean lightWasOn = true;
          glGetBooleanv(GL_LIGHTING,&lightWasOn);
          glDisable(GL_LIGHTING);

          if(useCustomShader) {
            o->getFragmentShader()->deactivate();
          } else if(m_renderSettings->useImprovedShading) {
            util->deactivateShaders();
          }

          if(creatingDisplayList){
            glBegin(GL_POINTS);
            if(!o->m_vertexColors.size()){
              GeomColor c = o->getDefaultVertexColor();
              glColor4f(c[0]/255,c[1]/255,c[2]/255,c[3]/255);
              for(unsigned int j=0;j<ps.size();++j){
                glVertex3fv(ps[j].data());
              }
            }else{
              for(unsigned int j=0;j<ps.size();++j){
                glColor4fv(((o->m_vertexColors[j])/255.0).data());
                glVertex3fv(ps[j].data());
              }
            }
            glEnd();
          }else{
            if(o->m_vertexColors.size()){
              glEnableClientState(GL_VERTEX_ARRAY);
              glEnableClientState(GL_COLOR_ARRAY);

              glVertexPointer(4,GL_FLOAT,0,o->m_vertices.data());
              glColorPointer(4,GL_FLOAT,0,o->m_vertexColors.data());

              glDrawArrays(GL_POINTS, 0, o->m_vertices.size());

              glDisableClientState(GL_VERTEX_ARRAY);
              glDisableClientState(GL_COLOR_ARRAY);
            }else{
              GeomColor c = o->getDefaultVertexColor();
              glColor4f(c[0]/255,c[1]/255,c[2]/255,c[3]/255);
              glEnableClientState(GL_VERTEX_ARRAY);
              glVertexPointer(4,GL_FLOAT,0,o->m_vertices.data());
              glDrawArrays(GL_POINTS, 0, o->m_vertices.size());
              glDisableClientState(GL_VERTEX_ARRAY);
            }
          }
          if(lightWasOn){
            glEnable(GL_LIGHTING);
          }
        }
      } // is visible
      else if(o->isVisible() && !o->isInvisibleForCamera(camID)){
        o->customRender();
      }
      for(unsigned int i=0;i<o->m_children.size();++i){
        renderSceneObjectRecursive(util, o->m_children[i].get(), camID);
      }
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      if(useCustomShader) {
        o->getFragmentShader()->activate();
      } else if(m_renderSettings->useImprovedShading) {
        util->deactivateShaders();
      }

      o->unlock();
    }

    void Scene::renderSceneObjectRecursiveShadow(ShaderUtil* util, SceneObject *o, int camID) const{
      if(!creatingDisplayList){
        if(o->m_createDisplayListNextTime == 1){
          createDisplayList(o);
          o->m_createDisplayListNextTime = 0;
          return;
        }else if(o->m_createDisplayListNextTime == 2){
          freeDisplayList(o);
          o->m_createDisplayListNextTime = 0;
        }else if(o->m_displayListHandle){
          glCallLists(1,GL_UNSIGNED_INT,o->m_displayListHandle);
          return;
        }
      }


      if(o->getCastShadowsEnabled()){
         o->prepareForRendering();

         o->lock();

         glMatrixMode(GL_MODELVIEW);
         glPushMatrix();
         const Mat &T = o->getTransformation(true);
         glMultMatrixf(T.transp().data());
         if(o->isVisible() && !o->isInvisibleForCamera(camID)){

           o->complexCustomRender(util);
           if(o->m_primitives.size()){
             const Primitive::RenderContext ctx = { o->m_vertices, o->m_normals, o->m_vertexColors,
                                                    o->m_sharedTextures,
                                                    o->m_lineColorsFromVertices,
                                                    o->m_triangleColorsFromVertices,
                                                    o->m_quadColorsFromVertices,
                                                    o->m_polyColorsFromVertices, o };

             for(unsigned int j=0;j<o->m_primitives.size();++j){
               Primitive *p = o->m_primitives[j];
               if(o->isVisible(p->type)){
                 p->render(ctx);
               }
             }
           }
         }
         for(unsigned int i=0;i<o->m_children.size();++i){
           renderSceneObjectRecursiveShadow(util, o->m_children[i].get(), camID);
         }
         glMatrixMode(GL_MODELVIEW);
         glPopMatrix();

         o->unlock();
      }
   }

    static void count_objs_recursive(const SceneObject *o, int &n, int &np, int &nv){
     ++n;
     np += o->getPrimitives().size();
     nv += o->getVertices().size();
     for(int i=0;i<o->getChildCount();++i){
       count_objs_recursive(o->getChild(i),n,np,nv);
     }
   }

   void Scene::renderScene(int camIndex, ICLDrawWidget3D *widget) const{
      glewInit();
      Mutex::Locker l(this);
      //update Sceneinfo
      ((Configurable*)this)->setPropertyValue("info.FPS",m_fps.getFPSString());

      /* this is not thread save!
          int nObjs = 0, nPrim = 0, nVert = 0;
          for(size_t i=0;i<m_objects.size();++i){
          count_objs_recursive(m_objects[i].get(),nObjs,nPrim,nVert);
          }

          ((Configurable*)this)->setPropertyValue("info.Objects in the Scene",nObjs);
          ((Configurable*)this)->setPropertyValue("info.Primitives in the Scene",nPrim);
          ((Configurable*)this)->setPropertyValue("info.Vertices in the Scene",nVert);
      */

      ICLASSERT_RETURN(camIndex >= 0 && camIndex < (int)m_cameras.size());

      Rect currentImageRect = widget ? widget->getImageRect(true) : Rect::null;
      //    Size currentImageSize = widget ? widget->getImageSize(true) : Size::null;
      Size widgetSize = widget ? widget->getSize() : Size::null;

      Camera cam = m_cameras[camIndex];
      if(widget){
        cam.getRenderParams().viewport = currentImageRect;
      }
      m_renderSettings->lightingEnabled = ((Configurable*)this)->getPropertyValue("enable lighting");
      m_renderSettings->useImprovedShading = ((Configurable*)this)->getPropertyValue("shadows.use improved shading");
      m_renderSettings->shadowBias = ((Configurable*)this)->getPropertyValue("shadows.bias");

      string lineSmoothing = ((Configurable*)this)->getPropertyValue("line smoothing");
      if(lineSmoothing == "force off")m_renderSettings->lineSmoothing=1;
      else if(lineSmoothing == "force on")m_renderSettings->lineSmoothing=2;
      else m_renderSettings->lineSmoothing=0;

      string pointSmoothing = ((Configurable*)this)->getPropertyValue("point smoothing");
      if(pointSmoothing == "force off")m_renderSettings->pointSmoothing=1;
      else if(pointSmoothing == "force on")m_renderSettings->pointSmoothing=2;
      else m_renderSettings->pointSmoothing=0;

      string polygonSmoothing = ((Configurable*)this)->getPropertyValue("polygon smoothing");
      if(polygonSmoothing == "force off")m_renderSettings->polygonSmoothing=1;
      else if(polygonSmoothing == "force on")m_renderSettings->polygonSmoothing=2;
      else m_renderSettings->polygonSmoothing=0;

      m_renderSettings->wireframe = ((Configurable*)this)->getPropertyValue("wireframe");

      vector<Mat> project2shadow;

      if(m_renderSettings->lightingEnabled && m_renderSettings->useImprovedShading) {

        //update cameras and check if the lightsetup has changed
        bool lightSetupChanged = false;
        int numShadowLights = 0;
        for(int i=0;i<8;++i){
          if(m_lights[i]) {
            m_lights[i]->updatePositions(*this,getCamera(camIndex));
            if(m_lights[i]->on != m_previousLightState[i][0]) {
              lightSetupChanged = true;
            }
            m_previousLightState[i][0] = m_lights[i]->on;

            if(m_lights[i]->getShadowEnabled() != m_previousLightState[i][1]) {
              lightSetupChanged = true;
            }
            m_previousLightState[i][1] = m_lights[i]->getShadowEnabled();

            if(m_lights[i]->getTwoSidedEnabled() != m_previousLightState[i][2]) {
              lightSetupChanged = true;
            }
            m_previousLightState[i][2] = m_lights[i]->getTwoSidedEnabled();

            if(m_lights[i]->getProjectionEnabled() != m_previousLightState[i][3]) {
              lightSetupChanged = true;
            }
            m_previousLightState[i][3] = m_lights[i]->getProjectionEnabled();

            if(m_lights[i]->getShadowEnabled()) {
              numShadowLights++;
            }
          }
        }

        if(lightSetupChanged) {
          ShaderUtil::recompilePerPixelShader(m_shaders,m_lights,numShadowLights);
          m_shaders[0]->activate();
          m_shaders[0]->deactivate();
        }

        //bind the projection textures
        for(int i = 0; i <8;i++) {
          if(m_lights[i] && m_lights[i]->getShadowEnabled() && m_lights[i]->getProjectionEnabled() && m_lights[i]->getProjectionImage()) {
            GLImg *pi = m_lights[i]->getProjectionImage();
            pi->lock();
            pi->bind(0,0,8+i);
            pi->unlock();
          }
        }

        //recreate the shadowbuffer if the the lightsetup, or the resolution has changed
        unsigned int resolution = ((Configurable*)this)->getPropertyValue("shadows.resolution");
        if(m_fboData->shadow_size != resolution || lightSetupChanged) {
          m_fboData->setShadowFBO(resolution,numShadowLights);
        }

        //bind texture if it has been created
        if(m_fboData->shadow_size>0) {
         glActiveTextureARB(GL_TEXTURE7);
         glBindTexture(GL_TEXTURE_2D,m_fboData->glints.shadowTexture);
        }

        //render the shadows and create the projection matrices for the vertex shader
        int currentShadow = 0;
        for(int i = 0; i < 8; i++) {
          if(m_lights[i]) {
            if(m_lights[i]->on && m_lights[i]->getShadowEnabled()) {
              renderShadow(i, currentShadow++, m_fboData->shadow_size, camIndex);
              project2shadow.push_back(m_lights[i]->getShadowCam()->getProjectionMatrixGL()
                             * m_lights[i]->getShadowCam()->getCSTransformationMatrixGL()
                             * cam.getCSTransformationMatrixGL().inv());
            }
          }
        }

      }else {
        m_fboData->freeShadowFBO();
        if(m_renderSettings->lightingEnabled) {
          for(int i=0;i<8;++i){
            if(m_lights[i] && m_lights[i]->on) {
              m_lights[i]->updatePositions(*this,getCamera(camIndex));
            }
          }
        }
      }


      glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_NORMALIZE);

      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(GLMatrix(cam.getCSTransformationMatrixGL()));

      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(GLMatrix(cam.getProjectionMatrixGL()));

      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
      // specular lighting is still not working ..
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);

      if(m_renderSettings->wireframe)glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      else glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

      if(m_renderSettings->lightingEnabled){
        float size = ((Configurable*)this)->getPropertyValue("light object size");
        glEnable(GL_LIGHTING);
        for(int i=0;i<8;++i){
          if(m_lights[i]) {
            ((SceneLight*)m_lights[i].get())->setObjectSize(size);
            m_lights[i]->setupGL(*this,getCamera(camIndex));
          }else{
            glDisable(GL_LIGHT0+i);
          }
        }
      }else{
        glDisable(GL_LIGHTING);
      }

      glLightModeliv(GL_LIGHT_MODEL_AMBIENT, m_globalAmbientLight.begin());

      if(widget){
        if (widget->getFitMode() == ICLWidget::fmZoom) {
          // transforms y in case of having zoom activated
          float dy = (currentImageRect.height-widgetSize.height);
          glViewport(currentImageRect.x,-dy-currentImageRect.y,currentImageRect.width,currentImageRect.height);
        } else {
          glViewport(currentImageRect.x,currentImageRect.y,currentImageRect.width,currentImageRect.height);
        }
      }else{
        const Size &s = cam.getRenderParams().chipSize;
        glViewport(0,0,s.width,s.height);
      }

      glEnable(GL_DEPTH_TEST);

      if(m_renderSettings->lightingEnabled && m_renderSettings->useImprovedShading){
        ShaderUtil util(&cam, m_shaders, &project2shadow, m_renderSettings->shadowBias);
        for(size_t i=0;i<m_objects.size();++i){
          renderSceneObjectRecursive(&util, (SceneObject*)m_objects[i].get(), camIndex);
        }
      } else {
        for(size_t i=0;i<m_objects.size();++i){
          renderSceneObjectRecursive((SceneObject*)m_objects[i].get(), camIndex);
        }
      }

      if(((Configurable*)this)->getPropertyValue("visualize cameras")){
        for(unsigned int i=0;i<m_cameraObjects.size();++i){
          if((int)i == camIndex) continue;
          renderSceneObjectRecursive((SceneObject*)m_cameraObjects[i].get(), camIndex);
        }
      }

      glPushAttrib(GL_ENABLE_BIT);

      if(m_renderSettings->lightingEnabled && (bool)(((Configurable*)this)->getPropertyValue("visualize lights"))){
        for(int i=0;i<8;++i){
          if(m_lights[i] && m_lights[i]->on){
            if((m_lights[i]->anchor != SceneLight::CamAnchor) ||
               (m_lights[i]->camAnchor != camIndex && m_lights[i]->camAnchor != -1)){
              m_lights[i]->updatePositions(*this,getCamera(camIndex));
              renderSceneObjectRecursive((SceneObject*)m_lights[i]->getLightObject(), camIndex);
              if(m_lights[i]->getShadowEnabled()) {
                renderSceneObjectRecursive((SceneObject*)m_shadowCameraObjects[i].get(), camIndex);
              }
            }
          }
        }
      }

      glPopAttrib();

      if(getDrawObjectFramesEnabled()){
        float size = ((Configurable*)this)->getPropertyValue("object frame size");
        if(!m_objectFrameObject){
          m_objectFrameObject = new ComplexCoordinateFrameSceneObject(size,size/20);
          //m_objectFrameObject->createDisplayList();
        }else{
          float currSize = ((ComplexCoordinateFrameSceneObject*)m_objectFrameObject.get())->getAxisLength();
          if(size != currSize){
            m_objectFrameObject = new ComplexCoordinateFrameSceneObject(size,size/20);
            //m_objectFrameObject->createDisplayList();
          }
        }
        for(size_t i=0;i<m_objects.size();++i){
          renderObjectFramesRecursive((SceneObject*)m_objects[i].get(), (SceneObject*)m_objectFrameObject.get(), camIndex);
        }
      }

      if(getDrawCoordinateFrameEnabled()){
        float size = ((Configurable*)this)->getPropertyValue("world frame size");
        if(!m_coordinateFrameObject){
          m_coordinateFrameObject = new ComplexCoordinateFrameSceneObject(size,size/20);
          //m_coordinateFrameObject->createDisplayList();
        }else{
          float currSize = ((ComplexCoordinateFrameSceneObject*)m_coordinateFrameObject.get())->getAxisLength();
          if(size != currSize){
            m_coordinateFrameObject = new ComplexCoordinateFrameSceneObject(size,size/20);
            //m_coordinateFrameObject->createDisplayList();
          }
        }
        int minumum_ambience[] = {
          iclMax(m_globalAmbientLight[0],250),
          iclMax(m_globalAmbientLight[1],250),
          iclMax(m_globalAmbientLight[2],250),
          iclMax(m_globalAmbientLight[3],250),
        };
        glLightModeliv(GL_LIGHT_MODEL_AMBIENT, minumum_ambience);
        renderSceneObjectRecursive((SceneObject*)m_coordinateFrameObject.get(), camIndex);
      }

      if(((Configurable*)this)->getPropertyValue("visualize cursor")){
        renderSceneObjectRecursive(m_cursor, camIndex);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        m_cursor->setDepthTestEnabled(false);
        renderSceneObjectRecursive(m_cursor, camIndex);
        m_cursor->setDepthTestEnabled(true);
        if(!m_renderSettings->wireframe)glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      }

    }

    void Scene::renderShadow(const unsigned int light, const unsigned int shadow, unsigned int size, int camID) const{
      FBOData::Glints &glints = m_fboData->glints;
      if(!glints.created)m_fboData->createShadowFBO();
      GLint prev_framebuffer;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prev_framebuffer);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, glints.shadowFBO);

      //Using the fixed pipeline
      glUseProgram(0);

      // Set the viewport to the slot in the buffer
      glViewport(size * shadow,0,size,size);

      // Clear previous frame values
      glScissor(size * shadow,0,size,size);
      glEnable(GL_SCISSOR_TEST);
      glClear(GL_DEPTH_BUFFER_BIT);

      //Disable color rendering, we only want to write to the Z-Buffer
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(GLMatrix(m_lights[light]->getShadowCam()->getCSTransformationMatrixGL()));

      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(GLMatrix(m_lights[light]->getShadowCam()->getProjectionMatrixGL()));

      glEnable(GL_DEPTH_TEST);

      glEnable(GL_CULL_FACE);

      bool cullFront = ((Configurable*)this)->getPropertyValue("shadows.cull object front for shadows");
        if(cullFront) {
          glCullFace(GL_FRONT);
      } else {
          glCullFace(GL_BACK);
      }

        ShaderUtil util;
      for(size_t i=0;i<m_objects.size();++i){
        renderSceneObjectRecursiveShadow(&util, (SceneObject*)m_objects[i].get(), camID);
      }
        // enable the default framebuffer
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, prev_framebuffer);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
    }

    void Scene::renderObjectFramesRecursive(SceneObject *o, SceneObject *cs, int camID) const{
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      const Mat &T = o->getTransformation(true);
      glMultMatrixf(T.transp().data());

      renderSceneObjectRecursive(cs, camID);

      for(unsigned int i=0;i<o->m_children.size();++i){
        renderObjectFramesRecursive(o->m_children[i].get(),cs, camID);
      }

      glPopMatrix();

    }

    void Scene::setDrawLightsEnabled(bool enabled, float lightSize){
      setPropertyValue("visualize lights",enabled);
      setPropertyValue("light object size",lightSize);
    }

    bool Scene::getDrawLightsEnabled() const {
      return ((Configurable*)this)->getPropertyValue("visualize lights");
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
      if(m_bounds){
        float maxLen = iclMax(m_bounds.get()[0].getLength(), m_bounds.get()[1].getLength());
        maxLen = iclMax(m_bounds.get()[2].getLength(), maxLen);
        newSceneMouseHandler->setSensitivities(maxLen);
			} else {
        newSceneMouseHandler->setSensitivities(getMaxSceneDim());
      }
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

    void Scene::setDrawCamerasEnabled(bool enabled){
      setPropertyValue("visualize cameras",enabled);
    }

    void Scene::setDrawCoordinateFrameEnabled(bool enabled, float size){
      setPropertyValue("visualize world frame",enabled);
      setPropertyValue("world frame size",size);
    }

    bool Scene::getDrawCamerasEnabled() const{
      return ((Configurable*)this)->getPropertyValue("visualize cameras");
    }

    bool Scene::getDrawCoordinateFrameEnabled() const{
      return ((Configurable*)this)->getPropertyValue("visualize world frame");
    }

    void Scene::setBackgroundColor(const GeomColor &color){
      setPropertyValue("background color",Color(color[0],color[1],color[2]));
    }

    GeomColor Scene::getBackgroundColor() const{
      Color c = ((Configurable*)this)->getPropertyValue("background color");
      return GeomColor(c[0],c[1],c[2],255);
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
      Range32f orig[3] = {rangeXYZ[0], rangeXYZ[1], rangeXYZ[2] };

      for(int i=0;i<3;++i){
        std::swap(rangeXYZ[i].minVal,rangeXYZ[i].maxVal);
      }
      for(unsigned int i=0;i<m_objects.size();++i){
        extendMaxSceneDimRecursive(rangeXYZ[0].minVal,rangeXYZ[0].maxVal,
                                   rangeXYZ[1].minVal,rangeXYZ[1].maxVal,
                                   rangeXYZ[2].minVal,rangeXYZ[2].maxVal,
                                   const_cast<SceneObject*>(m_objects[i].get()));
      }
      if(getDrawCamerasEnabled()){
        for(unsigned i=0;i<m_cameraObjects.size();++i){
          extendMaxSceneDimRecursive(rangeXYZ[0].minVal,rangeXYZ[0].maxVal,
                                     rangeXYZ[1].minVal,rangeXYZ[1].maxVal,
                                     rangeXYZ[2].minVal,rangeXYZ[2].maxVal,
                                     const_cast<SceneObject*>(m_cameraObjects[i].get()));
        }
      }
      if(getDrawCoordinateFrameEnabled()){
        extendMaxSceneDimRecursive(rangeXYZ[0].minVal,rangeXYZ[0].maxVal,
                                   rangeXYZ[1].minVal,rangeXYZ[1].maxVal,
                                   rangeXYZ[2].minVal,rangeXYZ[2].maxVal,
                                   const_cast<SceneObject*>(m_coordinateFrameObject.get()));
      }
      /// special case: no objects contained at all (scene dim is assumed to be O(1m)
      if(rangeXYZ[0] == orig[0] && rangeXYZ[1] == orig[1] && rangeXYZ[2] == orig[2]){
        return 1000;
      }

      float r = iclMax(iclMax(rangeXYZ[1].getLength(),rangeXYZ[2].getLength()),rangeXYZ[0].getLength());
      return r ? r : 1;
    }

    SceneLight &Scene::getLight(int index) throw (ICLException){
      if(index < 0 || index > 7) throw ICLException("invalid light index");
      if(!m_lights[index]){
        m_lights[index] = SmartPtr<SceneLight>(new SceneLight(this,index));
        m_shadowCameraObjects[index] = SmartPtr<SceneObject>(new CameraObject(this,index,1,true));
      }
      return *m_lights[index];
    }

    const SceneLight &Scene::getLight(int index) const throw (ICLException){
      return const_cast<Scene*>(this)->getLight(index);
    }


    void Scene::setLightingEnabled(bool flag){
      setPropertyValue("enable lighting",flag);
    }

    void Scene::setDrawObjectFramesEnabled(bool enabled, float size){
      setPropertyValue("visualize object frames",enabled);
      setPropertyValue("object frame size",size);
    }

    bool Scene::getDrawObjectFramesEnabled() const{
      return ((Configurable*)this)->getPropertyValue("visualize object frames");
    }

    void Scene::setCursor(Vec newPosition) {
        Mat4 trans = m_cursor->getTransformation();
        trans(3,0) = newPosition(0,0);
        trans(3,1) = newPosition(0,1);
        trans(3,2) = newPosition(0,2);
        m_cursor->setTransformation(trans);
    }
    Vec Scene::getCursor() {
        Mat4 trans = m_cursor->getTransformation();
        return Vec(trans(3,0),trans(3,1),trans(3,2));
    }

    void Scene::activateCursor(bool activate) {
        ((Configurable*)this)->setPropertyValue("visualize cursor",activate);
    }


    SceneObject *Scene::getObject(int index) throw (ICLException){
      if(index < 0 || index >= (int)m_objects.size()) throw ICLException("Scene::getObject: invalid index");
      return m_objects[index].get();
    }

    const SceneObject *Scene::getObject(int index) const throw (ICLException){
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


    Hit Scene::findObject(const ViewRay &v){
      std::vector<Hit> hits;
      Hit h;
      for(unsigned int i=0;i<m_objects.size();++i){
        if( (h=m_objects[i]->hit(v) ) ){
          hits.push_back(h);
        }
      }
      return hits.size() ? *std::min_element(hits.begin(),hits.end()) : Hit();
    }

    /// retunrs all objects intersected by the given viewray
    std::vector<Hit> Scene::findObjects(const ViewRay &v){
      std::vector<Hit> hits;
      for(unsigned int i=0;i<m_objects.size();++i){
        std::vector<Hit> ohits = m_objects[i]->hits(v);
        std::copy(ohits.begin(),ohits.end(),std::back_inserter(hits));
      }
      std::sort(hits.begin(),hits.end());
      return hits;
    }

    void Scene::freeDisplayList(void *handle){
      glDeleteLists(*(GLuint*)handle,1);
      delete (GLuint*)handle;
    }


    void Scene::createDisplayList(SceneObject *o) const{
      if(!o->m_displayListHandle){
        o->m_displayListHandle = new GLuint(0);
        *(GLuint*)o->m_displayListHandle = glGenLists(1);
      }
      creatingDisplayList = true;
      glNewList(*(GLuint*)o->m_displayListHandle, GL_COMPILE_AND_EXECUTE);
      renderSceneObjectRecursive(o, -1);
      glEndList();
      creatingDisplayList = false;
    }

    void Scene::freeDisplayList(SceneObject *o) const{
      if(o->m_displayListHandle){
        freeDisplayList(o->m_displayListHandle);
        o->m_displayListHandle = 0;
      }
    }

    void Scene::setBounds(float minX, float maxX, float minY, float maxY, float minZ, float maxZ){
      m_bounds = SmartArray<Range32f>(new Range32f[3]);
      if(minX == maxX){
        maxX = -minX;
        if(minX == maxX){
          m_bounds = 0;
          return;
        }
      }
      if(minX > maxX){
        m_bounds.get()[0] = Range32f(maxX,minX);
      }else{
        m_bounds.get()[0] = Range32f(minX,maxX);
      }
      if(minY == maxY){
        m_bounds.get()[1] = Range32f(minX,maxX);
      }else{
        m_bounds.get()[1] = Range32f(minY,maxY);
      }
      if(minZ == maxZ){
        m_bounds.get()[2] = Range32f(minX,maxX);
      }else{
        m_bounds.get()[2] = Range32f(minZ,maxZ);
      }
    }

#ifdef ICL_HAVE_QT
    struct Scene::PBuffer{
      QGLPixelBuffer m_buffer;
      QImageConverter conv;
      SmartPtr<GLImg> background;
      PBuffer(Size s):m_buffer(s.width,s.height, QGLFormat::defaultFormat(), ICLApplication::instance()->sharedWidget){}
      void update(const Camera &cam) {
        depthCorr.update(cam);
      }

      const Img8u &getImage() {
        QImage img = m_buffer.toImage();
        conv.setQImage(&img);
        return *conv.getImgBase()->as8u();
      }


      struct DepthCorrection{
        std::vector<icl32f> factors;
        Size resolution;
        icl32f fX, fY, skew;
        Point32f ppOffs;

        static inline float compute_depth_norm(const Vec &dir, const Vec &centerDir){
          return sprod3(dir, centerDir) / (norm3(dir)*norm3(centerDir));
        }

        void update(const Camera &cam){
          const Camera::RenderParams &p = cam.getRenderParams();
          const float f = cam.getFocalLength();
          const int w = p.viewport.width, h = p.viewport.height;
          if (!factors.size() ||
            resolution != Size(w, h) ||
            fX != f*cam.getSamplingResolutionX() ||
            fY != f*cam.getSamplingResolutionY() ||
            skew != cam.getSkew() ||
            ppOffs != cam.getPrincipalPointOffset()){

            resolution = Size(w, h);
            fX = f*cam.getSamplingResolutionX();
            fY = f*cam.getSamplingResolutionY();
            skew = cam.getSkew();
            ppOffs = cam.getPrincipalPointOffset();

            factors.resize(w*h);

            Array2D<ViewRay> vs = cam.getAllViewRays();

            const Vec c = vs(w / 2 - 1, h / 2 - 1).direction;

            for (int idx = 0; idx<w*h; ++idx){
              factors[idx] = 1.0 / compute_depth_norm(vs[idx].direction, c);
            }
          }
        }
      }depthCorr;

    };

    Scene::PBufferIndex::PBufferIndex(const Size &size) :
      Size(size), threadID(pthread_self()){}

    bool Scene::PBufferIndex::operator<(const Scene::PBufferIndex &other) const{
	#ifdef WIN32
      if (other.threadID.p == threadID.p){
    #else
      if (other.threadID == threadID){
	#endif
        if (other.width == width) return other.height < height;
        else return other.width < width;
      }
	#ifdef WIN32
      else return other.threadID.p < threadID.p;
    #else
      else return other.threadID < threadID;
	#endif
    }

    void Scene::freeAllPBuffers(){
      struct DeleteEvent : public ICLApplication::AsynchronousEvent{
        std::map<PBufferIndex, PBuffer*> pbuffers;
        DeleteEvent(std::map<PBufferIndex, PBuffer*> pbuffers):pbuffers(pbuffers) {}

        void execute() {
          typedef std::map<PBufferIndex, PBuffer*>::iterator It;
          for (It it = pbuffers.begin(); it != pbuffers.end(); ++it){
            delete it->second;
          }
        }
      };
      ICLApplication *app = ICLApplication::instance();
      if(app)app->executeInGUIThread(new DeleteEvent(m_pbuffers),false);
    }

    void Scene::freePBuffer(const Size &size){
      typedef std::map<PBufferIndex, PBuffer*>::iterator It;
      PBufferIndex idx(size);
      It it = m_pbuffers.find(idx);
      if (it != m_pbuffers.end()){
        delete it->second;
        m_pbuffers.erase(it);
      }
    }

    void Scene::grab(PointCloudObjectBase &dst){
      int camID = getPropertyValue("point cloud grabber cam");
      bool withDepth = getPropertyValue("grab depth feature");
      if((int)m_cameras.size() <= camID) {
        ERROR_LOG("invalid camera id");
        return;
      }
      const Camera &c = m_cameras[camID];
      Img32f depthBuffer(c.getRenderParams().chipSize,1);
      const Img8u &image = render(camID,0,&depthBuffer,DistToCamPlane);
      PointCloudCreator pcc(c,c);
      pcc.create(depthBuffer,dst,&image,1,withDepth);
    }

    const Img8u &Scene::render(int camIndex, const ImgBase *background, Img32f *depthBuffer,
      DepthBufferMode mode, bool fastRendering) {
      struct RenderEvent : public ICLApplication::AsynchronousEvent{
        const Scene *scene;
        std::map<PBufferIndex, PBuffer*> *pbufferMap;
        int camIndex;
        const ImgBase *background;
        Img32f *depthBuffer;
        DepthBufferMode mode;
        Img8u const** out;
        bool fastRendering;
        RenderEvent(const Scene *scene, std::map<PBufferIndex, PBuffer*> *pbufferMap, int camIndex, const ImgBase *background, Img32f *depthBuffer, DepthBufferMode mode, Img8u const** out, bool fastRendering) :
          scene(scene),
          pbufferMap(pbufferMap),
          camIndex(camIndex),
          background(background),
          depthBuffer(depthBuffer),
          mode(mode),
          out(out),
          fastRendering(fastRendering){}

        virtual void execute(){
          scene->lock();
          const Camera &cam = scene->getCamera(camIndex);
          int w = cam.getResolution().width;
          int h = cam.getResolution().height;
          Size s(w, h);
          PBufferIndex idx(s);
          std::map<PBufferIndex, PBuffer*>::iterator it = pbufferMap->find(idx);
          PBuffer *pbuffer = (it == pbufferMap->end()) ? ((*pbufferMap)[idx] = new PBuffer(s)) : it->second;
          if(!fastRendering){
            pbuffer->update(cam);
          }
          pbuffer->m_buffer.makeCurrent();
          GeomColor c = scene->getBackgroundColor();
          glClearColor(c[0] / 255., c[1] / 255., c[2] / 255., 1);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glEnable(GL_TEXTURE_2D);
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          glEnable(GL_COLOR_MATERIAL);

          if (background){
            glOrtho(0, w, h, 0, -999999, 999999);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glDisable(GL_LIGHTING);
            SmartPtr<GLImg> &bg = pbuffer->background;
            if (!bg) bg = SmartPtr<GLImg>(new GLImg);
            bg->update(background);
            bg->draw2D(Rect(0, 0, w, h), s);
            glEnable(GL_LIGHTING);
            glClear(GL_DEPTH_BUFFER_BIT);
          }

          scene->renderScene(camIndex);
          *out = &pbuffer->getImage();
          if (depthBuffer){
            depthBuffer->setSize(Size(w, h));
            depthBuffer->setChannels(1);
            glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer->begin(0));
            depthBuffer->mirror(axisHorz);

            if (mode != RawDepth01){
              const float zNear = cam.getRenderParams().clipZNear;
              const float zFar = cam.getRenderParams().clipZFar;

              icl32f *db = depthBuffer->begin(0);

              const int dim = w*h;
              const float Q = zFar / (zFar - zNear);
              const float izFar = 1.0 / zFar;

              const float m = zFar - zNear;
              const float b = zNear;

              const float A = izFar * m;

              if (mode == DistToCamCenter){
                pbuffer->depthCorr.update(cam);
                const float *corr = pbuffer->depthCorr.factors.data();
                for (int i = 0; i < dim; ++i){
                  db[i] = corr[i] * (A / (Q - db[i]) + b) - 1;
                }
              }
              else{
                for (int i = 0; i < dim; ++i){
                  db[i] = (A / (Q - db[i]) + b) - 1;
                }
              }
            }
          }
          pbuffer->m_buffer.doneCurrent();
          scene->unlock();
        }
      };
      Img8u const* img = 0;
      ICLApplication *app = ICLApplication::instance();
      if(app)app->executeInGUIThread(new RenderEvent(this, &m_pbuffers, camIndex, background, depthBuffer, mode, &img,fastRendering), true);
      return *img;
    }
#endif //QT

    REGISTER_CONFIGURABLE(Scene, return new Scene);


    static PointCloudGrabber *create_scene_point_cloud_grabber(const std::map<std::string,std::string> &d){
      std::map<std::string,std::string>::const_iterator it = d.find("creation-string");
      if(it == d.end()) return 0;
      const std::string &params = it->second;
      std::vector<std::string> ts = tok(params,",");

      Scene *scene = new Scene;
      if(ts.size() == 1 || ts.size() == 2){
        //scene->addCamera(Camera());
        SceneObject *obj = 0;
        if(ts[0] == "cube"){
          obj = SceneObject::cube(0,0,0,2);
          obj->rotate(1,2,3);
        }else if(ts[0] == "sphere"){
          obj = SceneObject::sphere(0,0,0,2,30,30);
        }else{
          if(utils::File(ts[0]).exists()){
            obj = new SceneObject(ts[0]);
          }else{
            ERROR_LOG("file not found: " << ts[0]);
            return 0;
          }
        }
        scene->addObject(obj);
        if(ts.size() == 2){
          scene->addCamera(Camera(ts[1]));
        }else{
          scene->addCamera(Camera());
        }
        return scene;
      }
      ERROR_LOG("invalid parameters: " << params);
      return 0;
    }

    REGISTER_PLUGIN(PointCloudGrabber,scene,create_scene_point_cloud_grabber,
                    "Simulation based Grabber that renders a point cloud in a virtual scene",
                    "creation-string: {shape|object-file}[:camera-file] (shape can be sphere or cube)");

  } // namespace geom
}

