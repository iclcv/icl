/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/demos/chromaticity-space/chromaticity-space.cpp  **
** Module : ICLQt                                                  **
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

#include <ICLQt/ChromaGUI.h>
#include <ICLQt/Common.h>

using namespace icl;
using namespace std;

HBox gui;
ChromaGUI *cg;
GenericGrabber grabber;

void run(){
  const Img8u &image = *grabber.grab()->asImg<icl8u>();
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
           << Image().minSize(16,12).handle("image").label("Camera Image")
           << Image().minSize(16,12).handle("segimage").label("Semented Image")
           )
      << HBox().handle("box")
      << Show();


  cg = new ChromaGUI(*gui.get<BoxHandle>("box"));

}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}
