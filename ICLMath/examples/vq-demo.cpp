#include <ICLQt/Common.h>
#include <ICLMath/VectorQuantisation.h>
#include <ICLMath/FixedVector.h>

VBox gui;

typedef FixedColVector<float,2> V2;
typedef GenericVectorQuantisation<V2,float> VQ;

VQ vq;
std::vector<V2> D;

void run(){
  static ButtonHandle b = gui["run"];
  while(!b.wasTriggered()){
    Thread::msleep(10);
  }
  
  static PlotHandle plot = gui["plot"];
  for(int i=0;i<100;++i){
    VQ::Result r = vq.run(D.begin(),D.end(), 1, !i);
    
    plot->clear();
    plot->linewidth(2);
    plot->color(255,0,0);
    plot->label("data");
    plot->sym('.');
    plot->scatter(&D[0][0],&D[0][1],D.size(),2,2);
    
    plot->color(0,100,255);
    plot->label("centers");
    plot->sym('x');
    plot->scatter(&r.centers[0][0],&r.centers[0][1],r.centers.size(),2,2);
    
    plot->render();
    Thread::msleep(25);
  }
}

void init(){
  gui << Plot().handle("plot")
      << Button("run").handle("run") 
      << Show();

  vq.init(pa("-n"));
  
  URand cs(-5,5);
  GRand gr(0,pa("-v"));
  int n=pa("-s");

  for(int i=pa("-c");i>=0;--i){
    V2 c(cs,cs);
    for(int j=0;j<n;++j){
      D.push_back(c + V2(gr,gr));
    }
  }
  
  PlotHandle plot = gui["plot"];
  plot->setPropertyValue("tics.y-distance",2);
  plot->setPropertyValue("tics.x-distance",2);

  plot->label("data");
  plot->scatter(&D[0][0],&D[0][1],D.size(),2,2);
  plot->linewidth(2);
}




int main(int n, char **ppc){
  return ICLApp(n,ppc,"-num-centers|-n(int=5) "
                "-num-clusters|-c(int=5) "
                "-cluster-size|-s(int=1000) "
                "-cluster-var|-v(float=1) ",init,run).exec();
}
