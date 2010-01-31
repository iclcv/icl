#include <ICLQuick/Common.h>
#include <ICLIO/FileWriter.h>

GUI gui("vsplit");

std::vector<SmartPtr<GenericGrabber,PointerDelOp> > gs;

void init(){
  for(int i=1;i<=4;++i){
    std::string arg=str("-input")+str(i);
    if(pa(arg)){
      try{
        gs.push_back(SmartPtr<GenericGrabber,PointerDelOp>(new GenericGrabber(FROM_PROGARG(arg))));
        gs.back()->setDesiredSize(pa("-size"));
        if(pa("-t")){
          gs.back()->setProperty("trigger-power","on"); // or off?
        }
      }catch(const ICLException &ex){
        WARNING_LOG("unable to instantiate grabber from argument " << arg << "(skipping)");
      }
    }
  }
  if(!gs.size()){
    ERROR_LOG("program was unable to instantiate at least one input device (aborting)");
    exit(-1);
  }
  
  for(unsigned int i=0;i<gs.size();++i){
    gui << "image[@handle=image"+str(i)+"@minsize=12x8]";
  }
  gui << ( GUI ("hbox") 
           << "fps(10)[@handle=fps@label=current fps@maxsize=100x3]" 
           << "camcfg(dc,pwc)" );
  gui.show();
}


void run(){
  gui_FPSHandle(fps);
  static Size size = pa("-size");

  
  while(1){
    for(unsigned int i=0;i<gs.size();i++){
      gui["image"+str(i)] = gs[i]->grab();
      gui["image"+str(i)].update();
    }
    fps.update();
    Thread::msleep(gs.size()*10);
  }
}


int main(int n, char **ppc){
  paex
  ("-size","grabbing size")
  ("-t","use external trigger (only for devices that support this)")
  ("-r","reset dc-bus before instantiating grabbers")
  ("-input1","first possible input device and params")
  ("-input2","second possible input device and params")
  ("-input3","third possible input device and params")
  ("-input4","fourth possible input device and params");
  
  return ICLApp(n,ppc,"-size|-s(Size=QVGA) "
                "-external-trigger|-t [m]-input1|-1(device,device-params) -reset-bus|-r "
                "-input2|-2(device,device-params) -input3|-3(device,device-params) "
                "-input4|-4(device,device-params)",init,run).exec();
  
}
