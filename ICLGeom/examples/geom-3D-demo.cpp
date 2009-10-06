#include <iclQuick.h>
#include <iclGeom.h>

#include <iclScene.h>
#include <iclCamera.h>
#include <iclCubeObject.h>

#include <iclCommon.h>

GUI gui("hsplit");

ImgQ image;
Scene scene(Camera(Vec(0,0,30,0),Vec(0,0,-1,0),Vec(1,0,0,0)));
ICLDrawWidget *w;
ICLWidget *w2;

void init(){
  gui << "draw[@minsize=16x12@handle=left@label=Rendered into DrawWidget]";
  gui << "image[@minsize=16x12@handle=right@label=Rendered into ImgQ]";

  GUI con("vbox[@maxsize=6x100]");
  con << "fslider(0.1,10,1.7,vertical)[@label=F@out=f]";
  
  gui << con;
  gui.show();
  
  w = *gui.getValue<DrawHandle>("left");
  w2 = *gui.getValue<ImageHandle>("right");
  
  image = ImgQ(Size(640,480),formatRGB);
  w->setImage(&image);
  w2->setImage(&image);
  
  
  for(int x=-1;x<2;x++){
    for(int y=-1;y<2;y++){
      for(int z=-1;z<2;z++){
        scene.add(new CubeObject(10*x,10*y,10*z,5));
      }
    }
  }
}


void run(){
  Camera &cam = scene.getCam();
  
  while(1){
    cam.setFocalLength(gui["f"].as<float>()); /// 1 equals 90° view arc !
    w->lock();
    w->reset();
    
    scene.transformAllObjs(create_hom_4x4<float>(0.02,0.03,0));  
    
    for(int x=0;x<3;x++){
      for(int y=0;y<3;y++){
        scene.getCam().setViewPort(Rect(x*640/3,y*480/3,640/3,480/3).enlarged(-10));
        scene.update();
        scene.render(w);
      }
    }
    
    w->unlock();
    w->update();
    
    scene.getCam().setViewPort(Rect(0,0,640,480));
    scene.update();
    image.clear();
    scene.render(&image);
    w2->setImage(&image);
    w2->update();
    Thread::msleep(20);
  }
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}

