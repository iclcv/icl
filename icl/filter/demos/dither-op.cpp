// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common.h>
#include <icl/core/Image.h>
#include <icl/filter/DitheringOp.h>


HSplit gui;
GenericGrabber grabber;
DitheringOp op;

void init(){
  grabber.init(pa("-i"));

  gui << Display().handle("image").minSize(32,24).label("input image")
      << Display().handle("result").minSize(32,24).label("dithered image")
      << ( VBox().maxSize(12,99).minSize(12,1)
           << Combo("2,3,4,5,6,7,8").handle("levels").label("Levels")
           << CamCfg()
           )
      << Show();
}




void run(){
  op.setLevels(parse<int>(gui["levels"].as<std::string>()));
  Image image = grabber.grabImage();
  Image dithered = op.apply(image);

  gui["image"] = image;
  gui["result"] = dithered;
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(2)",init,run).exec();


}
