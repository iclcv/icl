// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/filter/PseudoColorOp.h>
#include <mutex>

using icl::filter::PseudoColorOp;

HSplit gui;
GenericGrabber grabber;
PseudoColorOp pcc;
Image input_img;
Image color_img;

void step(const std::string &handle){
  static std::recursive_mutex mutex;
  std::scoped_lock lock(mutex);

  const float mult = gui["mult"];
  static float lastMult = -1;
  const int maxVal = mult*256;
  if(mult != lastMult){
    color_img = Img32f::from(maxVal+1, 50, 1, [](int x, int, int){ return x; });
  }
  lastMult = mult;

  if(handle == "load"){
    pcc.load(openFileDialog("XML Files (*.xml)"));
  }else if(handle == "save"){
    pcc.save(saveFileDialog("XML Files (*.xml)"));
  }else{
    if(gui["custom"]){
      std::vector<PseudoColorOp::Stop> colors;
      for(int i=0;i<6;++i){
        if(gui["use"+str(i)]){
          colors.push_back(PseudoColorOp::Stop(gui["relPos"+str(i)],gui["color"+str(i)]));
        }
      }
      try{
        pcc.setColorTable(PseudoColorOp::Custom,colors,maxVal);
      }catch(...){}
    }else{
      pcc.setColorTable(PseudoColorOp::Default,{},maxVal);
    }
  }

  gui["color"] = pcc.apply(color_img);

  Time t = Time::now();
  if(maxVal == 255){
    gui["image"] = pcc.apply(input_img);
  }else{
    gui["image"] = pcc.apply(input_img * mult);
  }
  gui["dt"] = str(t.age().toMilliSecondsDouble()) + "ms";
}

void stop_chooser(GUI &dst, int idx, float pos, float r, float g, float b){
  std::string si = str(idx);
  dst << ( HBox()
           << CheckBox("use",true).out("use"+si).handle(si)
           << FSlider(0,1,pos).out("relPos"+si).handle("p"+si)
           << ColorSelect(r,g,b).out("color"+si).handle("c"+si)
         );
}

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired(formatGray);
  grabber.useDesired(depth8u);
  input_img = grabber.grabImage();

  GUI colors = ( VBox().minSize(18,1)
                 << CheckBox("use custom gradient below").out("custom").handle("customH")
               );
  stop_chooser(colors,0,0.1,0,0,0);
  stop_chooser(colors,1,0.2,0,255,0);
  stop_chooser(colors,2,0.3,0,255,255);
  stop_chooser(colors,3,0.4,0,0,255);
  stop_chooser(colors,4,0.5,255,0,255);
  stop_chooser(colors,5,0.6,255,255,255);

  colors << ( HBox()
              << Button("load").handle("load")
              << Button("save").handle("save")
            )
         << FSlider(0.1,10,1).handle("mult").label("range multiplier")
         << Label("--").handle("dt").label("time");

  gui << ( VBox()
           << Display().handle("color").minSize(32,10)
           << Display().handle("image").minSize(32,24)
         )
      << colors
      << Show();

  gui.registerCallback(step,"customH,load,save");
  for(int i=0;i<6;++i){
    gui.registerCallback(step,str(i)+",c"+str(i)+",p"+str(i));
  }

  step("");
}

void run(){
  input_img = grabber.grabImage();
  step("");
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input|-i(2)",init,run).exec();
}
