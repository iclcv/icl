/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/demos/region-detection/region-detection.cpp      **
** Module : ICLCV                                                  **
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
#include <ICLCV/RegionDetector.h>
#include <ICLFilter/ColorDistanceOp.h>

VBox gui;
GenericGrabber grabber;
RegionDetector rd(100,1E9,255,255);
ColorDistanceOp cd(Color(0,120,240),100);

void mouse(const MouseEvent &e){
  if(e.isLeft()){
    cd.setReferenceColor(e.getColor());
  }
}

void init(){
  grabber.init(pa("-i"));
  gui << Draw().handle("image") << Show();

  gui["image"].install(mouse);
}

void run(){
  DrawHandle draw = gui["image"];
  const ImgBase *I = grabber.grab();

  draw = I;

  std::vector<ImageRegion> rs = rd.detect(cd.apply(I));
  for(size_t i=0;i<rs.size();++i){
    draw->linestrip(rs[i].getBoundary());
  }
  draw->render();
}
int main(int n,char **v){
  return ICLApp(n,v,"-input|-i(2)",init,run).exec();
}
