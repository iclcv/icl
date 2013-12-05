/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/demos/mean-shift/mean-shift.cpp                  **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <ICLQt/Common.h>
#include <ICLCV/MeanShiftTracker.h>
#include <ICLQt/MouseHandler.h>


Mutex m;
HSplit gui;
GenericGrabber *grabber = 0;
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
  grabber = new GenericGrabber();
  grabber -> init(pa("-i"));
  grabber->useDesired(depth32f);
  grabber->useDesired(formatRGB);
  grabber->useDesired<Size>(pa("-size"));

  gui << Draw().handle("image").minSize(32,24).label("image stream ")
      << ( VBox()
           << Image().handle("kernel").minSize(8,6).label("kernel image")
           << Image().handle("color").minSize(8,6).label("current color")
           << Slider(1,1000,20).out("maxCycles").label("max cycles")
           << FSlider(0.1,5,1.0).out("convergence").label("conv. crit.")
           << Slider(4,200,50).out("bandwidth").label("kernel bandwidth")
           << Combo("epanechnikov,gaussian").handle("kernel-type").label("kernel type")
           << Combo("color image,weight image").handle("vis").label("shown image")
           )
      << Show();

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
  const Img32f *image = grabber->grab()->asImg<icl32f>();

  m.lock();
  const Img32f &wi = create_weight_image(*image,COLOR);
 
  static int &maxCycles = gui.get<int>("maxCycles");
  static float &convergence = gui.get<float>("convergence");
  static ComboHandle &kernelType = gui.get<ComboHandle>("kernel-type");
  static ComboHandle &shownImage = gui.get<ComboHandle>("vis");
  static int &bandwidth = gui.get<int>("bandwidth");
  
  static MeanShiftTracker ms(MeanShiftTracker::epanechnikov, 1);
  
  if(ms.getKernel() != kernelType.getSelectedIndex()||
     ms.getBandwidth() != bandwidth){
    ms.setKernel((MeanShiftTracker::kernelType)kernelType.getSelectedIndex(),bandwidth,bandwidth/2);
    gui["kernel"] = ms.getKernelImage();
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
