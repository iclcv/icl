#include <iclWidget.h>
#include <iclPWCGrabber.h>
#include <iclUnicapGrabber.h>
#include <QApplication>
#include <iclProgArg.h>
#include <QThread>
#include <iclFileWriter.h>
#include <iclFileReader.h>


using namespace icl;

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
          grabber = new UnicapGrabber("device=/dev/video1394-0");
        }
      }else{
        grabber = new PWCGrabber(Size(640,480));
      }
    }
    widget->setGeometry(200,200,640,480);
    widget->show();
  }
  virtual void run(){
    FileWriter *w=0;
    if(pa_defined("-file")){
      w = new FileWriter(pa_subarg("-file",0,std::string("./image_##.ppm")));
    }
    while(1){
      widget->setImage(grabber->grab(image));
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
  pa_init(nArgs,ppcArg,"-format(1) -depth(1) -file(1) -input(1) -source(1)");
  MyThread x;
  x.start();
  return a.exec();
}
