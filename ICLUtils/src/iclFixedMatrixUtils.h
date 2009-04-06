#ifndef ICL_FIXED_MATRIX_UTILS_H
#define ICL_FIXED_MATRIX_UTILS_H

#include <iclFixedMatrix.h>
#include <iclFixedVector.h>

#ifdef HAVE_IPP
#include <ippm.h>
#endif


namespace icl{

  template<class T, unsigned int WIDTH, unsigned int HEIGHT>
  void decompose_QR(FixedMatrix<T,WIDTH,HEIGHT> A, FixedMatrix<T,WIDTH,HEIGHT> &Q, FixedMatrix<T,WIDTH,WIDTH> &R){
    FixedColVector<T,HEIGHT> a,q;
    
    R = T(0.0);
    
    for (unsigned int i = 0; i < WIDTH; i++) {
      a = A.col(i);
      R(i,i)  = a.length();
      if(!R(i,i)) throw QRDecompException("Error in QR-decomposition");
      q = a/R(i,i);		// Normalization.
     
      Q.col(i) = q;
      
      // remove vector components parallel to q(*,i)
      for (unsigned int j = i+1; j < WIDTH; j++) {
        a = A.col(j);
        R(i,j) = (q.transp()*a)[0];
        a = a - q * R(i,j);
        A.col(j) = a;
      }
    }
    R = R.transp();
  }
  
  template<class T,unsigned  int WIDTH,unsigned  int HEIGHT>
  FixedMatrix<T,HEIGHT,WIDTH> pinv(const FixedMatrix<T,WIDTH,HEIGHT> &M){
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
}

#endif
