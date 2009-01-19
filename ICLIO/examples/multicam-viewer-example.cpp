#include <iclCommon.h>
#include <iclGenericGrabber.h>
#include <stdexcept>

#define DBG(X) if(pa_defined("-v")) std::cout << X << std::endl

GUI gui("vbox");
int numCams = 0;
std::vector<std::string> names;
std::vector<std::string> infos;
std::vector<GenericGrabber*> grabbers;

void init_gui(){
  GUI g1("hbox[@minsize=1x1]");
  GUI g2("hbox[@minsize=1x1]");
  for(int i=0;i<4 && i<numCams;++i){
    (i<2?g1:g2) << std::string("image[@minsize=5x5@handle=view-")+str(i)+"@label=cam: "+names[i]+"|info:"+infos[i]+"|]";
  }
  gui << g1 << g2;
  gui.show();
  
  GUI g5("camcfg(dc)");
  g5.show();
}

void run(){
  for(int i=0;i<numCams;++i){
    ImageHandle &ih = gui.getValue<ImageHandle>(std::string("view-")+str(i));
    const ImgBase *image = grabbers[i]->grab();
    DBG("grabber " << names[i] << "(" << infos[i] << ") grabbed image:" << *image << " TimStamp:" << image->getTime().toString());
    ih = image;
    ih.update();
    Thread::msleep(10);
  }  
}

int main(int n, char **ppc){
  ExecThread x(run);
  
  pa_explain("-1","defines the first cameras parameter e.g. -1 dc 0\n"
             "uses the first dc device found on the bus\n"
             "currently supported (pwc,dc,file,demo,unicap)");
  pa_explain("-2","see -1");
  pa_explain("-3","see -1");
  pa_explain("-4","see -1");
  pa_explain("-size","set grabbers desired grabbing size");
  pa_explain("-reset-bus","calls dc1394_reset_bus before creating dc grabbers");
  pa_explain("-v","show some debuggin output");
  pa_init(n,ppc,"-1(2) -2(2) -3(2) -4(2) -size(1) -v -reset-bus");
  QApplication app(n,ppc);
  
  for(int i=0;i<4;++i){
    std::string p=std::string("-")+str(i+1);
    if(pa_defined(p)){
      std::string name = pa_subarg<std::string>(p,0,"");
      if(pa_defined("-reset-bus")){
        static bool first = true;
        if(first && name == "dc"){
          first = false;
          DBG("resetting dc bus");
          GenericGrabber::resetBus("dc");
          DBG("unsuported platform (only supported on linux)");
          DBG("dc bus has been resetted");
        }
      }
      
      std::string info = name+"="+pa_subarg<std::string>(p,1,"0");
      if(name != "" && info != ""){
        names.push_back(name);        
        infos.push_back(info);
        grabbers.push_back(new GenericGrabber(name,info));
        if(pa_defined("-size")){
          grabbers.back()->setDesiredSize(translateSize(pa_subarg<std::string>("-size",0,"320x240")));
        }
        numCams++;
      }      
    }
  }
  if(!numCams){
    throw std::logic_error("at least on camera must be defined!");
  }
  
  init_gui();
  
  x.run();
  
  app.exec();
}
