/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/kinect-segmentation/kinect-segmentation. **
**          cpp                                                    **
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
#include <ICLGeom/Geom.h>

#include <ICLGeom/PointCloudCreator.h>
#include <ICLGeom/PointCloudObject.h>
#include <ICLGeom/PointCloudNormalEstimator.h>
#include <ICLCore/PseudoColorConverter.h>
#include <ICLFilter/MotionSensitiveTemporalSmoothing.h>

#include <ICLGeom/Segmentation3D.h>

HSplit gui;
GenericGrabber grabDepth, grabColor;
int KINECT_CAM=0,VIEW_CAM=1;

Camera depthCam, colorCam;

PointCloudObject *obj;
PointCloudCreator *creator;
PointCloudNormalEstimator *normalEstimator;
PseudoColorConverter *pseudoColorConverter;
MotionSensitiveTemporalSmoothing *temporalSmoothing;
Segmentation3D *segmentation;

ButtonGroupHandle usedModeHandle;
ButtonGroupHandle usedFilterHandle;
ButtonGroupHandle usedSmoothingHandle;
ButtonGroupHandle usedAngleHandle;

Scene scene;

Img8u normalImage;
Img8u edgeImage;


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
  
  normalEstimator = new PointCloudNormalEstimator(size);

  std::vector<PseudoColorConverter::Stop> stops; //create heatmap
  stops.push_back(PseudoColorConverter::Stop(0.25, Color(255,0,0)));
  stops.push_back(PseudoColorConverter::Stop(0.35, Color(255,255,0)));
  stops.push_back(PseudoColorConverter::Stop(0.45, Color(0,255,0)));
  stops.push_back(PseudoColorConverter::Stop(0.55, Color(0,255,255)));
  stops.push_back(PseudoColorConverter::Stop(0.8, Color(0,0,255)));
  pseudoColorConverter = new PseudoColorConverter(stops, 2046);
  
  temporalSmoothing = new MotionSensitiveTemporalSmoothing(2047, 15);
  
  segmentation = new Segmentation3D(size);
  
  GUI controlsGeneral = HBox().minSize(12,12);
  GUI controlsLowLevel = HBox().minSize(12,12);
  GUI controlsHighLevel = HBox().minSize(12,12);
  GUI controlsTabs = HBox().minSize(12,12);
  
  controlsGeneral 
           << ( VBox()
           << Button("reset view").handle("resetView")
           << Fps(10).handle("fps")
           << FSlider(0.9,1.1,1.0).out("depthScaling").label("depth scaling")
           << CamCfg("")
           << ButtonGroup("RGB, Regions, Surfaces, Blobs, Greedy").handle("usedMode")
           << Button("disable CL"," enable CL").out("disableCL")
          );
  
  controlsLowLevel
           << ( VBox()
           << FSlider(0.7,1.0,0.89).out("threshold").label("threshold").handle("thresholdHandle")
           << ButtonGroup("unfiltered,median3x3,median5x5").handle("usedFilter").label("used filter")
           << Slider(1,15,1).out("normalrange").label("normal range").handle("normalrangeHandle")
           << Button("disable averaging","enable averaging").out("disableAveraging")
           << ButtonGroup("linear,gauss").handle("usedSmoothing").label("used smoothing")
           << Slider(1,15,2).out("avgrange").label("averaging range").handle("avgrangeHandle")
           << ButtonGroup("max,mean").handle("usedAngle").label("used angle")
           << Slider(1,15,1).out("neighbrange").label("neighborhood range").handle("neighbrangeHandle")
           << Button("enable temp. smoothing","!disable temp. smoothing").out("enableSmoothing")
           << Slider(1,15,6).out("filterSize").label("filterSize").maxSize(100,2).handle("filterSize-handle")
           << Slider(1,22,10).out("difference").label("difference").maxSize(100,2).handle("difference-handle")
          );
    
  controlsHighLevel 
           << ( VBox()
           << Slider(0,100,25).out("minClusterSize").label("min Cluster Size").handle("minClusterSizeHandle")
           << Button("ROI","FULL").out("useROI")
           << FSlider(-2000.0,0.0,-565.0).out("xMin").label("xMin").handle("xMinHandle")
           << FSlider(0.0,2000.0,550.0).out("xMax").label("xMax").handle("xMaxHandle")
           << FSlider(-2000.0,0.0,115.0).out("yMin").label("yMin").handle("yMinHandle")
           << FSlider(0.0,2000.0,1033.0).out("yMax").label("yMax").handle("yMaxHandle")
           << Button("Fast Growing","Normal Growing").out("useFastGrowing")
           << Slider(1,15,5).out("assignmentRadius").label("assignment radius").handle("assignmentRadiusHandle")
           << FSlider(5.0,30.0,15.0).out("assignmentMaxDistance").label("assignment max distance").handle("assignmentMaxDistanceHandle")
           << Slider(5,30,15).out("RANSACeuclDistance").label("RANSAC eucl distance").handle("RANSACeuclDistanceHandle")
           << Slider(1,100,20).out("RANSACpasses").label("RANSAC passes").handle("RANSACpassesHandle")
           << Slider(4,100,30).out("RANSACtolerance").label("RANSAC tolerance").handle("RANSACtoleranceHandle")
           << Slider(1,20,2).out("RANSACsubset").label("RANSAC subset").handle("RANSACsubsetHandle")
           << Slider(5,50,15).out("BLOBSeuclDistance").label("BLOBS eucl distance").handle("BLOBSeuclDistanceHandle")
          );
  
  controlsTabs
           << (Tab("general, low level, high level")
           << controlsGeneral
           << controlsLowLevel
           << controlsHighLevel
          );
  
  gui << ( VBox() 
           << Draw3D().handle("hdepth").minSize(10,8)
           << Button("heatmap","gray").out("heatmap")
           << Draw3D().handle("hcolor").minSize(10,8)
         )
      << ( VBox() 
           << Draw3D().handle("hedge").minSize(10,8)
           << Draw3D().handle("hnormal").minSize(10,8)
         )
      << ( HSplit() 
           << Draw3D().handle("draw3D").minSize(40,30)
           << controlsTabs
           )
      << Show();

  if(pa("-d")){//get depth cam
    string depthcamname = pa("-d").as<std::string>();
    depthCam = Camera(depthcamname);
  }
  else{//default depth cam
    depthCam = Camera();
    depthCam.setResolution(size);
  }
  depthCam.setName("Kinect Depth Camera");
 
  if(pa("-c")){//get color cam
    string colorcamname = pa("-c").as<std::string>();
    colorCam = Camera(colorcamname);
  }
  colorCam.setName("Kinect Color Camera");

  usedModeHandle= gui.get<ButtonGroupHandle>("usedMode");
	usedModeHandle.select(0);
  usedFilterHandle= gui.get<ButtonGroupHandle>("usedFilter");
  usedFilterHandle.select(1);
  usedAngleHandle= gui.get<ButtonGroupHandle>("usedAngle");
  usedAngleHandle.select(1);

  // kinect camera
  scene.addCamera(depthCam);
  
  //  view camera
  scene.addCamera(depthCam);
  
  scene.setDrawCoordinateFrameEnabled(true);
  scene.setDrawCamerasEnabled(true);
    
  obj = new PointCloudObject(size.width, size.height,true,true,true);
    
  //create pointcloud
  if(pa("-c")){
    creator = new PointCloudCreator(depthCam, colorCam, PointCloudCreator::KinectRAW11Bit);
  }
  else{
    creator = new PointCloudCreator(depthCam, PointCloudCreator::KinectRAW11Bit);
  }
  
  scene.addObject(obj);
  scene.setBounds(1000);
  
  DrawHandle3D draw3D = gui["draw3D"];
  
  mouse = new AdaptedSceneMouseHandler(scene.getMouseHandler(VIEW_CAM));
  draw3D->install(mouse);
 
  scene.setLightingEnabled(false);

  DrawHandle3D hdepth = gui["hdepth"];
  hdepth->setRangeMode(ICLWidget::rmAuto);
  
  obj->setPointSize(3);
}


void run(){
  ButtonHandle resetView = gui["resetView"];
  DrawHandle3D hdepth = gui["hdepth"];
  DrawHandle3D hcolor = gui["hcolor"];
  DrawHandle3D hedge = gui["hedge"];
  DrawHandle3D hnormal = gui["hnormal"];  
 
  if(gui["disableCL"]){//enable/disable OpenCL
    normalEstimator->setUseCL(false);
    temporalSmoothing->setUseCL(false);
    creator->setUseCL(false);
    segmentation->setUseCL(false);
  }else{
    normalEstimator->setUseCL(true);
    temporalSmoothing->setUseCL(true);
    creator->setUseCL(true);
    segmentation->setUseCL(true);
  }

  obj->lock();

  if(resetView.wasTriggered()){
    scene.getCamera(VIEW_CAM) = scene.getCamera(KINECT_CAM);
  }
  
  //grab images
  const ImgBase &colorImage = *grabColor.grab();
  const ImgBase &depthImage = *grabDepth.grab();
    
  static ImgBase *heatmapImage = 0;  
    
  int filterSize = gui["filterSize"];
  int difference = gui["difference"];
  temporalSmoothing->setFilterSize(filterSize);
  temporalSmoothing->setDifference(difference);
  
  static ImgBase *filteredImage = 0;
  if(gui["enableSmoothing"]){//temporal smoothing
    temporalSmoothing->apply(&depthImage,&filteredImage);    
    if(gui["heatmap"]){//heatmap image  
      pseudoColorConverter->apply(filteredImage,&heatmapImage);
      hdepth = heatmapImage;//->as8u();
    }else{//depth image
      hdepth = filteredImage;
    }
	}else{
    if(gui["heatmap"]){//heatmap image  
      pseudoColorConverter->apply(&depthImage,&heatmapImage);
      hdepth = heatmapImage;//->as8u();
    }else{//depth image
      hdepth = &depthImage;
    }
  }
    
  int normalrange = gui["normalrange"];
  int neighbrange = gui["neighbrange"];
  float threshold = gui["threshold"];
  int avgrange = gui["avgrange"];
	
  usedFilterHandle = gui.get<ButtonGroupHandle>("usedFilter");
  if(usedFilterHandle.getSelected()==1){ //median 3x3
    normalEstimator->setMedianFilterSize(3);
  }
  else if(usedFilterHandle.getSelected()==2){ //median 5x5
    normalEstimator->setMedianFilterSize(5);
  }

  normalEstimator->setNormalCalculationRange(normalrange);	
  normalEstimator->setNormalAveragingRange(avgrange);	
  
  usedSmoothingHandle = gui.get<ButtonGroupHandle>("usedSmoothing");
  usedAngleHandle = gui.get<ButtonGroupHandle>("usedAngle");
  if(usedAngleHandle.getSelected()==0){//max
    normalEstimator->setAngleNeighborhoodMode(0);
  }
  else if(usedAngleHandle.getSelected()==1){//mean
    normalEstimator->setAngleNeighborhoodMode(1);
  }
  else{
    std::cout<<"ANGLE CALCULATION METHOD NOT FOUND"<<std::endl;
  }

  normalEstimator->setAngleNeighborhoodRange(neighbrange);
  normalEstimator->setBinarizationThreshold(threshold);
    
  if(gui["enableSmoothing"]){//temporal smoothing
    if(usedFilterHandle.getSelected()==0){//unfiltered
      if(gui["disableAveraging"]){
        edgeImage=normalEstimator->calculate(*filteredImage->as32f(), false, false, false);
      }
      else{//normal averaging
        if(usedSmoothingHandle.getSelected()==0){//linear
          edgeImage=normalEstimator->calculate(*filteredImage->as32f(), false, true, false);
        }
        else if(usedSmoothingHandle.getSelected()==1){//gauss
          edgeImage=normalEstimator->calculate(*filteredImage->as32f(), false, true, true);
        }
      }
    }else{
      if(gui["disableAveraging"]){//filtered
        edgeImage=normalEstimator->calculate(*filteredImage->as32f(), true, false, false);
      }else{//normal averaging
        if(usedSmoothingHandle.getSelected()==0){//linear
          edgeImage=normalEstimator->calculate(*filteredImage->as32f(), true, true, false);
        }
        else if(usedSmoothingHandle.getSelected()==1){//gauss
          edgeImage=normalEstimator->calculate(*filteredImage->as32f(), true, true, true);
        }
      }
    }
  }else{
    if(usedFilterHandle.getSelected()==0){//unfiltered
      if(gui["disableAveraging"]){
        edgeImage=normalEstimator->calculate(*depthImage.as32f(), false, false, false);
      }
      else{//normal averaging
        if(usedSmoothingHandle.getSelected()==0){//linear
          edgeImage=normalEstimator->calculate(*depthImage.as32f(), false, true, false);
        }
        else if(usedSmoothingHandle.getSelected()==1){//gauss
          edgeImage=normalEstimator->calculate(*depthImage.as32f(), false, true, true);
        }
      }
    }else{
      if(gui["disableAveraging"]){//filtered
        edgeImage=normalEstimator->calculate(*depthImage.as32f(), true, false, false);
      }else{//normal averaging
        if(usedSmoothingHandle.getSelected()==0){//linear
          edgeImage=normalEstimator->calculate(*depthImage.as32f(), true, true, false);
        }
        else if(usedSmoothingHandle.getSelected()==1){//gauss
          edgeImage=normalEstimator->calculate(*depthImage.as32f(), true, true, true);
        }
      }
    }
  }
      
  if(pa("-d")){
    normalEstimator->applyWorldNormalCalculation(depthCam);
    normalImage=normalEstimator->getRGBNormalImage();
  } 
	
	usedModeHandle = gui.get<ButtonGroupHandle>("usedMode");
	int mode = usedModeHandle.getSelected();
	
	float depthScaling=gui["depthScaling"];
	
	if(pa("-c") && mode==0){//RGB
	  if(gui["enableSmoothing"]){
      creator->create(*filteredImage->as32f(), *obj, colorImage.as8u(), depthScaling);
    }else{
      creator->create(*depthImage.as32f(), *obj, colorImage.as8u(), depthScaling);
	  }  
  }else if(mode==0){//UniColor
	  GeomColor c(1.,0.,0.,1.);
	  obj->selectRGBA32f().fill(c);
	  if(gui["enableSmoothing"]){
	    creator->create(*filteredImage->as32f(), *obj, 0, depthScaling);//, colorImage.as8u());
	  }else{
	    creator->create(*depthImage.as32f(), *obj, 0, depthScaling);//, colorImage.as8u());
	  }
  }
  else{//all other modes
    if(gui["enableSmoothing"]){
	    creator->create(*filteredImage->as32f(), *obj, 0, depthScaling);//, colorImage.as8u());
	  }else{
	    creator->create(*depthImage.as32f(), *obj, 0, depthScaling);//, colorImage.as8u());
	  }
	}
  
  unsigned int minClusterSize = gui["minClusterSize"];
  int assignmentRadius = gui["assignmentRadius"];
  float assignmentMaxDistance = gui["assignmentMaxDistance"];
  int RANSACeuclDistance = gui["RANSACeuclDistance"];
  int RANSACpasses = gui["RANSACpasses"];
  int RANSACtolerance = gui["RANSACtolerance"];
  int RANSACsubset = gui["RANSACsubset"];
  int BLOBSeuclDistance = gui["BLOBSeuclDistance"];
  
  segmentation->setMinClusterSize(minClusterSize);
  segmentation->setAssignmentRadius(assignmentRadius);
  segmentation->setAssignmentMaxDistance(assignmentMaxDistance);
  segmentation->setRANSACeuclDistance(RANSACeuclDistance);
  segmentation->setRANSACpasses(RANSACpasses);
  segmentation->setRANSACtolerance(RANSACtolerance);
  segmentation->setRANSACsubset(RANSACsubset);
  segmentation->setBLOBSeuclDistance(BLOBSeuclDistance);
  
  if(gui["useROI"]){
    float xMin=gui["xMin"];
    float xMax=gui["xMax"];
    float yMin=gui["yMin"];
    float yMax=gui["yMax"];
    segmentation->setUseROI(true);
    segmentation->setROI(xMin,xMax,yMin,yMax);
  }else{
    segmentation->setUseROI(false);
  }
  
  if(gui["useFastGrowing"]){
    segmentation->setUseFastGrowing(true);
  }else{
    segmentation->setUseFastGrowing(false);
  }

  if(mode==1){//Regions
	  segmentation->setXYZH(obj->selectXYZH());
	  segmentation->setEdgeImage(edgeImage);
	  segmentation->clearData();  
	  segmentation->regionGrow();
	  segmentation->colorPointcloud();
    obj->setColorsFromImage(segmentation->getSegmentColorImage());
  }
  
  else if(mode==2){//Surfaces
	  segmentation->setXYZH(obj->selectXYZH());
	  segmentation->setEdgeImage(edgeImage);
    segmentation->clearData();
	  segmentation->regionGrow();
	  segmentation->calculatePointAssignmentAndAdjacency();
	  segmentation->colorPointcloud();
    obj->setColorsFromImage(segmentation->getSegmentColorImage());
  }
  else if(mode==3){//Blobs
    if(gui["enableSmoothing"]){
	    obj->setColorsFromImage(segmentation->segmentationBlobs(obj->selectXYZH(),edgeImage,*filteredImage->as32f()));
	  }else{
	    obj->setColorsFromImage(segmentation->segmentationBlobs(obj->selectXYZH(),edgeImage,*depthImage.as32f()));
	  }
  }
  
  else if(mode==4){//Greedy
	  if(gui["enableSmoothing"]){
	    obj->setColorsFromImage(segmentation->segmentation(obj->selectXYZH(),edgeImage,*filteredImage->as32f()));
	  }else{
	    obj->setColorsFromImage(segmentation->segmentation(obj->selectXYZH(),edgeImage,*depthImage.as32f()));
	  }
  }
  
  
  obj->unlock();
   
  hcolor = &colorImage;
  hedge = &edgeImage;
  if(pa("-d")){
    hnormal = &normalImage;
  }
  
  gui["fps"].render();
  hdepth.render();
  hcolor.render();
  hedge.render();
  hnormal.render();

  gui["draw3D"].link(scene.getGLCallback(VIEW_CAM));
  
  gui["draw3D"].render();  
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"-size|-s(Size=QVGA) -depth-cam|-d(file) -color-cam|-c(file))",init,run).exec();
}
