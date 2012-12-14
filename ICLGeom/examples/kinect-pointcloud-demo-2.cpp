/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/kinect-pointcloud-demo-2.cpp          **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann                                       **
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

#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>

#include <ICLGeom/PointCloudCreator.h>
#include <ICLGeom/PointCloudObject.h>
#include <ICLGeom/PointNormalEstimation.h>
#include <ICLCore/PseudoColorConverter.h>
#include <ICLFilter/MotionSensitiveTemporalSmoothing.h>

HSplit gui;
GenericGrabber grabDepth, grabColor;
int KINECT_CAM=0,VIEW_CAM=1;

Camera depthCam, colorCam;

PointCloudObject *obj;
PointCloudCreator *creator;
PointNormalEstimation *normalEstimator;
PseudoColorConverter *pseudoColorConverter;
MotionSensitiveTemporalSmoothing *temporalSmoothing;

ButtonGroupHandle usedVisualizationHandle;
ButtonGroupHandle usedFilterHandle;

Scene scene;

Img8u normalsImage;

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

  normalEstimator = new PointNormalEstimation(size);
  
  std::vector<PseudoColorConverter::Stop> stops; //create heatmap
  stops.push_back(PseudoColorConverter::Stop(0.25, Color(255,0,0)));
  stops.push_back(PseudoColorConverter::Stop(0.35, Color(255,255,0)));
  stops.push_back(PseudoColorConverter::Stop(0.45, Color(0,255,0)));
  stops.push_back(PseudoColorConverter::Stop(0.55, Color(0,255,255)));
  stops.push_back(PseudoColorConverter::Stop(0.8, Color(0,0,255)));
  pseudoColorConverter = new PseudoColorConverter(stops, 2046);
  
  temporalSmoothing = new MotionSensitiveTemporalSmoothing(2047, 15);

  //create GUI
  GUI controls = HBox().minSize(12,12);
  GUI controls2 = HBox().minSize(12,12);
    
  controls << ( VBox()
            << Button("reset view").handle("resetView")
            << Fps(10).handle("fps")
            << FSlider(0.9,1.1,1.0).out("depthScaling").label("depth scaling")
            << Slider(1,10,3).out("pointSize").label("point size")
            << Slider(1,10,1).out("lineWidth").label("line width")
            << ButtonGroup("UniColor, RGB, Depth, Normals").handle("usedVisualization")
            << CamCfg("")
            << Button("disable CL"," enable CL").out("disableCL")
            << Button("draw NormalLines"," hide normalLines").out("drawLines")
            << FSlider(2.0,100.0,40.0).out("lineLength").label("normalLineLength")
            << Slider(1,20,4).out("lineGranularity").label("normalLineGranularity")
          );
  controls2 << ( VBox()
            << Slider(0,255,0).out("unicolorR").label("unicolor R").handle("unicolorRHandle")
            << Slider(0,255,0).out("unicolorG").label("unicolor G").handle("unicolorGHandle")
            << Slider(0,255,255).out("unicolorB").label("unicolor B").handle("unicolorBHandle")
            << ButtonGroup("unfiltered, median3x3, median5x5").handle("usedFilter").label("normals filter")
            << Slider(1,15,2).out("normalrange").label("normal range").handle("normalrangeHandle")
            << Button("disable averaging"," enable averaging").out("disableAveraging")
            << Slider(1,15,1).out("avgrange").label("normal averaging range").handle("avgrangeHandle")
            << Button("enable temp. smoothing"," disable temp. smoothing").out("enableSmoothing")
            << Slider(1,15,5).out("filterSize").label("filterSize").maxSize(100,2).handle("filterSize-handle")
            << Slider(1,22,10).out("difference").label("difference").maxSize(100,2).handle("difference-handle")
        );
  gui << ( VBox() 
           << Draw3D().handle("hdepth").minSize(10,8)
           << Button("heatmap","gray").out("heatmap")
           << Draw3D().handle("hcolor").minSize(10,8)
         )
      << ( HSplit() 
           << Draw3D().handle("draw3D").minSize(40,30)
           << controls
           << controls2
           )
      << Show();

  if(pa("-d")){//get depth cam
    string depthcamname = pa("-d");
    depthCam = Camera(depthcamname);
  }
  else{//default depth cam
    depthCam = Camera();
    depthCam.setResolution(size);
  }
  depthCam.setName("Kinect Depth Camera");
 
  if(pa("-c")){//get color cam
    string colorcamname = pa("-c");
    colorCam = Camera(colorcamname);
  }
  colorCam.setName("Kinect Color Camera");

  usedVisualizationHandle= gui.get<ButtonGroupHandle>("usedVisualization");
	usedVisualizationHandle.select(0);
  usedFilterHandle= gui.get<ButtonGroupHandle>("usedFilter");
  usedFilterHandle.select(1);

  // kinect camera
  scene.addCamera(depthCam);
  
  //  view camera
  scene.addCamera(depthCam);
  
  scene.setDrawCoordinateFrameEnabled(true);
  scene.setDrawCamerasEnabled(true);
  
  usedVisualizationHandle.disable(1);
  usedVisualizationHandle.disable(3);
    
  if(pa("-d") && pa("-c")){//enable all options for depth and color
    usedVisualizationHandle.enable(1);
    usedVisualizationHandle.enable(3);
  }
  else if(pa("-d")){//enable normal mapping for depth only
    usedVisualizationHandle.enable(3);
  }
  
  obj  = new PointCloudObject(size.width, size.height,true,true,true);
  
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
}


void run(){
  bool heatSet=false;
 
  if(gui["disableCL"]){//enable/disable OpenCL
    normalEstimator->setUseCL(false);
    temporalSmoothing->setUseCL(false);
    creator->setUseCL(false);
  }else{
    normalEstimator->setUseCL(true);
    temporalSmoothing->setUseCL(true);
    creator->setUseCL(true);
  }

  obj->lock();

  int normalrange = gui["normalrange"];
	int avgrange = gui["avgrange"];
	
	int lineWidth = gui["lineWidth"];
	obj->setLineWidth(lineWidth);
	
	int granularity =gui["lineGranularity"];
	float lineLength =gui["lineLength"];

  int pointSize=gui["pointSize"];
  float depthScaling=gui["depthScaling"];
  obj->setPointSize(pointSize);
 
  ButtonHandle resetView = gui["resetView"];
  DrawHandle3D hdepth = gui["hdepth"];
  DrawHandle3D hcolor = gui["hcolor"];

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
      heatSet=true;//heatmap image calculated
      hdepth = heatmapImage;//->as8u();
    }else{//depth image
      hdepth = filteredImage;
    }
	}else{
    if(gui["heatmap"]){//heatmap image  
      pseudoColorConverter->apply(&depthImage,&heatmapImage);
      heatSet=true;//heatmap image calculated
      hdepth = heatmapImage;//->as8u();
    }else{//depth image
      hdepth = &depthImage;
    }
  }
	
	usedVisualizationHandle = gui.get<ButtonGroupHandle>("usedVisualization");
	
	if(usedVisualizationHandle.getSelected()==0){//unicolor
	  int cR=gui["unicolorR"];
	  int cG=gui["unicolorG"];
	  int cB=gui["unicolorB"];
	  GeomColor c(cR/255.,cG/255.,cB/255.,1.);
	  obj->selectRGBA32f().fill(c);
	  if(gui["enableSmoothing"]){
	    creator->create(*filteredImage->as32f(), *obj, 0, depthScaling);//, colorImage.as8u());
	  }else{
	    creator->create(*depthImage.as32f(), *obj, 0, depthScaling);//, colorImage.as8u());
	  }
  
  }else if(usedVisualizationHandle.getSelected()==1){//rgb
    if(gui["enableSmoothing"]){
      creator->create(*filteredImage->as32f(), *obj, colorImage.as8u(), depthScaling);
    }else{
      creator->create(*depthImage.as32f(), *obj, colorImage.as8u(), depthScaling);
	  }
  
  }else if(usedVisualizationHandle.getSelected()==2){//pseudocolor
    if(heatSet==false){//if heatmap image not calculated
      if(gui["enableSmoothing"]){
        pseudoColorConverter->apply(filteredImage,&heatmapImage);
      }else{
        pseudoColorConverter->apply(&depthImage,&heatmapImage);
      }
    }
    if(gui["enableSmoothing"]){
      creator->create(*filteredImage->as32f(), *obj, 0, depthScaling);
    }else{
      creator->create(*depthImage.as32f(), *obj, 0, depthScaling);
    }
    obj->setColorsFromImage(*heatmapImage);////*heatmapImage->as8u());
	  
  }else if(usedVisualizationHandle.getSelected()==3){//normals cam    
    usedFilterHandle = gui.get<ButtonGroupHandle>("usedFilter");
	  if(usedFilterHandle.getSelected()==1){ //median 3x3
		  normalEstimator->setMedianFilterSize(3);
		  if(gui["enableSmoothing"]){
		    normalEstimator->setDepthImage(*filteredImage->as32f());
		  }else{
		    normalEstimator->setDepthImage(*depthImage.as32f());
		  }
      normalEstimator->medianFilter();
	  }
	  else if(usedFilterHandle.getSelected()==2){ //median 5x5
		  normalEstimator->setMedianFilterSize(5);
		  if(gui["enableSmoothing"]){
		    normalEstimator->setDepthImage(*filteredImage->as32f());
      }else{
        normalEstimator->setDepthImage(*depthImage.as32f());
      }
      normalEstimator->medianFilter();
	  }
	  else{
	    if(gui["enableSmoothing"]){
		    normalEstimator->setFilteredImage((Img32f&)depthImage);
		  }else{
		    normalEstimator->setDepthImage(*depthImage.as32f());
		  }
	  }
	  normalEstimator->setNormalCalculationRange(normalrange);	
	  normalEstimator->setNormalAveragingRange(avgrange);	   
    if(gui["disableAveraging"]){
      normalEstimator->setUseNormalAveraging(false);
    }
    else{
      normalEstimator->setUseNormalAveraging(true);
    }  
    normalEstimator->normalCalculation();
    normalEstimator->worldNormalCalculation(depthCam);
    normalsImage=normalEstimator->getNormalImage();
    if(gui["enableSmoothing"]){
      creator->create(*filteredImage->as32f(), *obj, 0, depthScaling);
    }else{
      creator->create(*depthImage.as32f(), *obj, 0, depthScaling);
	  }
    obj->setColorsFromImage(normalsImage);
        
    const DataSegment<float,3> xyz = obj->selectXYZ();//get pointcloud data
    const DataSegment<float,4> normal = obj->selectNormal();//get pointcloud normal data 
    
    PointNormalEstimation::Vec4* norms=normalEstimator->getWorldNormals(); //get world normals
    
    DataSegment<float,4>((float*)norms,sizeof(PointNormalEstimation::Vec4),normal.getDim()).deepCopy(normal); //set pointcloud normal data
  }
  
  bool drawLines=gui["drawLines"];
  if(drawLines && usedVisualizationHandle.getSelected()==3){//draw normal lines
    obj->setUseDrawNormalLines(true, lineLength, granularity);
  }else{
    obj->setUseDrawNormalLines(false, lineLength, granularity);
  }
  
  obj->unlock();
      
  hcolor = &colorImage;
  
  gui["fps"].render();
  hdepth.render();
  hcolor.render();

  gui["draw3D"].link(scene.getGLCallback(VIEW_CAM));
  gui["draw3D"].render();  
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"-size|-s(Size=QVGA) -depth-cam|-d(file) -color-cam|-c(file))",init,run).exec();
}

