/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/demos/orb-feature-detection/orb-feature-detection.cpp **
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

#include <ICLCore/OpenCV.h>
#include <ICLQt/Common.h>
#include <ICLCV/ORBFeatureDetector.h>

HSplit gui;
GenericGrabber grabber;
ORBFeatureDetector orb;

void init(){
   orb.setConfigurableID("orb");

   grabber.init(pa("-i"));
   gui << Draw().handle("image")
       << ( VBox().minSize(16,1).maxSize(16,99)
            << Combo("input,gray,contrast enhanced").handle("vis")
            << Prop("orb").label("orb properties")
          )
       << Show();


}
void run(){
  const Img8u &image = *grabber.grab()->as8u();

  ORBFeatureDetector::FeatureSet fs = orb.detect(image);

  static DrawHandle draw = gui["image"];

  draw = orb.getIntermediateImage(gui["vis"]);
  draw->draw(fs->vis());
  draw.render();
}
int main(int n, char **args){
   return ICLApp(n,args,"-input|-i(2)",init,run).exec();
}


