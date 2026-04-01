// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/ChromaGUI.h>
#include <ICLQt/Common.h>

using namespace icl;

HBox gui;
ChromaGUI *cg;
GenericGrabber grabber;

void run(){
  Image grabImg = grabber.grabImage();
  const Img8u &image = grabImg.as8u();
  static Img8u segImage(image.getSize(),1);

  Channel8u c[3] = {image[0],image[1],image[2] };
  Channel8u s = segImage[0];

  ChromaAndRGBClassifier classi = cg->getChromaAndRGBClassifier();

  const int dim = image.getDim();
  for(int i=0;i<dim;++i){
    s[i] = 255 * classi(c[0][i],c[1][i],c[2][i]);
  }

  gui["image"] = &image;
  gui["segimage"] = &segImage;
}

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired(depth8u);
  gui << ( VBox()
           << Display().minSize(16,12).handle("image").label("Camera Image")
           << Display().minSize(16,12).handle("segimage").label("Semented Image")
           )
      << HBox().handle("box")
      << Show();


  cg = new ChromaGUI(*gui.get<BoxHandle>("box"));

}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}
