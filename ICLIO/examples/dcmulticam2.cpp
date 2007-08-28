#include <iclDCGrabber.h>
#include <iclTestImages.h>
#include <iclFileWriter.h>

using namespace icl;
using namespace std;

Size size(160,120);

class MyThread {
public:
  MyThread(){
    std::vector<DCDevice> devs = DCGrabber::getDeviceList();
    if(!devs.size()){
      ERROR_LOG("no dc device found -> exiting!");
      exit(-1);
    }
    
    for(unsigned int i=0;i<devs.size();i++){
      gs.push_back(new DCGrabber(devs[i]));
      printf("adding camera %s \n",devs[i].getModelID().c_str());
      gs[gs.size()-1]->setDesiredSize(Size(640,480));
      gs[gs.size()-1]->setDesiredFormat(formatRGB);
      gs[gs.size()-1]->setProperty("format","DC1394_VIDEO_MODE_640x480_MONO8@DC1394_FRAMERATE_30");
    }
    image = new Img8u(Size(devs.size()*640,480),formatRGB);
  }
  
  ~MyThread(){
    for(unsigned int i=0;i<gs.size();i++){
      delete gs[i];
    }
    gs.clear();
  }
  
  virtual void run(){
    while(1){
      for(unsigned int i=0;i<gs.size();i++){
        image->setROI(Rect(i*640,0,640,480));
        gs[i]->grab()->deepCopyROI(&image);
      }
      system("if [ ! -d images ] ; then mkdir images ; fi");
      static icl::FileWriter soWriter("images/Frame###.ppm");
      soWriter.write(image);
    }
  }
  int device;
  std::vector<DCGrabber*> gs;
  ImgBase *image;
};


int main(int nArgs, char **ppcArg){
  
  MyThread *x=new MyThread;
  x->run();
  delete x;

  return 0;
  
}
