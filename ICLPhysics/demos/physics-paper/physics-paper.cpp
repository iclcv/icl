#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLUtils/FPSLimiter.h>

#include <ICLPhysics/PhysicsDefs.h>
#include <ICLPhysics/PhysicsWorld.h>
#include <ICLPhysics/PhysicsUtils.h>
#include "DefaultGroundObject.h"
#include "DefaultPhysicsScene.h"
#include <ICLPhysics/ManipulatablePaper.h>

#include "SceneMultiCamCapturer.h"
#include <ICLUtils/ConsoleProgress.h>
#include "InteractiveFoldLine.h"

InteractiveFoldLine foldLine;
DefaultPhysicsScene scene;
HSplit gui;
VBox propGUI;



DefaultGroundObject ground;

SmartPtr<ManipulatablePaper> paper;
SmartPtr<SceneMultiCamCapturer> capturer;

void fold_line_cb(const Point32f &a, const Point32f &b){
  //TODO IMPLEMENt LOCK PhysicsWorld::Locker lock(world);
  paper->adaptStiffnessAlongLine(a,b,gui["soften"].as<bool>() ? 0.001 : 0.9);
}

void reset_paper(){
  //IMPLEMENT LOCK PhysicsWorld::Locker lock(world);
  paper->adaptGlobalStiffness(0.9);
}
void change_global_stiffness(){
  //IMPLEMENT LOCK PhysicsWorld::Locker lock(world);
  paper->adaptGlobalStiffness(gui["globalStiffness"]);
}

void paper_coords_test(){
  Img8u image = scene.render(0);
  Array2D<geom::ViewRay> rays = scene.getCamera(0).getAllViewRays();
  progress_init("creating paper-coords image");
  for(int y=0;y<480;y+=2){
    progress(y,479);
    for(int x=0;x<640;x+=2){
      Point32f coords = paper->getPaperCoordinates(rays(x,y));
      if(coords.x > 0){
        image(x,y,0) = 255.0 * coords.x / float(paper->getDimensions().width) ;
        image(x,y,1) = 0;
        image(x,y,2) = 255.0 * coords.y / float(paper->getDimensions().height) ;

        image(x+1,y) = image(x,y);
        image(x+1,y+1) = image(x,y);
        image(x,y+1) = image(x,y);
      }
    }
  }
  progress_finish();

  show(image);
}

void init(){

  //  scene.getLight(0).setOn(false);
  scene.getLight(0).setDiffuse(GeomColor(255,255,255,50));

  SceneLight &l = scene.getLight(1);
  static Camera cam(Vec(0,0,600,1),
                    Vec(0,0,-1,1),
                    Vec(0,-1,0,1));
  cam.setResolution(Size(1024,1024));
  scene.setGravity(Vec(0,0,-1000));

  l.setShadowCam(new Camera(cam));
  l.setShadowEnabled(true);
  l.setAnchorToWorld();
  l.setPosition(Vec(0,0,600,1));
  l.setOn(true);
  l.setSpecularEnabled(true);
  l.setDiffuseEnabled(true);
  l.setSpecular(GeomColor(0,100,255,255));
  l.setDiffuse(GeomColor(255,100,0,30));

  scene.setPropertyValue("shadows.use improved shading",true);
  scene.setPropertyValue("shadows.resolution",2048);
  scene.setPropertyValue("shadows.bias",10);


  //static const int W=20,H=13,DIM=W*H;
  static const int W=pa("-paper-dim",0), H=pa("-paper-dim",1);
  static Img8u frontFace = load<icl8u>(*pa("-ff"));
  static Img8u backFace = load<icl8u>(*pa("-bf"));

  const Size s(210,297);
  const Vec corners[4] = {
    Vec(-s.width/2, -s.height/2, 150,1),
    Vec(s.width/2, -s.height/2, 150,1),
    Vec(s.width/2, s.height/2, 150,1),
    Vec(-s.width/2, s.height/2, 150,1),
  };

  paper = new ManipulatablePaper(&scene,&scene,W,H,corners,true,&frontFace,&backFace);
  //scene.removeObject(paper);
  //paper->addShadow(-74.5);


  if(pa("-o")){
    std::vector<Camera> cams;
    for(int i=0;i<3;++i){
      cams.push_back(Camera(*pa("-c",i)));
    }
    capturer = new SceneMultiCamCapturer(scene, cams);
    //    Scene::enableSharedOffscreenRendering();
    scene.setDrawCamerasEnabled(false);
  }

  gui << Draw3D(Size::VGA).minSize(32,24).handle("draw")
      << (VBox().maxSize(12,100).minSize(12,1)
          << ( HBox()
               << Fps(10).handle("fps")
               << Button("add clutter").handle("add")
               )
          << ( HBox()
               << Button("stopped","running",true).out("run")
               << Button("paper ...").handle("props")
               )
          << ( HBox()
               << CheckBox("show cubes").out("showCubes")
               << CheckBox("show texture",false).out("showTexture")
               << CheckBox("show links",false).out("showLinks")
             )
          << FSlider(0,1,0.5).out("vertexMoveFactor").label("manual force")
          << FSlider(1,100,10).out("attractorStreangth").label("attractor force")
          << FSlider(0.0001,0.9999,0.9).handle("globalStiffness").label("global paper stiffness")
          << ( HBox()
               << Button("reset paper").handle("resetPaper")
               << Combo("1,5,10,25,!200,300,500").handle("maxFPS").label("max FPS")
               )
          << FSlider(0.1,20,2).handle("cm").label("collision margin")

          << ( HBox()
               << Button("memorize").handle("mem")
               << CheckBox("soften with mouse",true).handle("soften")
               << Button("test").handle("pct")
             )
          )

      << Show();

  propGUI << Prop("paper").minSize(16,1).maxSize(16,100) << Create();

  gui["pct"].registerCallback(paper_coords_test);
  gui["props"].registerCallback(utils::function((GUI&)propGUI,&GUI::switchVisibility));
  gui["resetPaper"].registerCallback(reset_paper);
  gui["globalStiffness"].registerCallback(change_global_stiffness);

  scene.PhysicsWorld::addObject(&ground);


  DrawHandle3D draw = gui["draw"];
  draw->install(paper->createMouseHandler(0));
  draw->install(&foldLine);
  draw->link(scene.getGLCallback(0));

  foldLine.cb = utils::function(fold_line_cb);



}

void run(){
  static ButtonHandle mem = gui["mem"];
  static ButtonHandle add = gui["add"];
  static DrawHandle3D draw = gui["draw"];
  static FPSLimiter fpslimit(10,10);

  fpslimit.setMaxFPS(parse<int>(gui["maxFPS"].as<std::string>()));

  paper->setTextureVisible(gui["showTexture"]);
  paper->setCubesVisible(gui["showCubes"]);
  paper->setShowAllConstraints(gui["showLinks"]);

  static float lastMargin = -1;
  float currMargin = gui["cm"];
  if(lastMargin != currMargin){
    paper->getSoftBody()->getCollisionShape()->setMargin(icl2bullet(currMargin));
    lastMargin = currMargin;
  }

  if(mem.wasTriggered()){
    paper->memorizeDeformation();
  }
  static bool first = true;
  if(add.wasTriggered() || (first && pa("-a"))){
    first = false;
    add_clutter(&scene,&scene,30);
    //TODO CHECK THIS OUT add_clutter(&scene, &world, 1, 5, false);
  }
  remove_fallen_objects(&scene, &scene);


  static Time lastTime = Time::now();
  Time now = Time::now();
  double dt = (now-lastTime).toSecondsDouble();
  lastTime = now;
  if(gui["run"]){
    paper->applyAllForces(gui["attractorStreangth"],gui["vertexMoveFactor"]);
    scene.Scene::lock();
    paper->lock();
    scene.step( dt * 5, 1);
    paper->unlock();
    scene.Scene::unlock();
  }

  foldLine.visualize(**draw);
  draw.render();
  gui["fps"].render();

  if(capturer){
    capturer->capture();
  }
  fpslimit.wait();
}


int main(int n, char **ppc){
    return ICLApp(n,ppc,"-o -output-cams|-c(cam0=etc/cam-0.xml,cam1=etc/cam-1.xml,cam2=etc/cam-2.xml) "
                  "-front-face|-ff(file=etc/front-face.ppm.gz) "
                  "-back-face|-bf(file=etc/back-face.ppm.gz) -add-clutter-on-start|-a "
                  "-paper-dim(W=15,H=21)",init,run).exec();
}

