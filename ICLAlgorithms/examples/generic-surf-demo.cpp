/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/examples/opensurf-demo.cpp               **
** Module : ICLAlgorithms                                          **
** Authors: Christian Groszewski                                   **
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
#include <ICLAlgorithms/GenericSurfDetector.h>

using namespace icl;

Mutex mutex;
GenericGrabber grabber;
//GenericSurfDetector surf(0,false,3,4,3);
//GenericSurfDetector surf(0,500,1,3,4);
#ifdef HAVE_OPENSURF
SmartPtr<GenericSurfDetector> surf = new GenericSurfDetector("opensurf");
#else
SmartPtr<GenericSurfDetector> surf = new GenericSurfDetector("opencv");
#endif

GUI gui("hsplit");

void gui_cb(const std::string &handle){
  Mutex::Locker lock(mutex);
  if(!handle.find("os")){
    bool os = gui["os"];
    const ImgBase *im = surf->getObjectImg()->deepCopy();
    surf = SmartPtr<GenericSurfDetector>(new GenericSurfDetector(os?"opensurf":"opencv"));
    surf->setObjectImg(im);
  }else if(!handle.find("ri")){
    bool ri = gui["ri"];
    if(surf->getImpl() == "opensurf"){
      surf->setRotationInvariant(ri);
    }else{ 
      surf->setExtended(ri);
    }
  }else if(!handle.find("snap")){
    const ImgBase *tmp = grabber.grab();
    surf->setObjectImg(tmp);
    gui_DrawHandle(draw_object);
    draw_object = surf->getObjectImg();
    draw_object->lock();
    draw_object->reset();
    draw_object->unlock();
    draw_object->update(); // we can risk that because we are in the gui-thread
  }else if(!handle.find("oct")){
    surf->setOctaves(gui["octaves"]);
  }else if(!handle.find("intervals")){
    surf->setOctavelayer(gui["intervals"]);
  }else if(!handle.find("samples")){
    surf->setInitSamples(gui["samples"]);
  }else if(!handle.find("thresh")){
    surf->setThreshold(gui["thresh"]);
  }
}

void init(){
  grabber.init(FROM_PROGARG("-i"));
  grabber.setIgnoreDesiredParams(true);

  if(pa("-f")){
    Img8u obj = load<icl8u>(pa("-f"));
    surf->setObjectImg(&obj);
  }
  
  gui << (GUI("vbox")
          <<(GUI("hbox")
             << (GUI("vbox")
                 << "draw[@handle=draw_object@minsize=16x12@label=result of surf match]"
                 << "button(snapshot)[@handle=snap_handle]")
             << "draw[@handle=draw_result@minsize=16x12@label=result of surf match]"
             << "draw[@handle=draw_image@minsize=16x12@label=result of surf]")
          
          <<(GUI("hbox")
             << "fps(10)[@handle=fps@maxsize=100x2@minsize=8x2]"))
      << (GUI("vbox[@minsize=8x1]")
          << "checkbox(opensurf,checked)[@out=os@handle=os_handle]"
          << "checkbox(show_matches,unchecked)[@out=sm@handle=sm_handle]"
          << "checkbox(show_features,unchecked)[@out=sf@handle=sf_handle]"
          << "checkbox(rotatationinvariance/extended,unchecked)[@out=ri@handle=ri_handle]"
          << "slider(0,20,4)[@out=octaves@handle=oct_handle@label=octaves]"
          << "slider(0,20,4)[@out=intervals@handle=intervals_handle@label=intervals/octavelayer]"
          << "slider(0,10,2)[@out=samples@handle=sample_handle@label=init samples]"
          << "fslider(0,0.04,0.001)[@out=thresh@handle=thresh_handle@label=threshold]"
          );
  
  gui.show();
  
  gui.registerCallback(new GUI::Callback(gui_cb),"os_handle,ri_handle,snap_handle,oct_handle,intervals_handle,sample_handle,thresh_handle");
  gui["draw_object"] =  surf->getObjectImg();
}


void run(){
  Mutex::Locker lock(mutex);
  const ImgBase *image = grabber.grab();
  
  gui_DrawHandle(draw_result);
  gui_DrawHandle(draw_object);
  gui_DrawHandle(draw_image);
  draw_result = image;
  draw_image = image;
  draw_object = surf->getObjectImg();
  
  draw_result->lock();
  draw_object->lock();
  draw_image->lock();
  
  draw_image->reset();
  draw_result->reset();
  draw_object->reset();
  
  gui_CheckBox(sf_handle);
  if(sf_handle.isChecked()){
    const std::vector<GenericSurfDetector::GenericPoint> features = surf->extractFeatures(image);
    surf->visualizeFeatures(**draw_image,features);
  }
  
  gui_CheckBox(sm_handle);
  if(sm_handle.isChecked()){
    const std::vector<std::pair<GenericSurfDetector::GenericPoint,
    GenericSurfDetector::GenericPoint> > &matches = surf->match(image);
    surf->visualizeMatches(**draw_object,**draw_result,matches);
  }
  
  draw_result->unlock();
  draw_object->unlock();
  draw_image->unlock();
  draw_object.update();
  draw_result.update();
  draw_image.update();
  
  gui_FPSHandle(fps);
  fps.update();
}


int main(int n, char **args){
  return ICLApp(n,args,"-input|-i(devicetype,device) -file|-f(objectfile)",init,run).exec();
}
