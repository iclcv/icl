/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/demos/template-matching/template-matching.cpp    **
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
#include <ICLCV/CV.h>
#include <ICLCV/RegionDetector.h>
#include <ICLCV/ViewBasedTemplateMatcher.h>

Size imageSize(640,480);

HBox gui;
Img8u currTempl;
Img8u templMask(Size(1,1),1);

Img8u currImage;
Img8u imageMask = cvt8u(ones(imageSize.width,imageSize.height,1)*255);

Mutex mutex;
bool dragging = false;
Rect currRect = Rect(Point::null,imageSize).enlarged(-imageSize.width/2);

bool dragging_R  = false;
Rect currROI(Point::null,imageSize);


void mouse(const MouseEvent &e){
  if(e.isPressEvent()){
    if(e.isLeft()){
      currRect = Rect(e.getPos(),Size(1,1));
      dragging_R=false;
    }else{
      currROI = Rect(e.getPos(),Size(1,1));
      dragging_R=true;
    }
      dragging = true;
  }else if(e.isReleaseEvent()){
    Mutex::Locker l(mutex);
    if(dragging_R){
      currROI = currROI.normalized() & Rect(Point::null,imageSize);
      dragging = false;
    }else{
      Rect r = currRect.normalized() & Rect(Point::null,imageSize);
      currImage.setROI(r);
      currTempl.setSize(r.getSize());
      currImage.deepCopyROI(&currTempl);
      templMask.setSize(currTempl.getSize());
      templMask.clear(-1,255,false);
      currImage.setFullROI();
      dragging =false;
    }
  }else if(e.isDragEvent()){
    if(dragging_R){
      currROI.width = e.getX()-currROI.x;
      currROI.height = e.getY()-currROI.y;
    }else{
      currRect.width = e.getX()-currRect.x;
      currRect.height = e.getY()-currRect.y;
    }
  }
}

GenericGrabber g;


void init(){
  g.init(pa("-input"));
  g.useDesired(depth8u);

  gui << Draw().label("image").minSize(32,24).handle("image")
      << ( VBox()
           << Draw().label("template").minSize(10,6).handle("templ")
           << Draw().label("buffer").minSize(10,6).handle("buf")
           )
      << (VBox().minSize(7,7)
          << FSlider(0,1,0.9).handle("significance-handle").label("significance").out("significance")
          << Fps(50).handle("fps").minSize(5,5)
          << Button("no masks"," with masks").out("use-masks")
          << Button("dont clip buffers","clip buffers").out("clip-buffers")
          << Button("square distance","norm. cross corr").out("mode")
          )
      << Show();

  gui["image"].install(mouse);
}

void vis_roi(ICLDrawWidget *w){
  Rect r = currROI.normalized();
  w->color(0,0,0,0);
  w->fill(0,0,0,200);
  w->rect(Rect(Point::null,Size(imageSize.width,r.y)));
  w->rect(Rect(0,r.y,r.x,imageSize.height-r.y));
  w->rect(Rect(r.right(),r.y,imageSize.width-r.right(),imageSize.height-r.y));
  w->rect(Rect(r.x,r.bottom(),r.width,imageSize.height-r.bottom()));

  w->color(200,0,255,200);
  w->fill(255,255,255,0);
  w->rect(r);
}


void run(){
  g.useDesired(imageSize);

  while(1){
    mutex.lock();
    g.grab(bpp(currImage));
    if(!dragging){
      currImage.setROI(currROI.normalized() & currImage.getImageRect());
    }
    mutex.unlock();

    static DrawHandle &image = gui.get<DrawHandle>("image");
    static DrawHandle &templ = gui.get<DrawHandle>("templ");
    static DrawHandle &buf = gui.get<DrawHandle>("buf");
    static float &significance = gui.get<float>("significance");
    static bool &useMasks = gui.get<bool>("use-masks");
    static bool &clipBuffers = gui.get<bool>("clip-buffers");
    static bool &mode = gui.get<bool>("mode");
    static FPSHandle &fps = gui.get<FPSHandle>("fps");


    fps.render();

    image = &currImage;

    if(dragging){
      if(dragging_R){
        vis_roi(*image);
      }else{
        image->color(255,0,0,200);
        image->fill(255,255,255,50);
        image->rect(currRect);
      }
    }else if(currTempl.getDim()){

      if(currROI != currImage.getImageRect()){
        vis_roi(*image);
      }

      static ViewBasedTemplateMatcher matcher;
      matcher.setSignificance(significance);
      matcher.setMode(mode ? ViewBasedTemplateMatcher::crossCorrelation : ViewBasedTemplateMatcher::sqrtDistance);
      matcher.setClipBuffersToROI(clipBuffers);

      const std::vector<Rect> &rs = matcher.match(currImage,currTempl,useMasks?imageMask:Img8u::null,useMasks?templMask:Img8u::null);

      image->color(255,0,0,200);
      image->fill(255,255,255,0);
      for(unsigned int i=0;i<rs.size();++i){
        image->rect(rs[i]);
      }

      buf = matcher.getBuffer();
      buf.render();

    }
    image.render();




    mutex.lock();
    if(currTempl.getDim()){
      templ = &currTempl;
      templ.render();
    }
    mutex.unlock();


    Thread::msleep(10);

  }
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}

