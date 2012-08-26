#include <ICLCV/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLIO/GenericImageOutput.h>
Scene scene;
HSplit gui;
GUI prevGUI = HBox();

GenericImageOutput colorOut, depthOut;
SceneObject *obj = 0;

SmartPtr<Mat> relTM;
Camera initDepthCam;

void init(){
  bool cOut = pa("-c"), dOut = pa("-d");
  
  if(cOut){
    colorOut.init(pa("-c"));
  }
  if(dOut){
    depthOut.init(pa("-d"));
  }
  
  if(cOut || dOut){
    prevGUI << (cOut ? Image().handle("color") : Dummy())
            << (dOut ? Image().handle("depth") : Dummy())
            << Create();
  }

  gui << Draw3D().handle("draw")
      << ( VBox().minSize(10,2)
           << FSlider(-10,10,0).out("x").label("translate x")
           << FSlider(-10,10,0).out("y").label("translate y")
           << FSlider(1.5,10,0).out("z").label("translate z")
           
           << FSlider(-4,4,0).out("rx").label("rotate x")
           << FSlider(-4,4,0).out("ry").label("rotate y")
           << FSlider(-4,4,0).out("rz").label("rotate z")

           << ((cOut||dOut) ? (const GUIComponent&)Button("show","hide").label("preview").handle("preview") 
               : (const GUIComponent&)Dummy() )
           << Button("reset view").handle("resetView")
         )
      << Show();

  
  if(cOut || dOut){
    gui["preview"].registerCallback(function(&prevGUI,&GUI::switchVisibility));
    if(dOut){
      ImageHandle d = prevGUI["depth"];
      d->setRangeMode(ICLWidget::rmAuto);
    }
  }
  
  Camera defaultCam(Vec(4.73553,-3.74203,8.06666,1),
                    Vec(-0.498035,0.458701,-0.735904,1),
                    Vec(0.787984,-0.116955,-0.604486,1));
  scene.addCamera( !pa("-cam").as<bool>() ? defaultCam : Camera(*pa("-cam")));
  initDepthCam = scene.getCamera(0);
  
  if(pa("-ccam")){
    scene.addCamera(*pa("-ccam"));
    Mat D=scene.getCamera(0).getCSTransformationMatrix();
    Mat C=scene.getCamera(1).getCSTransformationMatrix();
    
    relTM = new Mat( C * D.inv() );
  }

  SceneObject* ground = SceneObject::cuboid(0,0,0,200,200,3);
  ground->setColor(Primitive::quad,GeomColor(100,100,100,255));
  
  scene.addObject(ground);
  if(pa("-object")){
    scene.addObject( (obj = new SceneObject(*pa("-object"))) );
  }else{
    scene.addObject( (obj = SceneObject::cube(0,0,3, 3) ) );
  }
  obj->setColor(Primitive::quad, GeomColor(0,100,255,255));
  obj->setColor(Primitive::triangle, GeomColor(0,100,255,255));
  obj->setColor(Primitive::polygon, GeomColor(0,100,255,255));
  obj->setVisible(Primitive::line | Primitive::vertex, false);
  
  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(scene.getMouseHandler(0));

  scene.setDrawCamerasEnabled(false);

  scene.addCamera(scene.getCamera(0));
}

void run() {
  static ButtonHandle resetView = gui["resetView"];
  if(resetView.wasTriggered()){
    scene.getCamera(0) = initDepthCam;
  }
  bool cOut = pa("-c"), dOut = pa("-d");
  
  obj->removeTransformation();
  obj->rotate(gui["rx"],gui["ry"],gui["rz"]);
  obj->translate(Vec(gui["x"],gui["y"],gui["z"],1));

  static Img32f depthImage;
  if(cOut || dOut){
    static Scene::DepthBufferMode dbm = ( pa("-depth-map-dist-to-cam-center") ? 
                                          Scene::DistToCamCenter :
                                          Scene::DistToCamPlane );
    const Img8u colorImage = scene.render(0,0,dOut ? &depthImage : 0, dbm);
    
    if(relTM){
      Camera &d = scene.getCamera(0);
      Camera &c = scene.getCamera(1);

      c.setTransformation( *relTM * d.getCSTransformationMatrix() );
     
      const Img8u colorImage2 = scene.render(1);
      if(cOut) colorOut.send(&colorImage2);
      if(prevGUI.isVisible()){
        if(cOut) prevGUI["color"] = colorImage2;
        if(dOut) prevGUI["depth"] = depthImage;
      }
    }else{
      if(cOut) colorOut.send(&colorImage);
      if(prevGUI.isVisible()){
        if(cOut) prevGUI["color"] = colorImage;
        if(dOut) prevGUI["depth"] = depthImage;
      }
    }
    if(dOut) depthOut.send(&depthImage);
    
  }
  gui["draw"].render();
}  


int main(int n, char **v){
  paex("-d","depth image output stream (if given, depth image is exported)")
  ("-c","color image output stream (if given, the color image is exported)")
  ("-o","if given, the given obj-file is loaded into the scene")
  ("-cam","camera for depth rendering (also used for\n"
   "rendering color images if no color camera was given explicitly using -ccam)")
  ("-ccam","optionally given color camera (when the depth camera\n"
   "is moved using mouse input, the color camera will\n"
   "be move in order to make the relative transformations\n"
   "stay the same")
  ("-depth-map-dist-to-cam-center","this can be set in order to change the\n"
   "interpretation of the depth image values");

  return ICLApp(n,v,"-depth-out|-d(2) -color-out|-c(2) "
		"-object|-o(obj-filename) -camera|-cam(camerafile) "
                "-color-camera|-ccam(camerafile) "
                "-depth-map-dist-to-cam-center",init,run).exec();
}
