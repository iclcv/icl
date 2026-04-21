// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#define ICL_NO_USING_NAMESPACES

#include <icl/qt/Common.h>
#include <icl/utils/FPSLimiter.h>

#include <icl/utils/Random.h>
#include <deque>
#include <icl/math/FFTUtils.h>
#include <icl/core/OpenCV.h>

#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <icl/cv/HeartrateDetector.h>


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
  cv::cvtColor( frame_gray, frame_gray, cv::COLOR_RGB2GRAY );
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
  gui << Display().handle("image").minSize(16,12)
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
  Image grabbedImage = grabber.grabImage();
  Img8u *image = grabbedImage.as8u().deepCopy();

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
  ("-input","define input grabber parameters\ne.g. -dc 0 or -file *.ppm\ngrabber parameters \"list 0\" will list all available grabbers")
  ("-gl","use opengl to render the plot");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                      "-cascades|-c(string) "
                      "-maxfps(float=30) "
                      "-historydepth(int=512) "
                      "-use-opengl|-gl ",
                      init,run).exec();
  delete detector;
}
