#include <CircleModel.h>

namespace icl{
 
  template<class T>
  CircleModel<T>::CircleModel():QuadraticModel<T>(4){
  }
  
  template<class T>
  T CircleModel<T>::px(T y, T* params) const{
    // return b/a;
    return params[1]/params[0];
  }
  
  template<class T>
  T CircleModel<T>::qx(T y, T* params) const{
    // return (a*y*y+c*y+d)/a;
    return (params[0]*y*y+params[2]*y+params[3])/params[0];
  }
  
  template<class T>
  T CircleModel<T>::py(T x, T* params) const{
    // return  c/a; 
    return params[2]/params[0];
  }
  
  template<class T>
  T CircleModel<T>::qy(T x, T* params) const{
    // return (a*x*x+b*x+d)/a; 
    return (params[0]*x*x+params[1]*x+params[3])/params[0];
  }

  template<class T>
  void CircleModel<T>::features(T x,T y, T *dst) const{
    dst[0] = x*x+y*y;
    dst[1] = x;
    dst[2] = y;
    dst[3] = 1;
  }

  template class CircleModel<icl32f>;
  template class CircleModel<icl64f>;
}

