#include <ICLQuick/Common.h>
#include <ICLFilter/CannyOp.h>
#include <ICLFilter/ConvolutionOp.h>


GUI gui("vsplit");


void update();

void init(){
  gui << "image[@handle=image@minsize=32x24]";
  
  GUI gui2;
  gui2 << "fslider(0,2000,10)[@out=low@label=low@maxsize=100x2@handle=low-handle]";
  gui2 << "fslider(0,2000,100)[@out=high@label=high@maxsize=100x2@handle=high-handle]";
  gui2 <<  ( GUI("hbox")  
           << "togglebutton(off,on)[@out=preGauss@handle=pre-gauss-handle@label=gaussian]"
           << "label(time)[@handle=dt@label=filter time in ms]"
           << "togglebutton(stopped,running)[@out=running@label=capture]"
           << "camcfg()" );

  gui << gui2;

  gui.show();


  gui.registerCallback(new GUI::Callback(update),"low-handle,high-handle,pre-gauss-handle");
  
  update();
}


void update(){
  static Mutex mutex;
  Mutex::Locker l(mutex);

  static GenericGrabber grabber(FROM_PROGARG("-input"));
  grabber.setIgnoreDesiredParams(true);
  
  gui_ImageHandle(image);
  gui_LabelHandle(dt);
  gui_float(low);
  gui_float(high);
  gui_bool(preGauss);
  
  CannyOp canny(low,high,preGauss);
  static ImgBase *dst = 0;

  Time t = Time::now();
  canny.apply(grabber.grab(),&dst);
  
  dt = (Time::now()-t).toMilliSecondsDouble();
  
  image = dst;
  image.update();
}

void run(){
  gui_bool(running);
  while(!running){
    Thread::msleep(100);
  }
  Thread::msleep(1);
  update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input(2)",init,run).exec();
 
  
}
