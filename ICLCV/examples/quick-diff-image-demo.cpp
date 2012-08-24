/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/examples/quick-diff-image-demo.cpp               **
** Module : ICLCV                                                  **
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

#include <ICLCV/Common.h>

GUI gui;
GenericGrabber grabber;
ImgQ last;  // last image

void init(){
  gui << Image().handle("image")
      << Slider(0,255,127).out("thresh")
      << Show();
  grabber.init(pa("-i"));
}

void run(){
  // use cvt to create an Img32 (aka ImgQ)
  ImgQ curr = cvt(grabber.grab());
  // nested use of operators
  gui["image"] = thresh(abs(last-curr),gui["thresh"]);
  last = curr;
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input|-i(2)",init,run).exec();
}
