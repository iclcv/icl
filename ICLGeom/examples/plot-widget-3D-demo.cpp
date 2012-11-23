#include <ICLQt/Common.h>
#include <ICLGeom/Plot3D.h>
#include <ICLGeom/PointCloudObject.h>
#include <ICLUtils/Random.h>

GUI gui;
Scene scene;

void init(){
  gui << Plot3D().handle("plot").minSize(32,24) << Show();
}

void run(){
  PlotHandle3D plot = gui["plot"];
  
  static const int N = 10000;
  static PointCloudObject o(N);
  DataSegment<float,3> s = o.selectXYZ();
  
  GRand r(0,4);
  FixedMatrix<float,3,3> T(1,0,0,
                           0,2,0,
                           0,0,4);
  T *= create_rot_3D<float>(1,2,3);
  
  for(int i=0;i<N;++i){
    s[i] = Vec3(3,1,0) + T * Vec3(r,r,r);
  }
  
  plot->clear();
  plot->add(&o,false);
  plot->render();
  
  Thread::msleep(30);
}

int main(int n, char **a){
  
  
  
  return ICLApp(n,a,"",init,run).exec();
}
