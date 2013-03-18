/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/apps/rectify-image/rectify-image.cpp         **
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
#include <ICLQt/DefineQuadrangleMouseHandler.h>
#include <ICLFilter/ImageRectification.h>

HSplit gui;
GenericGrabber grabber;
DefineQuadrangleMouseHandler mouse;
ImgBase *rect = 0;
ImageRectification<icl8u> ir;

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired(depth8u);
  
  gui << Draw().handle("draw") 
      << (VBox() 
          << Image().handle("rectified") 
          << (HBox().label("target size").maxSize(99,3)
              << Spinner(2,2000,512).handle("width")
              << Label("x")
              << Spinner(2,2000,512).handle("height")
             )
          << (HBox().label("rectify").maxSize(99,3)
              << Button("now").handle("now")
              << CheckBox("auto",true).handle("auto")
             )
         )
      << Show();
  
  mouse.init(grabber.grab()->getSize());

  gui["draw"].install(&mouse);
}

void run(){
  DrawHandle draw = gui["draw"];
  ButtonHandle now = gui["now"];
  bool automatic = gui["auto"];
  
  const Img8u image = *grabber.grab()->as8u();
  draw = image;
  
  if(now.wasTriggered() || automatic){
    Size s(gui["width"],gui["height"]);
    std::vector<Point> ps = mouse.getQuadrangle();
    std::vector<Point32f> psf(ps.begin(),ps.end());
    try{
      const Img8u &rectf = ir.apply(psf.data(), image, s);
      gui["rectified"] = rectf;
    }catch(const ICLException &e){
      WARNING_LOG("rectification failed: " << e.what());
    }
  }
  
  draw->draw(mouse.vis());
  draw->render();
}

int main(int n, char **args){
  return ICLApp(n,args,"-input|-i(2)",init,run).exec();
}
