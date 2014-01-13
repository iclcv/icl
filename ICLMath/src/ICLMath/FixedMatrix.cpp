/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/FixedMatrix.cpp                    **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLMath/FixedMatrix.h>
#include <ICLMath/FixedVector.h>
#include <ICLUtils/ConfigFile.h>
#include <cmath>
#include <string>
#include <boost/math/tools/precision.hpp>

namespace icl{
  namespace math{
    const AXES AXES_DEFAULT = rxyz;

    template<class T, bool skip3rd> 
    inline void get_2x2_rot_data(T r ,T *p){
      *p++=cos(r); *p++=-sin(r); if(skip3rd) p++;
      *p++=sin(r); *p++=cos(r);
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

    static const unsigned int AXES2TUPLE[][4] = 
    {/*sxyz*/ {0, 0, 0, 0}, /*sxyx*/ {0, 0, 1, 0}, /*sxzy*/ {0, 1, 0, 0},
     /*sxzx*/ {0, 1, 1, 0}, /*syzx*/ {1, 0, 0, 0}, /*syzy*/ {1, 0, 1, 0},
     /*syxz*/ {1, 1, 0, 0}, /*syxy*/ {1, 1, 1, 0}, /*szxy*/ {2, 0, 0, 0},
     /*szxz*/ {2, 0, 1, 0}, /*szyx*/ {2, 1, 0, 0}, /*szyz*/ {2, 1, 1, 0},
     /*rzyx*/ {0, 0, 0, 1}, /*rxyx*/ {0, 0, 1, 1}, /*ryzx*/ {0, 1, 0, 1},
     /*rxzx*/ {0, 1, 1, 1}, /*rxzy*/ {1, 0, 0, 1}, /*ryzy*/ {1, 0, 1, 1},
     /*rzxy*/ {1, 1, 0, 1}, /*ryxy*/ {1, 1, 1, 1}, /*ryxz*/ {2, 0, 0, 1},
     /*rzxz*/ {2, 0, 1, 1}, /*rxyz*/ {2, 1, 0, 1}, /*rzyz*/ {2, 1, 1, 1}};
    static const unsigned int NEXT_AXIS[4] = {1, 2, 0, 1};

    template<class T>
    FixedMatrix<T,3,3> create_rot_3D (T ai, T aj, T ak, AXES axes) {
      const unsigned int* const tuple = AXES2TUPLE[axes];
      const unsigned int 
        firstaxis  = tuple[0],
        parity     = tuple[1],
        repetition = tuple[2],
        frame      = tuple[3]; // static=0 or rotated=1
      unsigned int 
        i = firstaxis,
        j = NEXT_AXIS[i+parity],
        k = NEXT_AXIS[i-parity+1];
      if (frame)  std::swap (ai, ak);
      if (parity) {ai*=-1; aj*=-1, ak*=-1;}

      T si = sin(ai), sj = sin(aj), sk = sin(ak);
      T ci = cos(ai), cj = cos(aj), ck = cos(ak);
      T cc = ci*ck, cs = ci*sk;
      T sc = si*ck, ss = si*sk;

      FixedMatrix<T,3,3> m;
      if (repetition) {
        m(i,i) = cj;
        m(j,i) = sj*si;
        m(k,i) = sj*ci;
        m(i,j) = sj*sk;
        m(j,j) = -cj*ss+cc;
        m(k,j) = -cj*cs-sc;
        m(i,k) = -sj*ck;
        m(j,k) = cj*sc+cs;
        m(k,k) = cj*cc-ss;
      }else{
        m(i,i) = cj*ck;
        m(j,i) = sj*sc-cs;
        m(k,i) = sj*cc+ss;
        m(i,j) = cj*sk;
        m(j,j) = sj*ss+cc;
        m(k,j) = sj*cs-sc;
        m(i,k) = -sj;
        m(j,k) = cj*si;
        m(k,k) = cj*ci;
      }
      return m;
    }

    template<class T,unsigned int COLS, unsigned int ROWS>
    FixedMatrix<T,1,3> 
    extract_euler_angles (const FixedMatrix<T,COLS,ROWS> &m,
                          AXES axes) {
      static const T EPS = 4.0 * boost::math::tools::epsilon<T>();
      T ai, aj, ak;
      
      const unsigned int* const tuple = AXES2TUPLE[axes];
      const unsigned int 
        firstaxis  = tuple[0],
        parity     = tuple[1],
        repetition = tuple[2],
        frame      = tuple[3]; // static=0 or rotated=1
      unsigned int 
        i = firstaxis,
        j = NEXT_AXIS[i+parity],
        k = NEXT_AXIS[i-parity+1];

      if (repetition) {
        T sy = sqrt(m(j,i)*m(j,i) + m(k,i)*m(k,i));
        if (sy > EPS) {
            ai = atan2( m(j,i),  m(k,i));
            aj = atan2( sy,      m(i,i));
            ak = atan2( m(i,j), -m(i,k));
        }else{
            ai = atan2(-m(k,j),  m(j,j));
            aj = atan2( sy,      m(i,i));
            ak = 0.0;
        }
      }else{
        T cy = sqrt(m(i,i)*m(i,i) + m(i,j)*m(i,j));
        if (cy > EPS) {
            ai = atan2( m(j,k),  m(k,k));
            aj = atan2(-m(i,k),  cy);
            ak = atan2( m(i,j),  m(i,i));
        }else{
            ai = atan2(-m(k,j),  m(j,j));
            aj = atan2(-m(i,k),  cy);
            ak = 0.0;
        }
      }

      if (parity) {ai*=-1; aj*=-1, ak*=-1;}
      if (frame)  std::swap (ai, ak);

      return FixedMatrix<T,1,3>(ai, aj, ak);
    }
    template<class T>
    FixedMatrix<T,1,3> extract_euler_angles (const FixedMatrix<T,3,3> &m, AXES axes) {
      return extract_euler_angles<T,3,3>(m, axes);
    }
    template<class T>
    FixedMatrix<T,1,3> extract_euler_angles (const FixedMatrix<T,4,4> &m, AXES axes) {
      return extract_euler_angles<T,4,4>(m, axes);
    }

    template<class T, bool skip4th> 
    inline void get_3x3_rot_data(T rx, T ry, T rz,  T *p){
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
  
    /// defined for float and double
    template<class T>
    FixedMatrix<T,4,4> create_hom_4x4(T rx, T ry, T rz, 
                                      T dx, T dy, T dz, 
                                      T v0, T v1, T v2,
                                      AXES axes){
      FixedMatrix<T,4,4> m;
#if 1
      FixedMatrix<T,3,3> r = create_rot_3D (rx, ry, rz, axes);
//      FixedMatrix<T,3,3> r = create_rot_3D (-rz,rx,-ry, rzxy);

      // The layout in memory is transposed! This is correct:
      T *M = m.data(), *R = r.data();
      *M++ = *R++;  *M++ = *R++;  *M++ = *R++;  *M++ = dx;
      *M++ = *R++;  *M++ = *R++;  *M++ = *R++;  *M++ = dy;
      *M++ = *R++;  *M++ = *R++;  *M++ = *R++;  *M++ = dz;
      *M++ = v0;    *M++ = v1;    *M++ = v2;    *M++ = 1;
#else
      get_3x3_rot_data<T,true>(rx,ry,rz,m.data());
      m(3,0) = dx;
      m(3,1) = dy;
      m(3,2) = dz;
      m(3,3) = 1;
      
      m(0,3) = v0;
      m(1,3) = v1;
      m(2,3) = v2;      
#endif
      return m;
    }
  
    
    
  #define INSTANTIATE(T)                                          \
    template FixedMatrix<T,2,2> create_rot_2D(T);                 \
    template FixedMatrix<T,3,3> create_hom_3x3(T,T,T,T,T);        \
    template FixedMatrix<T,3,3> create_rot_3D(T,T,T,AXES);        \
    template FixedMatrix<T,4,4> create_hom_4x4(T,T,T,T,T,T,T,T,T,AXES); \
    template FixedMatrix<T,1,3> extract_euler_angles(const FixedMatrix<T,3,3>&, AXES);\
    template FixedMatrix<T,1,3> extract_euler_angles(const FixedMatrix<T,4,4>&, AXES);\

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
  


    struct FixedMatrixConfigFileStaticRegistration{
      FixedMatrixConfigFileStaticRegistration(){
        // todo:: move this to core!
        // copied from ICLCore/Color.h
        typedef FixedColVector<icl8u,3> Color;
        typedef FixedColVector<icl32f,3> Color32f;
        typedef FixedColVector<icl8u,4> Color4D;
        typedef FixedColVector<icl32f,4> Color4D32f;
        
        REGISTER_CONFIG_FILE_TYPE(Color);
        REGISTER_CONFIG_FILE_TYPE(Color32f);
        REGISTER_CONFIG_FILE_TYPE(Color4D);
        REGISTER_CONFIG_FILE_TYPE(Color4D32f);
        
        typedef  FixedRowVector<float,3> RVec3;
        typedef  FixedRowVector<float,4> RVec;
  
        REGISTER_CONFIG_FILE_TYPE(RVec3);
        REGISTER_CONFIG_FILE_TYPE(RVec);

        typedef  FixedMatrix<float,3,3> Mat3x3;
        typedef  FixedMatrix<float,3,4> Mat3x4;
        typedef  FixedMatrix<float,4,3> Mat4x3;
        typedef  FixedMatrix<float,4,4> Mat;
  
        REGISTER_CONFIG_FILE_TYPE(Mat3x3);
        REGISTER_CONFIG_FILE_TYPE(Mat3x4);
        REGISTER_CONFIG_FILE_TYPE(Mat4x3);
        REGISTER_CONFIG_FILE_TYPE(Mat);

        typedef  FixedColVector<float,3> Vec3;
        typedef  FixedColVector<float,4> Vec;
        REGISTER_CONFIG_FILE_TYPE(Vec3);
        REGISTER_CONFIG_FILE_TYPE(Vec);
      }
    } FixedMatrixConfigFileStaticRegistration__Instance;
  } // namespace math
}
