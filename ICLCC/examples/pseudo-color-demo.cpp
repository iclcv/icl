/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCC/examples/pseudo-color-demo.cpp                   **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/


#include <ICLQuick/Common.h>
#include <ICLCC/PseudoColorConverter.h>

GUI gui("hsplit");
GenericGrabber grabber;
PseudoColorConverter pcc;
Img8u color,image;

void step(const std::string &handle){
  static Mutex mutex;
  Mutex::Locker lock(mutex);

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
        pcc.setColorTable(PseudoColorConverter::Custom,colors);
      }catch(...){}
    }else{
      pcc.setColorTable(PseudoColorConverter::Default);
    }
  }
  gui["color"] = &pcc.apply(::color);
  gui["color"].update();
  
  gui["image"] = &pcc.apply(::image);
  gui["image"].update();

}

void stop_chooser(GUI &dst, int idx,float pos, float r, float g, float b){
  std::string si = str(idx);
  std::string sp = str(pos);
  
  dst <<( GUI("hbox") 
          << "checkbox(use,checked)[@out=use"+si+"@handle="+si+"]"
          << "fslider(0,1,"+sp+")[@out=relPos"+si+"@handle=p"+si+"]"
          << "color("+str(r)+","+str(g)+","+str(b)+")[@out=color"+si+"@handle=c"+si+"]" );
}

void init(){
  grabber.init(FROM_PROGARG("-i"));
  grabber.setUseDesiredParams(true);
  grabber.setDesiredFormat(formatGray);
  
  ::color = Img8u(Size(256,50),1);
  ::image = *grabber.grab()->asImg<icl8u>();
  for(unsigned int i=0;i<256;++i){
    for(unsigned int j=0;j<50;++j){
      ::color(i,j,0) = i;
    }
  }
  
  GUI colors("vbox[@minsize=18x1]");
  colors << "checkbox(use custom gradient below,unchecked)[@out=custom@handle=customH]";
  stop_chooser(colors,0,0.1,0,0,0);
  stop_chooser(colors,1,0.2,0,255,0);
  stop_chooser(colors,2,0.3,0,255,255);
  stop_chooser(colors,3,0.4,0,0,255);
  stop_chooser(colors,4,0.5,255,0,255);
  stop_chooser(colors,5,0.6,255,255,255);
  
  colors << ( GUI("hbox") << "button(load)[@handle=load]" << "button(save)[@handle=save]" ); 
  
  gui << ( GUI("vbox") 
           << "image[@handle=color@minsize=32x10]" 
           << "image[@handle=image@minsize=32x24]"
         ) << colors << "!show";

  
  gui.registerCallback(new GUI::Callback(step),"customH,load,save");
  for(int i=0;i<6;++i){
    gui.registerCallback(new GUI::Callback(step),str(i)+",c"+str(i)+",p"+str(i));
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
