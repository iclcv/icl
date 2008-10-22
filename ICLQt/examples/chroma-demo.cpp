#include <iclChromaGUI.h>
#include <iclCommon.h>
#include <iclImgChannel.h>

using namespace icl;
using namespace std;

GUI *gui;
ChromaGUI  *cg;

void run(){
  Size size = Size(320,240);
  GenericGrabber grabber("dc,pwc,file",string("dc=0,pwc=0,file=")+pa_subarg<string>("-file",0,"./images/*.ppm"));

  Img8u segImage(size,1);
  Img8u *image = new Img8u(size,formatRGB);
  ImgBase *imageBase = image;
  
  while(1){
    grabber.grab(&imageBase);
    gui->getValue<ImageHandle>("image") = image;
    gui->getValue<ImageHandle>("image").update();
    
    ImgChannel8u c[3] = { 
      pickChannel(image->asImg<icl8u>(),0),
      pickChannel(image->asImg<icl8u>(),1),
      pickChannel(image->asImg<icl8u>(),2)
    };
    ImgChannel8u s = pickChannel(&segImage,0);
    
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

int main(int nArgs, char **ppcArgs){
  QApplication app(nArgs,ppcArgs);
  pa_init(nArgs, ppcArgs,"-file(1)");
  
  gui = new GUI("hbox");
  (*gui) << ( GUI("vbox")  
              << "image[@minsize=16x12@handle=image@label=Camera Image]" 
              << "image[@minsize=16x12@handle=segimage@label=Semented Image]" );
  (*gui) << "hbox[@handle=box]";
  
  gui->show();

  cg = new ChromaGUI(*gui->getValue<BoxHandle>("box"));

  exec_threaded(run);
  
  return app.exec();
}
