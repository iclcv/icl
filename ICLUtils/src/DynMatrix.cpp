/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/DynMatrix.cpp                             **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/Macros.h>
#include <stdint.h>
#include <complex>


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
#endif
  
  template<class T>
  static double dot(const DynMatrix<T> &a, const DynMatrix<T> &b){
    ICLASSERT_RETURN_VAL(a.dim() == b.dim(),0.0);
    double s = 0;
    for(unsigned int i=0;i<a.dim();++i){
      s += a[i] * b[i];
    }
    return s;
  }



  /// strikes out certain row and column -> optimization: Use a boolean array for that
  template<class T>
  static void get_minor_matrix(const DynMatrix<T> &M,int col, int row, DynMatrix<T> &D){
    /// we assert M is squared here, and D has size M.size()-Size(1,1)
    
    int nextCol=0,nextRow=0;
    const unsigned int dim = M.cols();
    for(unsigned int i=0;i<dim;++i){  
      if((int)i!=row){  
        nextCol = 0;
        for(unsigned int j=0;j<dim;j++){  
          if((int)j!=col){
            D(nextCol++,nextRow) = M(j,i);
          }  
        }  
        nextRow++;
      }  
    }  
  }

  
  template<class T>
  DynMatrix<T> DynMatrix<T>::inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){
    double detVal = det();
    if(!detVal) throw SingularMatrixException("Determinant was 0 -> (matrix is singular to machine precision)");
    detVal = 1.0/detVal;

    DynMatrix M(cols()-1,cols()-1),I(cols(),cols());
  
    for(unsigned int i=0;i<cols();i++){      
      for(unsigned int j=0;j<cols();j++){  
        get_minor_matrix(*this,i,j,M);
        I(j,i) = detVal * M.det();
        if((i+j)%2){  
          I(j,i) *= -1;
        }  
      }  
    }
    return I;
  }  
  
  template<class T>
  T DynMatrix<T>::det() const throw (InvalidMatrixDimensionException){
    unsigned int order = cols();
    if(order != rows()) throw(InvalidMatrixDimensionException("Determinant can only be calculated on squared matrices"));

    switch(order){
      case 0: throw(InvalidMatrixDimensionException("Matrix order must be > 0"));
      case 1: return *m_data;
      case 2: return m_data[0]*m_data[3]-m_data[1]*m_data[2];
      case 3: {
        const T *src = m_data;
        const T &a = *src++; const T &b = *src++; const T &c = *src++;
        const T &d = *src++; const T &e = *src++; const T &f = *src++;
        const T &g = *src++; const T &h = *src++; const T &i = *src++;
        return ( a*e*i + b*f*g + c*d*h ) - ( g*e*c + h*f*a + i*d*b);
      }
      case 4: {
        const T *src = m_data;
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
      default:{  
        // the determinant value  
        T det = 0;  
        DynMatrix<T> D(order-1,order-1);
        for(unsigned int i=0;i<order;++i){
          get_minor_matrix(*this,i,0,D);
          det += ::pow(-1.0,i) * (*this)(i,0) * D.det();
        }
        return det;
      }
    }
  }

  template<class T>
  void DynMatrix<T>::decompose_QR(DynMatrix<T> &Q, DynMatrix<T> &R) const throw (QRDecompException){
    
    DynMatrix<T> A = *this; // Working copy
    DynMatrix<T> a(1,rows()), q(1,rows());
    
    Q.setBounds(cols(),rows());
    R.setBounds(cols(),cols());
    
    std::fill(R.begin(),R.end(),0.0);

    for (unsigned int i=0;i<cols();i++) {
      a = A.col(i);
      R(i,i) = a.norm();
      if(!R(i,i)) throw QRDecompException("Error in QR-decomposition");
      q = a/R(i,i);   // Normalization.
     
      Q.col(i) = q;
      // remove components parallel to q(*,i)
      for (unsigned int j=i+1;j<cols();j++) {
        a = A.col(j);
        R(j,i) = dot(q, a);
        A.col(j) = a - q * R(j,i);
      }
    }
  }
  
  template<class T> 
  DynMatrix<T> DynMatrix<T>::pinv() const throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException){
    DynMatrix<T> Q(1,1),R(1,1);
    if(cols() > rows()){
      transp().decompose_QR(Q,R);
      return (R.inv() * Q.transp()).transp();
    }else{
      decompose_QR(Q,R);
      return R.inv() * Q.transp();
    }
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


  template<class T>
  static inline std::ostream &icl_to_stream(std::ostream &s, T t){
    return s << t;
  }

  template<class T>
  static inline std::istream &icl_from_stream(std::istream &s, T &t){
    return s >> t;
  }
  
  template<> inline std::ostream &icl_to_stream(std::ostream &s, uint8_t t){
    return s << (int)t;
  }
  
  template<> inline std::istream &icl_from_stream(std::istream &s, uint8_t &t){
    int tmp;
    s >> tmp;
    t = (uint8_t)tmp;
    return s;
  }


  template<class T>
  std::ostream &operator<<(std::ostream &s,const DynMatrix<T> &m){
    for(unsigned int i=0;i<m.rows();++i){
      s << "| ";
      for(unsigned int j=0;j<m.cols();++j){
        icl_to_stream<T>(s,m(j,i)) << " ";
      }
      s << "|";
      if(i<m.rows()-1){
        s << std::endl;
      }
    }
    return s;
  }

  template<class T>
  std::istream &operator>>(std::istream &s,DynMatrix<T> &m){
    char c;
    for(unsigned int i=0;i<m.rows();++i){
      s >> c; // trailing '|'
      if ( ((c >= '0') && (c <= '9')) || c=='-' ){
        s.unget();
      }
      for(unsigned int j=0;j<m.cols();++j){
        icl_from_stream<T>(s,m(j,i));
        s >> c;
        if( c != ',') s.unget();
      }
      s >> c; // ending '|'
      if ( ((c >= '0') && (c <= '9')) || c=='-' ){
        s.unget();
      }
    }
    return s;
  }

#define X(T)                                                            \
  template std::ostream &operator<<(std::ostream&,const DynMatrix<T >&); \
  template std::istream &operator>>(std::istream&,DynMatrix<T >&)
  
  X(uint8_t);
  X(int16_t);
  X(int32_t);
  X(float);
  X(double);
  
  X(std::complex<float>); 
  X(std::complex<double>); 
}

