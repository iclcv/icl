/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/examples/llm-test-2D.cpp                 **
** Module : ICLAlgorithms                                          **
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

#include <ICLAlgorithms/LLM.h>
#include <ICLQuick/Common.h>
#include <ICLCore/Mathematics.h>


static LLM llm(2,3);
GUI gui("hbox");
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

    GUI controls("vbox[@minsize=15x0]");
    controls << "button(Train Step)[@handle=train]";
    controls << "togglebutton(Train Off,Train On)[@out=train-loop]";
    controls << "togglebutton(NO Soft-Max,Soft-Max)[@out=use-soft-max]";
    controls << "slider(1,10000,1000)[@out=steps@label=Steps per Cycle]";
    controls << "disp(3,1)[@handle=mse@label=MSE@minsize=5x2]";
    controls << "fslider(0,0.1,0.01)[@out=e-in@label=Epsilon In]";
    controls << "fslider(0,0.1,0.01)[@out=e-out@label=Epsilon Out]";
    controls << "fslider(0,0.5,0.1)[@out=e-a@label=Epsilon A]";
    controls << "fslider(0,0.0001,0.0)[@out=e-sigma@label=Epsilon Sigma]";
    controls << "fslider(1,100,10)[@out=init-sigma@label=Initial Sigma]";
    controls << "button(Show Kernels)[@handle=show-k]";
    controls << "button(Reset)[@handle=reset]";
    controls << "int(1,1000,20)[@out=kc@label=Kernel Count]";
    
    gui << "image[@minsize=20x15@label=Original@handle=orig-image]";
    gui << "image[@minsize=20x15@label=Net Output@handle=net-image]";
    gui << controls;
    
    gui.show();
    
    gui.getValue<ImageHandle>("orig-image") = &image;    
    gui.getValue<ImageHandle>("orig-image").update();
    
    initLLM();
  }
  virtual void run(){
    static ICLWidget &w = **gui.getValue<ImageHandle>("net-image");
    static ButtonHandle &trainButton = gui.getValue<ButtonHandle>("train");    
    static ButtonHandle &showKernelsButton = gui.getValue<ButtonHandle>("show-k");
    static ButtonHandle &resetButton = gui.getValue<ButtonHandle>("reset");


    nKernels = &gui.getValue<int>("kc");
    mseDisp = &gui.getValue<DispHandle>("mse");
    nTrainingSteps = &gui.getValue<int>("steps");
    initialSigma = &gui.getValue<float>("init-sigma");
    softMaxEnabled = &gui.getValue<bool>("use-soft-max");
    //    ImgQ netImage(image.getSize(),formatRGB);
    ImgQ netImage = copy(image);
    w.setImage(&netImage);

    w.updateFromOtherThread();
    
    while(1){
      
      llm.setEpsilonIn(gui.getValue<float>("e-in"));
      llm.setEpsilonOut(gui.getValue<float>("e-out"));
      llm.setEpsilonA(gui.getValue<float>("e-a"));
      llm.setEpsilonSigma(gui.getValue<float>("e-sigma"));
      llm.setSoftMaxEnabled(*softMaxEnabled);
      
      if(trainButton.wasTriggered() || gui.getValue<bool>("train-loop")){
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
        w.updateFromOtherThread();
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
