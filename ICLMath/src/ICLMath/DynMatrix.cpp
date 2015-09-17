/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/DynMatrix.cpp                      **
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

#include <ICLMath/DynMatrix.h>
#include <stdint.h>
#include <complex>
#include <algorithm>
#include <fstream>

// Intel Math Kernel Library
#ifdef ICL_HAVE_MKL
#include "mkl_types.h"
#include "mkl_lapack.h"
#endif

#include <ICLMath/DynMatrixUtils.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;

namespace icl{
  namespace math{
  
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
            det += ::pow(-1.0,(int)i) * (*this)(i,0) * D.det();
          }
          return det;
        }
      }
    }
  
    template<class T>
    void DynMatrix<T>::decompose_QR(DynMatrix<T> &Q, DynMatrix<T> &R) const 
          throw (InvalidMatrixDimensionException,SingularMatrixException) {
      DynMatrix<T> A = *this; // Working copy
      DynMatrix<T> a(1,rows()), q(1,rows());
  
      Q.setBounds(cols(),rows());
      R.setBounds(cols(),cols());
  
      std::fill(R.begin(),R.end(),0.0);
  
      for (unsigned int i=0;i<cols();i++) {
        a = A.col(i);
        R(i,i) = a.norm();
        if(!R(i,i)){
          //throw QRDecompException("Error in QR-decomposition");
          q = a;          // No Normalization in case of R(i,i)=0
        }else{
          q = a/R(i,i);   // Normalization.
        }
  
        Q.col(i) = q;
        // remove components parallel to q(*,i)
        for (unsigned int j=i+1;j<cols();j++) {
          a = A.col(j);
          R(j,i) = icl::math::dot(q, a);
          A.col(j) = a - q * R(j,i);
        }
      }
    }
  
    template<class T>
    void DynMatrix<T>::decompose_RQ(DynMatrix<T> &R, DynMatrix<T> &Q) const 
          throw (InvalidMatrixDimensionException,SingularMatrixException) {
     // first reverse the rows of A and transpose it
      DynMatrix<T> A_(rows(),cols());
      for (unsigned int i = 0; i<rows(); i++){
        for (unsigned int j = 0; j<rows(); j++){
          A_(i,j) = (*this)(j,rows()-i-1);
        }
      }
  
      // get the QR-decomposition
      DynMatrix<T> R_(rows(),rows());
      DynMatrix<T> Q_(rows(),rows());
      A_.decompose_QR(Q_,R_);
  
  
      R.setBounds(rows(),rows());
      Q.setBounds(rows(),rows());
  
      // get R by reflecting all entries on the second diagonal
      for (unsigned int i = 0; i<rows(); i++){
        for (unsigned int j = 0; j<rows(); j++){
          R(i,j) = R_(rows()-1-j,rows()-1-i);
        }
      }
  
      // get Q by transposing Q_ and reversing all rows
      for (unsigned int i = 0; i<rows(); i++){
        for (unsigned int j = 0; j<rows(); j++){
          Q(i,j) = Q_(rows()-1-j,i);
        }
      }
    }
  
  
    template<class T>
    static inline bool is_close_to_zero(const T &t){
      //std::cout << "is close to zero (" << t << ") returns " << (fabs(t) < 1E-15 ? "true" : "false") << std::endl;
      return fabs(t) < 1E-15;
    }
  
    template<class T>
    static inline int find_non_zero_in_col(const DynMatrix<T> &U, int i, int m){
      for(int j=i+1;j<m;++j){
        if(!is_close_to_zero( U(i,j) )) return j;
      }
      return -1;
    }
    template<class Iterator>
    static inline void swap_range(Iterator beginA, Iterator endA, Iterator beginB){
      for(;beginA != endA; ++beginA, ++beginB) {
        std::swap(*beginA, *beginB);
      }
    }
  
  
    template<class T>
    void DynMatrix<T>::decompose_LU(DynMatrix &L, DynMatrix &U, T zeroThreshold) const{
      const DynMatrix &A = *this;
      unsigned int m = A.rows();
      unsigned int n = A.cols();
      U = A;
      L = DynMatrix<T>(m,m);
      for(unsigned int i=0;i<m;++i) L(i,i) = 1;
      DynMatrix<T> p(1,m);
      for(unsigned int i=0;i<m;++i) p[i] = i;
  
      for(unsigned int i=0;i<m-1;++i){
        if(is_close_to_zero(U(i,i))){ // here, we need an epsilon
          int k = find_non_zero_in_col(U,i,m);
          if(k != -1){
            //swap rows i and k
            std::swap(p[i],p[k]);
            swap_range(U.row_begin(i),U.row_end(i),U.row_begin(k));
            swap_range(L.row_begin(i),L.row_begin(i)+i,L.row_begin(k));
          }
        }else{
          T pivot = U(i,i);
          for(unsigned int k=i+1;k<m;++k){
            T m = U(i,k)/pivot;
            for(unsigned int j=0;j<n;++j){
              U(j,k) += -m * U(j,i);
            }
            L(i,k) = m;
          }
        }
      }
  
      DynMatrix<T> L2 = L;
      for(unsigned int i=0;i<m;++i){
        int j = p[i];
        std::copy(L2.row_begin(i),L2.row_end(i),L.row_begin(j));
      }
    }
  
    template<class T>
    DynMatrix<T> DynMatrix<T>::solve_upper_triangular(const DynMatrix &b) const throw(InvalidMatrixDimensionException){
      const DynMatrix &M = *this;
      ICLASSERT_THROW(M.cols() == M.rows(), ICLException("solve_upper_triangular only works for squared matrices"));
      int m = M.cols();
      DynMatrix<T> x(1,m);
      for(int i=m-1;i>=0;--i){
        float r = b[i];
        for(int j=m-1;j>i;--j) r -= M(j,i) * x[j];
        x[i] = r/M(i,i);
      }
      return x;
    }
  
    template<class T>
    DynMatrix<T> DynMatrix<T>::solve_lower_triangular(const DynMatrix &b) const throw(InvalidMatrixDimensionException){
      const DynMatrix &M = *this;
      ICLASSERT_THROW(M.cols() == M.rows(), ICLException("solve_lower_triangular: only works for squared matrices"));
      int m = M.cols();
      DynMatrix<T> x(1,m);
      for(int i=0;i<m;++i){
        float r = b[i];
        for(int j=0;j<i;++j) r -= M(j,i) * x[j];
        x[i] = r/M(i,i);
      }
      return x;
    }
  
    template<class T>
    DynMatrix<T> DynMatrix<T>::solve(const DynMatrix &b, const std::string &method ,T zeroThreshold)
      throw(InvalidMatrixDimensionException,  ICLException, SingularMatrixException){
      ICLASSERT_THROW(rows() == b.rows(), InvalidMatrixDimensionException("DynMatrix::solve (Mx=b -> x=M^(-1)b needs M.rows == b.rows)"));
      if(method == "lu"){
        DynMatrix<T> L,U;
        decompose_LU(L,U);
        return U.solve_upper_triangular(L.solve_lower_triangular(b));
      }else if(method == "svd"){
        return pinv(true) * b;
      }else if(method == "qr"){
        return pinv(false) * b;
      }else if(method == "inv"){
        return inv() * b;
      }
      throw ICLException("DynMatrix::solve: invalid solve-method");
      return DynMatrix<T>(0,0);
    }
  
  
  
    template<class T>
    DynMatrix<T> DynMatrix<T>::pinv(bool useSVD, T zeroThreshold) const
      throw (InvalidMatrixDimensionException,SingularMatrixException, ICLException){
      if(useSVD){
        DynMatrix<T> U,s,V;
        try{
          svd_dyn(*this,U,s,V);
        }catch(const ICLException &){
          return pinv(false,zeroThreshold);
        }
        DynMatrix S(U.cols(), V.rows(),0.0f);
        for(unsigned int i=0;i<s.rows();++i){
          S(i,i) = (fabs(s[i]) > zeroThreshold) ? 1.0/s[i] : 0;
        }
        return V * S * U.transp();
      }else{
        DynMatrix<T> Q,R;
        if(cols() > rows()){
          transp().decompose_QR(Q,R);
          return (R.inv() * Q.transp()).transp();
        }else{
          decompose_QR(Q,R);
          return R.inv() * Q.transp();
        }
      }
    }
  
  
    // fallback
    template<class T>
    DynMatrix<T> DynMatrix<T>::big_matrix_pinv(T zeroThreshold) const
      throw (InvalidMatrixDimensionException,SingularMatrixException, ICLException){
      return pinv( true, zeroThreshold );
    }
  
  #ifdef ICL_HAVE_MKL
    template<class T>
      DynMatrix<T> DynMatrix<T>::big_matrix_pinv(T zeroThreshold, GESDD gesdd, CBLAS_GEMM cblas_gemm) const
      throw (InvalidMatrixDimensionException,SingularMatrixException, ICLException){
  
      // create a copy of source matrix, because GESDD is destroying the input matrix
      DynMatrix<T> matrixCopy( *this );
  
      // matrix dimensions
      int r = rows();
      int c = cols();
  
      // calculate SVD
      DynMatrix<T> Vt, s, U;
      Vt.setBounds( c, c );
      s.setBounds( 1, c );
      U.setBounds( c, r );
      char jobz = 'S';
  
      // success message
      int info;
  
      // work buffer of size 1 to retrieve correct buffer size from first run of GESDD
      T* work = (T*) malloc( sizeof( T ) );
      ICLASSERT_THROW( work != 0, ICLException("Insufficient memory to allocate work buffer!") );
  
      // integer work buffer
      int* iwork = (int*) malloc( sizeof( int ) * std::max( 1, 8 * std::min( c, r ) ) );
      ICLASSERT_THROW( iwork != 0, 0 );
  
      // first run of GESDD to retrieve correct size of work buffer
      int lwork  = -1;
      gesdd( &jobz, &c, &r, matrixCopy.begin(), &c, s.begin(), Vt.begin(),
             &c, U.begin(), &c, work, &lwork, iwork, &info );
      ICLASSERT_THROW( info == 0, ICLException("GESDD failed!") );
      lwork = work[0];
  
      // free old work buffer and allocate a new one
      free( work );
      work = (T*) malloc( sizeof( T ) * lwork );
      ICLASSERT_THROW( work != 0, ICLException("Insufficient memory to allocate work buffer!") );
  
      // compute singular value decomposition of a general rectangular matrix
      // using a divide and conquer method.
      gesdd( &jobz, &c, &r, matrixCopy.begin(), &c, s.begin(), Vt.begin(),
             &c, U.begin(), &c, work, &lwork, iwork, &info );
      ICLASSERT_THROW( info == 0, ICLException("GESDD failed!") );
  
      // free buffers
      free( iwork );
      free( work );
  
      // prepare matrix S and check if singular values are below zero threshold
      DynMatrix<T> S( c, c, 0.0 );
      for ( int i(0); i < c; ++i )
          S( i, i ) = ( fabs( s[i] ) > zeroThreshold ) ? 1.0 / s[i] : 0.0;
  
      // dst = Vt.transp() * S * U.transp();
      DynMatrix<T> temp( c, c );
      cblas_gemm( CblasRowMajor, CblasTrans, CblasNoTrans, c, c, c, 1.0, Vt.begin(), c,
                  S.begin(), c, 0.0, temp.begin(), c );
  
      DynMatrix<T> pseudoInverse( r, c );
      cblas_gemm( CblasRowMajor, CblasNoTrans, CblasTrans, c, r, c, 1.0, temp.begin(), c,
                  U.begin(), c, 0.0, pseudoInverse.begin(), r );
  
      return pseudoInverse;
    }
  
    template<>
    ICLMath_API DynMatrix<float> DynMatrix<float>::big_matrix_pinv(float zeroThreshold) const
      throw (InvalidMatrixDimensionException,SingularMatrixException,ICLException){
      return big_matrix_pinv(zeroThreshold,sgesdd,cblas_sgemm);
    }
    template<>
    ICLMath_API DynMatrix<double> DynMatrix<double>::big_matrix_pinv(double zeroThreshold) const
      throw (InvalidMatrixDimensionException,SingularMatrixException,ICLException){
      return big_matrix_pinv(zeroThreshold,dgesdd,cblas_dgemm);
    }
  #endif
  
  
    // This function was taken from VTK Version 5.6.0
    // Jacobi iteration for the solution of eigenvectors/eigenvalues of a nxn
    // real symmetric matrix. Square nxn matrix a; size of matrix in n;
    // output eigenvalues in w; and output eigenvectors in v. Resulting
    // eigenvalues/vectors are sorted in decreasing order; eigenvectors are
    // normalized.
    template<class T>
    int jacobi_iterate_vtk(T **a, int n, T *w, T **v){
      int i, j, k, iq, ip, numPos;
      T tresh, theta, tau, t, sm, s, h, g, c, tmp;
      T bspace[4], zspace[4];
      T *b = bspace;
      T *z = zspace;
  
      // only allocate memory if the matrix is large
      if (n > 4){
        b = new T[n];
        z = new T[n];
      }
  
      // initialize
      for (ip=0; ip<n; ip++){
        for (iq=0; iq<n; iq++){
          v[ip][iq] = 0.0;
        }
        v[ip][ip] = 1.0;
      }
      for (ip=0; ip<n; ip++){
        b[ip] = w[ip] = a[ip][ip];
        z[ip] = 0.0;
      }
  
      static const int MAX_ROTATIONS = 30;
  
      // begin rotation sequence
      for (i=0; i<MAX_ROTATIONS; i++){
        sm = 0.0;
        for (ip=0; ip<n-1; ip++){
          for (iq=ip+1; iq<n; iq++){
            sm += fabs(a[ip][iq]);
          }
        }
        if (sm == 0.0){
          break;
        }
  
        if (i < 3){                                // first 3 sweeps
          tresh = 0.2*sm/(n*n);
        }
        else{
          tresh = 0.0;
        }
        for (ip=0; ip<n-1; ip++){
          for (iq=ip+1; iq<n; iq++){
            g = 100.0*fabs(a[ip][iq]);
            // after 4 sweeps
            if (i > 3 && (fabs(w[ip])+g) == fabs(w[ip]) && (fabs(w[iq])+g) == fabs(w[iq])){
              a[ip][iq] = 0.0;
            }else if (fabs(a[ip][iq]) > tresh) {
              h = w[iq] - w[ip];
              if ( (fabs(h)+g) == fabs(h)){
                t = (a[ip][iq]) / h;
              }else {
                theta = 0.5*h / (a[ip][iq]);
                t = 1.0 / (fabs(theta)+sqrt(1.0+theta*theta));
                if (theta < 0.0){
                  t = -t;
                }
              }
              c = 1.0 / sqrt(1+t*t);
              s = t*c;
              tau = s/(1.0+c);
              h = t*a[ip][iq];
              z[ip] -= h;
              z[iq] += h;
              w[ip] -= h;
              w[iq] += h;
              a[ip][iq]=0.0;
  
  #define ROTATE(a,i,j,k,l) g=a[i][j];h=a[k][l];a[i][j]=g-s*(h+g*tau);    \
              a[k][l]=h+s*(g-h*tau)
  
              // ip already shifted left by 1 unit
              for (j = 0;j <= ip-1;j++){
                ROTATE(a,j,ip,j,iq);
              }
              // ip and iq already shifted left by 1 unit
              for (j = ip+1;j <= iq-1;j++){
                ROTATE(a,ip,j,j,iq);
              }
              // iq already shifted left by 1 unit
              for (j=iq+1; j<n; j++){
                ROTATE(a,ip,j,iq,j);
              }
              for (j=0; j<n; j++){
                ROTATE(v,j,ip,j,iq);
              }
  #undef ROTATE
            }
          }
        }
  
        for (ip=0; ip<n; ip++) {
          b[ip] += z[ip];
          w[ip] = b[ip];
          z[ip] = 0.0;
        }
      }
  
      // sort eigenfunctions                 these changes do not affect accuracy
      for (j=0; j<n-1; j++){                  // boundary incorrect
        k = j;
        tmp = w[k];
        for (i=j+1; i<n; i++){                // boundary incorrect, shifted already
          if (w[i] >= tmp){                   // why exchage if same?
            k = i;
            tmp = w[k];
          }
        }
        if (k != j) {
          w[k] = w[j];
          w[j] = tmp;
          for (i=0; i<n; i++) {
            tmp = v[i][j];
            v[i][j] = v[i][k];
            v[i][k] = tmp;
          }
        }
      }
      // insure eigenvector consistency (i.e., Jacobi can compute vectors that
      // are negative of one another (.707,.707,0) and (-.707,-.707,0). This can
      // reek havoc in hyperstreamline/other stuff. We will select the most
      // positive eigenvector.
      int ceil_half_n = (n >> 1) + (n & 1);
      for (j=0; j<n; j++){
        for (numPos=0, i=0; i<n; i++){
          if ( v[i][j] >= 0.0 ){
            numPos++;
          }
        }
        //    if ( numPos < ceil(double(n)/double(2.0)) )
      if ( numPos < ceil_half_n){
        for(i=0; i<n; i++){
          v[i][j] *= -1.0;
        }
      }
      }
      if (n > 4){
        delete [] b;
        delete [] z;
      }
      return 1;
    }
  
  
  
    template<class T>
    void find_eigenvectors(const DynMatrix<T> &a, DynMatrix<T> &eigenvectors, DynMatrix<T> &eigenvalues, T *buffer = 0){
      const int n = a.cols();
      T ** pa = new T*[n], *pvalues=new T[n], **pvectors=new T*[n];
      for(int i=0;i<n;++i){
        pa[i] = new T[n];
        for(int j=0;j<n;++j){
          pa[i][j] = a(j,i); // maybe (j,i) !!
        }
        pvectors[i] = new T[n];
      }
      jacobi_iterate_vtk<T>(pa,n,pvalues,pvectors);
  
      for(int i=0;i<n;++i){
        for(int j=0;j<n;++j){
          eigenvectors(j,i) = pvectors[i][j];
        }
        eigenvalues[i] = pvalues[i];
  
        delete [] pa[i];
        delete [] pvectors[i];
      }
      delete [] pvalues;
    }
  
  #ifdef ICL_HAVE_IPP
    template<> ICLMath_API
    void find_eigenvectors(const DynMatrix<icl32f> &a, DynMatrix<icl32f> &eigenvectors, DynMatrix<icl32f> &eigenvalues, icl32f* buffer){
      const int n = a.cols();
  
      icl32f * useBuffer = buffer ? buffer : new icl32f[n*n];
      IppStatus sts = ippmEigenValuesVectorsSym_m_32f (a.begin(), n*sizeof(icl32f), sizeof(icl32f), useBuffer,
                                                       eigenvectors.begin(), n*sizeof(icl32f), sizeof(icl32f),
                                                       eigenvalues.begin(),n);
      if(!buffer) delete [] useBuffer;
  
      if(sts != ippStsNoErr){
        throw ICLException(std::string("IPP-Error in ") + __FUNCTION__ + "\"" +ippGetStatusString(sts) +"\"");
      }
    }
    template<> ICLMath_API
    void find_eigenvectors(const DynMatrix<icl64f> &a, DynMatrix<icl64f> &eigenvectors, DynMatrix<icl64f> &eigenvalues, icl64f* buffer){
      const int n = a.cols();
      icl64f * useBuffer = buffer ? buffer : new icl64f[n*n];
      IppStatus sts = ippmEigenValuesVectorsSym_m_64f (a.begin(), n*sizeof(icl64f), sizeof(icl64f), useBuffer,
                                                       eigenvectors.begin(), n*sizeof(icl64f), sizeof(icl64f),
                                                       eigenvalues.begin(),n);
      if(!buffer) delete [] useBuffer;
  
      if(sts != ippStsNoErr){
        throw ICLException(std::string("IPP-Error in ") + __FUNCTION__ + "\"" +ippGetStatusString(sts) +"\"");
      }
    }
  #endif
  
    template<class T>
    void DynMatrix<T>::eigen(DynMatrix<T> &eigenvectors, DynMatrix<T> &eigenvalues) const throw(InvalidMatrixDimensionException, ICLException){
      ICLASSERT_THROW(cols() == rows(), InvalidMatrixDimensionException("find eigenvectors: input matrix a is not a square-matrix"));
      const int n = cols();
      eigenvalues.setBounds(1,n);
      eigenvectors.setBounds(n,n);
  
      find_eigenvectors<T>(*this,eigenvectors,eigenvalues,0);
    }
  
    template<class T>
    void DynMatrix<T>::svd(DynMatrix &V, DynMatrix &s, DynMatrix &U) const throw (ICLException){
      svd_dyn<T>(*this,V,s,U);
    }
  
  
#ifdef ICL_HAVE_IPP
  #define DYN_MATRIX_INV(T, ippFunc) \
    template<> ICLMath_API DynMatrix<T> DynMatrix<T>::inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){ \
      if(this->cols() != this->rows()){ \
        throw InvalidMatrixDimensionException("inverse matrix can only be calculated on square matrices"); \
      } \
      unsigned int wh = this->cols(); \
      DynMatrix<T> d(wh, wh, 0.0); \
      std::vector<T> buffer(wh*wh + wh); \
      IppStatus st = ippFunc(this->data(), wh*sizeof(T), sizeof(T), \
                             buffer.data(), \
                             d.data(), wh*sizeof(T), sizeof(T), \
                             wh); \
      if(st != ippStsNoErr){ \
        throw SingularMatrixException("matrix is too singular"); \
      } \
      return d; \
    }
    
  #define DYN_MATRIX_DET(T, ippFunc) \
    template<> ICLMath_API T DynMatrix<T>::det() const throw (InvalidMatrixDimensionException){ \
      if(this->cols() != this->rows()){ \
        throw InvalidMatrixDimensionException("matrix determinant can only be calculated on square matrices"); \
      } \
      unsigned int wh = this->cols(); \
      std::vector<T> buffer(wh*wh+wh); \
      T det(0); \
      IppStatus st = ippFunc(this->data(),wh*sizeof(T),sizeof(T), \
                             wh,buffer.data(),&det); \
      if(st != ippStsNoErr){ \
        ERROR_LOG("matrix determinant could not be calculated"); \
      } \
      return det; \
    }
  
    DYN_MATRIX_INV(float, ippmInvert_m_32f);
    DYN_MATRIX_INV(double, ippmInvert_m_64f);
    DYN_MATRIX_DET(float, ippmDet_m_32f);
    DYN_MATRIX_DET(double, ippmDet_m_64f);

    #undef DYN_MATRIX_INV
    #undef DYN_MATRIX_DET
#else
    template ICLMath_API DynMatrix<float> DynMatrix<float>::inv()const throw (InvalidMatrixDimensionException, SingularMatrixException);
    template ICLMath_API DynMatrix<double> DynMatrix<double>::inv()const throw (InvalidMatrixDimensionException, SingularMatrixException);

    template ICLMath_API float DynMatrix<float>::det()const throw (InvalidMatrixDimensionException);
    template ICLMath_API double DynMatrix<double>::det()const throw (InvalidMatrixDimensionException);
#endif
  
  
    template ICLMath_API void DynMatrix<float>::svd(DynMatrix<float>&, DynMatrix<float>&, DynMatrix<float>&) const throw (ICLException);
    template ICLMath_API void DynMatrix<double>::svd(DynMatrix<double>&, DynMatrix<double>&, DynMatrix<double>&) const throw (ICLException);
  
    template ICLMath_API void DynMatrix<float>::eigen(DynMatrix<float>&, DynMatrix<float>&) const throw(InvalidMatrixDimensionException, ICLException);
    template ICLMath_API void DynMatrix<double>::eigen(DynMatrix<double>&, DynMatrix<double>&) const throw(InvalidMatrixDimensionException, ICLException);
  
    template ICLMath_API void DynMatrix<float>::decompose_QR(DynMatrix<float> &Q, DynMatrix<float> &R) const
      throw (InvalidMatrixDimensionException,SingularMatrixException);
    template ICLMath_API void DynMatrix<double>::decompose_QR(DynMatrix<double> &Q, DynMatrix<double> &R) const
      throw (InvalidMatrixDimensionException,SingularMatrixException);
  
    template ICLMath_API void DynMatrix<float>::decompose_RQ(DynMatrix<float> &R, DynMatrix<float> &Q) const
      throw (InvalidMatrixDimensionException,SingularMatrixException);
    template ICLMath_API void DynMatrix<double>::decompose_RQ(DynMatrix<double> &R, DynMatrix<double> &Q) const
      throw (InvalidMatrixDimensionException,SingularMatrixException);
  
    template ICLMath_API void DynMatrix<float>::decompose_LU(DynMatrix<float> &L, DynMatrix<float> &U, float zeroThreshold) const;
    template ICLMath_API void DynMatrix<double>::decompose_LU(DynMatrix<double> &L, DynMatrix<double> &U, double zeroThreshold) const;
  
    template ICLMath_API DynMatrix<float> DynMatrix<float>::solve_upper_triangular(const DynMatrix<float> &b)
      const throw(InvalidMatrixDimensionException);
    template ICLMath_API DynMatrix<double> DynMatrix<double>::solve_upper_triangular(const DynMatrix<double> &b)
      const throw(InvalidMatrixDimensionException);
  
    template ICLMath_API DynMatrix<float> DynMatrix<float>::solve_lower_triangular(const DynMatrix<float> &b)
      const throw(InvalidMatrixDimensionException);
    template ICLMath_API DynMatrix<double> DynMatrix<double>::solve_lower_triangular(const DynMatrix<double> &b)
      const throw(InvalidMatrixDimensionException);
  
    template ICLMath_API DynMatrix<float> DynMatrix<float>::solve(const DynMatrix<float> &b, const std::string &method, float zeroThreshold)
      throw(InvalidMatrixDimensionException,  ICLException, SingularMatrixException);
    template ICLMath_API DynMatrix<double> DynMatrix<double>::solve(const DynMatrix<double> &b, const std::string &method, double zeroThreshold)
      throw(InvalidMatrixDimensionException,  ICLException, SingularMatrixException);
  
  
    template ICLMath_API DynMatrix<float> DynMatrix<float>::pinv(bool, float) const
      throw (InvalidMatrixDimensionException,SingularMatrixException,ICLException);
    template ICLMath_API DynMatrix<double> DynMatrix<double>::pinv(bool, double) const
      throw (InvalidMatrixDimensionException,SingularMatrixException,ICLException);
  
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
    template ICLMath_API std::ostream &operator<<(std::ostream&,const DynMatrix<T >&); \
    template ICLMath_API std::istream &operator>>(std::istream&,DynMatrix<T >&)
  
    X(uint8_t);
    X(int16_t);
    X(int32_t);
    X(float);
    X(double);
    X(bool);
  
    X(std::complex<float>);
    X(std::complex<double>);
  
  #undef X
    
    template<class T>
    DynMatrix<T> DynMatrix<T>::loadCSV(const std::string &filename) throw (ICLException){
      std::ifstream s(filename.c_str());
      if(!s.good()) throw ICLException("DynMatrix::loadCSV: invalid filename ' "+ filename +'\'');
      
      std::vector<T> data;
      data.reserve(256);
      int lineLen = -1;
      
      std::string line;
      while(!s.eof()){
        std::getline(s,line);
        if(!line.length() || line[0] == '#' || line[0] == ' ') continue;
        std::vector<T> v = icl::utils::parseVecStr<T>(line,","); 
        int cLen = (int)v.size();
        if(lineLen == -1) lineLen = cLen;
        else if(lineLen != cLen){
          throw ICLException("DynMatrix::loadCSV: row lengths differ");
        }
        std::copy(v.begin(),v.end(),std::back_inserter(data));
      }
      if(!lineLen) throw ICLException("DynMatrix::loadCSV: no data found in file ' " + filename + '\'');
      DynMatrix<T> M(lineLen,data.size()/lineLen, data.data());
      return M;
    }
      
    /// writes the current matrix to a csv file
    /** supported types T are all icl8u, icl16s, icl32s, icl32f, icl64f */
    template<class T>
    void DynMatrix<T>::saveCSV(const std::string &filename) throw (ICLException){
      std::ofstream s(filename.c_str());
      if(!s.good()) throw ICLException("DynMatrix::saveCSV:");
      for(unsigned int y=0;y<rows();++y){
        for(unsigned int x=0;x<cols();++x){
          icl_to_stream(s, (*this)(x,y)) << ',';
        }
        s << std::endl;
      }
    }
  
    
  #define ICL_INSTANTIATE_DEPTH(D)                                        \
    template ICLMath_API DynMatrix<icl##D> DynMatrix<icl##D>::loadCSV(const std::string &filename) throw (ICLException); \
    template ICLMath_API void DynMatrix<icl##D>::saveCSV(const std::string&) throw (ICLException);
    ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
  } // namespace math
}

