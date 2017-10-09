/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/kinect-depth-image-segmentation/         **
**          kinect-depth-image-segmentation.cpp                    **
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

#include <ICLGeom/ConfigurableDepthImageSegmenter.h>
#include <ICLCore/PseudoColorConverter.h>
#include <ICLGeom/Scene.h>
#include <ICLQt/Common.h>

HSplit gui;

GenericGrabber grabDepth, grabColor;
int KINECT_CAM=0,VIEW_CAM=1;

Camera depthCam;

PseudoColorConverter *pseudoColorConverter;
ConfigurableDepthImageSegmenter *segmentation;
PointCloudObject *obj;
Scene scene;

struct AdaptedSceneMouseHandler : public MouseHandler{
  Mutex mutex;
  MouseHandler *h;

  AdaptedSceneMouseHandler(MouseHandler *h):h(h){
  }

  void process(const MouseEvent &e){
    Mutex::Locker l(mutex);
      h->process(e);
  }

} *mouse = 0;


void init(){
  grabDepth.init("kinectd","kinectd=0");
  grabDepth.setPropertyValue("depth-image-unit","raw");
  grabColor.init("kinectc","kinectc=0");
  grabDepth.useDesired(depth32f, pa("-size"), formatMatrix);
  grabColor.useDesired(depth8u, pa("-size"), formatRGB);

  Size size = pa("-size");

  if(pa("-d")){//get depth cam
    string depthcamname = pa("-d").as<std::string>();
    depthCam = Camera(depthcamname);
  }else{//default depth cam
    depthCam = Camera();
    depthCam.setResolution(size);
  }
  depthCam.setName("Kinect Depth Camera");

  //initialize depth heatmap mapping
  std::vector<PseudoColorConverter::Stop> stops; //create heatmap
  stops.push_back(PseudoColorConverter::Stop(0.25, Color(255,0,0)));
  stops.push_back(PseudoColorConverter::Stop(0.35, Color(255,255,0)));
  stops.push_back(PseudoColorConverter::Stop(0.45, Color(0,255,0)));
  stops.push_back(PseudoColorConverter::Stop(0.55, Color(0,255,255)));
  stops.push_back(PseudoColorConverter::Stop(0.8, Color(0,0,255)));
  pseudoColorConverter = new PseudoColorConverter(stops, 2046);

  obj = new PointCloudObject(size.width, size.height,true,true,true);

  if(pa("-fcpu")){
    segmentation = new ConfigurableDepthImageSegmenter(ConfigurableDepthImageSegmenter::CPU, depthCam);
  }else if(pa("-fgpu")){
    segmentation = new ConfigurableDepthImageSegmenter(ConfigurableDepthImageSegmenter::GPU, depthCam);
  }else{
    segmentation = new ConfigurableDepthImageSegmenter(ConfigurableDepthImageSegmenter::BEST, depthCam);
  }

  GUI controls = HBox().minSize(12,12);

  controls << ( VBox()
                << Button("reset view").handle("resetView")
                << Fps(10).handle("fps")
                << Prop("segmentation").minSize(10,8)
              );

  gui << ( VBox()
           << Draw3D().handle("hdepth").minSize(10,8)
           << Draw3D().handle("hcolor").minSize(10,8)
         )
      << ( VBox()
           << Draw3D().handle("hedge").minSize(10,8)
           << Draw3D().handle("hnormal").minSize(10,8)
         )
      << ( HSplit()
           << Draw3D().handle("draw3D").minSize(40,30)
           << controls
         )
      << Show();

  // kinect camera
  scene.addCamera(depthCam);

  //  view camera
  scene.addCamera(depthCam);

  if(pa("-d")){
    scene.setDrawCoordinateFrameEnabled(true);
  }else{
    scene.setDrawCoordinateFrameEnabled(false);
  }

  scene.setDrawCamerasEnabled(true);

  scene.addObject(obj);
  scene.setBounds(1000);

  DrawHandle3D draw3D = gui["draw3D"];

  mouse = new AdaptedSceneMouseHandler(scene.getMouseHandler(VIEW_CAM));
  draw3D->install(mouse);

  scene.setLightingEnabled(false);
  obj->setPointSize(3);
}


void run(){
  ButtonHandle resetView = gui["resetView"];
  DrawHandle3D hdepth = gui["hdepth"];
  DrawHandle3D hcolor = gui["hcolor"];
  DrawHandle3D hedge = gui["hedge"];
  DrawHandle3D hnormal = gui["hnormal"];

  //reset camera view
  if(resetView.wasTriggered()){
    scene.getCamera(VIEW_CAM) = scene.getCamera(KINECT_CAM);
  }

  //grab images
  const ImgBase &colorImage = *grabColor.grab();
  const ImgBase &depthImage = *grabDepth.grab();

  //create heatmap
  static ImgBase *heatmapImage = 0;
  pseudoColorConverter->apply(&depthImage,&heatmapImage);

  //segment
  segmentation->apply(*depthImage.as32f(), *obj);
  core::Img8u edgeImg = segmentation->getEdgeImage();
  core::Img8u normImg = segmentation->getNormalImage();

  //display
  hdepth = heatmapImage;
  hcolor = &colorImage;
  hedge = &edgeImg;
  hnormal = &normImg;

  gui["fps"].render();
  hdepth.render();
  hcolor.render();
  hedge.render();
  hnormal.render();

  gui["draw3D"].link(scene.getGLCallback(VIEW_CAM));
  gui["draw3D"].render();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"-size|-s(Size=QVGA) -depth-cam|-d(file) -fcpu|force-cpu -fgpu|force-gpu)",init,run).exec();
}
