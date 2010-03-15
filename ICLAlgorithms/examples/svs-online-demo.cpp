/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLAlgorithms/examples/svs-online-demo.cpp             **
** Module : ICLAlgorithms                                          **
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

#include <ICLAlgorithms/SVS.h>
#include <ICLQuick/Common.h>


static GUI gui("hbox");
static SVS svs;

void init(){
  gui << "image[@minsize=12x8@handle=left@label=left camera image]"
      << "image[@minsize=12x8@handle=center@label=disparity image]"
      << "image[@minsize=12x8@handle=right@label=right camera image]";
  
  GUI con;
  con << "togglebutton(off,on)[@out=fliplr@label=flip left/right]"
      << "togglebutton(off,on)[@out=halveimages@label=halve image sizes]";

  gui << con;
  gui.show();
  

}

void run(){
  static GenericGrabber leftG(FROM_PROGARG("-left-cam"));
  static GenericGrabber rightG(FROM_PROGARG("-right-cam"));

  gui_bool(halveimages);
  gui_bool(fliplr);
  gui_ImageHandle(left);
  gui_ImageHandle(right);
  gui_ImageHandle(center);

  
  leftG.setDesiredSize(Size::VGA/(halveimages?2:1));
  rightG.setDesiredSize(Size::VGA/(halveimages?2:1));

  leftG.setDesiredFormat(formatGray);
  rightG.setDesiredFormat(formatGray);

  leftG.setIgnoreDesiredParams(false);
  rightG.setIgnoreDesiredParams(false);
  
  const Img8u *lim = leftG.grab()->asImg<icl8u>();
  const Img8u *rim = rightG.grab()->asImg<icl8u>();
  
  if(fliplr){
    std::swap(lim,rim);
  }
  
  svs.load(lim,rim);
  static std::string filename = pa("-i");
  svs.loadCalibration(filename.c_str());
  svs.printvars();
  svs.doStereo();
  const ImgBase *disp = svs.getDisparity();
  

  left = lim;
  right = rim;
  center = disp;
  left.update();
  right.update();
  center.update();
  
  Thread::msleep(10);
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-ini-file|-i(filename) -left-cam|-l(device,device-params) "
                "-right-cam|-r(device,device-prams)",init,run).exec();
}
