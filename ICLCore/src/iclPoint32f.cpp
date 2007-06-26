#include <iclPoint32f.h>
#include <math.h>
namespace icl{
  const Point32f Point32f::null(0.0,0.0);

  float Point32f::norm(float p){
    return pow( pow(x,p)+ pow(y,p), float(1)/p);
  }
}
