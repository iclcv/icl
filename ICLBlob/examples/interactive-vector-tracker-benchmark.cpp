/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/examples/interactive-vector-tracker-benchmark.cpp  **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQuick/Common.h>
#include <ICLUtils/Random.h>
#include <ICLBlob/VectorTracker.h>

GUI gui;
VectorTracker vt;
Mutex mutex;


void updateVT(){
  Mutex::Locker l(mutex);
  static int &dim = gui.get<int>("dim-val");
  static bool &optOn = gui.get<bool>("trivial-val");
  static bool &normOn = gui.get<bool>("norm-val");
  static bool &ffOn = gui.get<bool>("ff-val");

  vt = VectorTracker(dim,
                     10000,
                     std::vector<float>(normOn?dim:0,0.9),
                     ffOn ? VectorTracker::firstFree : VectorTracker::brandNew,
                     0.0,
                     optOn);
}


void init(){
#ifdef OLD_GUI
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
#endif

  gui << Slider(1,1000,2).label("data dimension").handle("dim").out("dim-val")
      << Slider(1,300,30).label("num inputs").handle("num").out("num-val")
      << Slider(0,5,0).label("num std. deviation").handle("dev").out("dev-val")
      << ( HBox() 
           << Fps(10).handle("fps").minSize(5,2)
           << Button("off","on").label("try tivial").handle("trivial").out("trivial-val")
           << Button("off","on").label("use norm").handle("norm").out("norm-val")
           << Button("off","on").label("first free ID").handle("ff").out("ff-val")
           )
      << Show();

  
  gui.registerCallback(updateVT,"dim,ff,norm,trivial");
  
  updateVT();
}


void run(){

  static int &num = gui.get<int>("num-val");
  static int &dev = gui.get<int>("dev-val");
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
    
    gui["fps"].render();
    mutex.unlock();

    Thread::msleep(1);
  }
}


int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
