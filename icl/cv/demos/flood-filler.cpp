// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/cv/FloodFiller.h>
#include <icl/core/Color.h>
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
  static Img8u image = grabber.grabImage().as8u();
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
}

void init(){
  GUI selector;
  if(pa("-gray")){
    GUI x = Slider(0,255,255).handle("fill").label("fill");
    selector = x;
  }else{
    selector = ColorSelect(255,0,0).handle("fill").label("fill");
  }

  gui << Display().handle("image").minSize(16,12)
      << ( HBox().maxSize(100,3)
           << Slider(0,255,10).out("thresh").label("threshold")
           << selector
           << Label(0).handle("dt").label("dt")
           )
      << Show();


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
