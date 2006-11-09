#include <EllipseModel.h>

namespace icl{
 
  template<class T>
  EllipseModel<T>::EllipseModel():QuadraticModel<T>(6){
  }
  
  template<class T>
  T EllipseModel<T>::px(T y, T* params) const{
    // return (b*y+d)/a;
    return (params[1]*y+params[3])/params[0];
  }
  
  template<class T>
  T EllipseModel<T>::qx(T y, T* params) const{
    // return (c*y*y+e*y+f)/a;
    return (params[2]*y*y+params[4]*y+params[5])/params[0];
  }
  
  template<class T>
  T EllipseModel<T>::py(T x, T* params) const{
    // return (b*x+e)/c;
    return (params[1]*x+params[4])/params[2];
  }
  
  template<class T>
  T EllipseModel<T>::qy(T x, T* params) const{
    // return (a*x*x+d*x+f)/c;    
    return (params[0]*x*x+params[3]*x+params[5])/params[2];
  }

  template<class T>
  void EllipseModel<T>::features(T x,T y, T *dst) const{
    dst[0] = x*x;
    dst[1] = x*y;
    dst[2] = y*y;
    dst[3] = x;
    dst[4] = y;
    dst[5] = 1;
  }

  template class EllipseModel<icl32f>;
  template class EllipseModel<icl64f>;
  
}

