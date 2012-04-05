/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/chroma-demo.cpp                         **
** Module : ICLQt                                                  **
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

#include <ICLQt/ChromaGUI.h>
#include <ICLQuick/Common.h>

using namespace icl;
using namespace std;

GUI gui("hbox");
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
  gui["image"].update();
  gui["segimage"] = &segImage;
  gui["segimage"].update();
  Thread::msleep(10);
}

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired(depth8u);

  gui << ( GUI("vbox")  
           << "image[@minsize=16x12@handle=image@label=Camera Image]" 
           << "image[@minsize=16x12@handle=segimage@label=Semented Image]" 
         )
      << "hbox[@handle=box]"  
      << "!show";
  
  cg = new ChromaGUI(*gui.getValue<BoxHandle>("box"));

}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}
