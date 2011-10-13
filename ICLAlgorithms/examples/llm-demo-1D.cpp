/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/examples/llm-test-1D.cpp                 **
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

static LLM llm(1,1);
static const float MINX = 0;
static const float MAXX = 100;
static const float MINY = -10;
static const float MAXY = 10;
static LabelHandle *mseLabel=0;

GUI gui("hbox");

inline float tx(float x){
  return (x-MINX)/(MAXX-MINX); 
}
inline float ty(float y){
  return (y-MINY)/(MAXY-MINY); 
}

float func(float x){
  return sin(x/5)*6;;
  x-= 50;
  x/=20;
  return x>0 ? 2*x*x +3*x -10 : -2*x*x*x +3*x -10;
}


static void trainLLM(){
  int I = 1000;
  float mse = 0;
  for(int i=0;i<I;i++){
    float x = icl::random(MINX,MAXX);
    float y = func(x);
    llm.train(&x,&y,LLM::TRAIN_ALL);
    mse += pow((*llm.apply(&x))-(y),2);
  }
  (*mseLabel) = mse/I;
}

class MyThread : public Thread{
public:
  MyThread(){
    GUI controls("vbox[@minsize=15x0]");
    controls << "button(Train Step)[@handle=train]";
    controls << "togglebutton(Train Off,Train On)[@out=train-loop]";
    controls << "label(NAN)[@handle=mse@label=MSE]";
    controls << "fslider(0,0.1,0.01)[@out=e-in@label=Epsilon In]";
    controls << "fslider(0,0.1,0.01)[@out=e-out@label=Epsilon Out]";
    controls << "fslider(0,0.5,0.1)[@out=e-a@label=Epsilon A]";
    controls << "fslider(0,0.01,0.00)[@out=e-sigma@label=Epsilon Sigma]";
    controls << "button(Show Kernels)[@handle=show-k]";
    controls << "button(Reset)[@handle=reset]";
    controls << "int(1,1000,10)[@out=kc@label=Kernel Count]";

    gui << "draw[@minsize=40x30@label=View@handle=draw]";
    gui << controls;
    
    gui.show();

    
    llm.init(10, vector<Range<icl32f> >(1,Range<icl32f>(MINX,MAXX)),std::vector<float>(1,5));
    
  }
  virtual void run(){
    Img8u bg(Size(100,100),1);
    
    static ICLDrawWidget &w = **gui.getValue<DrawHandle>("draw");
    static ButtonHandle &trainButton = gui.getValue<ButtonHandle>("train");    
    static ButtonHandle &showKernelsButton = gui.getValue<ButtonHandle>("show-k");
    static ButtonHandle &resetButton = gui.getValue<ButtonHandle>("reset");
    static int &kcVal = gui.getValue<int>("kc");
    mseLabel = &gui.getValue<LabelHandle>("mse");
    w.setImage(&bg);
    w.update();
    
    while(1){
      
      llm.setEpsilonIn(gui.getValue<float>("e-in"));
      llm.setEpsilonOut(gui.getValue<float>("e-out"));
      llm.setEpsilonA(gui.getValue<float>("e-a"));
      llm.setEpsilonSigma(gui.getValue<float>("e-sigma"));
      
      if(trainButton.wasTriggered() || gui.getValue<bool>("train-loop")){
        trainLLM();
      }
      if(showKernelsButton.wasTriggered()){
        llm.showKernels();
      }
      if(resetButton.wasTriggered()){
        llm = LLM(1,1);
        llm.init(kcVal, vector<Range<icl32f> >(1,Range<icl32f>(MINX,MAXX)),std::vector<float>(1,5));
      }
      
      w.lock();
      w.reset();
      w.rel();
      
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
          w.color(255,0,0);
          w.line(tx(x),ty(y),tx(xLast),ty(yLast));
          w.color(0,255,0);
          w.line(tx(x),ty(ynet),tx(xLast),ty(ynetLast));
        }
        xLast = x;
        yLast = y;
        ynetLast = ynet;
      }
      
      w.color(255,255,0);
      for(unsigned int i=0; i< llm.numKernels();++i){
        w.color(255,255,0);
        float x = tx(llm[i].w_in[0]);
        float y = ty(llm[i].w_out[0]);
        w.line(x,y,x,0);
        
        float m = llm[i].A[0];
        static const float W = 0.05;
        w.line(x-W,y-m*W,x+W,y+m*W);
        
        w.color(255,0,255);
        float stddev = tx(sqrt(llm[i].var[0]));
        w.line(x-stddev,y-0.01,x+stddev,y-0.01);
      }
      
      
      w.unlock();
      w.update();
      msleep(50);
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
