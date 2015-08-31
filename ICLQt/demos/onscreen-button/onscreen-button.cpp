/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/demos/onscreen-button/onscreen-button.cpp        **
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

#include <ICLQt/Common.h>
#include <ICLQt/IconFactory.h>

GUI gui;
GenericGrabber grabber;
Mutex mtex;
const ImgBase *image;

void capture(){
  Mutex::Locker lock(mtex);
  std::string filename = saveFileDialog();
  if(filename.length()){
    save(cvt(image),filename);
  }
}

void init(){
  grabber.init(pa("-i"));
  
  gui << Image().handle("image") << Show();
  
  ICLWidget *w = gui["image"];
  
  ImgQ empty = cvt(IconFactory::create_image("empty"));
  ImgQ camera = empty.detached();

  color(255,255,255,255);
  fill(0,0,0,0);
  rect(camera,5,10,22,12);
  fill(255,255,255,255);
  circle(camera,16,16,4);
  fill(0,0,0,0);
  rect(camera,16,6,6,4);
  w->addSpecialButton("im",&camera,capture,"capture current image");
}

void run(){
  mtex.lock();
  image = grabber.grab();
  mtex.unlock();
  gui["image"] = image;
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input|-i(dev=create,id=lena)",init,run).exec();
}
