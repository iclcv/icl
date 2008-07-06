#include "iclFixedMatrix.h"
#include <cmath>

namespace icl{

  template<class T, bool skip3rd> 
  inline void get_2x2_rot_data(T r ,T *p){
    *p++=cos(r); *p++=sin(r); if(skip3rd) p++;
    *p++=sin(r); *p++=-cos(r);
  }

  
  /// defined for float and double
  template<class T>
  FixedMatrix<T,2,2> create_rot_2D(T angle){
    FixedMatrix<T,2,2> m;
    get_2x2_rot_data<T,false>(angle,m.data());
    return m;
  }
  
  /// defined for float and double
  template<class T>
  FixedMatrix<T,3,3> create_hom_3x3(T angle, T dx, T dy, T v0, T v1){
    FixedMatrix<T,3,3> m;
    get_2x2_rot_data<T,true>(angle,m.data());
    m(2,0) = dx;
    m(2,1) = dy;
    m(2,2) = 1;
    m(0,2) = v0;
    m(1,2) = v1;
    return m;
  }
  
  template<class T, bool skip4th> 
  inline void get_3x3_rot_data(T rx, T ry, T rz,T *p){
    T cx = cos(rx);
    T cy = cos(ry);
    T cz = cos(rz);
    T sx = sin(rx);
    T sy = sin(ry);
    T sz = sin(rz);
    *p++=cy*cz-sx*sy*sz; *p++=-sz*cx; *p++=cz*sy+sz*sx*cy; if(skip4th) p++;
    *p++=cy*sz+cz*sx*sy; *p++=cz*cx;  *p++=sz*sy-sx*cy*cz; if(skip4th) p++;
    *p++=-sy*cx;         *p++=sx;     *p++=cx*cy;       
  }

  template<class T>
  FixedMatrix<T,3,3> create_rot_3D(T rx,T ry,T rz){
    FixedMatrix<T,3,3> m;
    get_3x3_rot_data<T,false>(rx,ry,rz,m.data());
    return m;
  }

  /// defined for float and double
  template<class T>
  FixedMatrix<T,4,4> create_hom_4x4(T rx, T ry, T rz, T dx, T dy, T dz, T v0, T v1, T v2){
    FixedMatrix<T,4,4> m;
    get_3x3_rot_data<T,true>(rx,ry,rz,m.data());
    m(3,0) = dx;
    m(3,1) = dx;
    m(3,2) = dx;
    m(3,3) = 1;
    
    m(0,3) = v0;
    m(1,3) = v1;
    m(2,3) = v2;
    return m;
  }
  
#define INSTANTIATE(T)                                          \
  template FixedMatrix<T,2,2> create_rot_2D(T);                 \
  template FixedMatrix<T,3,3> create_hom_3x3(T,T,T,T,T);        \
  template FixedMatrix<T,3,3> create_rot_3D(T,T,T);             \
  template FixedMatrix<T,4,4> create_hom_4x4(T,T,T,T,T,T,T,T,T);

  INSTANTIATE(float);
  INSTANTIATE(double);
#undef INSTANTIATE
}
