// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common.h>
#include <icl/core/Image.h>
#include <icl/cv/RegionDetector.h>
#include <icl/filter/ColorDistanceOp.h>

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
  gui << Canvas().handle("image") << Show();

  gui["image"].install(mouse);
}

void run(){
  DrawHandle draw = gui["image"];
  Image I = grabber.grabImage();

  draw = I;

  Image filtered = cd.apply(I);
  std::vector<ImageRegion> rs = rd.detect(filtered);
  for(size_t i=0;i<rs.size();++i){
    draw->linestrip(rs[i].getBoundary());
  }
  draw->render();
}
int main(int n,char **v){
  return ICLApp(n,v,"-input|-i(2)",init,run).exec();
}
