#include <ICLQuick/Common.h>
#include <ICLFilter/ConvolutionOp.h>
#include <sstream>


GUI gui("hsplit");
GenericGrabber *grabber = 0;

void init(){
  GUI controls("vbox[@label=controls]");
  GUI images("vsplit");
  
  images << "draw[@minsize=16x12@handle=src@label=source image]"
         << "draw[@minsize=16x12@handle=dst@label=result image]";
  
  controls << "combo(QQVGA,CGA,QVGA,HVGA,!VGA,SVGA,XGA,HD720,WXGA,SXGA,SXGAP,HD1080)[@label=source size@handle=src-size@out=__0]"
           << "combo(depth8u,depth16s,depth32s,depth32f,depth64f)[@label=source depth@handle=src-depth@out=__1]"
           << "combo(full,ul,ur,ll,rl,center)[@label=source roi@handle=src-roi@out=__2]"
           << "combo(custom,sobelX3x3,sobelY3x3,sobelX5x5,sobelY5x5,!gauss3x3,gauss5x5,laplace3x3,laplace5x5)[@label=kernel-type@handle=kernel-type@out=__3]"
           << "togglebutton(off,on)[@label=force unsigned@out=force-unsigned]"
           << "togglebutton(off,on)[@label=clip to ROI@out=clip-to-roi]"
           << "fps(10)[@handle=fps@label=FPS]"
           << "label(--)[@handle=apply-time@label=apply time]";

  
  // add clip to roi toggle
  
  gui << controls << images;
  gui.show();
}

Rect get_roi(const std::string &info,const Rect &full){
  if(info == "full"){
    return full;
  }
  Size size = full.getSize()/2;

  if(info == "center"){
    return Rect(Point(full.width/4,full.height/4),size);
  }

  Point offs;
  if(info[0] == 'l') offs.y = full.height/2;
  if(info[1] == 'r') offs.x = full.width/2;
  return Rect(offs,size);
}

ConvolutionKernel::fixedType get_kernel(const std::string &name){
#define IF(X) if(name == #X) return ConvolutionKernel::X
  IF(sobelX3x3);IF(sobelY3x3);IF(sobelX5x5);IF(sobelY5x5);IF(gauss3x3);IF(gauss5x5);IF(laplace3x3);IF(laplace5x5);
#undef IF
  return ConvolutionKernel::custom;
}

void run(){
  static DrawHandle src = gui.getValue<DrawHandle>("src");
  static DrawHandle dst = gui.getValue<DrawHandle>("dst");

  static ComboHandle srcSize = gui.getValue<ComboHandle>("src-size");
  static ComboHandle srcDepth = gui.getValue<ComboHandle>("src-depth");
  static ComboHandle srcROI = gui.getValue<ComboHandle>("src-roi");
  static ComboHandle kernel = gui.getValue<ComboHandle>("kernel-type");
  
  static bool &forceUnsigned = gui.getValue<bool>("force-unsigned");
  static bool &clipToROI = gui.getValue<bool>("clip-to-roi");

  static FPSHandle fps = gui.getValue<FPSHandle>("fps");
  static LabelHandle applyTime = gui.getValue<LabelHandle>("apply-time");
  
  static GenericGrabber grabber(FROM_PROGARG("-input"));  
  grabber.setIgnoreDesiredParams(false);

  static ImgBase *dstImage = 0;


  
  grabber.setDesiredSize(parse<Size>(srcSize.getSelectedItem()));
  grabber.setDesiredDepth(parse<depth>(srcDepth.getSelectedItem()));
  
  const ImgBase *grabbedImage = grabber.grab();
  Rect roi = get_roi(srcROI.getSelectedItem(),grabbedImage->getImageRect());
  const ImgBase *roiedImage = grabbedImage->shallowCopy(roi);
  
  ConvolutionOp conv(ConvolutionKernel(get_kernel(kernel.getSelectedItem())),forceUnsigned);
  conv.setClipToROI(clipToROI);
  
  Time t = Time::now();
  conv.apply(roiedImage,&dstImage);
  applyTime = (Time::now()-t).toStringFormated("%Ss %#ms %-us");
  ostringstream sstr; sstr << *dstImage;
  
  src = roiedImage;
  (*src)->lock();
  (*src)->reset();
  (*src)->color(255,0,0,255);
  (*src)->fill(0,0,0,0);
  (*src)->rect(roi.enlarged(-1));
  (*src)->unlock();
  
  dst = dstImage;
  
  (*dst)->lock();
  (*dst)->reset();
  (*dst)->color(255,0,0,255);
  (*dst)->text(sstr.str(),10,10,-1,-1,7);
  (*dst)->unlock();
  
  
  src.update();
  dst.update();
  fps.update();
  
  delete roiedImage;
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input|-i(device,device-params)",init,run).exec();
}

/// there is an error when getting info from 16 bit image ???
/// add text field with output image string
