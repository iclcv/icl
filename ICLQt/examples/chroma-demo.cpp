/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
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
*********************************************************************/

#include <ICLQt/ChromaGUI.h>
#include <ICLQuick/Common.h>

using namespace icl;
using namespace std;

GUI *gui;
ChromaGUI  *cg;

void run(){
  Size size = Size(320,240);
  GenericGrabber grabber(FROM_PROGARG("-input"));

  Img8u segImage(size,1);
  Img8u *image = new Img8u(size,formatRGB);
  ImgBase *imageBase = image;
  
  while(1){
    grabber.grab(&imageBase);
    gui->getValue<ImageHandle>("image") = image;
    gui->getValue<ImageHandle>("image").update();
    
    Channel8u c[3]; image->asImg<icl8u>()->extractChannels(c);
    Channel8u s = segImage[0];
    
    ChromaAndRGBClassifier classi = cg->getChromaAndRGBClassifier();
    
    for(int x=0;x<size.width;x++){
      for(int y=0;y<size.height;y++){
        s(x,y) = 255 * classi(c[0](x,y),c[1](x,y),c[2](x,y));
      }
    }
    
    gui->getValue<ImageHandle>("segimage") = &segImage;     
    gui->getValue<ImageHandle>("segimage").update();
    Thread::msleep(40);
  }
}

void init(){
  gui = new GUI("hbox");
  (*gui) << ( GUI("vbox")  
              << "image[@minsize=16x12@handle=image@label=Camera Image]" 
              << "image[@minsize=16x12@handle=segimage@label=Semented Image]" );
  (*gui) << "hbox[@handle=box]";
  
  gui->show();
  
  cg = new ChromaGUI(*gui->getValue<BoxHandle>("box"));

}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}
