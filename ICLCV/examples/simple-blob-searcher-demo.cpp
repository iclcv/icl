/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/examples/simple-blob-searcher-demo.cpp         **
** Module : ICLCV                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQuick/Common.h>
#include <ICLCV/SimpleBlobSearcher.h>
#include <ICLUtils/FPSLimiter.h>

GUI gui;
GenericGrabber grabber;
SimpleBlobSearcher S;
Mutex mutex;

void mouse(const MouseEvent &e){
  if(e.hitImage() && e.isPressEvent()){
    static int &minSize = gui.get<int>("minSize");
    static int &maxSize = gui.get<int>("maxSize");
    static float &thresh = gui.get<float>("thresh");
    Mutex::Locker lock(mutex);
    int idx = e.isMiddle() ? 1 : e.isRight() ? 2 : 0;
    std::vector<double> color = e.getColor();
    ICLASSERT_RETURN(color.size() == 3);
    S.adapt(idx,Color(color[0],color[1],color[2]),thresh,Range32s(minSize,maxSize));
  }
}


void init(){
  gui << Draw().minSize(32,24).handle("draw");
  gui << ( HBox().maxSize(100,3) 
           << Spinner(1,100000,100).out("minSize").label("min size")
           << Spinner(1,100000,1000).out("maxSize").label("max size")
           << FSlider(0,300,30).out("thresh").label("threshold") 
           )
      << Show();

  
  gui["draw"].install(new MouseHandler(mouse));
  grabber.init(pa("-i"));
  
  S.add(Color(255,0,0),100,Range32s(100,100));
  S.add(Color(0,255,0),100,Range32s(100,100));
  S.add(Color(0,0,255),100,Range32s(100,100));
}

void run(){
  static DrawHandle draw = gui["draw"];
  static FPSLimiter fps(20);
  const Img8u *image = grabber.grab()->asImg<icl8u>();
  
  const std::vector<SimpleBlobSearcher::Blob> &blobs = S.detect(*image);
  
  draw = image;

  for(unsigned int i=0;i<blobs.size();++i){
    draw->color(255,255,255,255);
    draw->linestrip(blobs[i].region->getBoundary());
    draw->text(str(blobs[i].refColorIndex), blobs[i].region->getCOG().x,blobs[i].region->getCOG().y);
  }
  draw.render();
  fps.wait();
}

int main(int n, char **ppc){
  paex("-i","defines input device to use");
  return ICLApplication(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}