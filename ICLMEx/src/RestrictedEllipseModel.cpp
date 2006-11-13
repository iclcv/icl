#include <RestrictedEllipseModel.h>

namespace icl{
 
  template<class T>
  RestrictedEllipseModel<T>::RestrictedEllipseModel():QuadraticModel<T>(5){
  }
  
  template<class T>
  T RestrictedEllipseModel<T>::px(T y) const{
    // return c/a;
    return (*this)[2]/(*this)[0];
  }

  // real p = c/a;
  // real q = (b*y*y+d*y+e)/a;


  template<class T>
  T RestrictedEllipseModel<T>::qx(T y) const{
    // return  (b*y*y+d*y+e)/a;
    return ((*this)[1]*y*y+(*this)[3]*y+(*this)[4])/(*this)[0];
  }
  
  template<class T>
  T RestrictedEllipseModel<T>::py(T x) const{
    // return  d/b;
    return (*this)[3]/(*this)[1];
  }
  
  template<class T>
  T RestrictedEllipseModel<T>::qy(T x) const{
    // return  (a*x*x+c*x+e)/b;
    return ((*this)[0]*x*x+(*this)[2]*x+(*this)[4])/(*this)[1];
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

