#include "iclMat4D.h"

namespace icl{

  template<class T>
  Mat4D<T> Mat4D<T>::id() { 
    static Mat4D<T> ID_MAT( 1,0,0,0,
                            0,1,0,0,
                            0,0,1,0,
                            0,0,0,1 );
    return ID_MAT;
  }
  

  /* OpenGL matrix layout is: M = Rz*Rx*Ry
   * cy*cz-sx*sy*sz    -sz*cx     cz*sy+sz*sx*cy  x
   * cy*sz+cz*sx*sy     cz*cx     sz*sy-sx*cy*cz  y
   * -sy*cx              sx       cx*cy           z
   * 0                   0          0             1x
  */
  template<class T>
  Mat4D<T> Mat4D<T>::rot(double rx, double ry, double rz){
    double cx = cos(rx);
    double cy = cos(ry);
    double cz = cos(rz);
    double sx = sin(rx);
    double sy = sin(ry);
    double sz = sin(rz);
    return Mat4D<double>( cy*cz-sx*sy*sz , -sz*cx , cz*sy+sz*sx*cy , 0,
                          cy*sz+cz*sx*sy ,  cz*cx , sz*sy-sx*cy*cz , 0,
                          -sy*cx         ,  sx    , cx*cy          , 0,
                          0              ,  0     , 0              , 1  );
  }
  
  template<class T>
  Mat4D<T> Mat4D<T>::trans(T dx, T dy, T dz){
    return Mat4D<T>( 1 , 0 , 0 , dx ,
                     0 , 1 , 0 , dy ,
                     0 , 0 , 1 , dz ,
                     0 , 0 , 0 , 1 );
  }
  
  template<class T>
  Mat4D<T> Mat4D<T>::hom(double rx, double ry, double rz, T dx, T dy, T dz, double v1, double v2, double v3){
    double cx = cos(rx);
    double cy = cos(ry);
    double cz = cos(rz);
    double sx = sin(rx);
    double sy = sin(ry);
    double sz = sin(rz);
    return Mat4D<double>( cy*cz-sx*sy*sz , -sz*cx , cz*sy+sz*sx*cy , dx,
                          cy*sz+cz*sx*sy ,  cz*cx , sz*sy-sx*cy*cz , dy,
                          -sy*cx         ,  sx    , cx*cy          , dz,
                          v1             ,  v2    , v3             , 1  );
  }
  

  /** about 10 percent slower then the unfolded version below !
      template<class T>
      void Mat4D<T>::mult(const Mat4D<T> &A, const Mat4D<T> &B, Mat4D<T> &RESULT){
      for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
      RESULT[i][j] = A[0][j]*B[i][0] + A[1][j]*B[i][1] + A[2][j]*B[i][2] + A[3][j]*B[i][3];
      }
      }
      } 
  **/
   
  template<class T>
  void Mat4D<T>::mult(const Mat4D<T> &A, const Mat4D<T> &B, Mat4D<T> &RESULT){
    RESULT[0][0] = A[0][0]*B[0][0] + A[1][0]*B[0][1] + A[2][0]*B[0][2] + A[3][0]*B[0][3];
    RESULT[0][1] = A[0][1]*B[0][0] + A[1][1]*B[0][1] + A[2][1]*B[0][2] + A[3][1]*B[0][3];
    RESULT[0][2] = A[0][2]*B[0][0] + A[1][2]*B[0][1] + A[2][2]*B[0][2] + A[3][2]*B[0][3];
    RESULT[0][3] = A[0][3]*B[0][0] + A[1][3]*B[0][1] + A[2][3]*B[0][2] + A[3][3]*B[0][3];

    RESULT[1][0] = A[0][0]*B[1][0] + A[1][0]*B[1][1] + A[2][0]*B[1][2] + A[3][0]*B[1][3];
    RESULT[1][1] = A[0][1]*B[1][0] + A[1][1]*B[1][1] + A[2][1]*B[1][2] + A[3][1]*B[1][3];
    RESULT[1][2] = A[0][2]*B[1][0] + A[1][2]*B[1][1] + A[2][2]*B[1][2] + A[3][2]*B[1][3];
    RESULT[1][3] = A[0][3]*B[1][0] + A[1][3]*B[1][1] + A[2][3]*B[1][2] + A[3][3]*B[1][3];

    RESULT[2][0] = A[0][0]*B[2][0] + A[1][0]*B[2][1] + A[2][0]*B[2][2] + A[3][0]*B[2][3];
    RESULT[2][1] = A[0][1]*B[2][0] + A[1][1]*B[2][1] + A[2][1]*B[2][2] + A[3][1]*B[2][3];
    RESULT[2][2] = A[0][2]*B[2][0] + A[1][2]*B[2][1] + A[2][2]*B[2][2] + A[3][2]*B[2][3];
    RESULT[2][3] = A[0][3]*B[2][0] + A[1][3]*B[2][1] + A[2][3]*B[2][2] + A[3][3]*B[2][3];

    RESULT[3][0] = A[0][0]*B[3][0] + A[1][0]*B[3][1] + A[2][0]*B[3][2] + A[3][0]*B[3][3];
    RESULT[3][1] = A[0][1]*B[3][0] + A[1][1]*B[3][1] + A[2][1]*B[3][2] + A[3][1]*B[3][3];
    RESULT[3][2] = A[0][2]*B[3][0] + A[1][2]*B[3][1] + A[2][2]*B[3][2] + A[3][2]*B[3][3];
    RESULT[3][3] = A[0][3]*B[3][0] + A[1][3]*B[3][1] + A[2][3]*B[3][2] + A[3][3]*B[3][3];
  }

  template<class T>
  Mat4D<T> &Mat4D<T>::invert(){
    T &m00 = ((Mat4D<T>&)*this)[0][0];    T &m10 = ((Mat4D<T>&)*this)[1][0];    T &m20 = ((Mat4D<T>&)*this)[2][0];    T &m30 = ((Mat4D<T>&)*this)[3][0];
    T &m01 = ((Mat4D<T>&)*this)[0][1];    T &m11 = ((Mat4D<T>&)*this)[1][1];    T &m21 = ((Mat4D<T>&)*this)[2][1];    T &m31 = ((Mat4D<T>&)*this)[3][1];
    T &m02 = ((Mat4D<T>&)*this)[0][2];    T &m12 = ((Mat4D<T>&)*this)[1][2];    T &m22 = ((Mat4D<T>&)*this)[2][2];    T &m32 = ((Mat4D<T>&)*this)[3][2];
    T &m03 = ((Mat4D<T>&)*this)[0][3];    T &m13 = ((Mat4D<T>&)*this)[1][3];    T &m23 = ((Mat4D<T>&)*this)[2][3];    T &m33 = ((Mat4D<T>&)*this)[3][3];

    m00 = m12*m23*m31 - m13*m22*m31 + m13*m21*m32 - m11*m23*m32 - m12*m21*m33 + m11*m22*m33;
    m01 = m03*m22*m31 - m02*m23*m31 - m03*m21*m32 + m01*m23*m32 + m02*m21*m33 - m01*m22*m33;
    m02 = m02*m13*m31 - m03*m12*m31 + m03*m11*m32 - m01*m13*m32 - m02*m11*m33 + m01*m12*m33;
    m03 = m03*m12*m21 - m02*m13*m21 - m03*m11*m22 + m01*m13*m22 + m02*m11*m23 - m01*m12*m23;
    m10 = m13*m22*m30 - m12*m23*m30 - m13*m20*m32 + m10*m23*m32 + m12*m20*m33 - m10*m22*m33;
    m11 = m02*m23*m30 - m03*m22*m30 + m03*m20*m32 - m00*m23*m32 - m02*m20*m33 + m00*m22*m33;
    m12 = m03*m12*m30 - m02*m13*m30 - m03*m10*m32 + m00*m13*m32 + m02*m10*m33 - m00*m12*m33;
    m13 = m02*m13*m20 - m03*m12*m20 + m03*m10*m22 - m00*m13*m22 - m02*m10*m23 + m00*m12*m23;
    m20 = m11*m23*m30 - m13*m21*m30 + m13*m20*m31 - m10*m23*m31 - m11*m20*m33 + m10*m21*m33;
    m21 = m03*m21*m30 - m01*m23*m30 - m03*m20*m31 + m00*m23*m31 + m01*m20*m33 - m00*m21*m33;
    m22 = m01*m13*m30 - m03*m11*m30 + m03*m10*m31 - m00*m13*m31 - m01*m10*m33 + m00*m11*m33;
    m23 = m03*m11*m20 - m01*m13*m20 - m03*m10*m21 + m00*m13*m21 + m01*m10*m23 - m00*m11*m23;
    m30 = m12*m21*m30 - m11*m22*m30 - m12*m20*m31 + m10*m22*m31 + m11*m20*m32 - m10*m21*m32;
    m31 = m01*m22*m30 - m02*m21*m30 + m02*m20*m31 - m00*m22*m31 - m01*m20*m32 + m00*m21*m32;
    m32 = m02*m11*m30 - m01*m12*m30 - m02*m10*m31 + m00*m12*m31 + m01*m10*m32 - m00*m11*m32;
    m33 = m01*m12*m20 - m02*m11*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 + m00*m11*m22;
    
    ((Mat4D<T>&)*this)/=det();
    
    return *this;
  }
  
  
  template<class T>
  T Mat4D<T>::det() const{
    const T &m00 = (*this)[0][0];    const T &m10 = (*this)[1][0];    const T &m20 = (*this)[2][0];    const T &m30 = (*this)[3][0];
    const T &m01 = (*this)[0][1];    const T &m11 = (*this)[1][1];    const T &m21 = (*this)[2][1];    const T &m31 = (*this)[3][1];
    const T &m02 = (*this)[0][2];    const T &m12 = (*this)[1][2];    const T &m22 = (*this)[2][2];    const T &m32 = (*this)[3][2];
    const T &m03 = (*this)[0][3];    const T &m13 = (*this)[1][3];    const T &m23 = (*this)[2][3];    const T &m33 = (*this)[3][3];
    
    return 
    m03 * m12 * m21 * m30-m02 * m13 * m21 * m30-m03 * m11 * m22 * m30+m01 * m13    * m22 * m30+
    m02 * m11 * m23 * m30-m01 * m12 * m23 * m30-m03 * m12 * m20 * m31+m02 * m13    * m20 * m31+
    m03 * m10 * m22 * m31-m00 * m13 * m22 * m31-m02 * m10 * m23 * m31+m00 * m12    * m23 * m31+
    m03 * m11 * m20 * m32-m01 * m13 * m20 * m32-m03 * m10 * m21 * m32+m00 * m13    * m21 * m32+
    m01 * m10 * m23 * m32-m00 * m11 * m23 * m32-m02 * m11 * m20 * m33+m01 * m12    * m20 * m33+
    m02 * m10 * m21 * m33-m00 * m12 * m21 * m33-m01 * m10 * m22 * m33+m00 * m11    * m22 * m33;
  }
  
  
  using std::string;
  
  template<class T>
  void Mat4D<T>::show(const string &title) const{
    printf("%s: \n",title.c_str());
    printf("%4.4f %4.4f %4.4f %4.4f \n",cols[0][0],cols[1][0],cols[2][0],cols[3][0]);
    printf("%4.4f %4.4f %4.4f %4.4f \n",cols[0][1],cols[1][1],cols[2][1],cols[3][1]);
    printf("%4.4f %4.4f %4.4f %4.4f \n",cols[0][2],cols[1][2],cols[2][2],cols[3][2]);
    printf("%4.4f %4.4f %4.4f %4.4f \n",cols[0][3],cols[1][3],cols[2][3],cols[3][3]);
  }
  template<>
  void Mat4D<icl32s>::show(const string &title) const{
    printf("%s: \n",title.c_str());
    printf("%8d %8d %8d %8d \n",cols[0][0],cols[1][0],cols[2][0],cols[3][0]);
    printf("%8d %8d %8d %8d \n",cols[0][1],cols[1][1],cols[2][1],cols[3][1]);
    printf("%8d %8d %8d %8d \n",cols[0][2],cols[1][2],cols[2][2],cols[3][2]);
    printf("%8d %8d %8d %8d \n",cols[0][3],cols[1][3],cols[2][3],cols[3][3]);
  }

  
  template class Mat4D<icl32f>;
  template class Mat4D<icl64f>;
  template class Mat4D<icl32s>;

}
