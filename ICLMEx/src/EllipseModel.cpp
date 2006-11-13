#include <EllipseModel.h>

namespace icl{
 
  template<class T>
  EllipseModel<T>::EllipseModel():QuadraticModel<T>(6){
  }
  
  template<class T>
  T EllipseModel<T>::px(T y) const{
    // return (b*y+d)/a;
    return ((*this)[1]*y+(*this)[3])/(*this)[0];
  }
  
  template<class T>
  T EllipseModel<T>::qx(T y) const{
    // return (c*y*y+e*y+f)/a;
    return ((*this)[2]*y*y+(*this)[4]*y+(*this)[5])/(*this)[0];
  }
  
  template<class T>
  T EllipseModel<T>::py(T x) const{
    // return (b*x+e)/c;
    return ((*this)[1]*x+(*this)[4])/(*this)[2];
  }
  
  template<class T>
  T EllipseModel<T>::qy(T x) const{
    // return (a*x*x+d*x+f)/c;    
    return ((*this)[0]*x*x+(*this)[3]*x+(*this)[5])/(*this)[2];
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

