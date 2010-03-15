/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLCC/examples/colorspace-online-demo.cpp              **
** Module : ICLCC                                                  **
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

#include <ICLQuick/Common.h>

GUI gui;

void init(){
  gui << "image[@handle=image@minsize=16x12]"
      << "combo(Gray,RGB,HLS,YUV,LAB,Chroma,Matrix)"
         "[@out=cs@label=color space@maxsize=100x3]";
  
  gui.show();
}

void run(){
  gui_string(cs);
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setIgnoreDesiredParams(true);
  g.setDesiredSize(g.grab()->getSize());
  g.setDesiredFormat(parse<format>(cs));
  g.setIgnoreDesiredParams(false);
  
  gui["image"] = g.grab();
  gui["image"].update();
}

int main(int n, char **args){
  return ICLApplication(n,args,"-input(2)",init,run).exec();
}
