#include <iclWidget.h>
#include <iclPWCGrabber.h>
#include <iclUnicapGrabber.h>
#include <QApplication>
#include <iclProgArg.h>
#include <QThread>
#include <iclFileWriter.h>
#include <iclFileReader.h>
#include <string>



using namespace icl;
using namespace std;

class MyThread : public QThread{
public:
  MyThread():
    widget(new ICLWidget(0)),
    grabber(0),
    image(imgNew(translateDepth(pa_subarg("-depth",0,std::string("depth8u"))),
                 Size(640,480),translateFormat(pa_subarg("-format",0,std::string("rgb")))  ))
  {
    if(pa_defined("-input")){
      grabber = new FileReader(pa_subarg("-input",0,std::string("nofile.ppm")));
    }else{
      if(pa_defined("-source")){
        if(pa_subarg("-source",0,std::string("pwc")) == "pwc"){
          grabber = new PWCGrabber(Size(640,480));
        }else{
          std::vector<UnicapDevice> v = UnicapGrabber::getDeviceList();
          for(unsigned int i=0;i<v.size();i++){
            printf("%d = %s \n",i,v[i].getID().c_str());
          }
          
          int dev=-1;
          while(dev < 0 || dev >= (int)v.size()){
            printf("choose device: ");
            scanf("%d",&dev);
            printf("\n");
          }
          grabber = new UnicapGrabber(v[dev]);
          // v[dev].listFormats();
          if(grabber->supportsProperty("size")){
            grabber->setProperty("size","640x480");
          }
          if(grabber->supportsProperty("dma")){
            if(pa_defined("-dma")){
              grabber->setProperty("dma","on");
            }else{ 
              grabber->setProperty("dma","off");
            }
          }
          grabber->setDesiredSize(Size(640,480));
          //grabber->setDesiredDepth(depth32f);
        }
      }else{
        grabber = new PWCGrabber(Size(640,480));
      }
    }
    widget->setGeometry(200,200,640,480);
    widget->show();
  }

  ~MyThread(){
    exit();
    msleep(250);
  }
  virtual void run(){
    FileWriter *w=0;
    if(pa_defined("-file")){
      w = new FileWriter(pa_subarg("-file",0,std::string("./image_##.ppm")));
    }
    delete image;
    image = 0;
    ImgBase *img2 = image;
    while(1){
      widget->setImage(grabber->grab(&img2));
      if(w){
        w->write(image);
      }
      widget->update();
    }
  }
  ICLWidget *widget;
  Grabber *grabber;
  ImgBase *image;
};


int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  pa_init(nArgs,ppcArg,"-format(1) -depth(1) -file(1) -input(1) -source(1) -dma");
  MyThread x;
  x.start();
  return a.exec();
}
