// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLCore/CCFunctions.h>
GUI gui;
GenericGrabber grabber;

void init(){

  gui << Display().handle("image")
      << Combo("Gray,RGB,HLS,YUV,LAB,Chroma,Matrix").handle("fmt").maxSize(100,3)
      << Show();

  grabber.init(pa("-i"));

}

void run(){
  //grabber.useDesired(parse<format>(gui["fmt"]));
  //gui["image"] = grabber.grabImage();
  Image image = grabber.grabImage();
  static Img8u dst;
  dst.setFormat(parse<format>(gui["fmt"]));
  cc(image.ptr(),&dst);
  gui["image"] = dst;
}

int main(int n, char **args){
  return ICLApplication(n,args,"[m]-i|-input(2)",init,run).exec();
}
