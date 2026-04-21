// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
GUI gui;
GenericGrabber grabber;

void init(){

  gui << Display().handle("image")
      << Combo("Gray,RGB,HLS,YUV,LAB,Chroma,Matrix").handle("fmt").maxSize(100,3)
      << Show();

  grabber.init(pa("-i"));

}

void run(){
  Image image = grabber.grabImage();  
  gui["image"] = cc(image, gui["fmt"]);
}

int main(int n, char **args){
  return ICLApplication(n,args,"[m]-i|-input(2)",init,run).exec();
}
