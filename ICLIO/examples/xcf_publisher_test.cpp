#include <iclXCFPublisher.h>
#include <iclXCFPublisherGrabber.h>
#include <iclProgArg.h>
#include <iclQuick.h>
#include <iclThread.h>
#include <iclQt.h>
#include <iclGenericGrabber.h>
std::string uri = "the-uri";
std::string stream = "the-stream";

GUI gui;

bool first = true;

void receive_loop(){
  static XCFPublisherGrabber g(stream);
  while(true){
    const ImgBase *image = g.grab();
    static ICLWidget *widget = *gui.getValue<ImageHandle>("image");
    static FPSHandle &fps  = gui.getValue<FPSHandle>("fps");
    fps.update();
    widget->setImage(image);
    widget->updateFromOtherThread();
    Thread::msleep(pa_subarg("-sleep",0,1000));
  }
}

void send_app(){

  while(first || pa_defined("-loop")){
    static XCFPublisher p(stream,uri);
    Img8u image;
    if(pa_subarg("-source",0,string("create")) == "create"){;
      image = cvt8u(scale(create("parrot"),0.3));
    }else{
      static GenericGrabber g("file",string("file=")+pa_subarg<string>("-source",0,"./images/*.ppm"));
      image = const_cast<Img8u&>(*g.grab()->asImg<icl8u>());
    }
    p.publish(&image);
    first = false;
    Thread::msleep(pa_subarg("-sleep",0,1000));
  }
}

void receive_app(int n, char **ppc){
  
  QApplication app(n,ppc);

  gui << "image[@handle=image@minsize=32x24]" << "fps(20)[@size=32x3@handle=fps]";
  gui.show();

  if(pa_defined("-loop")){
    exec_threaded(receive_loop);
  }else{
    receive_loop();
  }
  app.exec();
}

int main(int n, char **ppc){
  pa_explain("-source","for sender application only allowed values are create|filepattern");
  pa_explain("-streamname","stream name for sender and receiver application");
  pa_explain("-imageuri","URI for image packages");
  pa_explain("-s","sender application");
  pa_explain("-r","receiver application");
  pa_explain("-loop","loop application");
  pa_explain("-sleep","sleep time between calls (in ms def=1000)");
  pa_init(n,ppc,"-s -r -loop -sleep(1) -source(1)");

  std::string uri = pa_subarg<std::string>("-imageuri",0,"the-uri");
  std::string stream = pa_subarg<std::string>("-streamname",0,"the-stream-name");
  
  if(pa_defined("-s")){
    send_app();
    
  }else if(pa_defined("-r")){
    receive_app(n,ppc);
  }else{
    pa_usage("please specify -r xor -s");
  }
  
}
