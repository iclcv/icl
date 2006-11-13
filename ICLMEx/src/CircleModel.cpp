#include <CircleModel.h>

namespace icl{
 
  template<class T>
  CircleModel<T>::CircleModel():QuadraticModel<T>(4){
  }
  
  template<class T>
  T CircleModel<T>::px(T y) const{
    // return b/a;
    return (*this)[1]/(*this)[0];
  }
  
  template<class T>
  T CircleModel<T>::qx(T y) const{
    // return (a*y*y+c*y+d)/a;
    return ((*this)[0]*y*y+(*this)[2]*y+(*this)[3])/(*this)[0];
  }
  
  template<class T>
  T CircleModel<T>::py(T x) const{
    // return  c/a; 
    return (*this)[2]/(*this)[0];
  }
  
  template<class T>
  T CircleModel<T>::qy(T x) const{
    // return (a*x*x+b*x+d)/a; 
    return ((*this)[0]*x*x+(*this)[1]*x+(*this)[3])/(*this)[0];
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

