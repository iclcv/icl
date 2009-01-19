#include <iclXCFPublisher.h>
#include <iclXCFPublisherGrabber.h>
#include <iclProgArg.h>
#include <iclQuick.h>
#include <iclThread.h>
#include <iclQt.h>
#include <iclGenericGrabber.h>
#include <iclFPSEstimator.h>
#include <iclIO.h>
std::string uri = "the-uri";
std::string stream = "the-stream";

GUI gui;

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
    Thread::msleep(pa_subarg("-sleep",0,1000));
    }
  }catch(XCF::InitializeException &ex){
    ERROR_LOG("exception:" << ex.reason);
  }
}


void send_app(){
  try{
  while(first || pa_defined("-loop")){
    static XCFPublisher p(stream,uri);
    Img8u image;
    if(pa_subarg("-source",0,string("create")) == "create"){
      static const Size imageSize = translateSize(pa_subarg<string>("-size",0,"320x240"));
      static Img8u createdImage = cvt8u(scale(create("parrot"),imageSize.width,imageSize.height));
      image = createdImage;
      image.setTime(Time::now());
      labelImage(&image,image.getTime().toString());
    }else{
      static GenericGrabber g("file",string("file=")+pa_subarg<string>("-source",0,"./images/*.ppm"));
      static const Size imageSize = translateSize(pa_subarg<string>("-size",0,"320x240"));
      g.setDesiredSize(imageSize);
      image = const_cast<Img8u&>(*g.grab()->asImg<icl8u>());
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
    first = false;
    Thread::msleep(pa_subarg("-sleep",0,1000));
    static bool showFPS = pa_defined("-fps");
    if(showFPS){
      static const int N = 10; // display each N times
      static int i = 0;
      static FPSEstimator fps(10);
      if(i++ == N){
        i = 0;
        std::cout << "sending with " << fps.getFpsString() << std::endl;
        std::cout << "image was " << image <<  " timestamp" << image.getTime().toString() << std::endl;
        std::cout << "" << std::endl;
      }else{
        fps.getFpsString();
      }
    }    
  }
  }catch(XCF::InitializeException &ex){
    ERROR_LOG("exception:" << ex.reason);
  }

}

void receive_app(int n, char **ppc){
  ExecThread x(receive_loop);
  QApplication app(n,ppc);

  gui << "image[@handle=image@minsize=32x24]" << "fps(20)[@size=32x3@handle=fps]";
  gui.show();

  if(pa_defined("-loop")){
    x.run();
  }else{
    receive_loop();
  }
  app.exec();
}

int main(int n, char **ppc){
  pa_explain("-source","for sender application only allowed values are create|filepattern");
  pa_explain("-streamname","stream name for sender and receiver application (by default: the-stream)");
  pa_explain("-imageuri","URI for image packages (by default the-uri)");
  pa_explain("-s","sender application");
  pa_explain("-r","receiver application");
  pa_explain("-loop","loop application");
  pa_explain("-sleep","sleep time between calls (in ms def=1000)");
  pa_explain("-emulate-mask","emulate 4th channel mask (sending only)");
  pa_explain("-size","output image size (sending only)");
  pa_explain("-fps","display fps while sending");
  pa_init(n,ppc,"-streamname(1) -imageuri(1) -s -r -loop -sleep(1) -source(1) -emulate-mask -size(1) -fps");

  uri = pa_subarg<std::string>("-imageuri",0,"the-uri");
  stream = pa_subarg<std::string>("-streamname",0,"the-stream");
  
  if(pa_defined("-s")){
    send_app();
    
  }else if(pa_defined("-r")){
    receive_app(n,ppc);
  }else{
    pa_usage("please specify -r xor -s");
  }
  
}
