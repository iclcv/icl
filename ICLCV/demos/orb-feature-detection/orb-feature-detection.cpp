// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#define ICL_NO_USING_NAMESPACES

#include <ICLCore/OpenCV.h>
#include <ICLQt/Common.h>
#include <ICLCV/ORBFeatureDetector.h>

using namespace icl::qt;
using icl::utils::pa;
using icl::core::Img8u;
using icl::core::Image;
using icl::io::GenericGrabber;
using icl::cv::ORBFeatureDetector;

HSplit gui;
GenericGrabber grabber;
ORBFeatureDetector orb;

void init(){
   orb.setConfigurableID("orb");

   grabber.init(pa("-i"));
   gui << Canvas().handle("image")
       << ( VBox().minSize(16,1).maxSize(16,99)
            << Combo("input,gray,contrast enhanced").handle("vis")
            << Prop("orb").label("orb properties")
          )
       << Show();


}
void run(){
  Image grabImg = grabber.grabImage();
  const Img8u &image = grabImg.as8u();

  ORBFeatureDetector::FeatureSet fs = orb.detect(image);

  static DrawHandle draw = gui["image"];

  draw = orb.getIntermediateImage(gui["vis"]);
  draw->draw(fs->vis());
  draw.render();
}
int main(int n, char **args){
   return ICLApp(n,args,"-input|-i(2)",init,run).exec();
}
