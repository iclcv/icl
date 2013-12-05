/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/demos/pseudo-color/pseudo-color.cpp            **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/


#include <ICLQt/Common.h>
#include <ICLCore/PseudoColorConverter.h>

HSplit gui;
GenericGrabber grabber;
PseudoColorConverter pcc;
Img8u image;
Img32f color;
void update_color(int maxSteps){
  ::color = Img32f(Size(maxSteps+1,50),1);
  Channel32f col = ::color[0];
  for(int i=0;i<=maxSteps;++i){
    for(unsigned int j=0;j<50;++j){
      col(i,j) = i;
    }
  }
}

void step(const std::string &handle){
  static Mutex mutex;
  Mutex::Locker lock(mutex);

  const float mult = gui["mult"];
  static float lastMult = -1;
  const int maxVal = mult*256;  
  if(mult != lastMult){
    update_color(maxVal);
  }

  lastMult = mult;

  if(handle == "load"){
    pcc.load(openFileDialog("XML Files (*.xml)"));
  }else if(handle == "save"){
    pcc.save(saveFileDialog("XML Files (*.xml)"));
  }else{
    if(gui["custom"]){
      std::vector<PseudoColorConverter::Stop> colors;
      for(int i=0;i<6;++i){
        if(gui["use"+str(i)]){
          colors.push_back(PseudoColorConverter::Stop(gui["relPos"+str(i)],gui["color"+str(i)]));
        }
      }try{
        pcc.setColorTable(PseudoColorConverter::Custom,colors,maxVal);
      }catch(...){}
    }else{
      pcc.setColorTable(PseudoColorConverter::Default,std::vector<PseudoColorConverter::Stop>(),maxVal);
    }
  }

  static ImgBase *colorOut = 0;
  pcc.apply(&::color,&colorOut);
  gui["color"] = colorOut;

  if(maxVal == 255){
    Time t = Time::now();
    const Img8u &res = pcc.apply(::image);
    gui["dt"] = str(t.age().toMilliSecondsDouble()) + "ms";
    gui["image"] = &res;
  }else{
    ImgQ fim = cvt(image) * mult;
    static ImgBase *result = 0;
    Time t = Time::now();
    pcc.apply(&fim,&result);
    gui["dt"] = str(t.age().toMilliSecondsDouble()) + "ms";
    gui["image"] = result;
  }
}

void stop_chooser(GUI &dst, int idx,float pos, float r, float g, float b){
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
  grabber.grab();
  
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
           << Image().handle("color").minSize(32,10) 
           << Image().handle("image").minSize(32,24)
         ) 
      << colors 
      << Show();
  
  gui.registerCallback(utils::function(step),"customH,load,save");
  for(int i=0;i<6;++i){
    gui.registerCallback(utils::function(step),str(i)+",c"+str(i)+",p"+str(i));
  }
  
  step("");
}

void run(){
  ::image = *grabber.grab()->asImg<icl8u>();
  step("");
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input|-i(2)",init,run).exec();
}
