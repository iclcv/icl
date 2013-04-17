/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/apps/crop/crop.cpp                               **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLQt/Common.h>
#include <ICLFilter/ImageRectification.h>
#include <ICLFilter/RotateOp.h>
#include <ICLQt/DefineRectanglesMouseHandler.h>

GenericGrabber grabber;
SmartPtr<SurfFeatureDetector> surf;
HSplit gui;
Img8u templ,vis(Size(1,1),formatRGB);
int iH=0, iW=0, tH=0, tW=0;
VisualizationDescription vdtempl;

inline int fround(float flt) {  
  return (int) floor(flt+0.5f); 
}

struct Mouse1 : public MouseHandler{
  std::vector<Point> ps;
  Rect bounds;
  int handles[4];
  int dragged;
  Mouse1(const Size &maxSize):ps(4),bounds(Point::null,maxSize-Size(1,1)){

    Rect r = bounds.enlarged(-20);
    ps[0] = r.ul();
    ps[1] = r.ur();
    ps[2] = r.lr();
    ps[3] = r.ll();
    std::fill(handles,handles+4,0);
    dragged = -1;
  }
  virtual void process(const MouseEvent &e){
    if(gui["rect"].as<bool>()) return;
    Point p = e.getPos();
    
    if(!bounds.contains(p.x,p.y)){
      return;
    }

    if(e.isReleaseEvent()){
      dragged = -1;
      std::fill(handles,handles+4,0);
    }
    if(e.isPressEvent()){
      for(int i=0;i<4;++i){
        if(ps[i].distanceTo(p) < 8){
          dragged = i;
          handles[i] = 2;
        }
      }
    }
    if(e.isDragEvent()){
      if(dragged != -1){
        ps[dragged] = p;
      }
    }else if(e.isMoveEvent()){
      for(int i=0;i<4;++i){
        if(ps[i].distanceTo(p) < 8){
          handles[i] = 1;
        }else{
          handles[i] = 0;
        }
      }
    }
  }

  VisualizationDescription vis() const{
    VisualizationDescription d;
    d.color(255,0,0,255);
    d.linewidth(2);
    d.fill(255,0,0,40);
    std::vector<Point> pso = ps;
    d.polygon(pso);
    d.linewidth(1);
    for(int i=0;i<4;++i){
      d.fill(255,0,0,1+127*handles[i]);
      d.rect(pso[i].x-5,pso[i].y-5,11,11);
    }
    return d;
  }
} *mouse_1 = 0;


struct Mouse2 : public DefineRectanglesMouseHandler{
  Mouse2(const Size &size) : DefineRectanglesMouseHandler(1,5){
    getOptions().handleWidth = 6;
    addRect(Rect(Point::null,size).enlarged(-5));
  }
  
  virtual void process(const MouseEvent &e){
    if(gui["rect"].as<bool>() && !e.isMiddle() ){
      DefineRectanglesMouseHandler::process(e);
    }
  }
} *mouse_2 = 0;

void init(){
  grabber.init(pa("-i"));

  gui << Draw().handle("draw").label("input image")
      << Image().handle("cropped",!pa("-r")).label("cropped")
      << ( VBox().maxSize(14,99).minSize(14,1)
           << Button("save as ..").handle("saveAs")
           << Button("overwrite input").handle("overwrite")
           << ComboBox("0,90,180,270").handle("rot").label("rotation")
           << CheckBox("rectangular",true).handle("rect")
           << ( HBox().label("rectification ratio")
                << Spinner(1,32,4).handle("ar1")
                << Label(":")
                << Spinner(1,32,3).handle("ar2")
                )
           << ( HBox().label("rectification size")
                << Spinner(1,4096,640).handle("s1")
                << Label(":")
                << Spinner(1,4096,480).handle("s")
                )
           << (HBox() 
               << Fps().handle("fps")
               << CamCfg()
               )
           )
      << Show();

  const ImgBase *image = grabber.grab();
  
  mouse_1 = new Mouse1(image->getSize());
  mouse_2 = new Mouse2;

  gui["draw"].install(mouse_1);
  gui["draw"].install(mouse_2);
}

void extract_template(const Img8u &image){
  Size32f ars(gui["ar1"],gui["ar2"]);
  float ar = ars.width/ars.height;
  static ImageRectification<icl8u> ir;

  Point32f ps[4] = { mouse->ps[0],  mouse->ps[1],  mouse->ps[2],  mouse->ps[3] };
  Rect r(ps[0],Size(1,1));
  r |= Rect(ps[1],Size(1,1));
  r |= Rect(ps[2],Size(1,1));
  r |= Rect(ps[3],Size(1,1));
  float dim = iclMax(r.width,r.height);
  Size32f size(dim*ar,dim);
  while(size.width > iW || size.height > iH
        || size.width > iH || size.height > iW){
    size.width = float(size.width) * 0.9;
    size.height = float(size.height) * 0.9;
  }
  ir.apply(ps,image,Size(size.width,size.height)).deepCopy(&templ);
  surf->setReferenceImage(&templ);
  vdtempl = vis_surf(surf->getReferenceFeatures(),0,(iH-tH)/2);
}

void rotate_template(){
  RotateOp r(90);
  r.apply(&templ)->deepCopy(bpp(templ));
  surf->setReferenceImage(&templ);
  vdtempl = vis_surf(surf->getReferenceFeatures(),0,(iH-tH)/2);
}


void run(){
  DrawHandle draw = gui["draw"];
  ImageHandle cropped = gui["cropped"];

  if(gui["rect"]){
    std::vector<utils::Rect> rs = mouse_2->getRects();
    ICLASSER_THROW(
  }else{
  
  
  }
    
   
 

}


int main(int n, char **args){
  return ICLApp(n,args,"[m]-input|-i(2) -rectification|-r",init,run).exec();
}
