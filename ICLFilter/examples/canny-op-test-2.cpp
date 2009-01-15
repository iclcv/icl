#include <iclCannyOp.h>
#include <iclCommon.h>
#include <iclConvolutionOp.h>
#include <iclCC.h>

GUI gui("vsplit");

void init(){
  gui << "image[@handle=image@minsize=32x24]";
  gui << ( GUI() 
           <<  "fslider(0,2000,10)[@out=low@label=low@maxsize=100x2@handle=low-handle]"
           << "fslider(0,2000,100)[@out=high@label=high@maxsize=100x2@handle=high-handle]"
           << "togglebutton(off,on)[@out=pre-gauss@handle=pre-gauss-handle@label=gaussian PP])"
           << "label(time)[@handle=dt@label=filter time in ms]"
          );
  gui.show();
}


void update(){
  static GenericGrabber grabber(FROM_PROGARG("-input"));
  grabber.setIgnoreDesiredParams(true);
  static ImageHandle image = gui.getValue<ImageHandle>("image");
  static LabelHandle dt = gui.getValue<LabelHandle>("dt");
  float &low = gui.getValue<float>("low");
  float &high = gui.getValue<float>("high");
  bool &preGauss = gui.getValue<bool>("pre-gauss");
  
  static ImgBase *depthAdapted = 0;
  
  CannyOp canny(low,high,preGauss);
  static ImgBase *dst = 0;

  Time t = Time::now();
  const ImgBase *im= grabber.grab();
  if(!pa_defined("-format")){
    canny.apply(im,&dst);
  }else{
    static std::string fmtStr = pa_subarg<std::string>("-format",0,"rgb");
    ensureCompatible(&depthAdapted,depth8u,im->getSize(),translateFormat(fmtStr));
    cc(im,depthAdapted);
    canny.apply(depthAdapted,&dst);
  }
  dt = (Time::now()-t).toMilliSecondsDouble();
  
  image = dst;
  image.update();
}


int main(int n, char **ppc){
  pa_init(n,ppc,"-input(2) -format(1)");
  QApplication app(n,ppc);
  
  init();
  update();
  gui.registerCallback(new GUI::Callback(update),"low-handle,high-handle,pre-gauss-handle");
  
  return app.exec();
  
}
