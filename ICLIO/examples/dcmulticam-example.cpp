#include <iclCommon.h>
#include <iclDCGrabber.h>
#include <iclTestImages.h>
#include <iclFileWriter.h>

using namespace icl;
using namespace std;

GUI gui("vsplit");

int device;
ICLWidget* w;
std::vector<DCGrabber*> gs;
Mutex m;
ImgBase *image;

void init(){
  std::vector<DCDevice> devs = DCGrabber::getDeviceList(pa_defined("-r"));
  if(!devs.size()){
    ERROR_LOG("no dc device found -> exiting!");
    ::exit(-1);
  }
  
  gui << "image[@handle=image@label=image preview@minsize=20x12]";
  gui << ( GUI ("hbox") 
           << "fps(10)[@handle=fps@label=current fps@maxsize=100x3]"
           << "camcfg(dc)"
           );
  gui.show();
  
  w = *gui.getValue<ImageHandle>("image");    
  
  static Size size = parse<Size>(pa_subarg<std::string>("-size",0,"VGA"));
  
  for(unsigned int i=0;i<devs.size();i++){
    gs.push_back(new DCGrabber(devs[i]));
    printf("adding camera %s \n",devs[i].getModelID().c_str());
    
    gs[gs.size()-1]->setDesiredSize(size);
    gs[gs.size()-1]->setDesiredFormat(formatRGB);
    std::string fmt = std::string("640x480-MONO8@")+pa_subarg<std::string>("-fps",0,"30")+"Hz";
    gs[gs.size()-1]->setProperty("format",fmt);
    if(pa_defined("-t")){
      gs[gs.size()-1]->setProperty("trigger-power","on");
    }else{
      gs[gs.size()-1]->setProperty("trigger-power","off");
    }
  }
  image = new Img8u(Size(devs.size()*size.width,size.height),formatRGB);
}

struct CleanUp{
  ~CleanUp(){
    for(unsigned int i=0;i<gs.size();i++){
      delete gs[i];
    }
  }
} CU;

void run(){
    static FPSHandle &fps = gui.getValue<FPSHandle>("fps");
    static Size size = parse<Size>(pa_subarg<std::string>("-size",0,"VGA"));
    static bool write = pa_defined("-output");
    static std::string prfx = pa_subarg<std::string>("-o",0,"/tmp/images/");
    
    static bool first = true;
    if(first){
      first = false;
      system((std::string("mkdirhier ") + prfx).c_str());
      Thread::sleep(0.5);
    }

    
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
    Thread::msleep(gs.size()*10);
}


int main(int n, char **ppc){

  ExecThread x(run);
  QApplication app(n,ppc);
  init();

  pa_explain("-size","grabbing size");
  pa_explain("-output","write grabbed into file (parameter is the directory)");
  pa_explain("-r","reset dc bus first");
  pa_explain("-fps","grabbers fps value");
  pa_explain("-t","use external trigger");
  pa_init(n,ppc,"-size(1) -output(1) -r -fps(1) -t");

  x.run();
  return app.exec();
  
}
