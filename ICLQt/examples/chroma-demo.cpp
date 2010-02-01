#include <ICLQt/ChromaGUI.h>
#include <ICLQuick/Common.h>

using namespace icl;
using namespace std;

GUI *gui;
ChromaGUI  *cg;

void run(){
  Size size = Size(320,240);
  GenericGrabber grabber(FROM_PROGARG("-input"));

  Img8u segImage(size,1);
  Img8u *image = new Img8u(size,formatRGB);
  ImgBase *imageBase = image;
  
  while(1){
    grabber.grab(&imageBase);
    gui->getValue<ImageHandle>("image") = image;
    gui->getValue<ImageHandle>("image").update();
    
    Channel8u c[3]; image->asImg<icl8u>()->extractChannels(c);
    Channel8u s = segImage[0];
    
    ChromaAndRGBClassifier classi = cg->getChromaAndRGBClassifier();
    
    for(int x=0;x<size.width;x++){
      for(int y=0;y<size.height;y++){
        s(x,y) = 255 * classi(c[0](x,y),c[1](x,y),c[2](x,y));
      }
    }
    
    gui->getValue<ImageHandle>("segimage") = &segImage;     
    gui->getValue<ImageHandle>("segimage").update();
    Thread::msleep(40);
  }
}

void init(){
  gui = new GUI("hbox");
  (*gui) << ( GUI("vbox")  
              << "image[@minsize=16x12@handle=image@label=Camera Image]" 
              << "image[@minsize=16x12@handle=segimage@label=Semented Image]" );
  (*gui) << "hbox[@handle=box]";
  
  gui->show();
  
  cg = new ChromaGUI(*gui->getValue<BoxHandle>("box"));

}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}
