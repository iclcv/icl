#include <iclRandom.h>
namespace icl{
  
  double gaussRandom(double mu, double sigma){
    static bool haveNextGaussian = false;
    static double nextGaussian = 0;
    if(haveNextGaussian){
      haveNextGaussian = false;
      return nextGaussian*sigma + mu;
    } else{
      double v1(0), v2(0), s(0);
      do{
        v1 = 2 * random(1.0)-1;
        v2 = 2 * random(1.0)-1;
        s = v1*v1 + v2*v2;
      }while(s>=1 || s == 0);
      double fac = sqrt(-2.0*log(s)/s);
      nextGaussian = v2 * fac;
      haveNextGaussian = true;
      return v1 * fac * sigma + mu;
    }
  }   

  
}
