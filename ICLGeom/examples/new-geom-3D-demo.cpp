#include <iclQuick.h>
#include <iclGeom.h>

#include <iclScene2.h>
#include <iclCamera.h>
#include <iclCommon.h>

GUI gui("hsplit");

ImgQ image;
Scene2 scene;
ICLDrawWidget *w;
ICLWidget *w2;
ICLDrawWidget3D *w3D;

struct CameraInteractor : public MouseHandler{
  Camera *camera;
  Point32f anchor;
  Camera camSave;
  CameraInteractor(Camera *camera):camera(camera){}
  
  virtual void process(const MouseEvent &evt){
    Mutex::Locker l(scene);

    if(evt.isPressEvent()){
      anchor = evt.getRelPos();
      camSave = *camera;
    }
    if(evt.isDragEvent()){
      Point32f delta = evt.getRelPos()-anchor;
      *camera = camSave;      
      if(evt.isLeft()){
        camera->transform(create_hom_4x4<float>(delta.x,delta.y,0));
      }else if(evt.isMiddle()){
        camera->translate( (camera->getUp()*delta.y*3) + 
                           (camera->getHorz()*delta.x*3) );
      }else if(evt.isRight()){
        camera->translate(camera->getNorm()*(-delta.y*10));
      }
    }
  }
};


void init(){
  gui << "draw[@minsize=16x12@handle=left@label=Rendered into DrawWidget]";
  gui << "image[@minsize=16x12@handle=right@label=Rendered into ImgQ]";
  gui << "draw3D[@minsize=16x12@handle=gl@label=Rendered into GL-Context]";

  GUI con("vbox[@maxsize=6x100]");
  con << "fslider(0.1,10,1.7,vertical)[@label=F@out=f]";
  
  gui << con;
  gui.show();
  
  w = *gui.getValue<DrawHandle>("left");
  w2 = *gui.getValue<ImageHandle>("right");  
  w3D = *gui.getValue<DrawHandle3D>("gl");
  
  image = ImgQ(Size(640,480),formatRGB);
  w->setImage(&image);
  w2->setImage(&image);
  w3D->setImage(&image);
  
  scene.addCamera(Camera());

  //  static CameraInteractor MouseNavigator(&scene.getCamera(0));
  w->install(scene.getMouseHandler(0));//MouseNavigator);
  w2->install(scene.getMouseHandler(0));
  w3D->install(scene.getMouseHandler(0));

  
  float cubeData[4] = {0,0,0,1};
  scene.addObject(new Object2("cube",cubeData));
  
  //float workTopData[] = {0,0,0,10};
  
  /*
      for(int x=-1;x<2;x++){
      for(int y=-1;y<2;y++){
      for(int z=-1;z<2;z++){
      scene.add(new CubeObject(10*x,10*y,10*z,5));
      }
      }
      }
  */
}


void run(){
  Camera &cam = scene.getCamera(0);
  cam.setViewPort(Rect(0,0,640,480));  

  while(1){
    cam.setFocalLength(gui["f"].as<float>()); /// 1 equals 90° view arc !
    w->lock();
    w->reset();
    
    scene.render(*w,0);
    
    w->unlock();
    w->update();
    
    image.clear();
    scene.render(image,0);
    w2->setImage(&image);
    w2->update();

    Thread::msleep(20);

    w3D->lock();
    w3D->reset3D();
    //w3D->reset();
    w3D->callback(scene.getGLCallback(0));
    //scene.render(*w3D,0);
    w3D->unlock();
    w3D->update();
  }
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}

