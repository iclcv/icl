#include <iclXCFPublisher.h>
#include <iclXCFPublisherGrabber.h>
#include <iclCommon.h>
#include <iclFPSEstimator.h>
#include <iclIO.h>
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


void send_app(){
  ImageHandle IH;
  FPSHandle FPS;
  if(!pa_defined("-no-gui")){
    IH = gui.getValue<ImageHandle>("image");
    FPS= gui.getValue<FPSHandle>("fps");
  }
 
  while(first || !pa_defined("-single-shot")){
    static XCFPublisher p(stream,uri);
    static GenericGrabber grabber(FROM_PROGARG("-input"));
    grabber.setDesiredSize(translateSize(pa_subarg<std::string>("-size",0,"VGA")));
    grabber.setIgnoreDesiredParams(false);

    Img8u image;
    grabber.grab()->convert(&image);
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
  pa_init(n,ppc,"-stream(1) -uri(1) -s -r -single-shot -sleep(1) -input(2) -emulate-mask -size(1) -no-gui");

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
