#include <iclXCFPublisher.h>
#include <iclXCFPublisherGrabber.h>
#include <iclCommon.h>
#include <iclFPSEstimator.h>
#include <iclFPSLimiter.h>
#include <iclIO.h>

#include <iclMedianOp.h>
#include <iclConvolutionOp.h>

std::string uri,stream;
GUI gui("vbox");

bool first = true;
bool *ppEnabled = 0;

GenericGrabber *grabber = 0;

void receive_loop(){
  try{
    static XCFPublisherGrabber g(stream);
    g.setIgnoreDesiredParams(true);
    while(true){
      const ImgBase *image = g.grab();
      static ICLWidget *widget = *gui.getValue<ImageHandle>("image");
      static FPSHandle &fps  = gui.getValue<FPSHandle>("fps");
      fps.update();
      widget->setImage(image);
      widget->updateFromOtherThread();
      Thread::msleep(pa_subarg("-sleep",0,100));
    }
  }catch(XCF::InitializeException &ex){
    ERROR_LOG("exception:" << ex.reason);
  }
}

std::vector<string> remove_size(const vector<string> &v){
  vector<string> r;
  for(unsigned int i=0;i<v.size();++i){
    if(v[i] != "size") r.push_back(v[i]);
  }
  return r;
}  

void init_grabber(){
  grabber = new GenericGrabber(FROM_PROGARG("-input"));

  grabber->setDesiredSize(parse<Size>(pa_subarg<std::string>("-size",0,"VGA")));
  grabber->setIgnoreDesiredParams(false);
  grabber->setDesiredDepth(parse<depth>(pa_subarg<std::string>("-depth",0,"depth8u")));
  if(pa_defined("-dist")){
    grabber->enableDistortion(DIST_FROM_PROGARG("-dist"),
                             parse<Size>(pa_subarg<std::string>("-size",0,"VGA")));
  }
  if(pa_defined("-camera-config")){
    grabber->loadProperties(pa_subarg<string>("-camera-config",0,""),false);
  }  
}

const ImgBase *grab_image(){
  
  const ImgBase *img = 0;
  //  const ImgBase *image = grabber.grab();
  if(!pa_defined("-flip")){
    img = grabber->grab();
  }else{
    ImgBase *hack = const_cast<ImgBase*>(grabber->grab());
    std::string axis = pa_subarg<std::string>("-flip",0,"");
    if(axis  ==   "x"){
      hack->mirror(axisVert);
    }else if(axis  ==  "y"){
      hack->mirror(axisHorz);
    }else if(axis == "both" || axis == "xy"){
      hack->mirror(axisBoth);
    }else{
      ERROR_LOG("nothing known about axis " <<  axis << "(allowed arguments are x,y or both)");
    }
    img = hack;
  }
  
  if(!pa_defined("-clip")){
    return img;
  }else{
    if(pa_subarg<std::string>("-clip",0,"")=="interactive"){
      throw ICLException("interactive clipmode is not yet implemented ...");
    }else{
      static Rect *r = 0;
      static ImgBase *clipped = 0;
      if(!r){
        r = new Rect;
        *r = parse<Rect>(pa_subarg<std::string>("-clip",0,""));
        
        ICLASSERT_THROW(r->width <= img->getWidth(),ICLException("clipping rect width is larger then image width"));
        ICLASSERT_THROW(r->height <= img->getHeight(),ICLException("clipping rect height is larger then image height"));
        ICLASSERT_THROW(r->x>= 0,ICLException("clipping x-offset < 0"));
        ICLASSERT_THROW(r->y>= 0,ICLException("clipping y-offset < 0"));
        ICLASSERT_THROW(r->right() < img->getWidth(),ICLException("clipping rect's right edge is outside the image rect"));
        ICLASSERT_THROW(r->bottom() < img->getHeight(),ICLException("clipping rect's right edge is outside the image rect"));
        clipped = imgNew(img->getDepth(),r->getSize(),img->getChannels(),img->getFormat()); 
      }
      const ImgBase *tmp = img->shallowCopy(*r);
      tmp->deepCopyROI(&clipped);
      delete tmp;
      img = clipped;
    }
  }
  return img;
}

void send_app(){
  static XCFPublisher p(stream,uri);
  ImageHandle IH;
  FPSHandle FPS;
  if(!pa_defined("-no-gui")){
    IH = gui.getValue<ImageHandle>("image");
    FPS= gui.getValue<FPSHandle>("fps");
  }
 
  while(first || !pa_defined("-single-shot")){

    const ImgBase *grabbedImage = grab_image();
    
    const ImgBase *ppImage = 0;
    if(pa_defined("-pp") && *ppEnabled){
      static UnaryOp *pp = 0;
      if(!pp){
        static std::string pps = pa_subarg<std::string>("-pp",0,"");
        if(pps == "gauss"){
          pp = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::gauss3x3));
        }else if(pps == "gauss5") {
          pp = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::gauss5x5));
        }else if(pps == "median"){
          pp = new MedianOp(Size(3,3));
        }else if(pps == "median5"){
          pp = new MedianOp(Size(5,5));
        }else{
          ERROR_LOG("undefined preprocessing mode");
          ::exit(0);
        }
      }
      pp->setClipToROI(false);
      ppImage  = pp->apply(grabbedImage);
    }else{
      ppImage = grabbedImage;
    }

    const ImgBase *normImage = 0;
    if(pa_defined("-normalize")){
      static ImgBase *buf = 0;
      ppImage->deepCopy(&buf);
      buf->normalizeAllChannels(Range64f(0,255));
      normImage = buf;
    }else{
      normImage = ppImage;
    }
    

    p.publish(normImage);
    if(!pa_defined("-no-gui")){
      IH = normImage;
      IH.update();
      FPS.update();
    }
    first = false;
    
    gui_int(fpsLimit);
    static FPSLimiter limiter(15,10);
    if(limiter.getMaxFPS() != fpsLimit) limiter.setMaxFPS(fpsLimit);
    limiter.wait();
  }

}

void receive_app(int n, char **ppc){
  ExecThread x(receive_loop);
  QApplication app(n,ppc);

  gui << "image[@handle=image@minsize=32x24]" << "fps(20)[@size=32x3@handle=fps]";
  gui.show();

  if(!pa_defined("-single-shot")){
    x.run();
  }else{
    receive_loop();
  }
  app.exec();
}

std::string create_camcfg(const std::string&, const std::string &hint){
  return str("camcfg(")+hint+")[@maxsize=5x2]";
}



int main(int n, char **ppc){
  pa_explain("-input","for sender application only allowed ICL default\n"
             " input specificationn e.g. -input pwc 0 or -input file bla/*.ppm");
  pa_explain("-stream","stream name for sender and receiver application (by default: the-stream)");
  pa_explain("-uri","URI for image packages (by default the-uri)");
  pa_explain("-s","sender application (default)");
  pa_explain("-r","receiver application");
  pa_explain("-single-shot","no loop application");
  pa_explain("-size","output image size (sending only, default: VGA)");
  pa_explain("-depth","output image size (sending only, default: depth8u)");

  pa_explain("-fps","initial max FPS count, further adjustable in the GUI");
  pa_explain("-no-gui","dont display a GUI (sender app only)");
  pa_explain("-flip","define axis to flip (allowed sub arguments are"
             " x, y or both");
  pa_explain("-clip","define clip-rect ala ((x,y)WxH) or string interactive (which is not yet supported)");
  pa_explain("-pp","select preprocessing (one of \n"
             "\t- gauss 3x3 gaussian blur\n"
             "\t- gauss5 5x5 gaussian blur\n"
             "\t- median 3x3 median filter\n"
             "\t- median5 5x5 median filter\n");
  pa_explain("-dist","give 4 parameters for radial lens distortion.\n"
             "\tThis parameters can be obtained using ICL application\n"
             "\ticl-calib-radial-distortion");
  pa_explain("-reset","reset bus on startup");
  pa_explain("-normalize","normalize resulting image to [0,255]");
  pa_explain("-camera-config","if a valid xml-camera configuration file was given here, the grabber is set up "
             "with this parameters internally. Valid parameter files can be created with icl-dccam-setup or with "
             "the icl-camcfg tool. Please note: some grabber parameters might make the grabber crash internally, "
             "so e.g. trigger setup parameters or the isospeed parameters must be removed from this file");
  pa_init(n,ppc,"-stream(1) -flip(1) -uri(1) -s -r -single-shot -input(2) -size(1) -no-gui -pp(1) -dist(4) -reset -fps(1) -clip(1) -camera-config(1) -depth(1) -normalize");

  if(pa_defined("-reset")){
    GenericGrabber::resetBus();
  }
  
  uri = pa_subarg<std::string>("-uri",0,"IMAGE");
  stream = pa_subarg<std::string>("-stream",0,"stream");
  
  if(!pa_defined("-r")){
    if(!pa_defined("-no-gui")){
      init_grabber();
      QApplication app(n,ppc);
      ExecThread x(send_app);

      if(pa_defined("-pp")){
         gui << "image[@handle=image@minsize=12x8]" 
            << ( GUI("hbox[@maxsize=100x4]") 
                 << create_camcfg(FROM_PROGARG("-input"))
                 << ("spinner(1,100,"+str(pa_subarg<int>("-fps",0,15))+")[@out=fpsLimit@label=max fps]")
                 << "fps(10)[@handle=fps]"
                 << "togglebutton(off,!on)[@handle=_@out=pp-on@label=preprocessing@minsize=5x2]"
               );
        gui.show();
        ppEnabled = &gui.getValue<bool>("pp-on");
      }else{
        gui << "image[@handle=image@minsize=12x8]" 
            << ( GUI("hbox[@maxsize=100x4]") 
                 << create_camcfg(FROM_PROGARG("-input"))
                 << ("spinner(1,100,"+str(pa_subarg<int>("-fps",0,15))+")[@out=fpsLimit@label=max fps]")
                 << "fps(10)[@handle=fps]"
                );
        gui.show();
      }
    
      x.run();
      return app.exec();
    }else{
      init_grabber();
      send_app();
    }
  }else {
    receive_app(n,ppc);
    init_grabber();
    send_app();
  }
  
}
