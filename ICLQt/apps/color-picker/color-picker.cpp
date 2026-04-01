// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLCore/CCFunctions.h>
#include <ICLUtils/Thread.h>
#include <mutex>

VBox gui;
GenericGrabber grabber;
std::recursive_mutex mtex;


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

std::vector<XC> colorbuffer;


void mouse(const MouseEvent &event){
  std::lock_guard<std::recursive_mutex> lock(mtex);
  std::string colormode = gui["colormode"].as<std::string>();
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
  std::lock_guard<std::recursive_mutex> lock(mtex);
  colorbuffer.clear();
  printf("cleared! \n----------------------------------------\n");
}
void calc_mean(){
  std::lock_guard<std::recursive_mutex> lock(mtex);
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


  gui << Display().label("image").handle("image").size(32,24);
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
  gui["image"] = grabber.grabImage();
}


int main(int n,char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}
