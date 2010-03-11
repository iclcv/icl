/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

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
  std::vector<UnicapDevice> l = UnicapGrabber::getDeviceList();
  printf ("devices: %d\n", (unsigned int)l.size());
  l = UnicapGrabber::getDeviceList();
  printf ("devices: %d\n", (unsigned int)l.size());
  l = UnicapGrabber::getDeviceList();
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
