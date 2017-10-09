/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/demos/llm-1D/llm-1D.cpp                        **
** Module : ICLMath                                                **
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

#include <ICLMath/LLM.h>
#include <ICLQt/Common.h>
#include <ICLUtils/Random.h>

static LLM llm(1,1);
static const float MINX = 0;
static const float MAXX = 100;
static const float MINY = -10;
static const float MAXY = 10;

HBox gui;

inline float tx(float x){
  return (x-MINX)/(MAXX-MINX);
}
inline float ty(float y){
  return (y-MINY)/(MAXY-MINY);
}

float func(float x){
  return sin(x/5)*6 + 0.1*x - 5;
}

void init(){
  llm.setConfigurableID("llm");

  gui << Draw().minSize(40,30).label("View").handle("draw")
      << ( VBox().minSize(15,0)
           << Button("Train Step").handle("train")
           << Button("Train Off","Train On").out("train-loop")
           << Label("NAN").handle("mse").label("mse")
           << Prop("llm")
           << Button("Show Kernels").handle("show-k")
           << Button("Reset").handle("reset")
           << Int(1,1000,10).out("kernel-count").label("Kernel Count")
           )
      << Show();



  llm.init(10, vector<Range<icl32f> >(1,Range<icl32f>(MINX,MAXX)),std::vector<float>(1,5));

}
void run(){
  static ButtonHandle train = gui["train"],
                      showKernels = gui["show-k"],
                      reset = gui["reset"];

  static DrawHandle draw = gui["draw"];
  draw->setViewPort(Size(100,100));

  if(train.wasTriggered() || gui["train-loop"].as<bool>()){
    int I = 1000;
    float mse = 0;
    for(int i=0;i<I;i++){
      float x = utils::random(MINX,MAXX);
      float y = func(x);
      llm.train(&x,&y,LLM::TRAIN_ALL);
      mse += pow((*llm.apply(&x))-(y),2);
    }
    gui["mse"] = mse/I;
  }
  if(showKernels.wasTriggered()){
    llm.showKernels();
  }
  if(reset.wasTriggered()){
    llm = LLM(1,1);
    llm.init(gui["kernel-count"], vector<Range<icl32f> >(1,Range<icl32f>(MINX,MAXX)),std::vector<float>(1,5));
  }

  draw->rel();

  // draw the function AND the net
  static const int STEPS = 100;
  static const float DX = (MAXX-MINX)/STEPS;
  float xLast = 0;
  float yLast = 0;
  float ynetLast = 0;
  for(int i=0;i<STEPS;++i){
    float x = DX*i;
    float y = func(x);
    float ynet = *llm.apply(&x);
    if(i){
      draw->color(255,0,0);
      draw->line(tx(x),ty(y),tx(xLast),ty(yLast));
      draw->color(0,255,0);
      draw->line(tx(x),ty(ynet),tx(xLast),ty(ynetLast));
    }
    xLast = x;
    yLast = y;
    ynetLast = ynet;
  }

  draw->color(255,255,0);
  for(unsigned int i=0; i< llm.numKernels();++i){
    draw->color(255,255,0);
    float x = tx(llm[i].w_in[0]);
    float y = ty(llm[i].w_out[0]);
    draw->line(x,y,x,0);

    float m = llm[i].A[0];
    static const float W = 0.05;
    draw->line(x-W,y-m*W,x+W,y+m*W);

    draw->color(255,0,255);
    float stddev = tx(sqrt(llm[i].var[0]));
    draw->line(x-stddev,y-0.01,x+stddev,y-0.01);
  }

  draw.render();
}



int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init,run).exec();
}
