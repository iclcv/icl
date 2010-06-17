/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/3D-demo.cpp                             **
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

#include <ICLQuick/Common.h>
#include <ICLQt/DrawWidget3D.h>
#include <ICLUtils/FPSEstimator.h>

Size size(320,240);
ICLDrawWidget3D *widget = 0;

void init(){
  widget = new ICLDrawWidget3D(0);
  widget->setGeometry(200,200,640,480);
  widget->show();
}
void run(){
  GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(size);
  
  static float rz = 2;    
  static float ry = 1;
  while(1){
    
    const ImgBase *image = g.grab();
    widget->setImage(image);
    
    widget->lock();
    widget->reset3D();
    
    widget->rotate3D(0,ry,rz);
    
    rz += 0.4;
    ry += 0.8;
    
    widget->color3D(1, 1, 1, 1);
    
    widget->imagecube3D(0,0,0,0.5,image);
    widget->supercube3D(0.2,0,0,0.5);
    // widget->scale3D(0.3,0.3,0.3);
    // widget->translate3D(-0.5,-0.5,0);
    //widget->image3D(0,0,0,640.0/480.0,0,0,0,1,0,image);
    
    
    // 2D Stuff
    widget->reset();
    widget->color(255,0,0,100);
    widget->fill(255,0,0,50);
    widget->rel();
    widget->rect(0.01,0.01,0.5,0.05);
    widget->color(255,255,255,200);
    widget->fill(255,255,255,200);
    static FPSEstimator fps(10);
    
    widget->text(fps.getFPSString(),0.02,0.02,0.2,0.03);
    
    widget->unlock();
    
    
    
    widget->update();
    Thread::msleep(1);
  }
}



int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input(device,device-params)",init,run).exec();
}
