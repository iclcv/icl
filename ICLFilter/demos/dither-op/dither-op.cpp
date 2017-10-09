/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/demos/dither-op/dither-op.cpp                **
** Module : ICLFilter                                              **
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
#include <ICLFilter/DitheringOp.h>


HSplit gui;
GenericGrabber grabber;
DitheringOp op;

void init(){
  grabber.init(pa("-i"));

  gui << Image().handle("image").minSize(32,24).label("input image")
      << Image().handle("result").minSize(32,24).label("dithered image")
      << ( VBox().maxSize(12,99).minSize(12,1)
           << Combo("2,3,4,5,6,7,8").handle("levels").label("Levels")
           << CamCfg()
           )
      << Show();
}




void run(){
  op.setLevels(parse<int>(gui["levels"].as<std::string>()));
  const ImgBase *image = grabber.grab();
  const ImgBase *dithered = op.apply(image);

  gui["image"] = image;
  gui["result"] = dithered;
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(2)",init,run).exec();


}
