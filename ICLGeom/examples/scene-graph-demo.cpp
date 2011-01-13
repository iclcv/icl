#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>

GUI gui;
Scene scene;

float p_cube[] = {0,0,0,1};

struct SphereSceneObject : public SceneObject{
  SphereSceneObject(float radius, float cubeRadius, const GeomColor &color, int na=30, int nb=20):
    SceneObject("cube",p_cube){
    for(float i=0;i<na;++i){
      for(float j=0;j<nb;++j){
        float alpha = i/na * 2 * M_PI;
        float beta = j/nb * M_PI;
        const float p[4] = { 
          radius*cos(alpha)*sin(beta),
          radius*sin(alpha)*sin(beta),
          radius*cos(beta),
          cubeRadius 
        };
        SceneObject *c = new SceneObject("cube",p);
        c->setVisible(Primitive::vertex,false);
        c->setVisible(Primitive::line,false);
        c->setColor(Primitive::quad,color);
        addChild(c);
      }
    }
  } 
};


struct Planet : public SphereSceneObject{
  float orbit;
  float speed;
  Time startTime;
  Planet(float radius, const GeomColor &color, float orbit, float speed):
    SphereSceneObject(radius,radius/10,color),orbit(orbit),speed(speed),startTime(Time::now()){
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
  
  scene.addCamera(Camera(Vec(0,0,1000,1)));
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
}

void run(){
  ICLDrawWidget3D *d = gui["view"];
  d->lock();
  d->reset3D();
  d->callback(scene.getGLCallback(0));
  d->unlock();
  
  d->updateFromOtherThread();
  
}

int main(int n, char **argv){
  return ICLApp(n,argv,"",init,run).exec();
}
