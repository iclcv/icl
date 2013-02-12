#include <ICLQt/Common.h>
#include <ICLCore/LineSampler.h>
#include <ICLUtils/StackTimer.h>
#include <ICLCore/Line.h>


int main(){
  Img8u image(Size::VGA,1);
  
  LineSampler ls(image.getImageRect());
  
  for(int y=0;y<480;y+=1){
    for(int x=0;x<640;x+=1){
      BENCHMARK_THIS_SECTION("line sampling")
      Point a(320,240);
      Point b(x,y);

      //Line l(a,b);
      //l.sample(image.getImageRect());
      LineSampler::Result r = ls.sample(a,b);
      //for(int j=0;j<r.n;++j){
      //  image(r[j].x,r[j].y,0) = 255;
      //}
    }
  }
  show(image);
}
