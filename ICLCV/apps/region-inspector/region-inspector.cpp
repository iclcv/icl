/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/apps/region-inspector/region-inspector.cpp       **
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

#include <ICLQt/Common.h>
#include <ICLCV/RegionDetector.h>
#include <ICLUtils/Time.h>
#include <ICLFilter/MedianOp.h>

HSplit gui;
RegionDetector rd;
GenericGrabber grabber;
Point mousePos;

void mouse(const MouseEvent &evt){
  mousePos = evt.getPos();
}


void init(){

  gui << Draw().minSize(32,24).label("image").handle("image")
      << ( VBox().label("Region information")
           << Label().label("Total Region Count").minSize(6,2).handle("total")
           << ( HBox()
                << Label().label("Region Size").handle("size-handle").minSize(6,2)
                << Label().label("Region COG").handle("cog-handle").minSize(6,2)
                )
           << ( HBox()
                << Label().label("Region Value").handle("val-handle").minSize(6,2)
                << Label().label("Region Form Factor").handle("ff-handle").minSize(6,2)
                )
           << ( HBox()
                << Label().label("Region EV-Ratio").handle("evratio-handle").minSize(6,2)
                << Label().label("Region Boundary Length").handle("bl-handle").minSize(6,2)
                )
           << ( HBox()
                << Label().label("sub regions").handle("nSub").minSize(6,2)
                << Label().label("all sub regions").handle("nAllSub").minSize(6,2)
                )
           << ( HBox() 
                << CheckBox("show boundary",true).out("showBoundary")
                << Button("normal","!thinned").out("showThinnedBoundary")
                )
           << ( HBox() 
                << CheckBox("show sub regions",true).out("showSubRegions")
                << Button("direct","all").out("showAllSubRegions")
                )
           << ( HBox() 
                << CheckBox("show sur. regions",true).out("showSurRegions")
                << Button("direct","all").out("showAllSurRegions")
                )
           << ( HBox() 
                << CheckBox("show neighbours").out("showNeighbours")
                << CheckBox("show bounding rect").out("showBB")
                )
           << ( HBox() 
                << Button("stopped","!grabbing").out("grabbing").handle("grab-handle").minSize(3,2)
                << Button("grab next").handle("grab-next-handle").minSize(3,2)
                )
           << Slider(2,256,10).out("levels").label("reduce levels")
           << Slider(1,10,0).out("medianSize").label("preprocessing median mask size")
           << Label("---").label("time for region detection").handle("timeRD")
           << Label("---").label("time for neighbour detection").handle("timeNB")
           << Label("---").label("time for sub region detection").handle("timeSR")
           << Label("---").label("time for sur. region detection").handle("timeSU")
           )
      << Show();

  grabber.init(pa("-i"));
  grabber.useDesired<Size>(pa("-s"));
  grabber.useDesired(formatGray);

  
  gui["image"].install(mouse);
}

void run(){
  static DrawHandle d = gui["image"];
  
  static LabelHandle timeRD = gui["timeRD"];
  static LabelHandle timeNB = gui["timeNB"];
  static LabelHandle timeSR = gui["timeSR"];
  static LabelHandle timeSU = gui["timeSU"];
  
  static LabelHandle nSub = gui["nSub"];
  static LabelHandle nAllSub = gui["nAllSub"];
  static LabelHandle total = gui["total"];
  
  static LabelHandle &valHandle = gui.get<LabelHandle>("val-handle");
  static LabelHandle &cogHandle = gui.get<LabelHandle>("cog-handle");
  static LabelHandle &sizeHandle = gui.get<LabelHandle>("size-handle");
  static LabelHandle &ffHandle = gui.get<LabelHandle>("ff-handle");
  static LabelHandle &evratioHandle = gui.get<LabelHandle>("evratio-handle");
  static LabelHandle &blHandle = gui.get<LabelHandle>("bl-handle");
  static bool &grabButtonDown = gui.get<bool>("grabbing");
  static ButtonHandle &grabNextHandle = gui.get<ButtonHandle>("grab-next-handle");

  int &levels = gui.get<int>("levels");
  int medianSize = gui["medianSize"];
  bool showSubRegions = gui["showSubRegions"];
  bool showAllSubRegions = gui["showAllSubRegions"];
  bool showSurRegions = gui["showSurRegions"];
  bool showAllSurRegions = gui["showAllSurRegions"];
  bool showNeighbours = gui["showNeighbours"];
  bool showBB = gui["showBB"];
  bool showBoundary = gui["showBoundary"];
  bool showThinnedBoundary = gui["showThinnedBoundary"];

  static int lastLevels = levels;
  static int lastMedianSize = medianSize;
  static const Img8u *grabbedImage;
  static Img8u reducedLevels;  

  static const Img8u *useImage = 0;
  static const std::vector<ImageRegion> *rs = 0;
  static SmartPtr<MedianOp> mo;
  
  
  int ms = medianSize;
  bool rdUpdated = false;
  if(grabNextHandle.wasTriggered() || !useImage || grabButtonDown){
    grabbedImage = grabber.grab()->as8u();
    useImage = grabbedImage;
    
    if(levels != 256){
      reducedLevels = cvt8u(icl::qt::levels(cvt(grabbedImage),levels));
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
      reducedLevels = cvt8u(icl::qt::levels(cvt(grabbedImage),levels));
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
    
  Point m = mousePos;
  
  if(useImage->getImageRect().contains(m.x,m.y)){
    // find the region, that contains mouseX,mouseY
    ImageRegion r = rd.click(m);
      
    if(r){
        
      d->nofill();
      if(showBoundary){
        d->color(0,150,255,200);
        d->linestrip(r.getBoundary(showThinnedBoundary));
      }
      if(showBB){
        d->color(255,0,0,255);
        d->rect(r.getBoundingBox());
      }

      if(showSurRegions){
        d->linewidth(4);
        d->color(255,200,100,100);
        Time t=Time::now();

        std::vector<ImageRegion> sur = showAllSurRegions ? r.getParentTree() : 
        std::vector<ImageRegion>(1,r.getParentRegion());

        timeSU = str((Time::now()-t).toMilliSeconds())+"ms";
        for(unsigned int i=0;i<sur.size();++i){
          if(sur[i]){
            d->linestrip(sur[i].getBoundary());
          }
        }
      }
      d->linewidth(1);

      if(showSubRegions){
        d->color(0,155,0,255);
        Time t=Time::now();
        const std::vector<ImageRegion> &sub = r.getSubRegions(!showAllSubRegions);
        timeSR = str((Time::now()-t).toMilliSeconds())+"ms";

        for(unsigned int i=0;i<sub.size();++i){
          if(!sub[i]) {
            ERROR_LOG("sub-region is null??");
          }else{
            d->linestrip(sub[i].getBoundary());
          }
        }
        if(showAllSubRegions){
          nAllSub = (int)sub.size();
        }else{
          nSub = (int)sub.size();
        }
      }
      if(showNeighbours){
        d->color(255,0,0,255);
        Time t=Time::now();
        const std::vector<ImageRegion> &ns = r.getNeighbours();
        timeNB = str((Time::now()-t).toMilliSeconds())+"ms";
        for(unsigned int i=0;i<ns.size();++i){
          d->linestrip(ns[i].getBoundary());
        }
          
      }
      valHandle = r.getVal();
      cogHandle = str(r.getCOG());
      sizeHandle = r.getSize();
      ffHandle = r.getFormFactor();
      evratioHandle = r.getPCAInfo().len2/r.getPCAInfo().len1;
      blHandle = r.getBoundaryLength();
    }else{
      valHandle = "no region";
      cogHandle = "";
      sizeHandle = "";
      ffHandle = "";
      evratioHandle = "";
      blHandle = "";
      
    }
  }
  d.render();
}


int main(int n, char **ppc){
  pa_explain("-input","define device parameters (e.g. -d dc 0 or -d file image/*.ppm)");
  return ICLApp(n,ppc,"-size|-s(size=VGA) [m]-input|-i(device,device-params)",
                init,run).exec();
}
