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


#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <ICLGeom/Scene.h>
#include <ICLGeom/CoordinateFrameSceneObject.h>
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>
#ifdef HAVE_QT
#include <ICLQt/DrawWidget.h>
#include <ICLQt/GLImg.h>
#include <ICLQt/GUI.h>
#include <ICLQt/ContainerGUIComponents.h>
#include <ICLQt/IconFactory.h>
#endif

#ifdef HAVE_GLX
#include <GL/glx.h>
#include <ICLCore/CCFunctions.h>
#include <ICLQt/GLContext.h>
#endif

#include <ICLQt/Quick.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/StringUtils.h>

#include <set>
#include <ICLUtils/Time.h>
#include <ICLQt/Quick.h>

#include <vector>


using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl{
  namespace geom{
  
    static bool creatingDisplayList = false;
  
    struct CameraObject : public SceneObject{
      Scene *scene;
      int cameraIndex;
      std::vector<Vec> origVertices;
      float S;
      bool haveName;
      Mutex mutex;
      std::string lastName;
      Img8u nameTexture;
  
      CameraObject(Scene *parent, int cameraIndex, float camSize):
        scene(parent),cameraIndex(cameraIndex), nameTexture(Size(1,1),4){
  
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
  
        addTexture(7,8,10,9,&nameTexture);
  
        origVertices = m_vertices;
      }
  
      virtual void prepareForRendering() {
        const Camera &cam = scene->getCamera(cameraIndex);
        int w = cam.getRenderParams().viewport.width;
        int h = cam.getRenderParams().viewport.height;
  
        Mat T = cam.getCSTransformationMatrix().inv();
  #if 1
  
        try{
          PlaneEquation p(T*Vec(0,0,S,1),T*Vec(0,0,1,1)-cam.getPosition());
          const Point32f ps[4] = { Point32f(w-1,0), Point32f(0,0), Point32f(w-1,h-1), Point32f(0,h-1) };
          std::vector<ViewRay> vs = cam.getViewRays(std::vector<Point32f>(ps,ps+4));
  
          for(int i=0;i<4;++i){
            m_vertices[i+3] = vs[i].getIntersection(p);
          }
  
        }catch(ICLException &e){
          WARNING_LOG("error visualizsing camera: " << e.what());
        }
  
  #else
        // old version with same issue
        try{
          PlaneEquation p(T*Vec(0,0,S,1),T*Vec(0,0,1,1)-cam.getPosition());
          SHOW(p.normal.transp());
          SHOW(cam.getViewRay(Point32f(w-1,0)).direction.transp());
          SHOW(cam.getViewRay(Point32f(0,0)).direction.transp());
          SHOW(cam.getViewRay(Point32f(0,h-1)).direction.transp());
          SHOW(cam.getViewRay(Point32f(w-1,h-1)).direction.transp());
  
          m_vertices[3] = cam.getViewRay(Point32f(w-1,0)).getIntersection(p);
          m_vertices[4] = cam.getViewRay(Point32f(0,0)).getIntersection(p);
          m_vertices[5] = cam.getViewRay(Point32f(w-1,h-1)).getIntersection(p);
          m_vertices[6] = cam.getViewRay(Point32f(0,h-1)).getIntersection(p);
        }catch(ICLException &e){
          WARNING_LOG("error visualizsing camera: " << e.what());
        }
  #endif
        std::string name = cam.getName();
  
        if(name != lastName){
          lock();
          Img8u newTexture = TextPrimitive::create_texture(name.length() ? name : str("camera"),GeomColor(255,255,255,255),30);
          dynamic_cast<TexturePrimitive*>(m_primitives.back())->texture.update(&newTexture);
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
  
  
  #ifdef HAVE_QT
    struct Scene::GLCallback : public ICLDrawWidget3D::GLCallback{
      int cameraIndex;
      Scene *parent;
      GUI *gui;
      bool needLink;

      void performLink(ICLDrawWidget *widget){
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
                                 function(gui,&GUI::switchVisibility),
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
        widget->setBackgroundColorSource(function(this,&Scene::GLCallback::bgfunc));
      }
      
      virtual void unlink(ICLDrawWidget3D *widget){
        TODO_LOG("implement unlink");
        widget->setBackgroundColorSource(ICLWidget::BGColorSource());
        //widget->removeSpecialButton("mousehandler"+str(this));      
      }
      
    };
  #endif
  
   
    Scene::Scene(){
      m_lights[0] = SmartPtr<SceneLight>(new SceneLight(this,0));
      m_globalAmbientLight = FixedColVector<int,4>(255,255,255,20);
      m_backgroundColor = GeomColor(0,0,0,255);
      
      addProperty("visualize cameras","flag","",false);
      addProperty("visualize world frame","flag","",false);
      addProperty("visualize object frames","flag","",false);
      addProperty("visualize lights","flag","",false);
      addProperty("enable lighting","flag","",true);
      addProperty("object frame size","float","[0,100000000]",100);
      addProperty("world frame size","float","[0,100000000]",100);
      addProperty("light object size","float","[0,100000000]",30);
      addProperty("background color","color","",Color(0,0,0));
      addProperty("shadows.use improved shading","flag","",false);
      addProperty("shadows.cull object front for shadows","flag","",true);
      addProperty("shadows.shadow resolution","menu","64,256,512,1024,2048",512);
      addProperty("shadows.shadow bias","float","[-0.1,0.1]",0.0001);
    }
    Scene::~Scene(){
  #ifdef HAVE_GLX
  #ifdef HAVE_QT
      freeAllPBuffers();
      delete m_perPixelShader;
      delete m_perPixelShaderTexture;
      delete m_perPixelShaderNoShadow;
      delete m_perPixelShaderTextureNoShadow;
      freeShadowFBO();
  #endif
  #endif
    }
    Scene::Scene(const Scene &scene){
      *this = scene;
    }
    
    Scene &Scene::operator=(const Scene &scene){
      m_cameras = scene.m_cameras;
      m_objects.resize(scene.m_objects.size());
      m_backgroundColor = scene.m_backgroundColor;
      for(unsigned int i=0;i<m_objects.size();++i){
        m_objects[i] = scene.m_objects[i]->copy();
      }
  #ifdef HAVE_QT
      m_mouseHandlers.resize(scene.m_mouseHandlers.size());
      for(unsigned int i=0;i<m_mouseHandlers.size();++i){
        m_mouseHandlers[i] = SmartPtr<SceneMouseHandler>(new SceneMouseHandler( *(scene.m_mouseHandlers[i].get()) ));
        m_mouseHandlers[i]->setParentScene( this );
      }
  
      m_glCallbacks.resize(scene.m_glCallbacks.size());
      for(unsigned int i=0;i<m_glCallbacks.size();++i){
        m_glCallbacks[i] = SmartPtr<GLCallback>(new GLCallback(scene.m_glCallbacks[i]->cameraIndex,this));
      }
  #ifdef HAVE_GLX
      freeAllPBuffers();
  #endif
      
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
      }
      
      if(scene.m_bounds){
        setBounds(scene.m_bounds[0].minVal,
                  scene.m_bounds[0].maxVal,
                  scene.m_bounds[1].minVal,
                  scene.m_bounds[1].maxVal,
                  scene.m_bounds[2].minVal,
                  scene.m_bounds[2].maxVal);
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
    
  
    void Scene::enableSharedOffscreenRendering(){
  #ifdef HAVE_QT
      GLImg::set_use_dirty_flag(false);
  #endif
    }
    
    void Scene::disableSharedOffscreenRendering(){
  #ifdef HAVE_QT
      GLImg::set_use_dirty_flag(true);
  #endif
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
      m_mouseHandlers.clear();
      m_glCallbacks.clear();
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

    void Scene::recompilePerPixelShader(int numShadowLights) const {
      delete m_perPixelShader;
      delete m_perPixelShaderTexture;
      delete m_perPixelShaderNoShadow;
      delete m_perPixelShaderTextureNoShadow;
      std::stringstream fragmentBuffer;
      std::stringstream vertexBuffer;
      
      //creating the vertex shader
      vertexBuffer
      <<"varying vec4 V;\n"
      <<"varying vec3 vertex_normal;\n";
      if(numShadowLights>0) {
        vertexBuffer
        <<"varying vec4 shadow_coord["<<numShadowLights<<"];\n"
        <<"uniform mat4 shadowMat["<<numShadowLights<<"];\n";
      }
      vertexBuffer
      <<"void main()\n"
      <<"{\n"
      <<"  V = gl_ModelViewMatrix * gl_Vertex;\n"
      <<"  vertex_normal = gl_NormalMatrix * gl_Normal;\n"
      <<"  gl_FrontColor = gl_Color;\n"
      <<"  gl_TexCoord[0] = gl_MultiTexCoord0;\n"
      <<"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n";

      //creating the fragment shader
      fragmentBuffer
      <<"varying vec4 V;\n"
      <<"varying vec3 vertex_normal;\n";
      if(numShadowLights>0) {
        fragmentBuffer
        <<"varying vec4 shadow_coord["<<numShadowLights<<"];\n"
        <<"const int num_shadow_lights = " <<numShadowLights<<";\n";
      }
      fragmentBuffer
      <<"vec3 N;\n"
      <<"vec4 texture_Color;\n"
      <<"uniform sampler2D shadow_map;\n"
      <<"uniform sampler2D image_map;\n"
      <<"uniform float bias;\n"
      <<"void computeColors(int light, out vec3 ambient, out vec3 diffuse, out vec3 specular){\n"
      <<"  vec3 L = normalize(gl_LightSource[light].position.xyz - V.xyz);\n"
      <<"  vec3 E = normalize(-V.xyz);\n"
      <<"  vec3 R = normalize(-reflect(L, N));\n"
      <<"  vec3 color = gl_Color.rgb;\n"
      <<"  if(use_texture)color = gl_Color.rgb * texture_Color.rgb;\n"
      <<"  ambient = gl_LightSource[light].ambient.rgb\n"
      <<"            * color;\n"
      <<"  diffuse = gl_LightSource[light].diffuse.rgb\n"
      <<"            * max(dot(N,L),0.0)\n"
      <<"            * color;\n"
      <<"  specular = gl_LightSource[light].specular.rgb\n"
      <<"             * pow(max(0.0,dot(R,E)),gl_FrontMaterial.shininess)\n"
      <<"             * gl_FrontMaterial.specular.rgb;\n"
      <<"}\n";
      if(numShadowLights>0) {
        fragmentBuffer
        <<"vec3 computeLightWithShadow(int light, int shadow){\n"
        <<"  vec3 ambient, diffuse, specular;\n"
        <<"  //compute phong lighting\n"
        <<"  computeColors(light, ambient, diffuse, specular);\n"
        <<"  //get screen space coordinates\n"
        <<"  vec4 shadow_divided = shadow_coord[shadow] / shadow_coord[shadow].w;\n"
        <<"  //check if the coordinate is out of bounds\n"
        <<"  if(shadow_divided.s < -1.0 || shadow_divided.s > 1.0) return ambient + diffuse + specular;\n"
        <<"  //transform to texture space coordinates\n"
        <<"  shadow_divided = shadow_divided * 0.5 + 0.5;\n"
        <<"  shadow_divided.s = (float(shadow) + shadow_divided.s) / float(num_shadow_lights);\n"
        <<"  //get shadow depth + offset\n"
        <<"  float shadow_depth = texture2D(shadow_map,shadow_divided.st).z + bias;\n"
        <<"  //check if fragment is in shadow\n"
        <<"  if(shadow_coord[shadow].w > 0.0)\n"
        <<"    if(shadow_divided.z > shadow_depth) return ambient;\n"
        <<"  return ambient + diffuse + specular;\n"
        <<"}\n";
      }
      fragmentBuffer
      <<"vec3 computeLight(int light){\n"
      <<"  vec3 ambient, diffuse, specular;\n"
      <<"  //compute phong lighting\n"
      <<"  computeColors(light, ambient, diffuse, specular);\n"
      <<"  return ambient + diffuse + specular;\n"
      <<"}\n"
      <<"void main(void){\n"
      <<"  N = normalize(vertex_normal);\n"
      <<"  texture_Color = texture2D(image_map, gl_TexCoord[0].st);\n"
      <<"  vec3 color = vec3(0,0,0);\n";
      
      int currentShadow = 0;
      for(unsigned int i = 0; i < 8; i++) {
        switch(m_previousLightState[i]) {
          case 1:
            fragmentBuffer << "  color += computeLight("<<i<<");\n";
            break;
          case 2:
            vertexBuffer << "  shadow_coord["<<currentShadow<<"] = shadowMat["<<currentShadow<<"] * V;\n";
            fragmentBuffer << "  if(render_shadow) {\n";
            fragmentBuffer << "    color += computeLightWithShadow("<<i<<","<<currentShadow<<");\n";
            fragmentBuffer << "  } else {\n";
            fragmentBuffer << "    color += computeLight("<<i<<");\n";
            fragmentBuffer << "  }\n";
            currentShadow++;
            break;
          default:
            break;
        }
      }
      vertexBuffer << "}\n";
      fragmentBuffer
      <<"  gl_FragColor =  vec4(color,gl_Color.a);\n"
      <<"  if(use_texture)gl_FragColor =  vec4(color,gl_Color.a * texture_Color.a);\n}\n";
      
      m_perPixelShader = new GLFragmentShader( vertexBuffer.str(), "bool use_texture = false;\nbool render_shadow = true;\n" + fragmentBuffer.str());
      m_perPixelShaderTexture = new GLFragmentShader( vertexBuffer.str(), "bool use_texture = true;\nbool render_shadow = true;\n" + fragmentBuffer.str());
      m_perPixelShaderNoShadow = new GLFragmentShader( vertexBuffer.str(), "bool use_texture = false;\nbool render_shadow = false;\n" + fragmentBuffer.str());
      m_perPixelShaderTextureNoShadow = new GLFragmentShader( vertexBuffer.str(), "bool use_texture = true;\nbool render_shadow = false;\n" + fragmentBuffer.str());
    }

    void Scene::renderSceneObjectRecursive(const vector<Mat> *project2shadow, SceneObject *o) const{
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
      
      if(o->m_lineSmoothingEnabled){
        glEnable(GL_LINE_SMOOTH);
      }else{
        glDisable(GL_LINE_SMOOTH);
      }
      
      if(o->m_pointSmoothingEnabled){
        glEnable(GL_POINT_SMOOTH);
      }else{
        glDisable(GL_POINT_SMOOTH);
      }
      
      if(o->m_polygonSmoothingEnabled){
        glEnable(GL_POLYGON_SMOOTH);
      }else{
        glDisable(GL_POLYGON_SMOOTH);
      }
      
      
      if(o->m_depthTestEnabled){
        glEnable(GL_DEPTH_TEST);
      }else{
        glDisable(GL_DEPTH_TEST);
      }
      
      
      GLFragmentShader *activeShader = 0;
      //check if the custom shader has been set
      bool useCustomShader = o->getFragmentShader();
      //check if the matrices have been set(indicates if improved lighting is being used)
      bool useImprovedShading = project2shadow;
      //shadow offset for the fragmentshader
      float bias;
      
      if(useImprovedShading) {
         bias = ((Configurable*)this)->getPropertyValue("shadows.shadow bias");
         if(useCustomShader){
            activeShader = o->getFragmentShader();
            o->getFragmentShader()->activate();
            o->getFragmentShader()->setUniform("bias", bias);
            o->getFragmentShader()->setUniform("shadowMat", *project2shadow);
            o->getFragmentShader()->setUniform("shadow_map", 7);
            o->getFragmentShader()->setUniform("image_map", 0);
         }
         else {
            if(o->getReceiveShadowsEnabled()) {
               activeShader = m_perPixelShader;
               m_perPixelShader->activate();
               m_perPixelShader->setUniform("bias", bias);
               m_perPixelShader->setUniform("shadowMat", *project2shadow);
               m_perPixelShader->setUniform("shadow_map", 7);
            } else {
               activeShader = m_perPixelShaderNoShadow;
               m_perPixelShaderNoShadow->activate();
               m_perPixelShaderNoShadow->setUniform("bias", bias);
               m_perPixelShaderNoShadow->setUniform("shadowMat", *project2shadow);
               m_perPixelShaderNoShadow->setUniform("shadow_map", 7);
            }
         }
      }else {
         if(useCustomShader){  
            activeShader = o->getFragmentShader();
            o->getFragmentShader()->activate();
         } 
      }

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



      if(o->isVisible()){
        
        o->customRender();
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
              if((p->type & (Primitive::text | Primitive::texture)) && useImprovedShading && !useCustomShader) {
                if(o->getReceiveShadowsEnabled()) {
                   m_perPixelShaderTexture->activate();
                   m_perPixelShaderTexture->setUniform("bias", bias);
                   m_perPixelShaderTexture->setUniform("shadowMat", *project2shadow);
                   m_perPixelShaderTexture->setUniform("shadow_map", 7);
                   m_perPixelShaderTexture->setUniform("image_map", 0);
                   p->render(ctx);
                   m_perPixelShader->deactivate();
                } else {
                   m_perPixelShaderTextureNoShadow->activate();
                   m_perPixelShaderTextureNoShadow->setUniform("bias", bias);
                   m_perPixelShaderTextureNoShadow->setUniform("shadowMat", *project2shadow);
                   m_perPixelShaderTextureNoShadow->setUniform("shadow_map", 7);
                   m_perPixelShaderTextureNoShadow->setUniform("image_map", 0);
                   p->render(ctx);
                   m_perPixelShaderTextureNoShadow->deactivate();
                }
                activeShader->activate();
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
      
      for(unsigned int i=0;i<o->m_children.size();++i){
        renderSceneObjectRecursive(project2shadow, o->m_children[i].get());
      }
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      if(activeShader) activeShader->deactivate();
  
      o->unlock();
    }
    
   void Scene::renderSceneObjectRecursiveShadow(SceneObject *o) const{
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
      o->prepareForRendering();
  
      o->lock();
      
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      const Mat &T = o->getTransformation(true);
      glMultMatrixf(T.transp().data());


      if(o->getCastShadowsEnabled()){
         if(o->isVisible()){
           
           o->customRender();
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
      }
      for(unsigned int i=0;i<o->m_children.size();++i){
        renderSceneObjectRecursiveShadow(o->m_children[i].get());
      }
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
  
      o->unlock();
   }
  
   void Scene::renderScene(int camIndex, ICLDrawWidget3D *widget) const{
  
      Mutex::Locker l(this);
      ICLASSERT_RETURN(camIndex >= 0 && camIndex < (int)m_cameras.size());
  
      Rect currentImageRect = widget ? widget->getImageRect(true) : Rect::null;
      //    Size currentImageSize = widget ? widget->getImageSize(true) : Size::null;
      Size widgetSize = widget ? widget->getSize() : Size::null;
  
      Camera cam = m_cameras[camIndex];
      if(widget){
        cam.getRenderParams().viewport = currentImageRect;
      }
      
      bool lightingEnabled = ((Configurable*)this)->getPropertyValue("enable lighting");
      bool useImprovedShading = ((Configurable*)this)->getPropertyValue("shadows.use improved shading");
      
      vector<Mat> project2shadow;
      if(lightingEnabled && useImprovedShading) {
        
      //update cameras and check if the lightsetup has changed
      bool lightSetupChanged = false;
      int numShadowLights = 0;
      for(int i=0;i<8;++i){
       if(m_lights[i]) {
         m_lights[i]->updatePositions(*this,getCamera(camIndex));
         if(m_lights[i]->on) {
           if(m_lights[i]->shadowOn) {
             
             if(m_previousLightState[i] != 2) {
               lightSetupChanged = true;
             }
             m_previousLightState[i] = 2;
             numShadowLights++;
           } else {
             if(m_previousLightState[i] != 1) {
               lightSetupChanged = true;
             }
             m_previousLightState[i] = 1;
           }
         } else {
           if(m_previousLightState[i] != 0) {
             lightSetupChanged = true;
           }
           m_previousLightState[i] = 0;
         }
       }
      }

      if(lightSetupChanged) {
         recompilePerPixelShader(numShadowLights);
         m_perPixelShader->activate();
         m_perPixelShader->deactivate();
      }

      //recreate the shadowbuffer if the the lightsetup, or the resolution has changed
      unsigned int resolution = ((Configurable*)this)->getPropertyValue("shadows.shadow resolution");
      if(shadowResolution != resolution || lightSetupChanged) {
       freeShadowFBO();
       //don"t create a shadowbuffer if there are no lights with shadows
       if(numShadowLights > 0) {
         createShadowFBO(resolution,numShadowLights); 
       }
      }

      //bind texture if it has been created
      if(shadowResolution>0) {
       glActiveTextureARB(GL_TEXTURE7);
       glBindTexture(GL_TEXTURE_2D,shadowTexture);
      }

      //render the shadows and create the projection matrices for the vertex shader
      int currentShadow = 0;
      for(int i = 0; i < 8; i++) {
       if(m_previousLightState[i] == 2) {
         renderShadow(i, currentShadow++, shadowResolution);
         project2shadow.push_back(m_lights[i]->getShadowCam().getProjectionMatrixGL() 
                         * m_lights[i]->getShadowCam().getCSTransformationMatrixGL() 
                         * cam.getCSTransformationMatrixGL().inv());
       }
      }
        
      }else {
        freeShadowFBO();
        if(lightingEnabled) {
          for(int i=0;i<8;++i){
            if(m_lights[i]) {
              m_lights[i]->updatePositions(*this,getCamera(camIndex));
            }
          }
        }
      }

      glEnable(GL_NORMALIZE);
      
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(GLMatrix(cam.getCSTransformationMatrixGL()));
  
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(GLMatrix(cam.getProjectionMatrixGL()));
  
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
      // specular lighting is still not working ..
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
      
      if( lightingEnabled){
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
      
      if(lightingEnabled && useImprovedShading){
        for(size_t i=0;i<m_objects.size();++i){
          renderSceneObjectRecursive(&project2shadow, (SceneObject*)m_objects[i].get());
        }
      } else {
        for(size_t i=0;i<m_objects.size();++i){
          renderSceneObjectRecursive((SceneObject*)m_objects[i].get());
        }
      }
      
      if(((Configurable*)this)->getPropertyValue("visualize cameras")){
        for(unsigned int i=0;i<m_cameraObjects.size();++i){
          if((int)i == camIndex) continue;
          renderSceneObjectRecursive((SceneObject*)m_cameraObjects[i].get());
        }
      }

      glPushAttrib(GL_ENABLE_BIT);
      
      if(lightingEnabled && ((Configurable*)this)->getPropertyValue("visualize lights")){
        for(int i=0;i<8;++i){
          if(m_lights[i] && m_lights[i]->on){
            if((m_lights[i]->anchor != SceneLight::CamAnchor) ||
               (m_lights[i]->camAnchor != camIndex && m_lights[i]->camAnchor != -1)){
              renderSceneObjectRecursive((SceneObject*)m_lights[i]->getLightObject());
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
          renderObjectFramesRecursive((SceneObject*)m_objects[i].get(), (SceneObject*)m_objectFrameObject.get());
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
        renderSceneObjectRecursive((SceneObject*)m_coordinateFrameObject.get());
      }
  
    }
    
   void Scene::renderShadow(const unsigned int light, const unsigned int shadow, unsigned int size) const{
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, shadowFBO);

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
      glLoadMatrixf(GLMatrix(m_lights[light]->getShadowCam().getCSTransformationMatrixGL()));
  
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(GLMatrix(m_lights[light]->getShadowCam().getProjectionMatrixGL()));
      
      glEnable(GL_DEPTH_TEST);
      
      glEnable(GL_CULL_FACE);
	    
      bool cullFront = ((Configurable*)this)->getPropertyValue("shadows.cull object front for shadows");
	    if(cullFront) {
	      glCullFace(GL_FRONT);
      } else {
	      glCullFace(GL_BACK);
      }
	    
      for(size_t i=0;i<m_objects.size();++i){
        renderSceneObjectRecursiveShadow((SceneObject*)m_objects[i].get());
      }
	    // enable the default framebuffer
	    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	    glDisable(GL_CULL_FACE);
	    glDisable(GL_SCISSOR_TEST);
    }

    void Scene::renderObjectFramesRecursive(SceneObject *o, SceneObject *cs) const{
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      const Mat &T = o->getTransformation(true);
      glMultMatrixf(T.transp().data());
      
      renderSceneObjectRecursive(cs);
      
      for(unsigned int i=0;i<o->m_children.size();++i){
        renderObjectFramesRecursive(o->m_children[i].get(),cs);
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
        float maxLen = iclMax(m_bounds[0].getLength(), m_bounds[1].getLength());
        maxLen = iclMax(m_bounds[2].getLength(), maxLen);
        newSceneMouseHandler->setSensitivities(maxLen);
      }else{
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
    
    void Scene::createShadowFBO(unsigned int size, unsigned int shadows) const{
	    GLenum FBOstatus;
      
      //create the shadow texture if simple filtering
	    glGenTextures(1, &shadowTexture);
	    glBindTexture(GL_TEXTURE_2D, shadowTexture);

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size * shadows, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	    glBindTexture(GL_TEXTURE_2D, 0);

	    // create a framebuffer object
	    glGenFramebuffers(1, &shadowFBO);
	    glBindFramebuffer(GL_FRAMEBUFFER_EXT, shadowFBO);

	    glDrawBuffer(GL_NONE);
	    glReadBuffer(GL_NONE);
   
	    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, shadowTexture, 0);

	    // check FBO status
	    FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	    if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
		    throw ICLException("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO");
		    
	    // switch back to previous framebuffer
	    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	    shadowResolution = size;
  }
  
  void Scene::freeShadowFBO() const{
      if(shadowResolution) {
	      glDeleteFramebuffersEXT(1, &shadowFBO);
	      glDeleteTextures(1, &shadowTexture);
	      shadowResolution = 0;
	    }
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
      }
      return *m_lights[index];
    }
  
    const SceneLight &Scene::getLight(int index) const throw (ICLException){
      return const_cast<Scene*>(this)->getLight(index);
    }
  
      
    void Scene::setLightingEnabled(bool flag){
      setPropertyValue("visualize lights",flag);
    }

    void Scene::setDrawObjectFramesEnabled(bool enabled, float size){
      setPropertyValue("visualize object frames",enabled);
      setPropertyValue("object frame size",size);
    }
    
    bool Scene::getDrawObjectFramesEnabled() const{
      return ((Configurable*)this)->getPropertyValue("visualize object frames");
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
      renderSceneObjectRecursive(o);
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
        m_bounds[0] = Range32f(maxX,minX);
      }else{
        m_bounds[0] = Range32f(minX,maxX);
      }
      if(minY == maxY){
        m_bounds[1] = Range32f(minX,maxX);
      }else{
        m_bounds[1] = Range32f(minY,maxY);
      }
      if(minZ == maxZ){
        m_bounds[2] = Range32f(minX,maxX);
      }else{
        m_bounds[2] = Range32f(minZ,maxZ);
      }
    }
    
  
  
  
  #ifdef HAVE_QT
  #ifdef HAVE_GLX
  
    struct Scene::PBuffer{
      
      static Mutex glxMutex; // glx context's are not thread-safe -> so offscreen rendering performed in an atomic code segment
      
      static Display *getDisplay(){
        static Display *d = XOpenDisplay(getenv("DISPLAY"));
        // this leads to errors due to missing x-server connection
        // obviously the connect is cut before the static context is free'd 
        // static struct F{ ~F(){ XCloseDisplay(d); }} freeDisplay;
        return d;
      }
      static GLXFBConfig &findGLXConfig() throw (ICLException){
        static int n = 0;
        static const int att[] = {GLX_RED_SIZE,8,GLX_GREEN_SIZE,8,GLX_BLUE_SIZE,8,GLX_DEPTH_SIZE,24,
                                  GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT, 0};
        static GLXFBConfig *configs = glXChooseFBConfig(getDisplay(), DefaultScreen(getDisplay()),att,&n);
        if(!configs){
          // We choose a less restrictive configuration
          static const int att2[] = {GLX_RED_SIZE,4,GLX_GREEN_SIZE,4,GLX_BLUE_SIZE,4,GLX_DEPTH_SIZE,16,
                                    GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT, 0};
          configs = glXChooseFBConfig(getDisplay(), DefaultScreen(getDisplay()),att2,&n);
          if(!configs){
            throw ICLException("Scene::render pbuffer based rendering is not supported on you machine");
          }
        } 
        return *configs;
      }
      
      PBuffer(){}
      PBuffer(const Size s):size(s){
        const int S[] = { GLX_PBUFFER_WIDTH,size.width, GLX_PBUFFER_HEIGHT, size.height, 0 };
        pbuffer = glXCreatePbuffer(getDisplay(), findGLXConfig(), S);
  
        // note: setting this to false makes rendering 50% slower
        context = glXCreateNewContext(getDisplay(), findGLXConfig(), GLX_RGBA_TYPE, 0, true /* try direct rendering context first*/);
        if(!context){
          context = glXCreateNewContext(getDisplay(), findGLXConfig(), GLX_RGBA_TYPE, 0, false);
          if(!context){
            throw ICLException("glXCreateNewContext failed: unable to create pbuffer rendering context");
          }
        }
        buf = Img8u(s,formatRGB);
        rgbbuf.resize(size.width*size.height*3);
      }
      ~PBuffer(){
        glXDestroyContext(getDisplay(), context);
        glXDestroyPbuffer(getDisplay(), pbuffer);
      }
      void makeCurrent(){
        glXMakeCurrent(getDisplay(),pbuffer,context);
        GLContext::set_current_glx_context(context,pbuffer,getDisplay());
      }
      
      GLXContext context;    /* OpenGL context */
      GLXPbuffer pbuffer;    /* Pbuffer */
      SmartPtr<GLImg> background; // optionally used background image
      Size size;
      std::vector<icl8u> rgbbuf;
      Img8u buf;
      
      struct DepthCorrection{
        std::vector<icl32f> factors;
        Size resolution;
        icl32f fX,fY,skew;
        Point32f ppOffs;
  
        static inline float compute_depth_norm(const Vec &dir, const Vec &centerDir){
          return sprod3(dir,centerDir)/(norm3(dir)*norm3(centerDir));
        }
        
        void update(const Camera &cam){
          const Camera::RenderParams &p = cam.getRenderParams();
          const float f = cam.getFocalLength();
          const int w = p.viewport.width, h = p.viewport.height;
          if(!factors.size() ||
             resolution != Size(w,h) ||
             fX != f*cam.getSamplingResolutionX() ||
             fY != f*cam.getSamplingResolutionY() ||
             skew != cam.getSkew() ||
             ppOffs != cam.getPrincipalPointOffset()){
            
            resolution = Size(w,h);
            fX = f*cam.getSamplingResolutionX();
            fY = f*cam.getSamplingResolutionY();
            skew = cam.getSkew();
            ppOffs = cam.getPrincipalPointOffset();
            
            factors.resize(w*h);
            
            Array2D<ViewRay> vs = cam.getAllViewRays();
            
            const Vec c = vs(w/2-1,h/2-1).direction;
  
            for(int idx=0;idx<w*h; ++idx){
              factors[idx] = 1.0/compute_depth_norm(vs[idx].direction,c);
            }
          }
        }
      } depthCorr;
    };
  
    Scene::PBufferIndex::PBufferIndex(const Size &size):
      Size(size),threadID(pthread_self()){}
  
    bool Scene::PBufferIndex::operator<(const Scene::PBufferIndex &other) const{
      if(other.threadID == threadID){
        if(other.width == width) return other.height < height;
        else return other.width < width;
      }else return other.threadID < threadID;
    }
  
    Mutex Scene::PBuffer::glxMutex;
    
    /** Benchmark results:
        
        Hardware:     using dell xt2 laptop (intel integrated graphics adapter)
                      times in braces are taken from an Quad-Core Xeon Workstation with
                      nvida quadro card 
        ViewPortSize: VGA
        Scene:        ICL's scene graph demo (including the parrot background image)
        
        context creation/
        buffer allocation   : 11ms (first time, then 0.06ms) (43ms, then 0.4ms)
        
        rendering the scene : 85ms (5ms)
        
        pbuffer read-out    : 31ms (3.2ms)
        
        interlavedToPlanar  : 0.8ms (0.2ms)
  
        flip vertically     : 0.6ms (0.13ms)
  
        grabbing the depth buffer: ?? (4.29ms)
      -------------------------------------
        total               : 117ms (9ms) + time for depth buffer
        
    */
    const Img8u &Scene::render(int camIndex, const ImgBase *background, Img32f *depthBuffer,
                               DepthBufferMode mode) const throw (ICLException){
      ICLASSERT_THROW(camIndex < (int)m_cameras.size(),ICLException("Scene::render: invalid camera index"));
      const Camera &cam = getCamera(camIndex);
      int w = cam.getRenderParams().viewport.width;
      int h = cam.getRenderParams().viewport.height;
      Size s(w,h);
      PBufferIndex idx(s);
  
      Mutex::Locker lock(PBuffer::glxMutex);
      //    PBuffer::glxMutex.lock();
      std::map<PBufferIndex,PBuffer*>::iterator it = m_pbuffers.find(idx);
      PBuffer &p = (it==m_pbuffers.end())?(*(m_pbuffers[idx] = new PBuffer(s))):(*it->second);
      // PBuffer::glxMutex.unlock();
  
  
      p.makeCurrent();
  
      GeomColor c = getBackgroundColor();
      glClearColor(c[0]/255.,c[1]/255.,c[2]/255.,1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );    
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_COLOR_MATERIAL);
  
      if(background){
        glOrtho(0, w, h, 0, -999999, 999999);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        SmartPtr<GLImg> &bg = p.background;
        if(!bg) bg = SmartPtr<GLImg>(new GLImg);
        bg->update(background);
        bg->draw2D(Rect(0,0,w,h),s);
        glEnable(GL_LIGHTING);
        glClear(GL_DEPTH_BUFFER_BIT );    
      }
  
      renderScene(camIndex);
      glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, p.rgbbuf.data());
      interleavedToPlanar(p.rgbbuf.data(),&p.buf);
      p.buf.mirror(axisHorz); // software mirroring takes only about 1ms (with ipp)
      
      if(depthBuffer){
        depthBuffer->setSize(Size(w,h));
        depthBuffer->setChannels(1);
        glReadPixels(0,0,w,h, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer->begin(0));
        depthBuffer->mirror(axisHorz);
  
        if(mode != RawDepth01){
          const float zNear = cam.getRenderParams().clipZNear;
          const float zFar = cam.getRenderParams().clipZFar;
          
          icl32f *db = depthBuffer->begin(0);
  
          const int dim = w*h;
          const float Q = zFar / ( zFar - zNear );
          const float izFar = 1.0/zFar;
          
          const float m = zFar-zNear;
          const float b = zNear;
          
          const float A = izFar * m;
  
          if(mode == DistToCamCenter){
            p.depthCorr.update(cam);
            const float *corr = p.depthCorr.factors.data();
            for(int i=0;i<dim;++i){
              db[i] = corr[i] * (A / (Q-db[i]) + b) - 1;
            }
          }else{
            for(int i=0;i<dim;++i){
              db[i] = (A / (Q-db[i]) + b) - 1;
            }
          }
        }
      }
      
      GLContext::unset_current_glx_context();
      p.buf.setTime(icl::utils::Time::now());
      if(depthBuffer) depthBuffer->setTime(p.buf.getTime());
      return p.buf;
    }
  
    void Scene::freeAllPBuffers(){
      typedef std::map<PBufferIndex,PBuffer*>::iterator It;
      for(It it = m_pbuffers.begin();it != m_pbuffers.end();++it){
        delete it->second;
      }
      m_pbuffers.clear();
    }
  
    void Scene::freePBuffer(const Size &size){
      typedef std::map<PBufferIndex,PBuffer*>::iterator It;
      PBufferIndex idx(size);
      It it = m_pbuffers.find(idx);
      if(it != m_pbuffers.end()){
        delete it->second;
        m_pbuffers.erase(it);
      }
    }
    
  #endif
  #endif
  
  
  } // namespace geom
}

