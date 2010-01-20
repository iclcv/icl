#include <ICLGeom/Geom.h>
#include <ICLQuick/Common.h>

GUI gui("hsplit");
Size size = Size::VGA;
Scene scene(Camera(Vec(0,0,-10,0),Vec(0,0,1,0),Vec(1,0,0,0)));

void init(){
  gui << "draw[@minsize=16x12@handle=image]";

  GUI con("vbox");
  con << "fslider(0.1,10,1.7)[@label=F@out=f]";
  con << ( GUI("vbox[@label=rotation]")
           << "fslider(0,360,0)[@label=x@out=rx]"
           << "fslider(0,360,0)[@label=y@out=ry]"
           << "fslider(0,360,0)[@label=z@out=rz]"
         )
      << ( GUI("vbox[@label=position]")
           << "fslider(-20,20,0)[@label=x@out=x]"
           << "fslider(-20,20,0)[@label=y@out=y]"
           << "fslider(-20,20,-10)[@label=z@out=z]"
        );

  gui << con;
  gui.show();
  
  gui["image"] = Img8u(size,1);
  gui["image"].update();
  
  
  scene.add(new CubeObject(0,0,0,1));
}


void run(){
  static float &x = gui.getValue<float>("x");
  static float &y = gui.getValue<float>("y");
  static float &z = gui.getValue<float>("z");
  static float &rx = gui.getValue<float>("rx");
  static float &ry = gui.getValue<float>("ry");
  static float &rz = gui.getValue<float>("rz");
  static float &f = gui.getValue<float>("f");

  static ICLDrawWidget *w = *gui.getValue<DrawHandle>("image");
  w->lock();
  w->reset();
  
  scene.setCam(Camera(Vec(x,y,z),Vec(rx,ry,rz)*(M_PI/180),size,f));
  scene.update();
  scene.render(w);

  w->unlock();
  w->update();
  
  Thread::msleep(20);
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}

