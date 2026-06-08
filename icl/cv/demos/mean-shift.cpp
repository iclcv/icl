// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/cv/MeanShiftTracker.h>
#include <icl/qt/MouseHandler.h>
#include <icl/qt/ui.h>
#include <mutex>


std::recursive_mutex m;
HSplit gui;
ImageSource *grabber = 0;
Point32f *newPos = 0;
Point32f pos;
std::vector<double> COLOR(3,255);

void mouse(const MouseEvent &evt){
  m.lock();
  if(evt.isLeft()){
    if(newPos) *newPos = evt.getPos();
    else newPos = new Point32f(evt.getPos());
  }else if(evt.isRight()){
    std::vector<double> newColor = evt.getColor();
    if(newColor.size() == 3){
      COLOR = newColor;
    }else{
      ERROR_LOG("colors must be given in color image mode!");
    }
    Img32f image(Size(4,3),formatRGB);
    std::fill(image.begin(0),image.end(0),COLOR[0]);
    std::fill(image.begin(1),image.end(1),COLOR[1]);
    std::fill(image.begin(2),image.end(2),COLOR[2]);
    gui["color"] = image;
  }
  m.unlock();
}

void init(){
  grabber = new ImageSource();
  grabber -> init(pa("-i"));
  grabber->useDesired(depth32f);
  grabber->useDesired(formatRGB);
  grabber->useDesired(utils::Size(pa("-size")));

  gui << ui::Canvas({.handle="image", .label="image stream ", .minSize={32, 24}})
      << ( ui::VBox()
           << ui::Display({.handle="kernel", .label="kernel image", .minSize={8, 6}})
           << ui::Display({.handle="color", .label="current color", .minSize={8, 6}})
           << ui::Slider(1, 1000, 20, {.handle="maxCycles", .label="max cycles"})
           << ui::FSlider(0.1, 5, 1.0, {.handle="convergence", .label="conv. crit."})
           << ui::Slider(4, 200, 50, {.handle="bandwidth", .label="kernel bandwidth"})
           << ui::Combo("epanechnikov,gaussian", {.handle="kernel-type", .label="kernel type"})
           << ui::Combo("color image,weight image", {.handle="vis", .label="shown image"})
           )
      << ui::Show();

  gui["image"].install(mouse);
  gui.get<ImageHandle>("kernel")->setRangeMode(ICLWidget::rmAuto);
}

struct ColorDist{
  float r,g,b;
  ColorDist(const std::vector<double> &color):
    r(color.at(0)),g(color.at(1)),b(color.at(2)){}
  static inline float sqr(float x){ return x*x; }
  void operator()(const icl32f src[3], icl32f dst[1]) const{
    *dst = 255.0 - sqrt(sqr(r-src[0])+sqr(g-src[1])+sqr(b-src[2]))/sqrt(3);
  }
};

const Img32f &create_weight_image(const Img32f &image, const std::vector<double> &color){
  static Img32f wi(Size(1,1),1);
  wi.setSize(image.getSize());
  image.reduce_channels<icl32f,3,1,ColorDist>(wi,ColorDist(color));
  return wi;
}



void run(){
  static Image grabbed;
  grabbed = grabber->grab();
  const Img32f *image = &grabbed.as32f();

  m.lock();
  const Img32f &wi = create_weight_image(*image,COLOR);

  int maxCycles = gui["maxCycles"];
  float convergence = gui["convergence"];
  ComboHandle kernelType = gui["kernel-type"];
  ComboHandle shownImage = gui["vis"];
  int bandwidth = gui["bandwidth"];

  static MeanShiftTracker ms(MeanShiftTracker::epanechnikov, 1);

  if(ms.getKernel() != kernelType.getSelectedIndex()||
     ms.getBandwidth() != bandwidth){
    ms.setKernel((MeanShiftTracker::kernelType)kernelType.getSelectedIndex(),bandwidth,bandwidth/2);
    gui["kernel"] = ms.getKernelDisplay();
  }
  if(newPos){
    pos = *newPos;
    ICL_DELETE(newPos);
  }
  pos = ms.step(wi,pos,maxCycles,convergence);
  m.unlock();

  static ICLDrawWidget &w = **gui.get<DrawHandle>("image");
  w.setImage( (shownImage.getSelectedIndex()) ? (&wi) : (image));
  w.color(255,0,0,255);
  w.fill(255,0,0,50);
  w.rect(pos.x-bandwidth,pos.y-bandwidth,2*bandwidth+1,2*bandwidth+1);
  w.symsize(20);
  w.sym(pos.x,pos.y, ICLDrawWidget::symPlus);
  w.render();

  Thread::msleep(50);
}





int main(int n, char **ppc){
  pa_explain
  ("-i","defines input device and params that should be used")
  ("-s","defines image size");
  return ICLApplication(n,ppc,"[m]-input|-i(device,device-params) -size|-s(Size=VGA)",init,run).exec();
}
