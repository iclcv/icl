#ifndef ICL_FIXED_MATRIX_UTILS_H
#define ICL_FIXED_MATRIX_UTILS_H

#include <iclFixedMatrix.h>

#ifdef HAVE_IPP
#include <ippm.h>
#endif


namespace icl{
  
  template<class T, int WIDTH, int HEIGHT, IppStatus (*QRDecomp)(const T*,int,int,T*,T*,int,int,int,int)>
  FixedMatrix<T, HEIGHT,WIDTH> pseudo_inverse_util(const FixedMatrix<T,WIDTH,HEIGHT> &src){
    T buf[HEIGHT];
    FixedMatrix<T,WIDTH,HEIGHT> QRres;
    IppStatus status = QRDecomp(src.begin(),  WIDTH*sizeof(T),sizeof(T),buf,
                                QRres.begin(),WIDTH*sizeof(T),sizeof(T),
                                WIDTH,HEIGHT);
    if(status != ippStsNoErr){
      throw ICLException("Error in QR-Decomposition" + str(ippGetStatusString(status)));
    }                                      
    
    FixedMatrix<T,WIDTH,WIDTH> R(0.0);
    for(int i=0;i<WIDTH;++i){
      for(int j=i;j<WIDTH;++j){
        R(i,j) = QRres(i,j);
      }
    }
    FixedMatrix<T,WIDTH,HEIGHT> &HH = QRres;
    for(int i=0;i<WIDTH;++i){
      for(int j=i;j<WIDTH;++j){
        HH(i,j) = (j==i);
      }
    }
    FixedMatrix<T,HEIGHT,HEIGHT> ID = FixedMatrix<T,HEIGHT,HEIGHT>::id();
    FixedMatrix<T,HEIGHT,HEIGHT> Q = ID;
    for(int i=0;i<WIDTH;++i){
      FixedColVector<T,HEIGHT> h = HH.col(i);
      T factor = 0;
      for(int j=0;j<HEIGHT;++j) factor+=h[j]*h[j];
      Q = Q*(ID-(h*h.transp())/(factor*0.5));
    }
    FixedMatrix<T,WIDTH,HEIGHT> Qred;
      for(int i=0;i<WIDTH;++i) Qred.col(i) = Q.col(i);
      return R.inv() * Qred.transp();
  }
  
  template<class T,int WIDTH,int HEIGHT> struct PseudoInverseUtilClass{
    static FixedMatrix<T,HEIGHT,WIDTH> apply(const FixedMatrix<T,WIDTH,HEIGHT> &src){
      throw ICLException("this function is only implemented with IPP support and only for icl32f and icl64f");
      return FixedMatrix<T,HEIGHT,WIDTH>(0.0);
    }
  };
  
#ifdef HAVE_IPP
  template<int WIDTH, int HEIGHT> struct PseudoInverseUtilClass<icl32f,WIDTH,HEIGHT>{
    static FixedMatrix<icl32f,HEIGHT,WIDTH> apply(const FixedMatrix<icl32f,WIDTH,HEIGHT> &src){
      return pseudo_inverse_util<icl32f,WIDTH,HEIGHT,ippmQRDecomp_m_32f>(src);
    }
  };

  template<int WIDTH, int HEIGHT> struct PseudoInverseUtilClass<icl64f,WIDTH,HEIGHT>{
    static FixedMatrix<icl64f,HEIGHT,WIDTH> apply(const FixedMatrix<icl64f,WIDTH,HEIGHT> &src){
      return pseudo_inverse_util<icl64f,WIDTH,HEIGHT,ippmQRDecomp_m_64f>(src);
    }
  };
#endif
  
  template<class T, int WIDTH, int HEIGHT>
  FixedMatrix<T,HEIGHT,WIDTH> pseudo_inverse(const FixedMatrix<T,WIDTH,HEIGHT> &src){
    return PseudoInverseUtilClass<T,WIDTH,HEIGHT>::apply(src);
  }
}

#endif
