#include <iclDynMatrix.h>
#include <iclMacros.h>

#ifdef HAVE_IPP
#include <ippm.h>
#endif


namespace icl{

#ifdef HAVE_IPP
 template<class T, IppStatus (*ippFunc)(const T*,int,int,T*,T*,int,int,int)>                                                               
 static DynMatrix<T> apply_dyn_matrix_inv(const DynMatrix<T> &s){                                                                                 
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
  static T apply_dyn_matrix_det(const DynMatrix<T> &s){
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
  static double dot(const DynMatrix<T> &a, const DynMatrix<T> &b){
    ICLASSERT_RETURN_VAL(a.dim() == b.dim(),0.0);
    double s = 0;
    for(int i=0;i<a.dim();++i){
      s += a[i] * b[i];
    }
    return s;
  }


  template<class T>
  static void apply_dyn_matrix_decompose_QR(const DynMatrix<T> &A, DynMatrix<T> &Q, DynMatrix<T> &R){
    int rows = A.rows();
    int cols = A.cols();
    DynMatrix<T> a(1,rows), q(1,rows);	// For storing column matrices.
    
    Q.setBounds(cols,rows);
    R.setBounds(cols,cols);
    
    std::fill(R.begin(),R.end(),0.0);
    
    for (int i = 0; i < cols; i++) {
      std::copy(A.col_begin(i),A.col_end(i),a.begin());
      R(i,i)  = a.norm();
      if(!R(i,i)) throw QRDecompException("Error in QR-decomposition");
      q = a/R(i,i);		// Normalization.
     
      std::copy(q.begin(),q.end(),Q.col_begin(i));
      
      // remove vector components parallel to q(*,i)
      for (int j = i+1; j < cols; j++) {
        std::copy(A.col_begin(j),A.col_end(j),a.begin());
        R(i,j) = dot(q, a);
        a = a - q * R(i,j);
        std::copy(a.begin(),a.end(),A.col_begin(j));
      }
    }
    R = R.transp();
  }


#endif

  
  template<class T>
  DynMatrix<T> DynMatrix<T>::inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){
    ERROR_LOG("not implemented (only with IPP and only for float and double)");
    return DynMatrix<T>(1,1);
  }

  template<class T>
  T DynMatrix<T>::det() const throw (InvalidMatrixDimensionException){
    ERROR_LOG("not implemented (only with IPP and only for float and double)");
    return T();
  }


  template<class T>
  void DynMatrix<T>::decompose_QR(DynMatrix<T> &Q, DynMatrix<T> &R) const 
    throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException){
    ERROR_LOG("not implemented (only with IPP and only for float and double)");
  }
  
  template<class T> 
  DynMatrix<T> DynMatrix<T>::pinv() const throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException){
    ERROR_LOG("not implemented (only with IPP and only for float and double)");
    return DynMatrix<T>();
  }
  



#ifdef HAVE_IPP
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

  template<> void DynMatrix<float>::decompose_QR(DynMatrix &Q, DynMatrix &R) const 
    throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException){
    return apply_dyn_matrix_decompose_QR<float>(*this,Q,R);
  }
  template<> void DynMatrix<double>::decompose_QR(DynMatrix &Q, DynMatrix &R) const 
    throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException){
    return apply_dyn_matrix_decompose_QR<double>(*this,Q,R);
  }

  template<> DynMatrix<double> DynMatrix<double>::pinv() const throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException){
    DynMatrix<double> Q(1,1),R(1,1);
    if(rows() < cols()){
      transp().decompose_QR(Q,R);
      return (R.inv() * Q.transp()).transp();
    }else{
      decompose_QR(Q,R);
      return R.inv() * Q.transp();
    }
  }
  template<> DynMatrix<float> DynMatrix<float>::pinv() const throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException){
    DynMatrix<float> Q(1,1),R(1,1);
    if(rows() > cols()){
      DynMatrix<float> T = transp();
      decompose_QR(Q,R);
      return (R.inv() * Q.transp()).transp();
    }else{
      decompose_QR(Q,R);
      return R.inv() * Q.transp();
    }
  }
  

#endif
  
  template DynMatrix<float> DynMatrix<float>::inv()const throw (InvalidMatrixDimensionException,SingularMatrixException);
  template DynMatrix<double> DynMatrix<double>::inv()const throw (InvalidMatrixDimensionException,SingularMatrixException);
  template float DynMatrix<float>::det()const throw (InvalidMatrixDimensionException);
  template double DynMatrix<double>::det()const throw (InvalidMatrixDimensionException);

  template void DynMatrix<float>::decompose_QR(DynMatrix<float> &Q, DynMatrix<float> &R) const 
    throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException);
  template void DynMatrix<double>::decompose_QR(DynMatrix<double> &Q, DynMatrix<double> &R) const 
    throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException);
    
  template DynMatrix<float> DynMatrix<float>::pinv() const 
    throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException);
  template DynMatrix<double> DynMatrix<double>::pinv() const 
    throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException);
}

/** C++ fallback hint TODO...
# // matrix inversioon  
# // the result is put in Y  
# void MatrixInversion(float **A, int order, float **Y)  
# {  
#     // get the determinant of a  
#     double det = 1.0/CalcDeterminant(A,order);  
#   
#     // memory allocation  
#     float *temp = new float[(order-1)*(order-1)];  
#     float **minor = new float*[order-1];  
#     for(int i=0;i<order-1;i++)  
#         minor[i] = temp+(i*(order-1));  
#   
#     for(int j=0;j<order;j++)  
#     {  
#         for(int i=0;i<order;i++)  
#         {  
#             // get the co-factor (matrix) of A(j,i)  
#             GetMinor(A,minor,j,i,order);  
#             Y[i][j] = det*CalcDeterminant(minor,order-1);  
#             if( (i+j)%2 == 1)  
#                 Y[i][j] = -Y[i][j];  
#         }  
#     }  
#   
#     // release memory  
#     delete [] minor[0];  
#     delete [] minor;  
# }  
#   
# // calculate the cofactor of element (row,col)  
# int GetMinor(float **src, float **dest, int row, int col, int order)  
# {  
#     // indicate which col and row is being copied to dest  
#     int colCount=0,rowCount=0;  
#   
#     for(int i = 0; i < order; i++ )  
#     {  
#         if( i != row )  
#         {  
#             colCount = 0;  
#             for(int j = 0; j < order; j++ )  
#             {  
#                 // when j is not the element  
#                 if( j != col )  
#                 {  
#                     dest[rowCount][colCount] = src[i][j];  
#                     colCount++;  
#                 }  
#             }  
#             rowCount++;  
#         }  
#     }  
#   
#     return 1;  
# }  

# // Calculate the determinant recursively.  
# double CalcDeterminant( float **mat, int order)  
# {  
#     // order must be >= 0  
#     // stop the recursion when matrix is a single element  
#     if( order == 1 )  
#         return mat[0][0];  
#   
#     // the determinant value  
#     float det = 0;  
#   
#     // allocate the cofactor matrix  
#     float **minor;  
#     minor = new float*[order-1];  
#     for(int i=0;i<order-1;i++)  
#         minor[i] = new float[order-1];  
#   
#     for(int i = 0; i < order; i++ )  
#     {  
#         // get minor of element (0,i)  
#         GetMinor( mat, minor, 0, i , order);  
#         // the recusion is here!  
#         det += pow( -1.0, i ) * mat[0][i] * CalcDeterminant( minor,order-1 );  
#     }  
#   
#     // release memory  
#     for(int i=0;i<order-1;i++)  
#         delete [] minor[i];  
#     delete [] minor;  
#   
#     return det;  
# }  


*/
