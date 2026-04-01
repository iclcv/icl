// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLCV/SimpleBlobSearcher.h>
#include <ICLUtils/FPSLimiter.h>
#include <mutex>

GUI gui;
GenericGrabber grabber;
SimpleBlobSearcher S;
std::recursive_mutex mtex;

void mouse(const MouseEvent &e){
  if(e.hitDisplay() && e.isPressEvent()){
    static int &minSize = gui.get<int>("minSize");
    static int &maxSize = gui.get<int>("maxSize");
    static float &thresh = gui.get<float>("thresh");
    std::lock_guard<std::recursive_mutex> lock(mtex);
    int idx = e.isMiddle() ? 1 : e.isRight() ? 2 : 0;
    std::vector<double> color = e.getColor();
    ICLASSERT_RETURN(color.size() == 3);
    S.adapt(idx,Color(color[0],color[1],color[2]),thresh,Range32s(minSize,maxSize));
  }
}


void init(){
  gui << Canvas().minSize(32,24).handle("draw");
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
  Image image = grabber.grabImage();
  const Img8u &image8u = image.as8u();

  const std::vector<SimpleBlobSearcher::Blob> &blobs = S.detect(image8u);

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
  pa_explain("-i","defines input device to use");
  return ICLApplication(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}
