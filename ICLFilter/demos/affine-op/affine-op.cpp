/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/demos/affine-op/affine-op.cpp                **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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
#include <ICLFilter/AffineOp.h>
#include <ICLUtils/StackTimer.h>

GUI gui = HSplit().minSize(32,24);
Img8u image;

void step(){
  AffineOp op(gui["interp"] ? interpolateNN : interpolateLIN);
  image.setROI(gui["clip"]?Rect(50,50,200,300):image.getImageRect());
  op.setClipToROI(gui["clip"]);
    
  op.scale(gui["scale"],gui["scale"]);
  op.rotate(gui["angle"].as<float>()*180/M_PI);
  
  static ImgBase *dst = 0;
  op.apply(&image,&dst);

  gui["draw"] = dst;
  Thread::msleep(10);
}

void bench(){
  static ImgBase *dst = 0;
  for(int i=0;i<100;++i){
    BENCHMARK_THIS_SECTION(linear interpolation);
    AffineOp op(interpolateLIN);
    image.setFullROI();
    op.scale(1.001,1.001);
    op.rotate(3.6*i);
    op.apply(&image,&dst);
  }
  for(int i=0;i<100;++i){
    BENCHMARK_THIS_SECTION(nn interpolation);
    AffineOp op(interpolateNN);
    image.setFullROI();
    op.scale(1.001,1.001);
    op.rotate(3.6*i);
    op.apply(&image,&dst);
  }
}

void init(){
  gui << Image().handle("draw").minSize(32,24)
      << ( VBox().maxSize(10,100) 
           << FSlider(0.1,5,1,true).out("scale").label("scale").handle("a")
           << FSlider(0,6.3,0,true).out("angle").label("angle").handle("b")
           << Button("clip","off").label("clip ROI").out("clip").handle("c")
           << Button("lin","nn").label("interp.").out("interp").handle("d")
           << Button("bench").handle("bench")
          )
      << Show();
  
  image = cvt8u(scale(create("parrot"),0.4));

  gui.registerCallback(function(step),"a,b,c,d");
  gui.registerCallback(function(bench),"bench");
  gui["draw"] = image;
}


int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init).exec();
}
