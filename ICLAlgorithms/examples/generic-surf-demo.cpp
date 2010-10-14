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
Rect r = Rect::null;
Rect objRect;

void gui_cb(const std::string &handle){
  Mutex::Locker lock(mutex);
  if(!handle.find("os")){
    bool os = gui["os"];
    const ImgBase *im = surf->getObjectImg()->deepCopy();
    surf = SmartPtr<GenericSurfDetector>(new GenericSurfDetector(os?"opensurf":"opencv"));
    surf->setObjectImg(im);
    objRect = im->getImageRect();
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
    SmartPtr<ImgBase> obj = surf->getObjectImg(); 
    draw_object = obj.get();
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

void select_object(const MouseEvent &m){
  if(m.isPressEvent() && m.isLeft()){
    r = Rect(m.getX(),m.getY(),1,1);
  }else if(m.isDragEvent()){
    if(r != Rect::null){
      r.width = m.getX()-r.x;
      r.height = m.getY()-r.y;
    }
  }else{
    if(r != Rect::null){
      Rect r2 = r.normalized();
      if(r2.width * r2.height > 4){
        Mutex::Locker lock(mutex);
        Img8u image = *grabber.grab()->asImg<icl8u>();
        Rect r3 = r2 & image.getImageRect();
        if(r3.width * r3.height > 4){
          image.setROI(r3);
          Img8u roi;
          image.deepCopyROI(&roi);
          surf->setObjectImg(&roi);
          objRect = roi.getImageRect();
          gui_DrawHandle(draw_object);
          draw_object = &roi;
          draw_object->lock();
          draw_object->reset();
          draw_object->unlock();
          draw_object->update();
        }
      }
      r = Rect::null;
    }
  }
}

void init(){
  grabber.init(FROM_PROGARG("-i"));
  grabber.setIgnoreDesiredParams(true);

  if(pa("-f")){
    Img8u obj = load<icl8u>(pa("-f"));
    surf->setObjectImg(&obj);
    objRect = obj.getImageRect();
  }else{
    const ImgBase *obj = grabber.grab();
    surf->setObjectImg(obj);
    objRect = obj->getImageRect();
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
  gui["draw_object"] =  surf->getObjectImg().get();
  gui["draw_image"].install(new MouseHandler(select_object));
}


typedef GenericSurfDetector::Match Match;

std::vector<Point32f> get_transformed_rect(const Rect &r, const std::vector<Match> &matches){
  std::vector<Point32f> v(4);
  v[0] = r.ul(); 
  v[1] = r.ur(); 
  v[2] = r.lr(); 
  v[3] = r.ll();

  if(!matches.size()){
    return v;
  }
  int N = matches.size();
  // compute TA = B (A = m.first, B = m.second) -> T = B * A.pinv();
  DynMatrix<float> A(N,3), B(N,3);
  for(int i=0;i<N;++i){
    Point32f a = matches[i].first.getCenter();
    Point32f b = matches[i].second.getCenter();
    A(i,0) = a.x;
    A(i,1) = a.y;
    A(i,2) = 1;
    B(i,0) = b.x;
    B(i,1) = b.y;
    B(i,2) = 1;
  }
  try{
    DynMatrix<float> T = A * B.pinv(true);
    DynMatrix<float> x(1,3),y(1,3);

    for(int i=0;i<4;++i){
      x[0] = v[i].x;
      x[1] = v[i].y;       
      x[2] = 1; 

      y = T * x;

      if(!y[2]){
        ERROR_LOG("y.h was 0");
        return v;
      }

      v[i].x = y[0]/y[2];
      v[i].y = y[1]/y[2];
    }
  }catch(...){
    return v;
  }
  return v;
}


void run(){
  Mutex::Locker lock(mutex);
  const ImgBase *image = grabber.grab();
  
  gui_DrawHandle(draw_result);
  gui_DrawHandle(draw_object);
  gui_DrawHandle(draw_image);
  draw_result = image;
  draw_image = image;
  
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

    std::vector<Point32f> ps = get_transformed_rect(objRect, matches);
    draw_image->color(0,100,255,255);
    draw_image->fill(0,100,255,40);
    draw_image->polygon(ps);
  }
  
  if(r != Rect::null){
    draw_image->color(255,0,0,255);
    draw_image->fill(255,0,0,20);
    draw_image->rect(r.normalized());
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
