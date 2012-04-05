/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/examples/flood-filler-demo.cpp                 **2
** Module : ICLBlob                                                **
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
#include <ICLBlob/FloodFiller.h>
#include <ICLCC/Color.h>
//TODO: crashes in the destructor

GUI gui;
GenericGrabber grabber;
FloodFiller ff;

struct Mouse : public MouseHandler{
  bool pressed;
  Rect lastImageRect;
  Point pos;
  Mouse():pressed(false){}

  void process(const MouseEvent &e){
    if(e.isPressEvent() && lastImageRect.contains(e.getX(),e.getY())){
      this->pressed = true;
      this->pos = e.getPos();
    }
  }
  
  bool wasPressed(){
    bool x = pressed;
    this->pressed = false;
    return x;
  }
} mouse;

void run(){
  static Img8u image = *grabber.grab()->as8u();
  mouse.lastImageRect = image.getImageRect();

  if(mouse.wasPressed()){
    Time t = Time::now();
    Time dt;
    if(pa("-gray")){
      icl8u ref = image(mouse.pos.x,mouse.pos.y,0);
      const FloodFiller::Result &res = ff.apply(&image,mouse.pos, ref,gui["thresh"]);
      //                                                     CustomCriterion(image(mouse.pos.x,mouse.pos.y,0),gui["thresh"]));
      dt = Time::now()-t;
      int val = gui["fill"];
      for(unsigned int i=0;i<res.pixels.size();++i){
        image(res.pixels[i].x,res.pixels[i].y,0) = val;
      }
    }else{
      PixelRef<icl8u> ref = image(mouse.pos.x,mouse.pos.y);
      const FloodFiller::Result &res = ff.applyColor(&image,mouse.pos, ref[0], ref[1], ref[2],gui["thresh"]);
      dt = Time::now()-t;
      
      Color c = gui["fill"];
      for(unsigned int i=0;i<res.pixels.size();++i){
        image(res.pixels[i].x,res.pixels[i].y) = c;
      }
      
    }
    gui["dt"] = str(dt.toMilliSecondsDouble()) + " ms";
  }
  
  gui["image"] = image;
  gui["image"].update();
}

void init(){
  gui << "image()[@handle=image@minsize=16x12]";
  gui << ( GUI("hbox[@maxsize=100x3]") 
           << "slider(0,255,10)[@out=thresh@label=threshold]"
           << str(pa("-gray") ? "slider(0,255,255)" : "color(255,0,0)")+"[@handle=fill@label=fill]"
           << "label(0)[@handle=dt@label=dt]"
          )
      << "!show";
  grabber.init(pa("-i"));

  if(pa("-gray")){
    grabber.useDesired(formatGray);
  }

  gui["image"].install(&mouse);
}

int main(int n, char**ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) -gray",
                init,run).exec();
}
