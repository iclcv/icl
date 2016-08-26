/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/apps/lens-undistortion-calibration/            **
**          lens-undistortion-calibration.cpp                      **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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
#include <ICLMarkers/FiducialDetector.h>
#include <ICLCore/PseudoColorConverter.h>
#include <ICLMarkers/FiducialDetectorPlugin.h>
#include <ICLCV/RegionDetector.h>
#include <ICLMarkers/MarkerGridEvaluater.h>
#include <ICLMarkers/MarkerGridBasedUndistortionOptimizer.h>
#include "UndistortionUtil.h"

HBox gui;
//SmartPtr<FiducialDetector> fd;
GenericGrabber grabber;

typedef AdvancedMarkerGridDetector Detector;
typedef Detector::MarkerGrid MarkerGrid;
typedef Detector::AdvancedGridDefinition GridDef;

Detector detector;

MarkerGridEvaluater gridEval;


MarkerGridBasedUndistortionOptimizer opt;
SmartPtr<UndistortionUtil> udist;


void init(){
  grabber.init(pa("-i"));
  
  Size32f dummy(1,1);
  std::vector<int> ids;
  if(pa("-mi")){
    std::vector<int> ids = FiducialDetectorPlugin::parse_list_str(*pa("-mi"));
  }
  GridDef g(pa("-g"), dummy, dummy, ids, pa("-m"));

  detector.init(g);
  detector.setConfigurableID("fd");
  detector.setPropertyValue("thresh.global threshold", 21);
  detector.setPropertyValue("thresh.mask size", 30);

  const Size &imageSize = grabber.grab()->getSize();
  udist = new UndistortionUtil(imageSize);
  udist->setConfigurableID("udist");
    

  FiducialDetector *fd = detector.getFiducialDetector();
  fd->setPropertyValue("max bch errors",1);

  gui << ( Tab("distorted input,undistorted image,"
               "difference image,undistortion map").handle("tab").minSize(32,24)
           << Draw().handle("image")
           << Draw().handle("uimage")
           << Image().handle("diff")
           << Draw(imageSize).handle("map")
         )
      << ( VBox().maxSize(14,99).minSize(14,1)
           << Fps(10).handle("fps").maxSize(99,2).minSize(1,2)
           << ( HBox() 
                << Label("--").label("src error").handle("error").maxSize(99,2).minSize(1,2)
                << Label("--").label("fixed error").handle("uerror").maxSize(99,2).minSize(1,2)
                )
           << ( HBox()
                << CamCfg().maxSize(99,2).minSize(1,2)
                << Combo(fd->getIntermediateImageNames()).handle("vis").maxSize(99,2)
              )
           << CheckBox("show detection overlay",true).handle("overlay")
           << ( Tab("udist,markers,optimize")
                << Prop("udist")
                << Prop("fd")
                << (VBox()
                    << Button("capture frame").handle("capture")
                    << Button("clear frames").handle("clear")
                    << CheckBox("Use OpenCL",true).handle("use opencl")
                    << Label("--").handle("ncap").label("num captured")
                    << Label("--").handle("caperr").label("base error")
                    << Combo("simplex,sampling").handle("calibmode").label("calibration mode")
                    << Button("optimize").handle("optimize")
                    << Button("save").handle("save")
                    )
                )
           )
      << Show();

  gridEval.setGrid(detector.getMarkerGrid());
}

void run(){
  ButtonHandle cap = gui["capture"];
  ButtonHandle clear = gui["clear"];
  ButtonHandle optimize = gui["optimize"];
  ButtonHandle save = gui["save"];
  TabHandle tab = gui["tab"];
  DrawHandle draw = gui["image"];
  DrawHandle udraw = gui["uimage"];

  FiducialDetector *fd = detector.getFiducialDetector();
  const ImgBase * image = grabber.grab();

  const MarkerGrid &grid = detector.detect(image);

  static  FiducialDetector abc ("bch","[0-23]",ParamList("size","40x40"));
  
  gui["error"] = gridEval.evalError();
  
  const Img8u &useImage = *fd->getIntermediateImage(gui["vis"])->as8u();
  draw = useImage;
  if(gui["overlay"]){
    draw->draw(grid.vis());
    draw->draw(gridEval.vis());
  }
  /// undistorted image
  const Img8u &uDistUseImage = udist->undistort(useImage);
  udraw = uDistUseImage;

  detector.detect(&uDistUseImage);
  
  gui["uerror"] = gridEval.evalError();
  if(gui["overlay"]){
    udraw->draw(grid.vis());
    udraw->draw(gridEval.vis());
  }

  /// optimization ...
  if(clear.wasTriggered()){
    opt.clear();
  }
  if(cap.wasTriggered()){
    opt.add(grid);
  }
  gui["ncap"] = opt.size();
  const float k[9] = {0,0,0,0,0, 
                      float(useImage.getWidth())/2, float(useImage.getHeight())/2,
                      float(useImage.getWidth())/2, float(useImage.getHeight())/2 };
  gui["caperr"] = opt.computeError(k);

  if(optimize.wasTriggered()){
    //    std::vector<float> kOpt = storage.optimizeSample(k, 0,-1,1, std::vector<int>(5,10));
    opt.setUseOpenCL(gui["use opencl"]);
    std::string mode = gui["calibmode"];
    std::vector<float> kOpt;
    if(mode == "simplex"){
      kOpt = opt.optimizeAutoSimplex(useImage.getSize());
    }else if(mode == "sampling"){
      kOpt = opt.optimizeAutoSample(useImage.getSize());
    }else{
      ERROR_LOG("invalid calibration mode");
    }
    udist->setParamVector(kOpt.data());
  }

  if(tab.current() == 2){
    gui["diff"] = abs(cvt(useImage) - cvt(uDistUseImage));
  }else if(tab.current() == 3){
    static Img8u *pseudo = 0;
    static icl8u *lut[3] = {0,0,0};
    if(!pseudo){
      Img8u s(Size(256,1),1);
      for(int i=0;i<s.getWidth();++i){
        s(i,0,0) = i;
      }
      PseudoColorConverter c;
      pseudo = new Img8u(s.getSize(), formatRGB);
      c.apply(s, *pseudo);
      lut[0] = pseudo->begin(0);
      lut[1] = pseudo->begin(1);
      lut[2] = pseudo->begin(2);
    }
    
    DrawHandle m = gui["map"];
    m->color(0,100,255,255);
    const Img32f &wm = udist->getWarpMap();
    const Channel32f cx = wm[0], cy = wm[1];
    float maxLen = 0;
    const int w = cx.getWidth(), h = cx.getHeight();

    static Img8u displacementMap(Size(w,h),1);
    Channel8u c = displacementMap[0];
    for(int y=0;y<h;++y){
      for(int x=0;x<w;++x){
        float d = sqrt(sqr(cx(x,y)-x) + sqr(cy(x,y)-y));
        c(x,y) = iclMin((int)d, 255);
        if(d > maxLen) maxLen = d;
      }
    }
    if(maxLen > 40){
      float f = 1./(1 + maxLen/40);
      for(int i=0;i<w*h;++i){
        c[i] *= f;
      }
    }
    
    m = displacementMap;
    static RegionDetector rd;
    const std::vector<ImageRegion> &rs = rd.detect(&displacementMap);
    m->color(255,255,255,255);
    m->fill(0,0,0,0);
    if(rs.size() < 1000){
      for(size_t i=0;i<rs.size();++i){
        m->linestrip(rs[i].getBoundary());
      }
    }

    for(int y=0;y<h;y+=20){
      for(int x=0;x<w;x+=20){
        float d = sqrt(sqr(cx(x,y)-x) + sqr(cy(x,y)-y));
        int cidx = d/maxLen * 255;
        m->color(lut[0][cidx], lut[1][cidx], lut[2][cidx], 255);
        m->line(x,y,cx(x,y), cy(x,y));
      }
    }
    m->color(0,0,0,255);
    m->text("max displacement: " + str(maxLen), 5,12);
    m->color(255,255,255,255);
    m->text("max displacement: " + str(maxLen), 4,11);

    m->render();
  }

  if(save.wasTriggered()){
    udist->save();
  }
  
  draw.render();
  udraw.render();

  gui["fps"].render();
}

int main(int n, char **ppc){
  pa_explain("-g","Defines the calibration marker grid, which must be put\n"
             "onto a 100%ly planar surface. The grid (usually printed on an\n"
             "A4 sheet of paper), has W x H markers. \n"
             "Marker grid templates for printing can be created with the tool\n"
             "icl-create-marker-grid-svg <-o filemane> [...] \n"
             "The tool creates a standard svg (you can use an external tool\n"
             "such as inkscape to convert the resulting vector graphics to pdf\n"
             "or to print it directly. Bby default icl-create-marker-grid-svg\n"
             "creates a very detailed grid of 30x21 markers. As each of the \n"
             "resulting markers is only 8x8 mm large, we consider this as an\n"
             "extreme example, which is suited for mega-pixel cameras only.\n"
             "icl-lens-undistortion-calibration would have to be called like\n"
             "lens-undistortion-calibration -g 30x21 to work\n"
             "with that grid");
  pa_explain("-m","Marker type to be used, actually this should stay 'bch'!");
  pa_explain("-mi","Marker IDs that were used. By default, the IDs [0, WxH-1]\n"
             "are used. The marker IDs are always interpreted in row-major\n"
             "order. The ID-string can either be a comma-seperated list of IDs\n"
             ", such as '{0,2,3,4,5,99}', or a range string, such as '[0-99]'");

  std::cout << "Please don't use this application! use icl-lens-undistortion-calibration-opencv instead"
            << std::endl;

  return ICLApp(n,ppc,"-input|-i(2) -grid-dim|-g(cellSize=30x21) "
                "-marker-type|-m(type=bch) -marker-ids|-mi(ids)", init, run).exec();
}
