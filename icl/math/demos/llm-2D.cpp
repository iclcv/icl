// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/LLM.h>
#include <icl/qt/Common2.h>
#include <icl/utils/Random.h>
#include <icl/core/CCFunctions.h>

static LLM llm(2,3);
HBox gui;
static constexpr int W = 100, H = 100;
static constexpr float MINX = 0, MAXX = iclMax(W,H);

float planar_image[W*H*3];  // interleaved RGB training data
Img32f netImage(Size(W,H), 3);

void resetLLM(){
  int nKernels = iclMax(1, (int)gui["kc"]);
  float sigma = gui["init-sigma"];
  llm = LLM(2,3);
  llm.init(nKernels, {Range<icl32f>(MINX,MAXX), Range<icl32f>(MINX,MAXX)}, {sigma, sigma});
  llm.setSoftMaxEnabled(gui["use-soft-max"].as<bool>());
  netImage.clear();
  gui["net-image"] = Image(netImage);
}

void trainLLM(){
  int N = gui["steps"].as<int>();
  float x[2];
  for(int i=0;i<N;i++){
    x[0] = (int)icl::utils::random(0,W-1);
    x[1] = (int)icl::utils::random(0,H-1);
    llm.train(x, &planar_image[((int)x[1]*W + (int)x[0])*3], LLM::TRAIN_ALL);
  }
}

void init(){
  Img32f src = scale(create("parrot", formatRGB, depth32f), W, H).as32f();
  planarToInterleaved(&src, planar_image);

  gui << Display().minSize(20,15).label("Original").handle("orig-image")
      << Display().minSize(20,15).label("Net Output").handle("net-image")
      << ( VBox().minSize(15,0)
           << Button("Train Step").handle("train")
           << Button("Train Off","Train On").out("train-loop")
           << Button("NO Soft-Max","Soft-Max").out("use-soft-max")
           << Slider(1,10000,1000).out("steps").label("Steps per Cycle")
           << Disp(3,1).handle("mse").label("MSE").minSize(5,2)
           << FSlider(0,0.1,0.01).out("e-in").label("Epsilon In")
           << FSlider(0,0.1,0.01).out("e-out").label("Epsilon Out")
           << FSlider(0,0.5,0.1).out("e-a").label("Epsilon A")
           << FSlider(0,0.0001,0.0).out("e-sigma").label("Epsilon Sigma")
           << FSlider(1,100,10).out("init-sigma").label("Initial Sigma")
           << Button("Show Kernels").handle("show-k")
           << Button("Reset").handle("reset")
           << Int(1,1000,20).handle("kc").label("Kernel Count")
         )
      << Show();

  gui["orig-image"] = scale(create("parrot"), W, H);
  resetLLM();
}

void run(){
  static ButtonHandle train = gui["train"],
                      showKernels = gui["show-k"],
                      reset = gui["reset"];
  static DispHandle &mse = gui.get<DispHandle>("mse");

  llm.setEpsilonIn(gui["e-in"]);
  llm.setEpsilonOut(gui["e-out"]);
  llm.setEpsilonA(gui["e-a"]);
  llm.setEpsilonSigma(gui["e-sigma"]);
  llm.setSoftMaxEnabled(gui["use-soft-max"].as<bool>());

  if(train.wasTriggered() || gui["train-loop"].as<bool>()){
    trainLLM();

    float xx[2] = {0,0};
    float mseVal[3] = {0,0,0};
    for(int x=0;x<W;++x){
      xx[0] = x;
      for(int y=0;y<H;++y){
        xx[1] = y;
        const float *ynet = llm.apply(xx);
        for(int c=0;c<3;++c){
          mseVal[c] += pow(ynet[c] - planar_image[(y*W+x)*3+c], 2);
          netImage(x, y, c) = ynet[c];
        }
      }
    }
    gui["net-image"] = Image(netImage);

    for(int c=0;c<3;++c) mse(c,0) = mseVal[c]/(W*H);
  }
  if(showKernels.wasTriggered()){
    llm.showKernels();
  }
  if(reset.wasTriggered()){
    resetLLM();
  }
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init,run).exec();
}
