#include <ICLQuick/Common.h>

GUI gui;
ImgQ imU,imD;

struct Params{
  float K[3];
  float P[5];
  int cx,cy;
};

Point xyu(int xd, int yd, const Params &p){
  float xdq = xd-p.cx;
  float ydq = yd-p.cy;
  float rd2 = xdq*xdq + ydq*ydq;
  
  return xd + xdq*( p.K[0]*rd2 + p.K[1]*rd2*rd2 + p.K[2]*rd*rd*rd)+
  (p.P[0]*(rd2+2*xdq*xdq) + P.P[1]* )*();
}

void step(){
  
}


void init(){
  gui << "image[@handle=image@minsize=32x24]"
      << ( GUI("vbox") 
           << "slider(0,1,0)[@out=k1@label=kappa 1]"
           << "slider(0,1,0)[@out=k2@label=kappa 2]"
           << "slider(0,1,0)[@out=k3@label=kappa 3]"
           << "slider(0,1,0)[@out=p1@label=peta 1]"
           << "slider(0,1,0)[@out=p2@label=peta 2]"
           << "slider(0,1,0)[@out=p3@label=peta 3]"
           << "slider(0,1,0)[@out=p4@label=peta 4]"
           << "slider(0,1,0)[@out=p5@label=peta 5]"
           ) 
      << "!show";
           
  gui.registerCallback(new GUI::Callback(step),"k1,k2,k3,p1,p2,p3,p4,p5");
  
  imU = zeros(640,480,1);
  imD = zeros(640,480,1);
}


int main(int n, char **p){
  return ICLApp(n,p,"",init).exec();
}
