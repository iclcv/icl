/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/warp-op-test.cpp                    **
** Module : ICLFilter                                              **
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
#include <ICLFilter/WarpOp.h>

GUI gui;

void init(){
  gui << "image()[@handle=image@minsize=32x24]";
  gui << ( GUI("hbox[@maxsize=100x3]") 
           << "togglebutton(no,!yes)[@label=enable-warping@handle=warp-h@out=warp]"
           << "togglebutton(nn,lin)[@label=interpolation@handle=__@out=interpolation]"
           << "combo(depth8u,depth16s,depth32s,depth32f,depth64f)[@label=image depth@out=depth]"
           << "fps(10)[@handle=fps@label=FPS]"
           << "label(---ms)[@label=apply time@handle=apply-time]"
         );
  gui.show();
}

void run(){
  static ImageHandle &H = gui.getValue<ImageHandle>("image");
  static bool &warp = gui.getValue<bool>("warp");
  static bool &lin = gui.getValue<bool>("interpolation");
  static GenericGrabber grabber(FROM_PROGARG("-input"));
  static std::string &d = gui.getValue<std::string>("depth");
  static FPSHandle &fps = gui.getValue<FPSHandle>("fps");
  static LabelHandle &l = gui.getValue<LabelHandle>("apply-time");

  grabber.setDesiredSize(Size::VGA);
  grabber.setDesiredFormat(formatRGB);
  grabber.setDesiredDepth(parse<depth>(d));

  if(warp){
    static WarpOp op(icl::load(pa("-w")));
    
    static  ImgBase *dst = 0;
    op.setScaleMode(lin?interpolateLIN:interpolateNN);
    
    Time t = Time::now();
    op.apply(grabber.grab(),&dst);
    l = str((Time::now()-t).toMilliSeconds())+"ms";
    
    H = dst;
    H.update();
  }else{
    H = grabber.grab();
    H.update();
  }
  fps.update();
  Thread::msleep(2);
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "[m]-warp-table|-w(filename)",init,run).exec();
}
