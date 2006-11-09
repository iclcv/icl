#include <RestrictedEllipseModel.h>

namespace icl{
 
  template<class T>
  RestrictedEllipseModel<T>::RestrictedEllipseModel():QuadraticModel<T>(5){
  }
  
  template<class T>
  T RestrictedEllipseModel<T>::px(T y, T* params) const{
    // return c/a;
    return params[2]/params[0];
  }

  // real p = c/a;
  // real q = (b*y*y+d*y+e)/a;


  template<class T>
  T RestrictedEllipseModel<T>::qx(T y, T* params) const{
    // return  (b*y*y+d*y+e)/a;
    return (params[1]*y*y+params[3]*y+params[4])/params[0];
  }
  
  template<class T>
  T RestrictedEllipseModel<T>::py(T x, T* params) const{
    // return  d/b;
    return params[3]/params[1];
  }
  
  template<class T>
  T RestrictedEllipseModel<T>::qy(T x, T* params) const{
    // return  (a*x*x+c*x+e)/b;
    return (params[0]*x*x+params[2]*x+params[4])/params[1];
  }

  template<class T>
  void RestrictedEllipseModel<T>::features(T x,T y, T *dst) const{
    dst[0] = x*x;
    dst[1] = y*y;
    dst[2] = x;
    dst[3] = y;
    dst[4] = 1;
  }

  template class RestrictedEllipseModel<icl32f>;
  template class RestrictedEllipseModel<icl64f>;
  
}

