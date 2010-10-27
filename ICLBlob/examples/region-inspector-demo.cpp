/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/examples/region-inspector-demo.cpp             **
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
#include <ICLBlob/RegionDetector.h>
#include <ICLUtils/Time.h>
#include <ICLFilter/MedianOp.h>

GUI gui("hsplit");
RegionDetector rd;
Mutex mutex;

struct MouseIO : public MouseHandler{
  int x,y;
  MouseIO():x(0),y(0){}
  virtual void process(const MouseEvent &evt){
    Mutex::Locker l(mutex);
    x = evt.getX();
    y = evt.getY();
  }
} mouseIO;


void init(){
  gui << "draw[@minsize=32x24@label=image@handle=image]";
  
  GUI labels("vbox[@label=Region information]");
  labels << "label()[@label=Total Region Count@minsize=6x2@handle=total]"
         << ( GUI("hbox")
              << "label()[@label=Region Size@handle=size-handle@minsize=6x2]"
              << "label()[@label=Region COG@handle=cog-handle@minsize=6x2]"
              )
         << ( GUI("hbox")
              << "label()[@label=Region Value@handle=val-handle@minsize=6x2]"
              << "label()[@label=Region Form Factor@handle=ff-handle@minsize=6x2]"
              )
         << ( GUI("hbox")
              << "label()[@label=Region EV-Ratio@handle=evratio-handle@minsize=6x2]"
              << "label()[@label=Region Boundary Length@handle=bl-handle@minsize=6x2]"
              )
         << ( GUI("hbox")
              << "label()[@label=sub regions@handle=nSub@minsize=6x2]"
              << "label()[@label=all sub regions@@handle=nAllSub@minsize=6x2]"
              )
         << ( GUI("hbox") 
              << "checkbox(show boundary,checked)[@out=showBoundary]"
              << "togglebutton(normal,!thinned)[@out=showThinnedBoundary]"
              )
         << ( GUI("hbox") 
              << "checkbox(show sub regions,checked)[@out=showSubRegions]"
              << "togglebutton(direct,all)[@out=showAllSubRegions]"
              )
         << ( GUI("hbox") 
              << "checkbox(show sur. regions,checked)[@out=showSurRegions]"
              << "togglebutton(direct,all)[@out=showAllSurRegions]"
              )
         << ( GUI("hbox") 
              << "checkbox(show neighbours,unchecked)[@out=showNeighbours]"
              << "checkbox(show bounding rect,unchecked)[@out=showBB]"
              )
         << ( GUI("hbox") 
              << "togglebutton(stopped,!grabbing)[@out=grabbing@handle=grab-handle@minsize=3x2]"
              << "button(grab next)[@handle=grab-next-handle@minsize=3x2]"
              );
  labels << "slider(2,256,10)[@out=levels@label=reduce levels]";
  labels << "slider(1,10,0)[@out=medianSize@label=preprocessing median mask size]";
  labels << "label(---)[@label=time for region detection@handle=timeRD]";
  labels << "label(---)[@label=time for neighbour detection@handle=timeNB]";
  labels << "label(---)[@label=time for sub region detection@handle=timeSR]";
  labels << "label(---)[@label=time for sur. region detection@handle=timeSU]";
  
  
  gui << labels;
  
  gui.show();
}

void run(){
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(pa("-s"));
  g.setIgnoreDesiredParams(false);
  g.setDesiredFormat(formatGray);

  static ICLDrawWidget &d = **gui.getValue<DrawHandle>("image");
  d.install(&mouseIO);

  gui_bool(showSubRegions);
  gui_bool(showAllSubRegions);
  gui_bool(showSurRegions);
  gui_bool(showAllSurRegions);
  gui_bool(showNeighbours);
  gui_bool(showBB);
  gui_bool(showBoundary);
  gui_bool(showThinnedBoundary);
  
  gui_LabelHandle(timeRD);
  gui_LabelHandle(timeNB);
  gui_LabelHandle(timeSR);
  gui_LabelHandle(timeSU);
  
  gui_LabelHandle(nSub);
  gui_LabelHandle(nAllSub);
  gui_LabelHandle(total);
  
  gui_int(medianSize);
  
  static LabelHandle &valHandle = gui.getValue<LabelHandle>("val-handle");
  static LabelHandle &cogHandle = gui.getValue<LabelHandle>("cog-handle");
  static LabelHandle &sizeHandle = gui.getValue<LabelHandle>("size-handle");
  static LabelHandle &ffHandle = gui.getValue<LabelHandle>("ff-handle");
  static LabelHandle &evratioHandle = gui.getValue<LabelHandle>("evratio-handle");
  static LabelHandle &blHandle = gui.getValue<LabelHandle>("bl-handle");
  static bool &grabButtonDown = gui.getValue<bool>("grabbing");
  static ButtonHandle &grabNextHandle = gui.getValue<ButtonHandle>("grab-next-handle");
  int &levels = gui.getValue<int>("levels");

  static int lastLevels = levels;
  static int lastMedianSize = medianSize;
  static const Img8u *grabbedImage;
  static Img8u reducedLevels;  

  const Img8u *useImage = 0;
  const std::vector<ImageRegion> *rs = 0;
  static SmartPtr<MedianOp> mo;
  while(1){
    int ms = medianSize;
    mutex.lock();
    bool rdUpdated = false;
    if(grabNextHandle.wasTriggered() || !useImage || grabButtonDown){
      grabbedImage = g.grab()->asImg<icl8u>();
      useImage = grabbedImage;

      if(levels != 256){
        reducedLevels = cvt8u(icl::levels(cvt(grabbedImage),levels));
        useImage = &reducedLevels;
      }

      if(ms){
        mo = SmartPtr<MedianOp>(new MedianOp(Size(ms,ms)));
        useImage = mo->apply(useImage)->asImg<icl8u>();
      }
      
      d.setImage(useImage);

      rd.setCreateGraph(showSubRegions || showNeighbours || showSurRegions);      
      Time t = Time::now();
      rs = &rd.detect(useImage);
      Time dt = (Time::now()-t);
      if(dt.toMilliSeconds() > 5){
        timeRD = str(dt.toMilliSeconds())+ "ms";
      }else{
        timeRD = dt.toStringFormated("%#ms %-usec ");
      }
      total = (int)rs->size();
      rdUpdated = true;
    }else if(lastLevels != levels || ms != lastMedianSize){
      if(levels != 256){
        reducedLevels = cvt8u(icl::levels(cvt(grabbedImage),levels));
        useImage = &reducedLevels;
        if(ms){
          mo = SmartPtr<MedianOp>(new MedianOp(Size(ms,ms)));
          useImage = mo->apply(useImage)->asImg<icl8u>();
        }
      }else if(ms != lastMedianSize){
        if(ms){
          mo = SmartPtr<MedianOp>(new MedianOp(Size(ms,ms)));
          useImage = mo->apply(useImage)->asImg<icl8u>();
        }
      }else{
        useImage = grabbedImage;
      }
      if(!rdUpdated){
        rd.setCreateGraph(showSubRegions || showNeighbours || showSurRegions );
        Time t = Time::now();
        rs = &rd.detect(useImage);
        Time dt = (Time::now()-t);
        if(dt.toMilliSeconds() > 5){
          timeRD = str(dt.toMilliSeconds())+ "ms";
        }else{
          timeRD = dt.toStringFormated("%#ms %-usec");
        }
        total = (int)rs->size();
        rdUpdated = true;
      }
      d.setImage(useImage);
    }
    lastLevels = levels;
    
    d.lock();
    d.reset();
    
    Point m(mouseIO.x,mouseIO.y);
  
    if(useImage->getImageRect().contains(m.x,m.y)){
      // find the region, that contains mouseX,mouseY
      ImageRegion r = rd.click(m);
      
      if(r){
        
        d.nofill();
        if(showBoundary){
          d.color(0,150,255,200);
          d.linestrip(r.getBoundary(showThinnedBoundary));
        }
        if(showBB){
          d.color(255,0,0,255);
          d.rect(r.getBoundingBox());
        }

        if(showSurRegions){
          d.linewidth(4);
          d.color(255,200,100,100);
          Time t=Time::now();

          std::vector<ImageRegion> sur = showAllSurRegions ? r.getParentTree() : 
                                         std::vector<ImageRegion>(1,r.getParentRegion());

          timeSU = str((Time::now()-t).toMilliSeconds())+"ms";
          for(unsigned int i=0;i<sur.size();++i){
            if(sur[i]){
              d.linestrip(sur[i].getBoundary());
            }
          }
        }
        d.linewidth(1);

        if(showSubRegions){
          d.color(0,155,0,255);
          Time t=Time::now();
          const std::vector<ImageRegion> &sub = r.getSubRegions(!showAllSubRegions);
          timeSR = str((Time::now()-t).toMilliSeconds())+"ms";

          for(unsigned int i=0;i<sub.size();++i){
            if(!sub[i]) {
              ERROR_LOG("sub-region is null??");
            }else{
              d.linestrip(sub[i].getBoundary());
            }
          }
          if(showAllSubRegions){
            nAllSub = (int)sub.size();
          }else{
            nSub = (int)sub.size();
          }
        }
        if(showNeighbours){
          d.color(255,0,0,255);
          Time t=Time::now();
          const std::vector<ImageRegion> &ns = r.getNeighbours();
          timeNB = str((Time::now()-t).toMilliSeconds())+"ms";
          for(unsigned int i=0;i<ns.size();++i){
            d.linestrip(ns[i].getBoundary());
          }
          
        }
        valHandle = r.getVal();
        cogHandle = str(r.getCOG());
        sizeHandle = r.getSize();
        ffHandle = r.getFormFactor();
        evratioHandle = r.getPCAInfo().len2/r.getPCAInfo().len1;
        blHandle = r.getBoundaryLength();
      }else{
        DEBUG_LOG("12");
        valHandle = "no region";
        cogHandle = "";
        sizeHandle = "";
        ffHandle = "";
        evratioHandle = "";
        blHandle = "";
      
      }
    }
    
    d.unlock();
    d.updateFromOtherThread();
    mutex.unlock();
    Thread::msleep(10);
  }
}


int main(int n, char **ppc){
  paex("-input","define device parameters (e.g. -d dc 0 or -d file image/*.ppm)");
  return ICLApp(n,ppc,"-size|-s(size=VGA) -input|-i(device,device-params)",
                init,run).exec();
}
