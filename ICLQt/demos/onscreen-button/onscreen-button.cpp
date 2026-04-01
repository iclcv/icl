// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLQt/IconFactory.h>
#include <mutex>

GUI gui;
GenericGrabber grabber;
std::recursive_mutex mtex;
Image image;

void capture(){
  std::lock_guard<std::recursive_mutex> lock(mtex);
  std::string filename = saveFileDialog();
  if(filename.length()){
    save(cvt(image.ptr()),filename);
  }
}

void init(){
  grabber.init(pa("-i"));

  gui << Display().handle("image") << Show();

  ICLWidget *w = gui["image"];

  ImgQ empty = cvt(IconFactory::create_image("empty"));
  ImgQ camera = empty.detached();

  color(255,255,255,255);
  fill(0,0,0,0);
  rect(camera,5,10,22,12);
  fill(255,255,255,255);
  circle(camera,16,16,4);
  fill(0,0,0,0);
  rect(camera,16,6,6,4);
  w->addSpecialButton("im",&camera,capture,"capture current image");
}

void run(){
  mtex.lock();
  image = grabber.grabImage();
  mtex.unlock();
  gui["image"] = image;
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input|-i(dev=create,id=lena)",init,run).exec();
}
