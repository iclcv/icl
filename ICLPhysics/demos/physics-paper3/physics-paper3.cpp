#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLPhysics/PhysicsPaper3MouseHandler.h>
#include <ICLPhysics/PhysicsPaper3.h>
#include <DefaultGroundObject.h>
#include <ICLUtils/File.h>
#include <ICLPhysics/PhysicsScene.h>

#include <QtWidgets/QPushButton>

DefaultGroundObject ground;
DefaultGroundObjectVisualization groundVis;
PhysicsScene scene;
HSplit gui;

PhysicsPaper3 *model = 0;
PhysicsPaper3MouseHandler *mouse = 0;

void fold_map_changed(const Img32f &image){
  gui["fm"] = image;
}

void init(){
  if(pa("-textures")){
    Img8u ft = load<icl8u>(*pa("-textures",0));
    Img8u bt = load<icl8u>(*pa("-textures",1));
    model = new PhysicsPaper3(&scene,pa("-self"),pa("-dim"), 0, &ft, &bt, pa("-it"), pa("-id"));
  }else{
    model = new PhysicsPaper3(&scene,pa("-self"),pa("-dim"),0,0,0, pa("-it"), pa("-id"));
  }
  model->setFoldMapChangedCallback(fold_map_changed);
  
  scene.addObject(model,false);

  if(pa("-shaders")){
    std::string v = File::read_file(pa("-shaders",0));
    std::string f = File::read_file(pa("-shaders",1));
    ground.setFragmentShader( new GLFragmentShader(v,f) );
  }

  scene.Scene::addObject(&groundVis,false);
  scene.addObject(&ground,false);

  scene.addCamera(Camera(Vec(345.808,-114.16,494.068,1),
                         Vec(-0.555173,0.189891,-0.809768,1),
                         Vec(0.802291,-0.14152,-0.579915,1),
                         3, Point32f(320,240),200,200,
                         0, Camera::RenderParams(Size(640,480),
                                                 1,10000, 
                                                 Rect(0,0,640,480),
                                                 0,1)));
  scene.setBounds(1000);
  
  gui << Draw3D().handle("draw3D").minSize(32,24) 
      << (VBox().minSize(12,1).maxSize(12,99)
          << CheckBox("physics on",true).handle("physics on")
          << CheckBox("gravity on",true).handle("gravity on")
          << CheckBox("vis lines",true).handle("vis lines")
          << CheckBox("vis 2nd order links",false).handle("vis links")
          << Button("pos interp. test").handle("test")
          << Button("hit  test").handle("hit test")
          << Button("link add test").handle("link test")
          << Button("view ray test").handle("vr")
          << Button("reset model").handle("reset")
          << FSlider(1,20,10).handle("f").label("move force")
          << FSlider(0.01,1,0.25).handle("r").label("move radius")
          << Image().label("fold map").handle("fm").minSize(8,8)
         )
      << Show();
  
  mouse = new PhysicsPaper3MouseHandler(model,&scene);
  
  gui["draw3D"].link(scene.getGLCallback(0));
  gui["draw3D"].install(mouse);

  //  scene.getLight(0).setOn(false);
  scene.getLight(0).setDiffuse(GeomColor(255,255,255,50));
  scene.setPropertyValue("shadows.use improved shading", true);
  scene.setPropertyValue("shadows.resolution", 2048);

  SceneLight &l = scene.getLight(1);
  l.setOn();
  l.setShadowEnabled();
  l.setTwoSidedEnabled(true);
  l.setAnchorToWorld();
  l.setPosition(Vec(0,0,1200,1));
  l.setOn(true);
  l.setAmbientEnabled();
  l.setDiffuseEnabled();
  l.setSpecularEnabled();

  l.setSpecular(GeomColor(0,100,255,255));
  l.setDiffuse(GeomColor(255,100,0,30));
  l.getShadowCam()->setNorm(Vec(0,0,-1,1));

  

  ButtonHandle h4 = gui["vr"], reset=gui["reset"];
  reset.disable();
  h4.disable();
  
}

void run(){
  ButtonHandle h = gui["test"];
  ButtonHandle h2 = gui["hit test"];
  ButtonHandle h3 = gui["link test"];
  ButtonHandle h4 = gui["vr"];
  ButtonHandle reset = gui["reset"];

  if(h.wasTriggered()){
    scene.Scene::addObject(model->approximateSurface());
  }
  if(h2.wasTriggered()){
    show(model->paperCoordinateTest(scene.getCamera(0)));
  }
  if(h3.wasTriggered()){
    model->createBendingConstraints(0.4);
  }
  if(h4.wasTriggered()){
    Array2D<ViewRay> vrs = scene.getCamera(0).getAllViewRays();
    SceneObject *o = new SceneObject;
    o->getVertices().push_back(vrs[0].offset);
    int i=0;
    for(int y=0;y<vrs.getHeight();y+=10){
      for(int x=0;x<vrs.getWidth();x+=10,++i){
        o->getVertices().push_back(vrs(x,y).offset + vrs(x,y).direction * 100.0f);
        o->getVertices().back()[3] = 1;
        o->addLine(0,i+1);
      }
    }
    scene.Scene::addObject(o);
  }
  if(reset.wasTriggered()){
      scene.Scene::lock();
      scene.PhysicsWorld::lock();
    
    scene.removeObject(model);

    delete model;

    model = new PhysicsPaper3(&scene,pa("-self"),pa("-dim"));
    model->setFoldMapChangedCallback(fold_map_changed);
    
    scene.addObject(model,false);

    scene.PhysicsWorld::unlock();
    scene.Scene::unlock();
  }
  
  model->setVisible(Primitive::line,gui["vis lines"]);
  //model->simulateSelfCollision();
  
  DrawHandle3D draw = gui["draw3D"];
  draw->draw(mouse->vis());
  draw.render();
  model->setLinksVisible(gui["vis links"]);
  scene.setGravityEnabled(gui["gravity on"]);
  mouse->applyForceToModel(gui["f"],gui["r"]);
  if(gui["physics on"]){
    scene.step(1,1);
  }
 
  Thread::msleep(10);
}

int main(int n, char **a){
  return ICLApp(n,a,"-shaders(vertex-shader-file,fragment-shader-file) "
                "-initial-paper-dim|-dim|-d(size=4x6) "
                "-enable-self-collision|-self|-s "
                "-textures|-t(2) -initial-stiffness|-it(float=-1) "
                "-initial-link-distance|-id(float=0.5)",init,run).exec();
}
