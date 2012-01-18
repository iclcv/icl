#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLIO/GenericImageOutput.h>
GUI gui;
Scene scene;
GenericImageOutput output;
GenericGrabber grabber;
Img8u image;


struct ImgObj : public SceneObject{
  Mutex mutex;
  ImgObj(){
    int w = image.getWidth(),h=image.getHeight();

    addVertex(Vec(-w/2,-h/2,0,1));
    addVertex(Vec(w/2,-h/2,0,1));
    addVertex(Vec(w/2,h/2,0,1));
    addVertex(Vec(-w/2,h/2,0,1));

    addTexture(0,1,2,3,&image,pa("-s"));

    
    setVisible(Primitive::vertex | Primitive::line,false);
    setVisible(Primitive::texture,true);
  }
} *obj = 0;

void mouse(const MouseEvent &e){
  if(!&e || e.isLeft() || e.isRight() || e.isMiddle() || e.isWheelEvent()){
    if(pa("-o")){
      output.send(&scene.render(0));
    }
  }
}


void run(){
  if(pa("-s")){
    Thread::msleep(1000);
  }else{
    grabber.grab()->convert(&image);
    if(pa("-o")){
      output.send(&scene.render(0));
    }
    gui["draw"].update();
    Thread::msleep(10);
  }
}

void init(){
  grabber.init(pa("-i"));
  if(pa("-o")){
    output.init(pa("-o"));
  }
  gui << "draw3D()[@handle=draw@minsize=20x15]" << "!show";

  grabber.grab()->convert(&image);  
  obj = new ImgObj;

  scene.addObject(obj);
  if(pa("-r")){
    scene.addCamera(Camera(*pa("-r")));
    scene.getCamera(0).setPosition(Vec(-123.914,18.5966,-633.489,1));
    scene.getCamera(0).setNorm(Vec(0.0202104,-0.00327371,0.99979,1));
    scene.getCamera(0).setUp(Vec(0.999566,-0.0213787,0.0202811,1));
  }else{
    scene.addCamera(Camera(Vec(-123.914,18.5966,-633.489,1),
                           Vec(0.0202104,-0.00327371,0.99979,1),
                           Vec(0.999566,-0.0213787,0.0202811,1)));
  }
  SHOW(scene.getCamera(0));

  scene.getLight(0).setAmbientEnabled(true);
  scene.getLight(0).setAmbient(GeomColor(255,255,255,150));
  
  DrawHandle3D draw = gui["draw"];
  draw->lock();
  draw->reset3D();
  draw->callback(scene.getGLCallback(0));
  draw->unlock();
  draw.update();

  draw->install(scene.getMouseHandler(0));
  if(pa("-s")){
    draw->install(new MouseHandler(mouse));
    mouse( *(MouseEvent*)0 );
  }
}



int main(int n, char **ppc){
  paex("-i","icl typical input specification")
  ("-o","generic image output output specification (e.g. -o sm xyz) writes images to shared memory segment \"xyz\"")
  ("-s","if given, the image will only be grabbed once");

  return ICLApp(n,ppc,"-input|-i(2) -o(2) -single-grab|-s -rendering-camera|-r(camerafile)" ,init,run).exec();
}
