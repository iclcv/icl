#include <iclDynMatrix.h>
#include <iclMacros.h>

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
    ERROR_LOG("not implemented (only with IPP and only for float and double)");
    return DynMatrix<T>(1,1);
  }
#ifdef WITH_IPP_OPTIMIZATION
  template<> DynMatrix<float> DynMatrix<float>::inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){
    return apply_dyn_matrix_inv<float,ippmInvert_m_32f>(*this);
  }
  template<> DynMatrix<double> DynMatrix<double>::inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){
    return apply_dyn_matrix_inv<double,ippmInvert_m_64f>(*this);
  }
#endif
  
  template DynMatrix<float> DynMatrix<float>::inv()const throw (InvalidMatrixDimensionException,SingularMatrixException);
  template DynMatrix<double> DynMatrix<double>::inv()const throw (InvalidMatrixDimensionException,SingularMatrixException);

}
