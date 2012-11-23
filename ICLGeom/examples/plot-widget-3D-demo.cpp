#include <ICLQt/Common.h>
#include <ICLGeom/Plot3D.h>
#include <ICLGeom/PointCloudObject.h>
#include <ICLUtils/Random.h>

GUI gui;
Scene scene;


static Range32f round_range(Range32f r){
  if(r.minVal > r.maxVal){
    std::swap(r.minVal,r.maxVal);
  }
  float m = fabs(r.maxVal - r.minVal);
  float f = 1;
  if( m > 1 ){
    while( m/f > 100){
      f *= 10;
    }
  }else{
    while( m/f < 10){
      f *= 0.1;
    }
  }
  r.minVal = floor(r.minVal/f) * f;
  r.maxVal = ceil(r.maxVal/f) * f;
  return r;
}


void init(){
  gui << Plot3D().handle("plot").minSize(32,24) << Show();
  if(pa("-r")){
    SHOW(round_range(Range32f(pa("-r",0),pa("-r",1))));
    ::exit(0);
  }
}


void add_scatter(PlotHandle3D &plot){
  static const int N = 100000;
  static PointCloudObject o(N);
  DataSegment<float,3> s = o.selectXYZ();
  
  GRand r(0,4);
  FixedMatrix<float,3,3> T(0.000001,0,0,
                           0,0.000002,0,
                           0,0,0.000004);
  //  T *= create_rot_3D<float>(1,2,3);
  
  for(int i=0;i<N;++i){
    s[i] =  Vec3(1,1,1) + T * Vec3(r,r,r);
  }
  plot->add(&o,false);
  
}


static Time tt = Time::now();
static float t = 0;

float f(float x, float y){
  return sin(x/10 + 2*t ) * cos(y/20+ t);
}


void add_function(PlotHandle3D &plot){
  TODO_LOG("setting the viewport explicitly seems to not work properly");

  //  plot->setViewPort(Range32f(0,0),Range32f(0,0),Range32f(-3,3)); 
  plot->nocolor();
  t = tt.age().toSecondsDouble();
  static SceneObject *o = 0;
  o = plot->surf(f,Range32f(0,100),Range32f(0,100), 100, 100, o);
}

void run(){
  PlotHandle3D plot = gui["plot"];
    
  plot->clear();
  
  //add_scatter(plot);
  add_function(plot);

  plot->render();
  
  Thread::msleep(30);
}

int main(int n, char **a){
  
  
  
  return ICLApp(n,a,"-r(2)",init,run).exec();
}
