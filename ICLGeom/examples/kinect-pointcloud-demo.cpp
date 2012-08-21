/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/kinect-pointcloud-demo.cpp            **
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

#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLQt/DrawWidget3D.h>

#include <ICLGeom/PointcloudSceneObject.h>
#include <ICLGeom/PointNormalEstimation.h>

#include <ICLUtils/ConfigFile.h>

GUI gui("hsplit");
GenericGrabber grabDepth, grabColor;
int KINECT_CAM=0,VIEW_CAM=1;

Camera cam;

PointcloudSceneObject *obj;
PointNormalEstimation *normalEstimator;

ButtonGroupHandle usedVisualizationHandle;
ButtonGroupHandle usedFilterHandle;

Scene scene;

Img8u heatmapImage;

FixedMatrix<float,3,3> homo;

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
  grabDepth.setProperty("depth-image-unit","raw");
  grabColor.init("kinectc","kinectc=0");
  
  grabDepth.useDesired(depth32f, pa("-size"), formatMatrix);
  grabColor.useDesired(depth8u, pa("-size"), formatRGB);
  
  Size size = pa("-size");

  normalEstimator = new PointNormalEstimation(size);

  if(pa("-rgb-udist")){
    string fn1 = pa("-rgb-udist");
    ImageUndistortion udistRGB=ImageUndistortion(fn1);
    grabColor.enableUndistortion(udistRGB);
  }
  if(pa("-ir-udist")){
    string fn2 = pa("-ir-udist");
    ImageUndistortion udistIR=ImageUndistortion(fn2);
    grabDepth.enableUndistortion(udistIR);
    grabDepth.setUndistortionInterpolationMode(interpolateNN);
  }
  
  GUI controls = HBox().minSize(12,12);
  GUI controls2 = HBox().minSize(12,12);
  controls << ( VBox()
                << Button("reset view").handle("resetView")
                << Fps(10).handle("fps")
                << FSlider(0.9,1.1,1.0).out("depthScaling").label("depth scaling")
                << Slider(1,10,3).out("pointSize").label("point size")
                << Slider(1,10,1).out("lineWidth").label("line width")
                << ButtonGroup("UniColor, RGB, Depth, Normals Cam, Normals World").handle("usedVisualization")
                << CamCfg("")
                << Button("disable CL"," enable CL").out("disableCL")
                << Button("vSync Off"," vSync On").out("vSync")
                << Button("show CoordFrame"," hide CoordFrame").out("showCoord")
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


  if(pa("-c")){
    string camname = pa("-c");
    cam = Camera(camname);
  }else if(size == Size::QVGA){
    cam = Camera("kinect-color-qvga.xml");
  }else if (size == Size::VGA){
    cam = Camera("kinect-color-vga.xml");
  }else{
    throw ICLException("unsupported size: supported is VGA and QVGA");
  }
  cam.setName("Kinect Camera");

  usedVisualizationHandle= gui.get<ButtonGroupHandle>("usedVisualization");
	usedVisualizationHandle.select(0);
  usedFilterHandle= gui.get<ButtonGroupHandle>("usedFilter");
  usedFilterHandle.select(1);

  // kinect camera
  scene.addCamera(cam);
  
  //  view camera
  scene.addCamera(cam);
  
  FixedMatrix<float,3,3> homogeneity = FixedMatrix<float,3,3>::id();
  
  try{
    string homname = pa("-h");    		
    //ConfigFile fhom("homogeneity.xml");
    ConfigFile fhom(homname);
    fhom.setPrefix("config.");
		homogeneity = fhom["homogeneity"].as<FixedMatrix<float,3,3> >();
  }catch(ICLException &ex){
    SHOW(ex.what());
  }catch(...){}
 
  homo=homogeneity;
  
  obj  = new PointcloudSceneObject(size,scene.getCamera(KINECT_CAM));
  scene.addObject(obj);
  
  DrawHandle3D draw3D = gui["draw3D"];
  
  mouse = new AdaptedSceneMouseHandler(scene.getMouseHandler(VIEW_CAM));
  draw3D->install(mouse);
 
  scene.setDrawCoordinateFrameEnabled(false);
  scene.setLightingEnabled(false);

  DrawHandle3D hdepth = gui["hdepth"];
  hdepth->setRangeMode(ICLWidget::rmAuto);
}


void run(){
  bool heatSet=false;
 
  if(gui["disableCL"]){
    obj->setUseCL(false);
    normalEstimator->setUseCL(false);
  }else{
    obj->setUseCL(true);
    normalEstimator->setUseCL(true);
  }

  if(gui["drawLines"]){
    obj->setUseDrawNormalLines(true);
  }else{
    obj->setUseDrawNormalLines(false);
  }
  
  scene.lock();
  if(gui["showCoord"]){
    scene.setDrawCoordinateFrameEnabled(true);
  }else{
    scene.setDrawCoordinateFrameEnabled(false);
  }
  scene.unlock();

  obj->lock();

  int normalrange = gui["normalrange"];
	int avgrange = gui["avgrange"];
	
	int lineWidth = gui["lineWidth"];
	obj->setLineWidth(lineWidth);
	
	int granularity =gui["lineGranularity"];
	float lineLength =gui["lineLength"];
	obj->setNormalLinesLength(lineLength);
	obj->setNormalLinesGranularity(granularity);

  int pointSize=gui["pointSize"];
  float depthS=gui["depthScaling"];
  obj->setDepthScaling(depthS);
  obj->setPointSize(pointSize);
 
  
  ButtonHandle resetView = gui["resetView"];
  DrawHandle3D hdepth = gui["hdepth"];
  DrawHandle3D hcolor = gui["hcolor"];

  if(resetView.wasTriggered()){
    scene.getCamera(VIEW_CAM) = scene.getCamera(KINECT_CAM);
  }
  const Img8u &colorImage = *grabColor.grab()->asImg<icl8u>();
  const Img32f &depthImage = *grabDepth.grab()->asImg<icl32f>();
    
  if(gui["heatmap"]){//heatmap image  
    if(gui["vSync"]){
      heatmapImage=obj->calculatePseudocolorDepthImage(depthImage,false);
    }else{
      heatmapImage=obj->calculatePseudocolorDepthImage(depthImage,true);
    }
    heatSet=true;//heatmap image calculated
    hdepth = &heatmapImage;
  }else{//depth image
    hdepth = &depthImage;
  }
	
	usedVisualizationHandle = gui.get<ButtonGroupHandle>("usedVisualization");
	
	if(usedVisualizationHandle.getSelected()==0){//unicolor
	  int cR=gui["unicolorR"];
	  int cG=gui["unicolorG"];
	  int cB=gui["unicolorB"];
      if(gui["vSync"]){//without vSync
        obj->calculateUniColor(depthImage, GeomColor(cR,cG,cB,255) ,false);
      }
      else{//with vSync
        obj->calculateUniColor(depthImage, GeomColor(cR,cG,cB,255), true);
      }
  
  }else if(usedVisualizationHandle.getSelected()==1){//rgb
    if(gui["vSync"]){
      obj->calculateRGBColor(depthImage,colorImage, homo, false);
    }else{
      obj->calculateRGBColor(depthImage,colorImage, homo, true);
    }
  }else if(usedVisualizationHandle.getSelected()==2){//pseudocolor
    if(heatSet==false){//if heatmap image not calculated
      if(gui["vSync"]){
        obj->calculatePseudoColor(depthImage, false);
      }else{
        obj->calculatePseudoColor(depthImage, true);
      }
    }else{//if heatmap image is calculated
      if(gui["vSync"]){
        obj->calculatePseudoColor(depthImage, heatmapImage, false);
      }else{
        obj->calculatePseudoColor(depthImage, heatmapImage, true);
      }
    }
  
  }else if(usedVisualizationHandle.getSelected()==3){//normals cam 
    usedFilterHandle = gui.get<ButtonGroupHandle>("usedFilter");
	  if(usedFilterHandle.getSelected()==1){ //median 3x3
		  normalEstimator->setMedianFilterSize(3);
		  normalEstimator->setDepthImage(depthImage);
      normalEstimator->medianFilter();
	  }
	  else if(usedFilterHandle.getSelected()==2){ //median 5x5
		  normalEstimator->setMedianFilterSize(5);
		  normalEstimator->setDepthImage(depthImage);
      normalEstimator->medianFilter();
	  }
	  else{
		  normalEstimator->setFilteredImage((Img32f&)depthImage);
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
    PointNormalEstimation::Vec4* normals=normalEstimator->getNormals();
    if(gui["vSync"]){
      obj->calculateNormalDirectionColor(depthImage, normals, false);
    }else{
      obj->calculateNormalDirectionColor(depthImage, normals, true);
    }
    
  }else if(usedVisualizationHandle.getSelected()==4){//normals world
    usedFilterHandle = gui.get<ButtonGroupHandle>("usedFilter");
	  if(usedFilterHandle.getSelected()==1){ //median 3x3
		  normalEstimator->setMedianFilterSize(3);
		  normalEstimator->setDepthImage(depthImage);
      normalEstimator->medianFilter();
	  }
	  else if(usedFilterHandle.getSelected()==2){ //median 5x5
		  normalEstimator->setMedianFilterSize(5);
		  normalEstimator->setDepthImage(depthImage);
      normalEstimator->medianFilter();
	  }
	  else{
		  normalEstimator->setFilteredImage((Img32f&)depthImage);
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
    PointNormalEstimation::Vec4* normals=normalEstimator->getNormals();
    if(gui["vSync"]){
      obj->calculateNormalDirectionColor(depthImage, normals, cam, false);
    }else{
      obj->calculateNormalDirectionColor(depthImage, normals, cam, true);
    }
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
  return ICLApp(n,ppc,"-size|-s(Size=QVGA) -cam-config|-c(file=cam-QVGA.xml) -rgb-udist(fn1) -ir-udist(fn2) -homogeneity|-h(file=homogeneity.xml)",init,run).exec();
}

