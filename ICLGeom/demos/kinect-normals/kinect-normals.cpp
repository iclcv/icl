/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/kinect-normals/kinect-normals.cpp        **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann                                       **
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
#include <ICLGeom/ObjectEdgeDetector.h>
#include <ICLUtils/Time.h>

HSplit gui;
GenericGrabber grabDepth, grabColor;

ObjectEdgeDetector *objectEdgeDetector;

ButtonGroupHandle usedFilterHandle;
ButtonGroupHandle usedSmoothingHandle;
ButtonGroupHandle usedAngleHandle;

Img8u edgeImage(Size(320,240), formatGray);
Img32f angleImage(Size(320,240), formatGray);
Img8u normalImage(Size(320,240), formatRGB);

Camera cam;

void init(){

  Size size = pa("-size");
  edgeImage.setSize(size);
  angleImage.setSize(size);

  if(pa("-fcpu")){
    objectEdgeDetector = new ObjectEdgeDetector(ObjectEdgeDetector::CPU);
    std::cout<<"force CPU"<<std::endl;
  }else if(pa("-fgpu")){
    objectEdgeDetector = new ObjectEdgeDetector(ObjectEdgeDetector::GPU);
    std::cout<<"force GPU"<<std::endl;
  }else{
    objectEdgeDetector = new ObjectEdgeDetector(ObjectEdgeDetector::BEST);
    std::cout<<"use best"<<std::endl;
  }

  grabDepth.init("kinectd","kinectd=0");
  grabDepth.setPropertyValue("depth-image-unit","raw");
  grabColor.init("kinectc","kinectc=0");
  grabDepth.useDesired(depth32f, size, formatMatrix);
  grabColor.useDesired(depth8u, size, formatRGB);
  GUI controls = VBox().minSize(12,2);
  controls << Fps(10).handle("fps")
           << FSlider(0.7,1.0,0.89).out("threshold").label("threshold").handle("thresholdHandle")
           << ButtonGroup("unfiltered,median3x3,median5x5").handle("usedFilter")
           << Slider(1,15,2).out("normalrange").label("normal range").handle("normalrangeHandle")
           << Button("disable averaging","enable averaging").out("disableAveraging")
           << ButtonGroup("linear,gauss").handle("usedSmoothing")
           << Slider(1,15,1).out("avgrange").label("averaging range").handle("avgrangeHandle")
           << ButtonGroup("max,mean").handle("usedAngle")
           << Slider(1,15,3).out("neighbrange").label("neighborhood range").handle("neighbrangeHandle");

  gui << Image().handle("depth").minSize(16,12)
      << Image().handle("color").minSize(16,12)
      << Image().handle("angle").minSize(16,12)
      << Image().handle("edge").minSize(16,12)
      << Image().handle("normal").minSize(16,12)
      << controls
      << Show();

  if(pa("-cam")){
    std::string camname = pa("-cam").as<std::string>();
    cam=Camera(camname);
    cam.setName("Depth Camera");
  }

  usedFilterHandle= gui.get<ButtonGroupHandle>("usedFilter");
  usedFilterHandle.select(1);
  usedSmoothingHandle= gui.get<ButtonGroupHandle>("usedSmoothing");
  usedSmoothingHandle.select(0);
  usedAngleHandle= gui.get<ButtonGroupHandle>("usedAngle");
  usedAngleHandle.select(0);

  gui.get<ImageHandle>("depth")->setRangeMode(ICLWidget::rmAuto);
  gui.get<ImageHandle>("angle")->setRangeMode(ICLWidget::rmAuto);
}

void run(){

	WARNING_LOG("Hello");

  Size size = pa("-size");

  const Img8u &colorImage = *grabColor.grab()->asImg<icl8u>();
  const Img32f &depthImage = *grabDepth.grab()->asImg<icl32f>();

  WARNING_LOG("Hello");

  int normalrange = gui["normalrange"];
  int neighbrange = gui["neighbrange"];
  float threshold = gui["threshold"];
  int avgrange = gui["avgrange"];

  Time start, end;
  start = Time::now();

  WARNING_LOG("Hello");

  usedFilterHandle = gui.get<ButtonGroupHandle>("usedFilter");
  if(usedFilterHandle.getSelected()==1){ //median 3x3
    objectEdgeDetector->setMedianFilterSize(3);
  }
  else if(usedFilterHandle.getSelected()==2){ //median 5x5
    objectEdgeDetector->setMedianFilterSize(5);
  }

  WARNING_LOG("Hello");

  objectEdgeDetector->setNormalCalculationRange(normalrange);
  objectEdgeDetector->setNormalAveragingRange(avgrange);

  usedSmoothingHandle = gui.get<ButtonGroupHandle>("usedSmoothing");
  usedAngleHandle = gui.get<ButtonGroupHandle>("usedAngle");

  objectEdgeDetector->setAngleNeighborhoodMode(usedAngleHandle.getSelected());

  objectEdgeDetector->setAngleNeighborhoodRange(neighbrange);
  objectEdgeDetector->setBinarizationThreshold(threshold);

  WARNING_LOG("Hello");

  bool disableAveraging = gui["disableAveraging"];
  edgeImage=objectEdgeDetector->calculate(depthImage, usedFilterHandle.getSelected(),
                                           !disableAveraging, usedSmoothingHandle.getSelected());

  WARNING_LOG("Hello");

  //access interim result
  angleImage=objectEdgeDetector->getAngleImage();

  if(pa("-cam")){
    objectEdgeDetector->applyWorldNormalCalculation(cam);
    normalImage=objectEdgeDetector->getRGBNormalImage();
  }
  end = Time::now();
  std::cout<<"Size: "<<size<<" ,Runtime: ";
  std::cout <<(end-start).toMicroSeconds() <<" ms" << std::endl;

  WARNING_LOG("Hello");
  gui["depth"] = depthImage;
  gui["color"] = colorImage;
  gui["angle"] = angleImage;
  gui["edge"] = edgeImage;
  gui["normal"] = normalImage;
  WARNING_LOG("Hello");

  gui["fps"].render();
  WARNING_LOG("Hello");
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-size|-s(Size=QVGA) -cam|-c(file) -fcpu|force-cpu -fgpu|force-gpu",init,run).exec();
}
