/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/demos/hough-line/hough-line.cpp                  **
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
#include <ICLCV/HoughLineDetector.h>

typedef FixedColVector<float,2> Pos;

HSplit gui;

HoughLineDetector hld;
Img8u edgeImage(Size::VGA,1);

struct Mouse : public MouseHandler {
  virtual void process(const MouseEvent &e){
    if(e.isLeft() || e.isRight()){
      edgeImage(e.getX(), e.getY(), 0) = 255 * e.isLeft();
    }
  }
};

void init(){
  edgeImage.fill(0);

  hld.setConfigurableID("hld");

  gui << Draw().handle("view").minSize(32,24)
      << ( VSplit()
           << Image().handle("lut").label("hough space").minSize(16,12)
           << ( VBox()
                << Button("load image").handle("load")
                << Spinner(0,100,1).out("maxlines").label("max lines")
                << Prop("hld")
                )
           )
      << Show();

  gui["view"].install(new Mouse);
  gui["view"] = Img8u(Size::VGA,1);
  gui["view"].render();

  ImageHandle lut = gui["lut"];
  (*lut)->setRangeMode(ICLWidget::rmAuto);
  (*lut)->setFitMode(ICLWidget::fmFit);
}

void run(){
  int maxlines = gui["maxlines"];

  static ButtonHandle load = gui["load"];
  if(load.wasTriggered()){
    try{
      edgeImage = qt::load<icl8u>(openFileDialog());
    }catch(...){}
  }

  hld.add(edgeImage);


  std::vector<float> sig;
  std::vector<StraightLine2D> ls = hld.getLines(maxlines,sig,false);

  ImageHandle lut = gui["lut"];
  DrawHandle view = gui["view"];
  lut = hld.getImage();
  hld.reset();

  view = edgeImage;

  (*view)->fill(255,255,255,0);
  (*view)->color(255,0,0,255);
  for(unsigned int i=0;i<ls.size();++i){
    StraightLine2D &l = ls[i];
    Pos a = l.o+l.v*3000;
    Pos b = l.o-l.v*3000;
    (*view)->line(a[0],a[1],b[0],b[1]);
    (*view)->text(str(sig[i]),l.o[0],l.o[1],-1,-1,8);
  }

  view->render();
  Thread::msleep(10);
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
