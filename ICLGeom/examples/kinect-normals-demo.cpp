/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/kinect-normals-demo.cpp               **
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
#include <ICLGeom/PointNormalEstimation.h>
#include <sys/time.h>

GUI gui("hsplit");
GenericGrabber grabDepth, grabColor;

PointNormalEstimation *normalEstimator;

ButtonGroupHandle usedFilterHandle;
ButtonGroupHandle usedAngleHandle;

Img32f edgeImage(Size(320,240), formatGray);
Img32f angleImage(Size(320,240), formatGray);

void init(){

  Size size = pa("-size");
  edgeImage.setSize(size);
  angleImage.setSize(size);
    
  normalEstimator = new PointNormalEstimation(size);
  
  grabDepth.init("kinectd","kinectd=0");
  grabDepth.setProperty("depth-image-unit","raw");
  grabColor.init("kinectc","kinectc=0");
  grabDepth.useDesired(depth32f, size, formatMatrix);
  grabColor.useDesired(depth8u, size, formatRGB);
    
  GUI controls("vbox[@minsize=12x2]");
  controls << "fps(10)[@handle=fps]"
	   << "fslider(0.7,1.0,0.89)[@out=threshold@label=threshold@handle=thresholdHandle]"
	   << "buttongroup(unfiltered, median3x3, median5x5)[@handle=usedFilter]"
	   << "slider(1,15,2)[@out=normalrange@label=normal range@handle=normalrangeHandle]"
	   << "togglebutton(disable averaging, enable averaging)[@out=disableAveraging]"
	   << "slider(1,15,1)[@out=avgrange@label=averaging range@handle=avgrangeHandle]"
	   << "buttongroup(max, mean)[@handle=usedAngle]"
	   << "slider(1,15,3)[@out=neighbrange@label=neighborhood range@handle=neighbrangeHandle]"
	   << "togglebutton(disable CL, enable CL)[@out=disableCL]";
  
  gui << "image[@handle=depth@minsize=16x12]"
      << "image[@handle=color@minsize=16x12]"
      << "image[@handle=angle@minsize=16x12]"
      << "image[@handle=edge@minsize=16x12]"
      << controls
      << "!show";
  
  usedFilterHandle= gui.getValue<ButtonGroupHandle>("usedFilter");
  usedFilterHandle.select(1);
  usedAngleHandle= gui.getValue<ButtonGroupHandle>("usedAngle");
  usedAngleHandle.select(0);
	
  gui_ImageHandle(depth);
  depth->setRangeMode(ICLWidget::rmAuto);
  gui_ImageHandle(angle);
  angle->setRangeMode(ICLWidget::rmAuto);
}

void run(){
  Size size = pa("-size");

  const Img8u &colorImage = *grabColor.grab()->asImg<icl8u>();
  const Img32f &depthImage = *grabDepth.grab()->asImg<icl32f>();
    
  if(gui["disableCL"]){
    normalEstimator->setUseCL(false);
  }
  else{
    normalEstimator->setUseCL(true);
  }
  
  int normalrange = gui["normalrange"];
  int neighbrange = gui["neighbrange"];
  float threshold = gui["threshold"];
  int avgrange = gui["avgrange"];
	
  timeval start, end;
  gettimeofday(&start, 0);

  usedFilterHandle = gui.getValue<ButtonGroupHandle>("usedFilter");
  if(usedFilterHandle.getSelected()==1){ //median 3x3
    normalEstimator->setMedianFilterSize(3);
  }
  else if(usedFilterHandle.getSelected()==2){ //median 5x5
    normalEstimator->setMedianFilterSize(5);
  }
  else{
    std::cout<<"FILTER NOT FOUND"<<std::endl;
  }

  normalEstimator->setNormalCalculationRange(normalrange);	
  normalEstimator->setNormalAveragingRange(avgrange);	
  usedAngleHandle = gui.getValue<ButtonGroupHandle>("usedAngle");
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
    
  if(usedFilterHandle.getSelected()==0){//unfiltered
    if(gui["disableAveraging"]){
      edgeImage=normalEstimator->calculate(depthImage, false, false);
    }
    else{//normal averaging
      edgeImage=normalEstimator->calculate(depthImage, false, true);
    }
  }else{
    if(gui["disableAveraging"]){//filtered
      edgeImage=normalEstimator->calculate(depthImage, true, false);
    }else{//normal averaging
      edgeImage=normalEstimator->calculate(depthImage, true, true);
    }
  }
    
  //access interim result
  angleImage=normalEstimator->getAngleImage();
    
  gettimeofday(&end, 0);
  if(normalEstimator->isCLReady()==true && normalEstimator->isCLActive()==true){
    std::cout<<"Size: "<<size<<" ,Open CL, Runtime: ";
  }
  else{ 
    std::cout<<"Size: "<<size<<" ,CPU, Runtime: ";
  }
  std::cout <<((end.tv_usec-start.tv_usec)+((end.tv_sec-start.tv_sec)*1000000))/1000 <<" ms" << endl;

  gui["depth"] = depthImage;
  gui["color"] = colorImage;
  gui["angle"] = angleImage;
  gui["edge"] = edgeImage;

  gui["fps"].render();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-size|-s(Size=QVGA)",init,run).exec();
}
