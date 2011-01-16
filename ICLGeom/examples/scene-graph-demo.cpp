#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLUtils/FPSLimiter.h>

GUI gui;
Scene scene;

float p_cube[] = {0,0,0,1};

struct SphereSceneObject : public SceneObject{
  SphereSceneObject(float radius, const GeomColor &color, int na=50, int nb=40){
    SceneObject *s = addSphere(0,0,0,radius,na,nb);
    s->setVisible(Primitive::vertex,false);
    s->setVisible(Primitive::line,false);
    s->setColor(Primitive::quad,color);
  } 
};


struct Planet : public SphereSceneObject{
  float orbit;
  float speed;
  Time startTime;
  Planet(float radius, const GeomColor &color, float orbit, float speed):
    SphereSceneObject(radius,color),orbit(orbit),speed(speed),startTime(Time::now()){
  }
  void prepareForRendering(){
    float dt = (Time::now()-startTime).toSecondsDouble();
    removeTransformation();
    translate(orbit,0,0);
    rotate(0,0,dt*speed);
  }
};

void init(){
  gui << "draw3D()[@minsize=32x24@handle=view]" << "!show";
  
  scene.addCamera(Camera(Vec(0,0,600,1)));
  scene.addObject(new Planet(40,GeomColor(255,255,100,255),0,0.5));
  scene.addObject(new Planet(20,GeomColor(255,200,200,255),300,1));
  scene.addObject(new Planet(22,GeomColor(255,200,200,255),240,1.23));
  Planet *p = new Planet(22,GeomColor(10,244,200,255),200,2.0);
  p->addChild(new Planet(10,GeomColor(255,10,60,255),80,3.5));
  p->addChild(new Planet(8,GeomColor(255,60,10,255),60,2.1));
  p->addChild(new Planet(8,GeomColor(255,60,255,255),43,1.84));
  scene.addObject(p);


  gui["view"].install(scene.getMouseHandler(0));
  
  ICLDrawWidget3D *d = gui["view"];
  d->setProperty("light-1","on");
  d->setProperty("light-1-pos","{100,100,100,1}");
  d->setProperty("light-1-diffuse","{0,0.5,0.9,1.0}");
  Img8u bg(Size::VGA,1);
  d->setImage(&bg);
}

void run(){
  ICLDrawWidget3D *d = gui["view"];
  static FPSLimiter fps(100,10);
  std::string fpsString = fps.getFPSString();
  d->lock();
  d->reset3D();
  d->reset();
  d->callback(scene.getGLCallback(0));
  d->color(255,0,0,255);
  d->text(fpsString,550,10,8);
  d->unlock();
  d->updateFromOtherThread();
}

int main(int n, char **argv){
  return ICLApp(n,argv,"",init,run).exec();
}
