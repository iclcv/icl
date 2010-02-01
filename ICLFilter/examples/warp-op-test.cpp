#include <ICLQuick/Common.h>
#include <ICLFilter/WarpOp.h>

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
  grabber.setDesiredDepth(parse<depth>(d));

  if(warp){
    static WarpOp op(icl::load(pa("-w")));
    
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
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "[m]-warp-table|-w(filename)",init,run).exec();
}
