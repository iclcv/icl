#include <ICLQuick/Common.h>
#include <ICLFilter/AffineOp.h>
#include <ICLUtils/StackTimer.h>

GUI gui("hsplit[@minsize=32x24]");
Img8u image;

void step(){
  gui_float(scale);
  gui_float(angle);
  gui_bool(clip);
  gui_bool(interp);
  
  AffineOp op(interp ? interpolateNN : interpolateLIN);
  image.setROI(clip?Rect(50,50,200,300):image.getImageRect());
  op.setClipToROI(clip);
    
  op.scale(scale,scale);
  op.rotate(angle*180/M_PI);
  
  static ImgBase *dst = 0;
  op.apply(&image,&dst);

  gui["draw"] = dst;
  gui["draw"].update();
  Thread::msleep(10);
}

void bench(){
  static ImgBase *dst = 0;
  for(int i=0;i<100;++i){
    BENCHMARK_THIS_SECTION(linear interpolation);
    AffineOp op(interpolateLIN);
    image.setFullROI();
    op.scale(1.001,1.001);
    op.rotate(3.6*i);
    op.apply(&image,&dst);
  }
  for(int i=0;i<100;++i){
    BENCHMARK_THIS_SECTION(nn interpolation);
    AffineOp op(interpolateNN);
    image.setFullROI();
    op.scale(1.001,1.001);
    op.rotate(3.6*i);
    op.apply(&image,&dst);
  }
}

void init(){
  gui << "draw[@handle=draw@minsize=32x24]";
  gui << ( GUI("vbox[@maxsize=10x100]") 
           << "fslider(0.1,5,1,vertical)[@out=scale@label=scale@handle=a]"
           << "fslider(0,6.3,0,vertical)[@out=angle@label=angle@handle=b]"
           << "togglebutton(clip,off)[@label=clip ROI@out=clip@handle=c]"
           << "togglebutton(lin,nn)[@label=interp.@out=interp@handle=d]"
           << "button(bench)[@handle=bench]"
          );
  gui.show();
  
  image = cvt8u(scale(create("parrot"),0.4));

  gui.registerCallback(new GUI::Callback(step),"a,b,c,d");
  gui.registerCallback(new GUI::Callback(bench),"bench");
  gui["draw"] = image;
  gui["draw"].update();
}


int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init).exec();
}
