#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLUtils/FPSLimiter.h>

GUI gui;
Scene scene;
struct Orbit : public SceneObject{
  Orbit(float orbit, const GeomColor &c){
    for(float a=0;a<2*M_PI;a+=0.01){
      addVertex(Vec(orbit*cos(a),orbit*sin(a),0,1),c);
    }
  }
};

struct Planet : public SceneObject{
  float orbit;
  float speed;
  Time startTime;
  Planet(float radius, const GeomColor &color, float orbit, float speed):
    orbit(orbit),speed(speed),startTime(Time::now()){
    SceneObject *s = addSphere(0,0,0,radius,20,20);
    s->setVisible(Primitive::vertex,false);
    s->setVisible(Primitive::line,false);
    s->setColor(Primitive::quad,color);   
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
  
  scene.addCamera(Camera(Vec(567,12,215,1),
                         Vec(-0.937548,-0.0012938,-0.347854,1),
                         Vec(0.333029,0.286923, -0.898202,1)));
  scene.addObject(new Planet(40,GeomColor(255,255,100,255),0,0.5));

  scene.addObject(new Planet(20,GeomColor(255,200,200,255),300,1));
  scene.addObject(new Orbit(300,GeomColor(255,200,200,255)));
  scene.addObject(new Planet(22,GeomColor(255,200,200,255),240,1.23));
  scene.addObject(new Orbit(240,GeomColor(255,200,200,255)));
  Planet *p = new Planet(22,GeomColor(10,244,200,255),200,2.0);
  p->addChild(new Planet(10,GeomColor(255,10,60,255),80,3.5));
  p->addChild(new Orbit(80,GeomColor(255,10,60,255)));
  p->addChild(new Planet(8,GeomColor(255,60,10,255),60,2.1));
  p->addChild(new Orbit(60,GeomColor(255,60,10,255)));
  p->addChild(new Planet(8,GeomColor(255,60,255,255),43,1.84));
  p->addChild(new Orbit(43,GeomColor(255,60,255,255)));
  scene.addObject(p);
  scene.addObject(new Orbit(200,GeomColor(10,244,200,255)));


  gui["view"].install(scene.getMouseHandler(0));
  
  ICLDrawWidget3D *d = gui["view"];
  d->setProperty("light-1","on");
  d->setProperty("light-1-pos","{100,100,100,1}");
  d->setProperty("light-1-diffuse","{0,0.5,0.9,0.3}");

  d->setProperty("light-2","on");
  d->setProperty("light-2-pos","{-100,-100,-100,1}");
  d->setProperty("light-2-diffuse","{1.0,0.2,0.1,0.2}");
  Img8u bg(Size::VGA,1);
  d->setImage(&bg);
}

void run(){
  ICLDrawWidget3D *d = gui["view"];
  static FPSLimiter fps(100,10);
  std::string fpsString = fps.getFPSString(); // wait outside widget lock!
  d->lock();
  d->reset3D();
  d->reset();
  d->callback(scene.getGLCallback(0));
  d->color(255,0,0,255);
  d->text(fpsString,550,10,8);
  d->unlock();
  d->updateFromOtherThread();
  
  SHOW(scene.getCamera(0));
}

int main(int n, char **argv){
  return ICLApp(n,argv,"",init,run).exec();
}
