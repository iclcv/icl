#include <iclSampledLine.h>
#include <iclRandom.h>

int main(){
  icl::randomSeed();
  icl::URand r(-1000,1000);
  for(int i=0;i<10;++i){
    int x=r,y=r,x2=r,y2=r;
    icl::SampledLine s(x,y,x2,x2);
    std::cout << "Line " << icl::Point(x,y) << " --> " << icl::Point(x2,y2) << std::endl;
    while(s){
      //icl::SampledLine tmp = s; works, but SLOW (of course)
      //s = tmp;
      icl::Point p = *s;
      ++s;
    }
  }

}
