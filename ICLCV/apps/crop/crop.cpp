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
#include <ICLUtils/FPSLimiter.h>
#include <ICLIO/FileList.h>

GenericGrabber grabber;
HSplit gui;
Img8u curr;
Mutex currMutex;

struct Mouse1 : public MouseHandler{
  std::vector<Point> ps;
  Rect bounds;
  int handles[4];
  int dragged;
  Mouse1(const Size &maxSize):ps(4),bounds(Point::null,maxSize-Size(1,1)){

    Rect r = bounds.enlarged(-iclMin(50,iclMax(maxSize.width,maxSize.height)/2));
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
    getOptions().handleWidth = 10;
    addRect(Rect(Point::null,size).enlarged(-5));
  }
  
  virtual void process(const MouseEvent &e){
    if(gui["rect"].as<bool>() && !e.isRight()){
      DefineRectanglesMouseHandler::process(e);
    }
  }
} *mouse_2 = 0;

void rectangular_changed(){
  if(gui["rect"]){
    gui["s1"].disable();
    gui["s2"].disable();
  }else{
    gui["s1"].enable();
    gui["s2"].enable();
  }
}

void save_as(){
  Mutex::Locker lock(currMutex);
  std::string filename = pa("-i",1);
  save(curr,filename);
}

void overwrite(){
  Mutex::Locker lock(currMutex);
  try{
    std::string filename = saveFileDialog();
    save(curr,filename);
  }catch(...){}
}

void init(){
  grabber.init(pa("-i"));

  gui << Draw().handle("draw").label("input image")
      << Image().handle("cropped").label("cropped")
      << ( VBox().maxSize(12,99).minSize(12,1)
           << Button("save as ..").handle("saveAs")
           << Button("overwrite input").handle("overwrite")
           << Combo("0,90,180,270").handle("rot").label("rotation")
           << CheckBox("rectangular",!pa("-r")).handle("rect")
           << ( HBox().label("rectification size")
                << Spinner(1,4096,640).handle("s1")
                << Label(":")
                << Spinner(1,4096,480).handle("s2")
                )
           << (HBox() 
               << Fps().handle("fps")
               << CamCfg()
               )
           )
      << Show();

  const ImgBase *image = grabber.grab();
  
  mouse_1 = new Mouse1(image->getSize());
  mouse_2 = new Mouse2(image->getSize());

  gui["draw"].install(mouse_1);
  gui["draw"].install(mouse_2);
  
  DrawHandle draw = gui["draw"];
  draw->setImageInfoIndicatorEnabled(false);
  
  gui["rect"].registerCallback(rectangular_changed);
  rectangular_changed();
  
  
  if(*pa("-i",0) != "file" || FileList(*pa("-i",1)).size() != 1){
    gui["overwrite"].disable();
  }else{
    gui["overwrite"].registerCallback(overwrite);
  }
  
  gui["saveAs"].registerCallback(save_as);

}

void run(){
  static FPSLimiter fpsLimit(30);
  fpsLimit.wait();

  const ImgBase *image = grabber.grab();
  DrawHandle draw = gui["draw"];
  ImageHandle cropped = gui["cropped"];

  draw = image;
  
  static RotateOp rot;
  rot.setAngle(parse<int>(gui["rot"]));

  const ImgBase *cro = 0;
  if(gui["rect"]){
    static Img8u roi;
    std::vector<utils::Rect> rs = mouse_2->getRects();
    ICLASSERT_THROW(rs.size() == 1, ICLException("expected exactly one rectangle"));
    mouse_2->visualize(**draw);
    SmartPtr<const ImgBase> tmp = image->shallowCopy(rs[0] & image->getImageRect());
    roi.setChannels(tmp->getChannels());
    roi.setFormat(tmp->getFormat());
    roi.setSize(tmp->getROISize());
    tmp->convertROI(&roi);
    cro = rot.apply(&roi);
  }else{
    draw->draw(mouse_1->vis());
    Size32f s(gui["s1"],gui["s2"]);
    Point32f ps[4] = { mouse_1->ps[0],  mouse_1->ps[1],  mouse_1->ps[2],  mouse_1->ps[3] };
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                \
      case depth##D:{                                           \
        static ImageRectification<icl##D> ir;                   \
        try{                                                    \
          cro = rot.apply(&ir.apply(ps,*image->as##D(),s));     \
        }catch(...){}                                           \
        break;                                                  \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }
  if(cro){
    cropped = cro;
    currMutex.lock();
    cro->convert(&curr);
    currMutex.unlock();
  }

  gui["draw"].render();
  gui["fps"].render();
}


int main(int n, char **args){
  return ICLApp(n,args,"[m]-input|-i(2) -rectification|-r",init,run).exec();
}
