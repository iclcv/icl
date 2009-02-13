#include <iclCommon.h>
#include <iclWarpOp.h>

GUI gui;

void init(){
  gui << "image()[@handle=image@minsize=32x24]";
  gui << ( GUI("hbox[@maxsize=100x3]") 
           << "togglebutton(no,!yes)[@label=enable-warping@handle=warp-h@out=warp]"
           << "togglebutton(nn,lin)[@label=interpolation@handle=__@out=interpolation]"
           << "combo(depth8u,depth16s,depth32s,depth32f,depth64f)[@label=image depth@out=depth]"
           << "fps(10)[@handle=fps@label=FPS]"
           << "label(---ms)[@label=apply time@handle=apply-time]"
         );
  gui.show();
}

void run(){
  static ImageHandle &H = gui.getValue<ImageHandle>("image");
  static bool &warp = gui.getValue<bool>("warp");
  static bool &lin = gui.getValue<bool>("interpolation");
  static GenericGrabber grabber(FROM_PROGARG("-input"));
  static std::string &d = gui.getValue<std::string>("depth");
  static FPSHandle &fps = gui.getValue<FPSHandle>("fps");
  static LabelHandle &l = gui.getValue<LabelHandle>("apply-time");

  grabber.setDesiredSize(Size::VGA);
  grabber.setDesiredFormat(formatRGB);
  grabber.setDesiredDepth(translateDepth(d));

  if(warp){
    static WarpOp op(load(pa_subarg<std::string>("-w",0,"no-input-defined")));
    
    static  ImgBase *dst = 0;
    op.setScaleMode(lin?interpolateLIN:interpolateNN);
    
    Time t = Time::now();
    op.apply(grabber.grab(),&dst);
    l = str((Time::now()-t).toMilliSeconds())+"ms";
    
    H = dst;
    H.update();
  }else{
    H = grabber.grab();
    H.update();
  }
  fps.update();
  Thread::msleep(2);
}

int main(int n, char **ppc){
  pa_init(n,ppc,"-w(1) -input(2)");

  if(!pa_defined("-w") || !pa_defined("-input")){
    pa_usage("please define -i arg");
    exit(-1);
  }
  
  QApplication app(n,ppc);
  ExecThread t(run);  
  init();
  t.run();
  return app.exec();
}
