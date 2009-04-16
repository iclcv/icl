#include "iclFixedMatrix.h"
#include <cmath>
#include <string>

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
    T cy = cos(-ry);
    T cz = cos(-rz);
    T sx = sin(rx);
    T sy = sin(-ry);
    T sz = sin(-rz);
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


#ifdef USE_OPTIMIZED_INV_AND_DET_FOR_2X2_3X3_AND_4X4_MATRICES
  template<class T> 
  void icl_util_get_fixed_4x4_matrix_inv(const T *src, T*dst){
    T d = icl_util_get_fixed_4x4_matrix_det(src);
    if(!d) throw SingularMatrixException("matrix is too singular");
    const T &m00=*src++; const T &m01=*src++; const T &m02=*src++; const T &m03=*src++;
    const T &m10=*src++; const T &m11=*src++; const T &m12=*src++; const T &m13=*src++;
    const T &m20=*src++; const T &m21=*src++; const T &m22=*src++; const T &m23=*src++;
    const T &m30=*src++; const T &m31=*src++; const T &m32=*src++; const T &m33=*src++;
    *dst++ = ( m12*m23*m31 - m13*m22*m31 + m13*m21*m32 - m11*m23*m32 - m12*m21*m33 + m11*m22*m33 )/d;
    *dst++ = ( m03*m22*m31 - m02*m23*m31 - m03*m21*m32 + m01*m23*m32 + m02*m21*m33 - m01*m22*m33 )/d;
    *dst++ = ( m02*m13*m31 - m03*m12*m31 + m03*m11*m32 - m01*m13*m32 - m02*m11*m33 + m01*m12*m33 )/d;
    *dst++ = ( m03*m12*m21 - m02*m13*m21 - m03*m11*m22 + m01*m13*m22 + m02*m11*m23 - m01*m12*m23 )/d;
    *dst++ = ( m13*m22*m30 - m12*m23*m30 - m13*m20*m32 + m10*m23*m32 + m12*m20*m33 - m10*m22*m33 )/d;
    *dst++ = ( m02*m23*m30 - m03*m22*m30 + m03*m20*m32 - m00*m23*m32 - m02*m20*m33 + m00*m22*m33 )/d;
    *dst++ = ( m03*m12*m30 - m02*m13*m30 - m03*m10*m32 + m00*m13*m32 + m02*m10*m33 - m00*m12*m33 )/d;
    *dst++ = ( m02*m13*m20 - m03*m12*m20 + m03*m10*m22 - m00*m13*m22 - m02*m10*m23 + m00*m12*m23 )/d;
    *dst++ = ( m11*m23*m30 - m13*m21*m30 + m13*m20*m31 - m10*m23*m31 - m11*m20*m33 + m10*m21*m33 )/d;
    *dst++ = ( m03*m21*m30 - m01*m23*m30 - m03*m20*m31 + m00*m23*m31 + m01*m20*m33 - m00*m21*m33 )/d;
    *dst++ = ( m01*m13*m30 - m03*m11*m30 + m03*m10*m31 - m00*m13*m31 - m01*m10*m33 + m00*m11*m33 )/d;
    *dst++ = ( m03*m11*m20 - m01*m13*m20 - m03*m10*m21 + m00*m13*m21 + m01*m10*m23 - m00*m11*m23 )/d;
    *dst++ = ( m12*m21*m30 - m11*m22*m30 - m12*m20*m31 + m10*m22*m31 + m11*m20*m32 - m10*m21*m32 )/d;
    *dst++ = ( m01*m22*m30 - m02*m21*m30 + m02*m20*m31 - m00*m22*m31 - m01*m20*m32 + m00*m21*m32 )/d;
    *dst++ = ( m02*m11*m30 - m01*m12*m30 - m02*m10*m31 + m00*m12*m31 + m01*m10*m32 - m00*m11*m32 )/d;
    *dst++ = ( m01*m12*m20 - m02*m11*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 + m00*m11*m22 )/d;
  }

  
  template<class T> 
  void icl_util_get_fixed_3x3_matrix_inv(const T *src, T*dst){
    T det = icl_util_get_fixed_3x3_matrix_det(src);

    if(!det) throw SingularMatrixException("matrix is too singular");
    
    const T &m11 = *src++; const T &m12 = *src++; const T &m13 = *src++;
    const T &m21 = *src++; const T &m22 = *src++; const T &m23 = *src++;
    const T &m31 = *src++; const T &m32 = *src++; const T &m33 = *src++;

    *dst++ =  (m33*m22-m32*m23)/det;
    *dst++ = -(m33*m12-m32*m13)/det;    
    *dst++ =  (m23*m12-m22*m13)/det;

    *dst++ = -(m33*m21-m31*m23)/det;
    *dst++ =  (m33*m11-m31*m13)/det;
    *dst++ = -(m23*m11-m21*m13)/det;

    *dst++ =  (m32*m21-m31*m22)/det;
    *dst++ = -(m32*m11-m31*m12)/det;
    *dst++ =  (m22*m11-m21*m12)/det;
    
  }
  template<class T> 
  void icl_util_get_fixed_2x2_matrix_inv(const T *src, T*dst){
    T d = icl_util_get_fixed_2x2_matrix_det(src);
    if(!d) throw SingularMatrixException("matrix is too singular");
    *dst++ = src[3]/d;
    *dst++ = -src[1]/d;
    *dst++ = -src[2]/d;
    *dst++ = src[0]/d;
  }

  template<class T> 
  T icl_util_get_fixed_4x4_matrix_det(const T *src){
    const T &m00=*src++; const T &m01=*src++; const T &m02=*src++; const T &m03=*src++;
    const T &m10=*src++; const T &m11=*src++; const T &m12=*src++; const T &m13=*src++;
    const T &m20=*src++; const T &m21=*src++; const T &m22=*src++; const T &m23=*src++;
    const T &m30=*src++; const T &m31=*src++; const T &m32=*src++; const T &m33=*src++;
    return
    m03 * m12 * m21 * m30-m02 * m13 * m21 * m30-m03 * m11 * m22 * m30+m01 * m13 * m22 * m30+
    m02 * m11 * m23 * m30-m01 * m12 * m23 * m30-m03 * m12 * m20 * m31+m02 * m13 * m20 * m31+
    m03 * m10 * m22 * m31-m00 * m13 * m22 * m31-m02 * m10 * m23 * m31+m00 * m12 * m23 * m31+
    m03 * m11 * m20 * m32-m01 * m13 * m20 * m32-m03 * m10 * m21 * m32+m00 * m13 * m21 * m32+
    m01 * m10 * m23 * m32-m00 * m11 * m23 * m32-m02 * m11 * m20 * m33+m01 * m12 * m20 * m33+
    m02 * m10 * m21 * m33-m00 * m12 * m21 * m33-m01 * m10 * m22 * m33+m00 * m11 * m22 * m33;
  }
  template<class T> 
  T icl_util_get_fixed_3x3_matrix_det(const T *src){
    const T &a = *src++; const T &b = *src++; const T &c = *src++;
    const T &d = *src++; const T &e = *src++; const T &f = *src++;
    const T &g = *src++; const T &h = *src++; const T &i = *src++;
    return ( a*e*i + b*f*g + c*d*h ) - ( g*e*c + h*f*a + i*d*b);
  }
  template<class T> 
  T icl_util_get_fixed_2x2_matrix_det(const T *src){
    return src[0]*src[3]-src[1]*src[2];
  }

#define INSTANTIATE_INV_AND_DET_OPT_FUNCS(T,D)                             \
  template void icl_util_get_fixed_##D##x##D##_matrix_inv<T>(const T*,T*); \
  template T icl_util_get_fixed_##D##x##D##_matrix_det<T>(const T*);

INSTANTIATE_INV_AND_DET_OPT_FUNCS(float,2);
INSTANTIATE_INV_AND_DET_OPT_FUNCS(float,3);
INSTANTIATE_INV_AND_DET_OPT_FUNCS(float,4);

INSTANTIATE_INV_AND_DET_OPT_FUNCS(double,2);
INSTANTIATE_INV_AND_DET_OPT_FUNCS(double,3);
INSTANTIATE_INV_AND_DET_OPT_FUNCS(double,4);

#undef INSTANTIATE_INV_AND_DET_OPT_FUNCS


#endif
  

  template<class T>
  FixedMatrix<T,3,3> create_rot_3D(T axisX, T axisY, T axisZ, T angle){
    angle /= 2;
    T a = cos(angle);
    T sa = sin(angle);
    T b = axisX * sa;
    T c = axisY * sa;
    T d = axisZ * sa;
    
    T a2=a*a, b2=b*b, c2=c*c, d2=d*d;
    T ab=a*b, ac=a*c, ad=a*d, bc=b*c, bd=b*d, cd=c*d;
    
    return FixedMatrix<T,3,3>(a2+b2-c2-d2,  2*bc-2*ad,   2*ac+2*bd,  
                              2*ad+2*bd,    a2-b2+c2-d2, 2*cd-2*ab,  
                              2*bd-2*ac,    2*ab+2*cd,   a2-b2-c2+d2);
  }


  template<class T>
  FixedMatrix<T,4,4> create_rot_4x4(T axisX, T axisY, T axisZ, T angle){
    angle /= 2;
    T a = cos(angle);
    T sa = sin(angle);
    T b = axisX * sa;
    T c = axisY * sa;
    T d = axisZ * sa;
    
    T a2=a*a, b2=b*b, c2=c*c, d2=d*d;
    T ab=a*b, ac=a*c, ad=a*d, bc=b*c, bd=b*d, cd=c*d;
    
    return FixedMatrix<T,4,4>(a2+b2-c2-d2,  2*bc-2*ad,   2*ac+2*bd,   0,
                              2*ad+2*bd,    a2-b2+c2-d2, 2*cd-2*ab,   0,
                              2*bd-2*ac,    2*ab+2*cd,   a2-b2-c2+d2, 0,
                              0,            0,           0,           1);
  }
  
  
  template FixedMatrix<float,4,4> create_rot_4x4(float,float,float,float);
  template FixedMatrix<double,4,4> create_rot_4x4(double,double,double,double);

  template FixedMatrix<float,3,3> create_rot_3D(float,float,float,float);
  template FixedMatrix<double,3,3> create_rot_3D(double,double,double,double);

}
