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

  template<class T, IppStatus (*ippFunc)(const T*,int,int,int,T*,T*)>
  T apply_dyn_matrix_det(const DynMatrix<T> &s){
    if(s.cols() != s.rows()){
      throw InvalidMatrixDimensionException("matrix determinant can only be calculated on square matrices");
    }
    unsigned int wh = s.cols();
    std::vector<T> buffer(wh*wh+wh);
    
    T det(0);
    IppStatus st = ippFunc(s.data(),wh*sizeof(T),sizeof(T),
                           wh,buffer.data(),&det);
    if(st != ippStsNoErr){
      ERROR_LOG("matrix determinant could not be calculated");
    }
    return det;
  }


  
  template<class T>
  DynMatrix<T> DynMatrix<T>::inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){
    ERROR_LOG("not implemented (only with IPP and only for float and double)");
    return DynMatrix<T>(1,1);
  }

  template<class T>
  T DynMatrix<T>::det() const throw (InvalidMatrixDimensionException){
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
  template<> float DynMatrix<float>::det() const throw (InvalidMatrixDimensionException){
    return apply_dyn_matrix_det<float,ippmDet_m_32f>(*this);
  }
  template<> double DynMatrix<double>::det() const throw (InvalidMatrixDimensionException){
    return apply_dyn_matrix_det<double,ippmDet_m_64f>(*this);
  }
#endif
  
  template DynMatrix<float> DynMatrix<float>::inv()const throw (InvalidMatrixDimensionException,SingularMatrixException);
  template DynMatrix<double> DynMatrix<double>::inv()const throw (InvalidMatrixDimensionException,SingularMatrixException);
  template float DynMatrix<float>::det()const throw (InvalidMatrixDimensionException);
  template double DynMatrix<double>::det()const throw (InvalidMatrixDimensionException);

}
