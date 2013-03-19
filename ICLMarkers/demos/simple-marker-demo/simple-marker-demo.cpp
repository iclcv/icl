/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/demos/simple-marker-demo/simple-marker-demo **
**          .cpp                                                   **
** Module : ICLMarkers                                             **
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
#include <ICLMarkers/FiducialDetector.h>

HSplit gui;
GenericGrabber grabber;
FiducialDetector fid("bch","[0-100]",ParamList("size",Size(30,30)));

void init(){
  fid.setConfigurableID("fid");
  gui << Draw().handle("draw").minSize(16,12).size(32,24)
      << Prop("fid").label("detection properties").maxSize(18,100).minSize(14,1)
      << Show();
  
  grabber.init(pa("-input"));
}


// working loop
void run(){
  static DrawHandle draw = gui["draw"];

  const ImgBase *image = grabber.grab();
  const std::vector<Fiducial> &fids = fid.detect(image);

  draw = image;
  draw->linewidth(2);
  for(unsigned int i=0;i<fids.size();++i){
    Point32f c = fids[i].getCenter2D();
    float rot = fids[i].getRotation2D();
    
    draw->color(0,100,255,255);
    draw->text(fids[i].getName(),c.x,c.y,10);
    draw->color(0,255,0,255);
    draw->line(c,c+Point32f( cos(rot), sin(rot))*100 );

    draw->color(255,0,0,255);
    draw->linestrip(fids[i].getCorners2D());

  }
  draw.render();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
