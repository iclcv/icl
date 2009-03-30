#include <iclCommon.h>
#include <iclDrawWidget3D.h>
#include <iclFPSEstimator.h>
using namespace icl;
using namespace std;

Size size(320,240);
ICLDrawWidget3D *widget = 0;

void init(){
  widget = new ICLDrawWidget3D(0);
  widget->setGeometry(200,200,640,480);
  widget->show();
}
void run(){
  GenericGrabber g(FROM_PROGARG_DEF("-input","pwc","0"));
  g.setDesiredSize(size);
  
  static float rz = 2;    
  static float ry = 1;
  while(1){
    
    const ImgBase *image = g.grab();
    widget->setImage(image);
    
    widget->lock();
    widget->reset3D();
    
    widget->rotate3D(0,ry,rz);
    
    rz += 0.4;
    ry += 0.8;
    
    widget->color3D(1, 1, 1, 1);
    
    widget->imagecube3D(0,0,0,0.5,image);
    widget->supercube3D(0.2,0,0,0.5);
    // widget->scale3D(0.3,0.3,0.3);
    // widget->translate3D(-0.5,-0.5,0);
    //widget->image3D(0,0,0,640.0/480.0,0,0,0,1,0,image);
    
    
    // 2D Stuff
    widget->reset();
    widget->color(255,0,0,100);
    widget->fill(255,0,0,50);
    widget->rel();
    widget->rect(0.01,0.01,0.5,0.05);
    widget->color(255,255,255,200);
    widget->fill(255,255,255,200);
    static FPSEstimator fps(10);
    
    widget->text(fps.getFPSString(),0.02,0.02,0.2,0.03);
    
    widget->unlock();
    
    
    
    widget->update();
    Thread::msleep(1);
  }
}



int main(int n, char **ppc){
  pa_explain("-input","e.g. -input dc 0 or -input file images/*.ppm1");
  pa_init(n,ppc,"-input(2)");
  ExecThread x(run);
  QApplication a(n,ppc);
  init();
  x.run();
  
  return a.exec();
  
}
