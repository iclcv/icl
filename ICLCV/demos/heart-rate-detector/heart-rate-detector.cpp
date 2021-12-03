/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2016 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/demos/plot-component/heart-rate-detector.cpp     **
** Module : ICLCV                                                  **
** Authors: Matthias Esau                                          **
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

#define ICL_NO_USING_NAMESPACES

#include <ICLQt/Common.h>
#include <ICLUtils/FPSLimiter.h>

#include <ICLUtils/Random.h>
#include <deque>
#include <ICLMath/FFTUtils.h>
#include <ICLCore/OpenCV.h>

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <ICLCV/HeartrateDetector.h>


using namespace icl::core;
using namespace icl::utils;
using namespace icl::io;
using namespace icl::qt;

GUI gui;
GenericGrabber grabber;
Rect face;
int faceCounter = 0;
icl::cv::HeartrateDetector *detector = 0;
cv::CascadeClassifier faceCascade;

Rect detectFace(Img8u *image)
{
  ::cv::Mat *frame = img_to_mat(image);
  cv::Mat frame_gray(*frame);
  cv::cvtColor( frame_gray, frame_gray, CV_RGB2GRAY );
  cv::equalizeHist( frame_gray, frame_gray );
  std::vector<cv::Rect> faces;
  faceCascade.detectMultiScale(frame_gray, faces);
  delete frame;
  if(faces.size()> 0) {
    int shrinkWidth = faces[0].width/5;
    int shrinkHeight = faces[0].height/5;
    return Rect(faces[0].x+shrinkWidth/2,faces[0].y+shrinkHeight/2,faces[0].width-shrinkWidth,faces[0].height-shrinkHeight);
  } else {
    return Rect(0,0,0,0);
  }
}

void init(){
  bool gl = pa("-gl");
  gui << Image().handle("image").minSize(16,12)
      << ( HBox().maxSize(100,2)
           << Fps(10).handle("fps").maxSize(100,2).minSize(5,2)
          )

      << (HBox()
          <<Plot(0,pa("-historydepth"),0,1,gl).handle("plot1").minSize(15,12)
          )
      << Show();

  grabber.init(pa("-i"));
  detector = new icl::cv::HeartrateDetector(pa("-maxfps"), pa("-historydepth"));
  std::string pathToCascades = pa_def("-c", std::string(""));
  if(pathToCascades.length() == 0)
    pathToCascades = std::string(ICL_OPENCV_INSTALL_PATH) + "/share/opencv/lbpcascades/lbpcascade_frontalface.xml";
  if(!faceCascade.load(pathToCascades)){
    ERROR_LOG("Loading face cascade: " << pathToCascades);
    exit(1);
  };
}

void run(){
  const ImgBase *grabbedImage = grabber.grab();
  Img8u *image = grabbedImage->asImg<icl::icl8u>()->deepCopy();

  if(faceCounter > detector->getFramerate()) {
    //add roi of the face to the heartrate detector
    image->setROI(face);
    detector->addImage(*image);
  } else {
    //find the face and make sure it's in the same position for about a second
    Rect newFace = detectFace(image);
    float faceDelta = 1.f;
    if(face.width > 0) {
      faceDelta = abs(face.x-newFace.x)/std::max(newFace.x,1)
                  +abs(face.y-newFace.y)/std::max(newFace.y,1)
                  +abs(face.width-newFace.width)/std::max(newFace.width,1)
                  +abs(face.height-newFace.height)/std::max(newFace.height,1);
      faceDelta /= 4;
    }
    face = newFace;
    if(faceDelta < 0.1) faceCounter++;
    else faceCounter = 0;
  }

  std::ostringstream oss;
  oss<<"heartrate:"<<detector->getHeartrate();

  gui["image"] = image;
  PlotHandle plot = gui["plot1"];
  plot->lock();
  plot->clear();

  //a plot showing the window that is currently considered for a possible heartrate
  plot->color(0,0,255);
  plot->fill(0,0,255,100);
  plot->label("window");
  plot->series(detector->getWindowBuffer());

  //a plot showing the current frequencies
  if(detector->getHeartrate()>0) {
      plot->color(0,255,0);
      plot->fill(0,255,0,100);
  } else {
      plot->color(255,0,0);
      plot->fill(255,0,0,100);
  }
  plot->label(oss.str());
  plot->series(detector->getAveragedFrequencies());
  plot->unlock();
  plot.render();

  static FPSLimiter fps(pa("-maxfps"),10);
  gui["fps"].render();
  fps.wait();

  delete image;
}

int main(int n, char **ppc){
  pa_explain
  ("-input","define input grabber parameters\ne.g. -dc 0 or -file *.ppm\ngrabber parameter -list 0 lists all available grabbers")
  ("-gl","use opengl to render the plot");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                      "-cascades|-c(string) "
                      "-maxfps(float=30) "
                      "-historydepth(int=512) "
                      "-use-opengl|-gl ",
                      init,run).exec();
  delete detector;
}


