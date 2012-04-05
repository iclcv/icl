/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : doc/fix-doc-screen-shot-edges.cpp                      **
** Module : doc                                                    **
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
#include <ICLIO/FileGrabber.h>
#include <ICLIO/FileWriter.h>


static icl32f edgeData[] = {
  0  , 0  , 50 , 200, 230,
  0  , 100, 200, 255, 255,
  50 , 200, 255, 255, 255,
  200, 255, 255, 255, 255,
  230, 255, 255, 255, 255
};

int main(int n, char **ppc){
  painit(n,ppc,"",true);
  if(pacount() != 2) {
    ERROR_LOG("usage: fix-doc-screen-shot-edges <source-image> <destination-image>");
    return -1;
  }
  FileGrabber g(*pa(0));
  g.setIgnoreDesiredParams(true);
  ImgQ image = cvt(g.grab());
  image.setChannels(4);
  std::fill(image.begin(3),image.end(3),0);
  
  if(image.getWidth() < 20 || image.getHeight() < 20){
    std::cout << "image size must be at least 20x20" << std::endl;
    return -1;
  }
  Img32f edgeImage(Size(5,5),1,std::vector<float*>(1,edgeData));
  Channel32f alpha = image[3], edge=edgeImage[0];
  int w = image.getWidth();
  int h = image.getHeight();
  for(unsigned int x=0;x<5;++x){
    for(unsigned int y=0;y<5;++y){
      alpha(x,y) = 255-edge(x,y);
      alpha(w-1-x,y) = 255-edge(x,y);
      alpha(x,h-1-y) = 255-edge(x,y);
      alpha(w-1-x,h-1-y) = 255-edge(x,y);
    }
  }
  
  Img8u i8 = cvt8u(image);
  FileWriter(*pa(1)).write(&i8);
}
