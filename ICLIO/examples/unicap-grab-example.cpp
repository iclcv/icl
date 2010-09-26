/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/examples/unicap-grab-example.cpp                 **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#include <ICLIO/UnicapGrabber.h>
#include <ICLQuick/Quick.h>
#include <ICLUtils/StackTimer.h>

void grab_single_frame(UnicapGrabber &g){
  BENCHMARK_THIS_FUNCTION;
  static ImgBase *image = new Img8u(Size(640,480),formatRGB);
  static const Img8u *image2 = 0;
  image2 = g.grab(&image)->asImg<icl8u>();
}

void grab_100_frames(UnicapGrabber &g){
  BENCHMARK_THIS_FUNCTION;
  for(int i=0;i<100;i++){
    grab_single_frame(g);
  }
}

int main(){
  std::vector<UnicapDevice> l = UnicapGrabber::getUnicapDeviceList();
  printf ("devices: %d\n", (unsigned int)l.size());
  l = UnicapGrabber::getUnicapDeviceList();
  printf ("devices: %d\n", (unsigned int)l.size());
  l = UnicapGrabber::getUnicapDeviceList();
  printf ("devices: %d\n", (unsigned int)l.size());
  if(!l.size()) {
     ERROR_LOG("no devices were found !");
     exit(-1001);
  }
  UnicapGrabber g(l[0]);
  //  l[0].listFormats();
  l[0].listProperties();
  //  g.setProperty("frame rate","30");
  
  //UnicapGrabber g("device=/dev/video1394-0");
  g.setDesiredParams(ImgParams(Size(640,480),formatRGB));
  g.setDesiredDepth(depth8u);

  grab_100_frames(g);
  
  //  ImgQ a = cvt(*image);
  //label(a,"grabbed image");
  //show(a);
  
}
