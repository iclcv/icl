#include <iclXCFPublisher.h>
#include <iclXCFPublisherGrabber.h>
#include <iclCommon.h>
#include <iclFPSEstimator.h>
#include <iclIO.h>

#include <iclMedianOp.h>
#include <iclConvolutionOp.h>

std::string uri,stream;
GUI gui("vbox");

bool first = true;

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

const ImgBase *grab_image(){
  static GenericGrabber grabber(FROM_PROGARG("-input"));
  bool first = true; 
  if(first){
    first = false;
    grabber.setDesiredSize(translateSize(pa_subarg<std::string>("-size",0,"VGA")));
    grabber.setIgnoreDesiredParams(false);
    if(pa_defined("-dist")){
      grabber.enableDistortion(DIST_FROM_PROGARG("-dist"),
                               translateSize(pa_subarg<std::string>("-size",0,"VGA")));
    }
  }
  const ImgBase *image = grabber.grab();
  return grabber.grab();
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

    Img8u image;
    if(pa_defined("-pp")){
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
      pp->apply(grab_image(),bpp(&image));
    }else{
      grab_image()->convert(&image);
    }


    if(pa_defined("-emulate-mask")){

      static Img8u mask;
      static bool first2 = true;
      if(first2){
        first2 = false;
        static ImgQ q = zeros(image.getWidth(),image.getHeight(),1);
        color(255,255,255,255);
        fill(255,255,255,255);
        Rect r = q.getImageRect();
        circle(q,r.center().x,r.center().y,r.height/2-5);
        mask = cvt8u(q);
        show(q);
      }
      image.append(&mask,0);
    }


    p.publish(&image);
    if(!pa_defined("-no-gui")){
      IH = image;
      IH.update();
      FPS.update();
    }
    first = false;
    static int sleep = pa_subarg("-sleep",0,100);
    Thread::msleep(sleep);
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
  pa_explain("-sleep","sleep time between calls (in ms def=100)");
  pa_explain("-emulate-mask","emulate 4th channel mask (sending only)");
  pa_explain("-size","output image size (sending only, default: VGA)");
  pa_explain("-no-gui","dont display a GUI (sender app only)");
  pa_explain("-pp","select preprocessing (one of \n"
             "\t- gauss 3x3 gaussian blur\n"
             "\t- gauss5 5x5 gaussian blur\n"
             "\t- median 3x3 median filter\n"
             "\t- median5 5x5 median filter\n");
  pa_explain("-dist","give 4 parameters for radial lens distortion.\n"
             "\tThis parameters can be obtained using ICL application\n"
             "\ticl-calib-radial-distortion");
  pa_explain("-reset","reset bus on startup");
  pa_init(n,ppc,"-stream(1) -uri(1) -s -r -single-shot -sleep(1) -input(2) -emulate-mask -size(1) -no-gui -pp(1) -dist(4) -reset");

  if(pa_defined("-reset")){
    GenericGrabber::resetBus();
  }
  
  uri = pa_subarg<std::string>("-uri",0,"IMAGE");
  stream = pa_subarg<std::string>("-stream",0,"stream");
  
  if(!pa_defined("-r")){
    if(!pa_defined("-no-gui")){
      QApplication app(n,ppc);
      ExecThread x(send_app);
      gui << "image[@handle=image@minsize=12x8]" 
          << ( GUI("hbox[@maxsize=100x2]") 
               << create_camcfg(FROM_PROGARG("-input"))
               << "fps(10)[@handle=fps]"
               );
      gui.show();
      x.run();
      return app.exec();
    }else{
      send_app();
    }
  }else {
    receive_app(n,ppc);
    
    send_app();
  }
  
}
