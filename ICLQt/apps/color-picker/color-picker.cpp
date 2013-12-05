/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/apps/color-picker/color-picker.cpp               **
** Module : ICLQt                                                  **
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
#include <ICLCore/CCFunctions.h>
#include <ICLUtils/Thread.h>

VBox gui;
GenericGrabber grabber;
Mutex mutex;


struct XC{
  XC(float a=0,float b=0,float d=0){
    c[0] = a;
    c[1] = b;
    c[2] = d;
  }
  XC(const float *f){
    c[0] = f[0];
    c[1] = f[1];
    c[2] = f[2];
  }
  float c[3];
  float &operator[](int i){return c[i]; }
};

vector<XC> colorbuffer;


  
void mouse(const MouseEvent &event){
  Mutex::Locker lock(mutex);
  std::string colormode = gui["colormode"];
  if(event.isPressEvent()){
    const std::vector<icl64f> &c = event.getColor();
    if(c.size() == 1){
      printf("color: %d \n",(int)c[0]);
    }else if(c.size() > 2){
      // Assertion ; input type is rgb!!
      if(colormode == "rgb"){
        printf("rgb: %d %d %d \n",(int)c[0],(int)c[1],(int)c[2]);          
        colorbuffer.push_back(XC(c[0],c[1],c[2]));
      }else if (colormode == "hls"){
        icl32f hls[3];
        cc_util_rgb_to_hls (c[0],c[1],c[2],hls[0],hls[1],hls[2]);
        printf("hls: %d %d %d \n",(int)hls[0],(int)hls[1],(int)hls[2]);      
        colorbuffer.push_back(XC(hls));
      }
      else if (colormode == "yuv"){
        icl32s yuv[3];
        cc_util_rgb_to_yuv ((int)c[0],(int)c[1],(int)c[2],yuv[0],yuv[1],yuv[2]);
        printf("yuv: %d %d %d \n",yuv[0],(int)yuv[1],(int)yuv[2]);          
        colorbuffer.push_back(XC(yuv[0],yuv[1],yuv[2]));
      }
      else if(colormode == "gray"){
        printf("gray: %d \n",(int)((c[0]+c[1]+c[2])/3));
      }
      else{
        printf("error color mode is (%s) \n",colormode.c_str());
      }
    }
  }
}  


void reset_list(){
  Mutex::Locker lock(mutex);
  colorbuffer.clear();
  printf("cleared! \n----------------------------------------\n");
}
void calc_mean(){
  Mutex::Locker lock(mutex);
  if(!colorbuffer.size()){
    return;
  }
  XC xM,xV;
  for(unsigned int i=0;i<colorbuffer.size();i++){
    xM[0] += colorbuffer[i][0];
    xM[1] += colorbuffer[i][1];
    xM[2] += colorbuffer[i][2];
  }
  xM[0] /= colorbuffer.size();
  xM[1] /= colorbuffer.size();  
  xM[2] /= colorbuffer.size();

  for(unsigned int i=0;i<colorbuffer.size();i++){
    xV[0] += pow(colorbuffer[i][0]-xM[0],2);
    xV[1] += pow(colorbuffer[i][1]-xM[1],2);
    xV[2] += pow(colorbuffer[i][2]-xM[2],2);
  }

  xV[0] /= colorbuffer.size();
  xV[1] /= colorbuffer.size();  
  xV[2] /= colorbuffer.size();


  printf("mean   = %3d %3d %3d \n",(int)xM[0],(int)xM[1],(int)xM[2]);
  printf("stddev = %3d %3d %3d \n",(int)sqrt(xV[0]),(int)sqrt(xV[1]),(int)sqrt(xV[2]));
  printf("-------------------------------------------------\n");  
}

void init(){
  grabber.init(pa("-i"));


  gui << Image().label("image").handle("image").size(32,24);
  gui << Button("Run!","Stop!",true).out("running");
  gui << ( HBox() 
              << Combo("!rgb,hls,gray,yuv").handle("colormode").label("colormode")
              << Button("Reset List").handle("reset")
              << Button("Calculate Mean").handle("calc")
              );
  
  gui.show();
  
  gui["reset"].registerCallback(reset_list);
  gui["calc"].registerCallback(calc_mean);
  
  gui["image"].install(mouse);
}


void run(){
  while(!gui["running"].as<bool>()){
    Thread::sleep(0.1);
  }
  gui["image"] = grabber.grab();
}


int main(int n,char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}
