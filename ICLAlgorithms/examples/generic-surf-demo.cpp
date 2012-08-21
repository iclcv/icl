/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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

HSplit gui;
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
    DrawHandle draw_object = gui["draw_object"];
    SmartPtr<ImgBase> obj = surf->getObjectImg(); 
    draw_object = obj.get();
    draw_object->render(); // we can risk that because we are in the gui-thread
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
          static DrawHandle draw_object = gui["draw_object"];
          draw_object = &roi;
          draw_object->render();
        }
      }
      r = Rect::null;
    }
  }
}

void init(){
  grabber.init(pa("-i"));

  if(pa("-f")){
    Img8u obj = load<icl8u>(pa("-f"));
    surf->setObjectImg(&obj);
    objRect = obj.getImageRect();
  }else{
    const ImgBase *obj = grabber.grab();
    surf->setObjectImg(obj);
    objRect = obj->getImageRect();
  }
  
  
  gui << (VBox()
          <<(HBox()
             << (VBox()
                 << Draw().handle("draw_object").minSize(16,12).label("result of surf match")
                 << Button("snapshot").handle("snap_handle"))
             << Draw().handle("draw_result").minSize(16,12).label("result of surf match")
             << Draw().handle("draw_image").minSize(16,12).label("result of surf"))
          
          <<(HBox()
             << Fps(10).handle("fps").maxSize(100,2).minSize(8,2)))
      << (VBox().minSize(8,1)
          << CheckBox("opensurf",true).out("os").handle("os_handle")
          << CheckBox("show_matches").out("sm").handle("sm_handle")
          << CheckBox("show_features").out("sf").handle("sf_handle")
          << CheckBox("rotatationinvariance/extended").out("ri").handle("ri_handle")
          << Slider(0,20,4).out("octaves").handle("oct_handle").label("octaves")
          << Slider(0,20,4).out("intervals").handle("intervals_handle").label("intervals/octavelayer")
          << Slider(0,10,2).out("samples").handle("sample_handle").label("init samples")
          << FSlider(0,0.04,0.001).out("thresh").handle("thresh_handle").label("threshold")
         );
  
  gui.show();

  
  gui.registerCallback(gui_cb,"os_handle,ri_handle,snap_handle,oct_handle,intervals_handle,sample_handle,thresh_handle");
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
  
  
  static DrawHandle draw_result = gui["draw_result"];
  static DrawHandle draw_object = gui["draw_object"];
  static DrawHandle draw_image = gui["draw_image"];
  static CheckBoxHandle sf_handle = gui["sf_handle"];
  static CheckBoxHandle sm_handle = gui["sm_handle"];
  draw_result = image;
  draw_image = image;

  if(sf_handle.isChecked()){
    const std::vector<GenericSurfDetector::GenericPoint> features = surf->extractFeatures(image);
    surf->visualizeFeatures(**draw_image,features);
  }
  
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
  
  draw_object.render();
  draw_result.render();
  draw_image.render();
  
  gui["fps"].render();
}


int main(int n, char **args){
  return ICLApp(n,args,"[m]-input|-i(devicetype,device) -file|-f(objectfile)",init,run).exec();
}
