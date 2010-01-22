#include <ICLQuick/Common.h>
#include <ICLBlob/SimpleBlobSearcher.h>
#include <ICLUtils/FPSLimiter.h>

GUI gui;
GenericGrabber *grabber;
SimpleBlobSearcher S;

void init(){
  gui << "draw[@minsize=32x24@handle=image]";
  gui.show();
  
  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  
  std::string args[3] = {"-a","-b","-c"};
  for(int i=0;i<3;++i){
    if(pa_defined(args[i])){
      S.add(Color(pa_subarg<int>(args[i],0,0),
                  pa_subarg<int>(args[i],1,0),
                  pa_subarg<int>(args[i],2,0)),
            pa_subarg<float>(args[i],3,0),
            Range32s(pa_subarg<float>(args[i],4,0),
                     pa_subarg<float>(args[i],5,0)));
    }
  }
}

void run(){
  static DrawHandle &h = gui.getValue<DrawHandle>("image");
  static FPSLimiter fps(20);
  
  const Img8u *image = grabber->grab()->asImg<icl8u>();
  
  const std::vector<SimpleBlobSearcher::Blob> &blobs = S.detect(*image);
  
  h = image;

  (*h)->lock();  
  (*h)->reset();
  for(unsigned int i=0;i<blobs.size();++i){
    (*h)->color(blobs[i].refColor[0],blobs[i].refColor[1],blobs[i].refColor[2],255);
    (*h)->linestrip(blobs[i].region->getBoundary());
    (*h)->text(str(blobs[i].refColorIndex), blobs[i].region->getCOG().x,blobs[i].region->getCOG().y);
  }
  (*h)->unlock();  
  h.update();
  fps.wait();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input(2) -a(6) -b(6) -c(6)",init,run).exec();
}
