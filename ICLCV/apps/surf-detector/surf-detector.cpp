/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/apps/surf-detector/surf-detector.cpp             **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLCV/SurfFeatureDetector.h>
#include <ICLQt/Common.h>
#include <ICLFilter/ImageRectification.h>
#include <ICLFilter/RotateOp.h>

GenericGrabber grabber;
SmartPtr<SurfFeatureDetector> surf;
HSplit gui;
Img8u templ,vis(Size(1,1),formatRGB);
int iH=0, iW=0, tH=0, tW=0;
VisualizationDescription vdtempl;

inline int fround(float flt) {  
  return (int) floor(flt+0.5f); 
}

struct Mouse : public MouseHandler{
  std::vector<Point> ps;
  Rect bounds;
  int handles[4];
  int dragged;
  int xoffset;
  Mouse(const Size &maxSize):ps(4),bounds(Point::null,maxSize-Size(1,1)),xoffset(0){

    Rect r = bounds.enlarged(-20);
    ps[0] = r.ul();
    ps[1] = r.ur();
    ps[2] = r.lr();
    ps[3] = r.ll();
    std::fill(handles,handles+4,0);
    dragged = -1;
  }
  virtual void process(const MouseEvent &e){
    Point p = e.getPos();
    p.x -= xoffset;
    
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
  void setXOffset(int xoffset){
    this->xoffset = xoffset;
  }

  VisualizationDescription vis() const{
    VisualizationDescription d;
    d.color(255,0,0,255);
    d.linewidth(2);
    d.fill(255,0,0,40);
    std::vector<Point> pso = ps;
    for(int i=0;i<4;++i) pso[i].x += xoffset;
    d.polygon(pso);
    d.linewidth(1);
    for(int i=0;i<4;++i){
      d.fill(255,0,0,1+127*handles[i]);
      d.rect(pso[i].x-5,pso[i].y-5,11,11);
    }
    return d;
  }
} *mouse = 0;


VisualizationDescription vis_surf(const std::vector<SurfFeature> &fs, int dx, int dy){
  VisualizationDescription d;
  for(size_t i=0;i<fs.size();++i){
    d += fs[i].vis(dx,dy);
  }
  return d;
}


void init(){
  grabber.init(pa("-i"));
  if(pa("-r")) grabber.resetBus();

  gui << Draw().handle("draw").minSize(48,18) 
      << ( VBox().maxSize(14,99).minSize(14,1)
           << ( HBox()
                << Button("extract template").handle("extract")
                << Button("rotate template").handle("rotate")
              )
           << ( HBox().label("extract aspect ratio")
                << Spinner(1,32,4).handle("ar1")
                << Label(":")
                << Spinner(1,32,3).handle("ar2")
               )
           << CheckBox("show matches only",true).handle("vis matches")
           << CheckBox("visualize associations",true).handle("vis lines")
           << CheckBox("visualize extraction quad",true).handle("vis quad")
           << Slider(0,20,4).handle("octaves").label("octaves")
           << Slider(0,20,4).handle("intervals").label("intervals/octavelayer")
           << Slider(0,10,2).handle("step").label("sample step")
           << FSlider(0,0.04,0.001).out("thresh").label("threshold")

           << (HBox() 
               << Fps().handle("fps")
               << CamCfg()
              )
         )
      << Show();
  grabber.grab()->convert(&templ);
  surf = new SurfFeatureDetector(5,4,2,0.00005,*pa("-p"));
  surf->setReferenceImage(&templ);

  mouse = new Mouse(templ.getSize());
  gui["draw"].install(mouse);


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
  
  surf->setOctaves(gui["octaves"]);
  surf->setIntervals(gui["intervals"]);
  surf->setSampleStep(gui["step"]);
  surf->setThreshold(gui["thresh"]);
  

  DrawHandle draw = gui["draw"];
  ButtonHandle extract = gui["extract"];
  ButtonHandle rotate = gui["rotate"];

  const Img8u &image = *grabber.grab()->as8u();
  iW = image.getWidth();
  iH = image.getHeight();
  tW = templ.getWidth();
  tH = templ.getHeight();

  if(extract.wasTriggered()) extract_template(image);
  if(rotate.wasTriggered()) rotate_template();

  vdtempl = vis_surf(surf->getReferenceFeatures(),0,(iH-tH)/2);
  
  mouse->setXOffset(tW);
  vis.setSize(Size(iW+tW,iH));
  
  
  vis.fill(0);
  vis.setROI(Rect(0,(iH-tH)/2, tW, tH));
  templ.deepCopyROI(&vis);
  vis.setROI(Rect(tW,0,iW,iH));
  image.deepCopyROI(&vis);
  vis.setFullROI();
  
  if(gui["vis matches"]){
    const std::vector<SurfMatch> &ms = surf->match(&image);
    std::vector<SurfFeature> is(ms.size()), ts(ms.size());
    for(size_t i=0;i<ms.size();++i){
      ts[i] = ms[i].second;
      is[i] = ms[i].first;
    }
    draw->draw(vis_surf(ts,0,(iH-tH)/2));
    draw->draw(vis_surf(is,tW,0));
    if(gui["vis lines"]){
      draw->linewidth(3);
      draw->color(0,100,255,100);
      for(size_t i=0;i<ms.size();++i){
        Point32f tm(ms[i].second.x,ms[i].second.y);
        Point32f im(ms[i].first.x,ms[i].first.y);
        draw->line(tm+Point32f(0,(iH-tH)/2), im+Point32f(tW,0));
      }
    }
  }else{
    draw->draw(vis_surf(surf->detect(&image),tW,0));
    draw->draw(vdtempl);
  }
  //
  // 
  // 
  draw = vis;
  if(gui["vis quad"]){
    draw->draw(mouse->vis());
  }
  draw.render();
  gui["fps"].render();

}


int main(int n, char **args){
  pa_explain("-p","select surf feature plugin type (one of clsurf, opensurf or best)");
  return ICLApp(n,args,"[m]-input|-i(2) -surf-feature-plugin|-p(plugin=best) "
                "-reset-camera-bus|-r",init,run).exec();
}
