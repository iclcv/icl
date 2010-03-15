/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLAlgorithms/examples/interactive-template-matching-demo.cpp  **
** Module : ICLAlgorithms                                          **
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
*********************************************************************/

#include <ICLQuick/Common.h>
#include <ICLAlgorithms/UsefulFunctions.h>
#include <ICLBlob/RegionDetector.h>
#include <ICLAlgorithms/ViewBasedTemplateMatcher.h>

Size imageSize(640,480);
GUI gui("hbox");
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

void init(){
  gui << "draw()[@label=image@minsize=32x24@handle=image]";
  gui << ( GUI("vbox") 
           << "draw()[@label=template@minsize=10x6@handle=templ]"
           << "draw()[@label=buffer@minsize=10x6@handle=buf]"
          );
           

  GUI controls("vbox[@minsize=7x7]");
  controls << "fslider(0,1,0.9)[@handle=significance-handle@label=significance@out=significance]";
  controls << "fps(50)[@handle=fps@minsize=5x5]";
  controls << "togglebutton(no masks, with masks)[@out=use-masks]";
  controls << "togglebutton(dont clip buffers,clip buffers)[@out=clip-buffers]";
  controls << "togglebutton(square distance,norm. cross corr)[@out=mode]";
  
  gui << controls;
   
  gui.show();

 
  
  (*gui.getValue<DrawHandle>("image"))->install(new MouseHandler(mouse));
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
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(imageSize);


    
  while(1){
    mutex.lock();
    g.grab()->convert(&currImage);
    if(!dragging){
      currImage.setROI(currROI.normalized() & currImage.getImageRect());
    }
    mutex.unlock();
    
    static DrawHandle &image = gui.getValue<DrawHandle>("image");
    static DrawHandle &templ = gui.getValue<DrawHandle>("templ");
    static DrawHandle &buf = gui.getValue<DrawHandle>("buf");
    static float &significance = gui.getValue<float>("significance");
    static bool &useMasks = gui.getValue<bool>("use-masks");
    static bool &clipBuffers = gui.getValue<bool>("clip-buffers");
    static bool &mode = gui.getValue<bool>("mode");
    static FPSHandle &fps = gui.getValue<FPSHandle>("fps");


    fps.update();

    image = &currImage;
    
    (*image)->lock();
    (*image)->reset();
    if(dragging){
      if(dragging_R){
        vis_roi(*image);
      }else{
        (*image)->color(255,0,0,200);
        (*image)->fill(255,255,255,50);
        (*image)->rect(currRect);
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
      
      (*image)->color(255,0,0,200);
      (*image)->fill(255,255,255,0);
      for(unsigned int i=0;i<rs.size();++i){
        (*image)->rect(rs[i]);
      }

      buf = matcher.getBuffer();
      buf.update();

    }
    (*image)->unlock();
    image.update();

    

    
    mutex.lock();
    if(currTempl.getDim()){
      templ = &currTempl;
      templ.update();
    }
    mutex.unlock();

    
    Thread::msleep(10);
    
  }
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input(device,device-params)",init,run).exec();
}

