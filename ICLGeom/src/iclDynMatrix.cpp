#include <iclDynMatrix.h>
#include <iclMacros.h>
#include <iclTypes.h>

#ifdef WITH_IPP_OPTIMIZATION
#include <ippm.h>
#endif


namespace icl{

  template<class T, IppStatus (*ippFunc)(const T*,int,int,T*,T*,int,int,int)>
  DynMatrix<T> apply_dyn_matrix_inv(const DynMatrix<T> &s){
    if(s.cols() != s.rows()){
      throw InvalidMatrixDimensionException("inverse matrix can only be calculated on square matrices");
    }
    unsigned int wh = s.cols();
    DynMatrix<T> d(wh,wh);
    std::vector<T> buffer(wh*wh+wh);
    
    IppStatus st = ippFunc(s.data(),wh*sizeof(T),sizeof(T),
                           buffer.data(),
                           d.data(),wh*sizeof(T),sizeof(T),
                           wh);
    if(st != ippStsNoErr){
      throw SingularMatrixException("matrix is too singular");
    }
    return d;
  }
  
  template<class T>
  DynMatrix<T> DynMatrix<T>::inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){
    ERROR_LOG("not implemented (only with IPP and only for icl32f and icl64f)");
    return DynMatrix<T>(1,1);
  }
#ifdef WITH_IPP_OPTIMIZATION
  template<> DynMatrix<icl32f> DynMatrix<icl32f>::inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){
    return apply_dyn_matrix_inv<icl32f,ippmInvert_m_32f>(*this);
  }
  template<> DynMatrix<icl64f> DynMatrix<icl64f>::inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){
    return apply_dyn_matrix_inv<icl64f,ippmInvert_m_64f>(*this);
  }
#endif
  
#define ICL_INSTANTIATE_DEPTH(D) \
  template DynMatrix<icl##D> DynMatrix<icl##D>::inv()const throw (InvalidMatrixDimensionException,SingularMatrixException);
  ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH

}
