/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/examples/llm-2D-demo.cpp                       **
** Module : ICLMath                                                **
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

#include <ICLMath/LLM.h>
#include <ICLQt/Common.h>
#include <ICLCore/Mathematics.h>


static LLM llm(2,3);
HBox gui;
static const int W = 100;
static const int H = 100;
static const float MINX = 0;
static const float MAXX = iclMax(W,H);
float I[W][H][3];
static int *nTrainingSteps = new int(100);
static ImgQ image = scale(create("parrot"),W,H);
//static ImgQ image = scale(load("sprirals1.ppm"),W,H);
DispHandle *mseDisp = 0;
static int *nKernels = new int(50);
static float *initialSigma = new float(10);
static bool *softMaxEnabled = new bool(true);

void initI(){
  for(int i=0;i<W;++i){
    for(int j=0;j<H;++j){
      for(int c=0;c<3;++c){
        I[i][j][c] = image(i,j,c);
      }
    }
  }
}
static void initLLM(){
  llm = LLM(2,3);
  llm.init(*nKernels, vector<Range<icl32f> >(2,Range<icl32f>(MINX,MAXX)),std::vector<float>(2,*initialSigma));
  llm.setSoftMaxEnabled(*softMaxEnabled);
}
static void trainLLM(){
  int N = *nTrainingSteps;
  float x[2];
  for(int i=0;i<N;i++){
    x[0] = (int)icl::random(0,W-1);
    x[1] = (int)icl::random(0,H-1);
    const float * y = I[(int)x[0]][(int)x[1]];
    llm.train(x,y,LLM::TRAIN_ALL);
  }
}

class MyThread : public Thread{
public:
  MyThread(){
    initI();

    
    gui << Image().minSize(20,15).label("Original").handle("orig-image")
        << Image().minSize(20,15).label("Net Output").handle("net-image")
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
             << Int(1,1000,20).out("kc").label("Kernel Count")
           )
        << Show();

    gui["orig-image"] = image;
    
    initLLM();
  }
  virtual void run(){
    static ICLWidget &w = **gui.get<ImageHandle>("net-image");
    static ButtonHandle &trainButton = gui.get<ButtonHandle>("train");    
    static ButtonHandle &showKernelsButton = gui.get<ButtonHandle>("show-k");
    static ButtonHandle &resetButton = gui.get<ButtonHandle>("reset");


    nKernels = &gui.get<int>("kc");
    mseDisp = &gui.get<DispHandle>("mse");
    nTrainingSteps = &gui.get<int>("steps");
    initialSigma = &gui.get<float>("init-sigma");
    softMaxEnabled = &gui.get<bool>("use-soft-max");
    //    ImgQ netImage(image.getSize(),formatRGB);
    ImgQ netImage = copy(image);
    w.setImage(&netImage);

    w.render();
    
    while(1){
      
      llm.setEpsilonIn(gui.get<float>("e-in"));
      llm.setEpsilonOut(gui.get<float>("e-out"));
      llm.setEpsilonA(gui.get<float>("e-a"));
      llm.setEpsilonSigma(gui.get<float>("e-sigma"));
      llm.setSoftMaxEnabled(*softMaxEnabled);
      
      if(trainButton.wasTriggered() || gui.get<bool>("train-loop")){
        trainLLM();

        float xx[2]={0,0};
        float mse[3]={0,0,0};
        for(int x=0;x<W;++x){
          xx[0]=x;
          for(int y=0;y<H;++y){
            xx[1]=y;
            const float *ynet = llm.apply(xx);
            for(int i=0;i<3;++i){
              mse[i] += pow(ynet[i]-I[x][y][i],2);
              netImage(x,y,i) = ynet[i];
            }
          }
        }
        w.setImage(&netImage);

        for(int j=0;j<3;++j){
          (*mseDisp)(j,0) = mse[j]/(W*H);    
        }
        w.render();
      }
      if(showKernelsButton.wasTriggered()){
        llm.showKernels();
      }
      if(resetButton.wasTriggered()){
        initLLM();
      }
      
      msleep(15);
    }
  }
};


int main(int n, char **ppc){
  QApplication app(n,ppc);
  
  randomSeed();

  MyThread x;
  x.start();
    
  return app.exec();
}
