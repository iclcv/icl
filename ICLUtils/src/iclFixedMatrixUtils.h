#ifndef ICL_FIXED_MATRIX_UTILS_H
#define ICL_FIXED_MATRIX_UTILS_H

#include <iclFixedMatrix.h>
#include <iclFixedVector.h>

namespace icl{
  
  /// computes inner product of two matrices (element-wise scalar product) \ingroup LINALG
  template<class T, unsigned int WIDTH, unsigned int HEIGHT, unsigned int WIDTH2>
  inline T inner_vector_product(const FixedMatrix<T,WIDTH,HEIGHT> &a, const FixedMatrix<T,WIDTH2,(WIDTH*HEIGHT)/WIDTH2> &b){
    return std::inner_product(a.begin(),a.end(),b.begin(),T(0));
  }

  /// computes the QR decomposition of a matrix \ingroup LINALG
  template<class T, unsigned int WIDTH, unsigned int HEIGHT>
  inline void decompose_QR(FixedMatrix<T,WIDTH,HEIGHT> A, FixedMatrix<T,WIDTH,HEIGHT> &Q, FixedMatrix<T,WIDTH,WIDTH> &R){
    FixedColVector<T,HEIGHT> a,q;
    R = T(0.0);
    for (unsigned int i=0;i<WIDTH;i++) {
      a = A.col(i);
      R(i,i) = a.length();
      if(!R(i,i)) throw QRDecompException("Error in QR-decomposition");
      q = a/R(i,i);   // Normalization.
      Q.col(i) = q;
      // remove components parallel to q(*,i)
      for (unsigned int j=i+1;j<WIDTH;j++) {
        a = A.col(j);
        R(j,i) = inner_vector_product(q, a);
        A.col(j) = a - q * R(j,i);
      }
    }
  }
  
  /// computes the pseudo-inverse of a matrix (using QR-decomposition based approach) \ingroup LINALG
  template<class T,unsigned  int WIDTH,unsigned  int HEIGHT>
  inline FixedMatrix<T,HEIGHT,WIDTH> pinv(const FixedMatrix<T,WIDTH,HEIGHT> &M){
    if(HEIGHT < WIDTH){
      FixedMatrix<T,HEIGHT,WIDTH> Q;
      FixedMatrix<T,HEIGHT,HEIGHT> R;
      decompose_QR(M.transp(),Q,R);
      return (R.inv() * Q.transp()).transp();
    }else{
      FixedMatrix<T,WIDTH,HEIGHT> Q;
      FixedMatrix<T,WIDTH,WIDTH> R;
      decompose_QR(M,Q,R);
      return R.inv() * Q.transp();
    }
  }

  /// Vertical Matrix concatenation  \ingroup LINALG
  /** like ICLQuick image concatenation, dont forget the brackets sometimes */
  template<class T,unsigned  int WIDTH,unsigned  int HEIGHT, unsigned int HEIGHT2>
  inline FixedMatrix<T,WIDTH,HEIGHT+HEIGHT2> operator%(const FixedMatrix<T,WIDTH,HEIGHT> &a,
                                                       const FixedMatrix<T,WIDTH,HEIGHT2> &b){
    FixedMatrix<T,WIDTH,HEIGHT+HEIGHT2> M;
    for(unsigned int i=0;i<HEIGHT;++i) M.row(i) = a.row(i);
    for(unsigned int i=0;i<HEIGHT2;++i) M.row(i+HEIGHT) = b.row(i);
    return M;
  }

  /// Horizontal Matrix concatenation  \ingroup LINALG
  /** like ICLQuick image concatenation, dont forget the brackets sometimes */
  template<class T,unsigned  int WIDTH,unsigned  int HEIGHT, unsigned int WIDTH2>
  inline FixedMatrix<T,WIDTH+WIDTH2,HEIGHT> operator,(const FixedMatrix<T,WIDTH,HEIGHT> &a,
                                                       const FixedMatrix<T,WIDTH2,HEIGHT> &b){
    FixedMatrix<T,WIDTH+WIDTH2,HEIGHT> M;
    for(unsigned int i=0;i<WIDTH;++i) M.col(i) = a.col(i);
    for(unsigned int i=0;i<WIDTH2;++i) M.col(i+WIDTH) = b.col(i);
    return M;
  }

  
}



#endif
