#include <iclCommon.h>
#include <iclDCGrabber.h>
#include <iclTestImages.h>
#include <iclFileWriter.h>

using namespace icl;
using namespace std;

GUI gui("vsplit");

class MyThread : public Thread{
public:
  MyThread(){
    std::vector<DCDevice> devs = DCGrabber::getDeviceList(pa_defined("-r"));
    if(!devs.size()){
      ERROR_LOG("no dc device found -> exiting!");
      ::exit(-1);
    }

    gui << "image[@handle=image@label=image preview]";
    gui << "fps(10)[@handle=fps@label=current fps@maxsize=100x3]";
    gui.show();
    
    w = *gui.getValue<ImageHandle>("image");    

    static Size size = translateSize(pa_subarg<std::string>("-size",0,"VGA"));
    
    for(unsigned int i=0;i<devs.size();i++){
      gs.push_back(new DCGrabber(devs[i],400,pa_defined("-o")));
      printf("adding camera %s \n",devs[i].getModelID().c_str());

      gs[gs.size()-1]->setDesiredSize(size);
      gs[gs.size()-1]->setDesiredFormat(formatRGB);
      std::string fmt = std::string("DC1394_VIDEO_MODE_640x480_MONO8@DC1394_FRAMERATE_")+pa_subarg<std::string>("-fps",0,"30");
      gs[gs.size()-1]->setProperty("format",fmt);
      if(pa_defined("-t")){
        gs[gs.size()-1]->setProperty("trigger-power","on");
      }else{
        gs[gs.size()-1]->setProperty("trigger-power","off");
      }
    }
    image = new Img8u(Size(devs.size()*size.width,size.height),formatRGB);
  }
  
  ~MyThread(){
    m.lock();
    stop();
    msleep(250);
    for(unsigned int i=0;i<gs.size();i++){
      delete gs[i];
    }
    delete w;
    gs.clear();
    m.unlock();
  }
  
  virtual void run(){
    static FPSHandle &fps = gui.getValue<FPSHandle>("fps");
    static Size size = translateSize(pa_subarg<std::string>("-size",0,"VGA"));
    bool write = pa_defined("-output");
    static std::string prfx = pa_subarg<std::string>("-o",0,"/tmp/images/");

    system((std::string("mkdirhier ") + prfx).c_str());
    Thread::sleep(0.5);

    while(1){

      fps.update();

      m.lock();
      for(unsigned int i=0;i<gs.size();i++){
        const ImgBase *cim = gs[i]->grab();
        image->setROI(Rect(i*size.width,0,size.width,size.height));
        cim->deepCopyROI(&image);
        
        if(write){

          std::string time = cim->getTime().toString();
          for(unsigned int j=0;j<time.length();++j){
            char &c = time[j];
            if(c == ' ') c = '_';
            else if(c == '/') c = '-';
          }
          std::string filename = prfx+"/image--"+str(i)+"--"+time+".ppm";
          try{
            FileWriter(filename).write(cim);
          }catch(...){
            ERROR_LOG("unable to write image... filename: " + filename);
          }
        }        
      }
      //  static icl::FileWriter soWriter("Frame###.ppm");
      // soWriter.write(image);
      w->setImage(image);
      w->update();
      m.unlock();
      msleep(gs.size()*10);
    }
  }
  int device;
  ICLWidget* w;
  std::vector<DCGrabber*> gs;
  Mutex m;
  ImgBase *image;
};


int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  pa_explain("-size","grabbing size");
  pa_explain("-output","write grabbed into file (parameter is the directory)");
  pa_explain("-r","reset dc bus first");
  pa_explain("-o","omit double frames while grabbing");
  pa_explain("-fps","grabbers fps value");
  pa_explain("-t","use external trigger");
  pa_init(nArgs,ppcArg,"-size(1) -output(1) -r -o -fps(1) -t");
  
  MyThread *x=new MyThread;
  x->start();
  a.exec();
  delete x;

  return 0;
  
}
