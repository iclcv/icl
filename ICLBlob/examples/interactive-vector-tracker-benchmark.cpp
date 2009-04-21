#include <iclCommon.h>
#include <iclRandom.h>
#include <iclVectorTracker.h>

GUI gui;
VectorTracker vt;
Mutex mutex;


void updateVT(){
  Mutex::Locker l(mutex);
  static int &dim = gui.getValue<int>("dim-val");
  static bool &optOn = gui.getValue<bool>("trivial-val");
  static bool &normOn = gui.getValue<bool>("norm-val");
  static bool &ffOn = gui.getValue<bool>("ff-val");

  vt = VectorTracker(dim,
                     10000,
                     std::vector<float>(normOn?dim:0,0.9),
                     ffOn ? VectorTracker::firstFree : VectorTracker::brandNew,
                     0.0,
                     optOn);
}


void init(){
  gui << "slider(1,1000,2)[@label=data dimension@handle=dim@out=dim-val]";
  gui << "slider(1,300,30)[@label=num inputs@handle=num@out=num-val]";
  gui << "slider(0,5,0)[@label=num std. deviation@handle=dev@out=dev-val]";
  gui << ( GUI("hbox") 
           << "fps(10)[@handle=fps@minsize=5x2]"
           << "togglebutton(off,on)[@label=try tivial@handle=trivial@out=trivial-val]"
           << "togglebutton(off,on)[@label=use norm@handle=norm@out=norm-val]"
           << "togglebutton(off,on)[@label=first free ID@handle=ff@out=ff-val]"
         );
  
  gui.show();
  
  gui.registerCallback(new GUI::Callback(updateVT),"dim,ff,norm,trivial");
  
  updateVT();
}


void run(){

  static int &num = gui.getValue<int>("num-val");
  static int &dev = gui.getValue<int>("dev-val");
  static std::vector<VectorTracker::Vec> data;

  while(true){
    mutex.lock();
    data.resize(clip(num+(int)gaussRandom(0,dev),0,300));
    
    for(unsigned int i=0;i<data.size();++i){
      data[i].resize(vt.getDim());
      std::fill(data[i].begin(),data[i].end(),URand(0,1));
    }

    vt.pushData(data);

    std::vector<int> ids(data.size());
    for(unsigned int i=0;i<data.size();++i){
      ids[i] = vt.getID(i);
    }
    
    gui["fps"].update();
    mutex.unlock();

    Thread::msleep(1);
  }
}


int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
