#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>

GUI gui("hbox");
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
    SceneObject *s = addSphere(0,0,0,radius,30,30);
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

struct PositionIndicator : public SceneObject{
  PositionIndicator(){
    addChild(new ComplexCoordinateFrameSceneObject());
    addVertex(Vec(0,0,120,1));
    addText(0,"pos is ...");
    setLockingEnabled(true);
  }
  void update(const Vec &w, const Vec &c){
    lock();
    removeTransformation();
    translate(w);
    //    m_primitives.clear();
    //addText(0,str(c.transp()));
    ((TextPrimitive*)m_primitives[0])->updateText(str(c.transp()));
    unlock();
  }
} *pos;


MouseHandler *sceneHandler = 0;
void mouse(const MouseEvent &evt){
  if(evt.isModifierActive(ShiftModifier) || 
     evt.isModifierActive(AltModifier) ||
     evt.isModifierActive(ControlModifier)){
    if(evt.isLeft() && evt.isPressEvent()){
      scene.lock();
      Hit h = scene.findObject(0, evt.getX(), evt.getY());
      if(h){
        Vec p = h.pos;
        Mat T = scene.getCamera(0).getCSTransformationMatrix();
        pos->update(p,T*p);
      }
      scene.unlock();
    }
  }else{
    sceneHandler->process(evt);
    //Vec p = pos->getTransformation().part<3,0,1,4>();
    //Mat T = scene.getCamera(0).getCSTransformationMatrix();
    //pos->update(p,T*p);
  }
}
void mouse_2(const MouseEvent &evt){
  static DrawHandle draw = gui["image"];
  if(evt.isLeft() && evt.isPressEvent()){
    std::vector<double> c = evt.getColor();
    if(c.size()){
      draw->lock();
      draw->reset();
      draw->color(255,0,0);
      draw->sym(evt.getPos(),'x');
      draw->text(str(c[0]),evt.getX(), evt.getY(), 10);
      draw->unlock();
      // draw.update();
    }
  }
}
//void capture(){
//  static ImgQ bg = create("parrot");
//  show(scene.render(0,&bg));
//}

void init(){
  gui << "draw3D()[@minsize=32x24@handle=view]" 
      << "draw[@handle=image@minsize=32x24]"
      << (GUI("vbox") 
          << "combo(none,rgb,depth)[@handle=capture@label=offscreen rendering]"
          << "combo(raw,dist. to z0,dist to cam center)[@handle=dmode@label=depth map mode]"
         )
      << "!show";
  
  //gui["capture"].registerCallback(new GUI::Callback(capture));

  scene.addCamera(Camera(Vec(567,12,215,1),
                         Vec(-0.937548,-0.0012938,-0.347854,1),
                         Vec(0.333029,0.286923, -0.898202,1)));
  scene.getCamera(0).getRenderParams().clipZFar = 1000;
  scene.addObject(new Planet(40,GeomColor(150,150,50,255),0,0.5));

  scene.addObject(new Planet(20,GeomColor(255,200,200,255),300,1));
  scene.addObject(new Orbit(300,GeomColor(255,200,200,255)));
  scene.addObject(new Planet(22,GeomColor(255,200,200,255),240,1.23));
  scene.addObject(new Orbit(240,GeomColor(255,200,200,255)));
  Planet *p = new Planet(22,GeomColor(255,10,10,255),200,2.0);
  p->addChild(new Planet(10,GeomColor(255,10,60,255),80,3.5));
  p->addChild(new Orbit(80,GeomColor(255,10,60,255)));
  p->addChild(new Planet(8,GeomColor(255,60,10,255),60,2.1));
  p->addChild(new Orbit(60,GeomColor(255,60,10,255)));
  p->addChild(new Planet(8,GeomColor(255,60,255,255),43,1.84));
  p->addChild(new Orbit(43,GeomColor(255,60,255,255)));
  scene.addObject(p);
  scene.addObject(new Orbit(200,GeomColor(10,244,200,255)));

  pos = new PositionIndicator;
  scene.addObject(pos);

  sceneHandler = scene.getMouseHandler(0);
  gui["view"].install(new MouseHandler(mouse));
  gui["image"].install(new MouseHandler(mouse_2));
  
  scene.getLight(0).setDiffuse(GeomColor(10,10,255,100));

  scene.getLight(1).setOn(true);
  scene.getLight(1).setDiffuse(GeomColor(255,100,100,255));
  scene.getLight(1).setDiffuseEnabled();
  scene.getLight(1).setPosition(Vec(0,0,60,1));
  scene.getLight(1).setAnchor(p);

  scene.getLight(2).setOn(true);
  scene.getLight(2).setDiffuse(GeomColor(255,100,100,255));
  scene.getLight(2).setDiffuseEnabled();
  scene.getLight(2).setPosition(Vec(0,0,-60,1));
  scene.getLight(2).setAnchor(p);

  scene.setDrawCoordinateFrameEnabled(true,400,10);
  
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
  
  int capture = gui["capture"];
  static Img32f db;
  switch(capture){
    case 1:
      gui["image"] = scene.render(0);
      gui["image"].update();
      break;
    case 2:
      scene.render(0,0,&db,(Scene::DepthBufferMode)gui["dmode"].as<int>());
      gui["image"] = db;
      gui["image"].update();
      break;
    default:
      break;
  }
}

int main(int n, char **argv){
  return ICLApp(n,argv,"",init,run).exec();
}
