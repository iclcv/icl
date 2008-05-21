#include <iclXCFPublisher.h>
#include <iclXCFPublisherGrabber.h>
#include <iclProgArg.h>
#include <iclQuick.h>
#include <iclThread.h>

const std::string &uri = "the-uri";
const std::string &stream = "the-stream";

bool first = true;
void send_app(){

  while(first || pa_defined("-loop")){
    DEBUG_LOG("send 1");
    static XCFPublisher p(stream,uri);
    DEBUG_LOG("send 2");
    
    static ImgQ image = scale(create("parrot"),0.3);
    DEBUG_LOG("send 3");
    p.publish(&image);
    DEBUG_LOG("send done");

    first = false;
    Thread::msleep(1000);
  }
}

void receive_app(){
  while(first || pa_defined("-loop")){
    DEBUG_LOG("receive 1");
    static XCFPublisherGrabber g(stream);
    DEBUG_LOG("receive 2");
    const ImgBase *image = g.grab();
    DEBUG_LOG("receive 3");
    
    show(cvt(image));
    DEBUG_LOG("receive done");
    first = false;
    Thread::msleep(1000);
  }
}

int main(int n, char **ppc){
  pa_explain("-s","sender application");
  pa_explain("-r","receiver application");
  pa_explain("-loop","loop application");
  pa_init(n,ppc,"-s -r -loop");

  if(pa_defined("-s")){
    send_app();
    
  }else if(pa_defined("-r")){
    receive_app();
  }else{
    pa_usage("please specify -r xor -s");
  }
  
}
