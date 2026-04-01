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

#include <ICLMath/BlasOps.h>
#include <ICLMath/LapackOps.h>

#include <ICLMath/DynMatrixUtils.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;

namespace icl{
  namespace math{

    // ---- Matrix multiplication via BLAS gemm ----

    namespace {
      // Generic fallback: inner_product loop (for integer types etc.)
      template<class T>
      void mult_generic(const DynMatrix<T> &a, const DynMatrix<T> &b, DynMatrix<T> &dst) {
        for(unsigned int c = 0; c < dst.cols(); ++c)
          for(unsigned int r = 0; r < dst.rows(); ++r)
            dst(c, r) = std::inner_product(a.row_begin(r), a.row_end(r), b.col_begin(c), T(0));
      }

      // BLAS gemm path for float/double
      template<class T>
      void mult_gemm(const DynMatrix<T> &a, const DynMatrix<T> &b, DynMatrix<T> &dst) {
        auto* impl = BlasOps<T>::instance()
            .template getSelector<typename BlasOps<T>::GemmSig>(BlasOp::gemm)
            .resolveOrThrow();
        int M = a.rows(), N = b.cols(), K = a.cols();
        impl->apply(false, false, M, N, K, T(1),
                     a.begin(), a.cols(), b.begin(), b.cols(),
                     T(0), dst.begin(), N);
      }
    }

    template<class T>
    DynMatrix<T>& DynMatrix<T>::mult(const DynMatrix<T> &m, DynMatrix<T> &dst) const {
      if(cols() != m.rows()) throw IncompatibleMatrixDimensionException("A*B : cols(A) must be rows(B)");
      dst.setBounds(m.cols(), rows());
      mult_generic(*this, m, dst);
      return dst;
    }

    // Specializations for float/double: use BLAS gemm
    template<>
    DynMatrix<float>& DynMatrix<float>::mult(const DynMatrix<float> &m, DynMatrix<float> &dst) const {
      if(cols() != m.rows()) throw IncompatibleMatrixDimensionException("A*B : cols(A) must be rows(B)");
      dst.setBounds(m.cols(), rows());
      mult_gemm(*this, m, dst);
      return dst;
    }

    template<>
    DynMatrix<double>& DynMatrix<double>::mult(const DynMatrix<double> &m, DynMatrix<double> &dst) const {
      if(cols() != m.rows()) throw IncompatibleMatrixDimensionException("A*B : cols(A) must be rows(B)");
      dst.setBounds(m.cols(), rows());
      mult_gemm(*this, m, dst);
      return dst;
    }

    template<class T>
    static double dot(const DynMatrix<T> &a, const DynMatrix<T> &b){
      ICLASSERT_RETURN_VAL(a.dim() == b.dim(),0.0);
      double s = 0;
      for(unsigned int i=0;i<a.dim();++i){
        s += a[i] * b[i];
      }
      return s;
    }



    template<class T>
    DynMatrix<T> DynMatrix<T>::inv() const{
      unsigned int n = cols();
      ICLASSERT_THROW(n == rows(), InvalidMatrixDimensionException("inv() requires square matrix"));

      // Use LU factorization (O(n³)) via LapackOps
      auto* getrfImpl = LapackOps<T>::instance()
          .template getSelector<typename LapackOps<T>::GetrfSig>(LapackOp::getrf)
          .resolveOrThrow();
      auto* getriImpl = LapackOps<T>::instance()
          .template getSelector<typename LapackOps<T>::GetriSig>(LapackOp::getri)
          .resolveOrThrow();

      DynMatrix<T> A(n, n);
      std::copy(begin(), end(), A.begin());
      std::vector<int> ipiv(n);

      int info = getrfImpl->apply(n, n, A.data(), n, ipiv.data());
      if(info != 0) throw SingularMatrixException("Matrix is singular (getrf info=" + str(info) + ")");

      info = getriImpl->apply(n, A.data(), n, ipiv.data());
      if(info != 0) throw SingularMatrixException("Matrix inverse failed (getri info=" + str(info) + ")");

      return A;
    }

    template<class T>
    T DynMatrix<T>::det() const{
      unsigned int order = cols();
      if(order != rows()) throw InvalidMatrixDimensionException("Determinant can only be calculated on squared matrices");

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
          // LU factorization: det = product of U diagonal * sign of permutation
          auto* getrfImpl = LapackOps<T>::instance()
              .template getSelector<typename LapackOps<T>::GetrfSig>(LapackOp::getrf)
              .resolveOrThrow();

          DynMatrix<T> A(order, order);
          std::copy(begin(), end(), A.begin());
          std::vector<int> ipiv(order);

          int info = getrfImpl->apply(order, order, A.data(), order, ipiv.data());
          if(info > 0) return T(0); // singular

          T d = T(1);
          for(unsigned int i = 0; i < order; i++) {
            d *= A(i, i); // diagonal of U
            if(ipiv[i] != (int)(i + 1)) d = -d; // permutation sign
          }
          return d;
        }
      }
    }

    template<class T>
    void DynMatrix<T>::decompose_QR(DynMatrix<T> &Q, DynMatrix<T> &R) const {
      int m = rows(), n = cols();
      int mn = std::min(m, n);

      auto* geqrfImpl = LapackOps<T>::instance()
          .template getSelector<typename LapackOps<T>::GeqrfSig>(LapackOp::geqrf)
          .resolveOrThrow();
      auto* orgqrImpl = LapackOps<T>::instance()
          .template getSelector<typename LapackOps<T>::OrgqrSig>(LapackOp::orgqr)
          .resolveOrThrow();

      // Copy input to working buffer (row-major, lda = n)
      DynMatrix<T> A(n, m);
      std::copy(begin(), end(), A.begin());
      std::vector<T> tau(mn);

      int info = geqrfImpl->apply(m, n, A.data(), n, tau.data());
      if(info != 0) throw ICLException("QR factorization failed (geqrf info=" + str(info) + ")");

      // Extract R (upper triangle of A, n×n)
      R.setBounds(n, n);
      std::fill(R.begin(), R.end(), T(0));
      for(int i = 0; i < mn; i++)
        for(int j = i; j < n; j++)
          R(j, i) = A(j, i);

      // Form Q from Householder reflectors (m×n)
      info = orgqrImpl->apply(m, n, mn, A.data(), n, tau.data());
      if(info != 0) throw ICLException("Q formation failed (orgqr info=" + str(info) + ")");

      Q.setBounds(n, m);
      std::copy(A.begin(), A.begin() + m * n, Q.begin());
    }

    template<class T>
    void DynMatrix<T>::decompose_RQ(DynMatrix<T> &R, DynMatrix<T> &Q) const {
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
      int m = rows();
      int n = cols();
      int mn = std::min(m, n);

      auto* getrfImpl = LapackOps<T>::instance()
          .template getSelector<typename LapackOps<T>::GetrfSig>(LapackOp::getrf)
          .resolveOrThrow();

      // Copy input (getrf overwrites)
      DynMatrix<T> A(n, m);
      std::copy(begin(), end(), A.begin());
      std::vector<int> ipiv(mn);

      getrfImpl->apply(m, n, A.data(), n, ipiv.data());

      // Extract L (unit lower triangular, m×m): P*A = L*U
      L.setBounds(m, m);
      std::fill(L.begin(), L.end(), T(0));
      for(int i = 0; i < m; i++) {
        L(i, i) = T(1);
        for(int j = 0; j < std::min(i, mn); j++)
          L(j, i) = A(j, i);
      }

      // Extract U (upper triangular, m×n)
      U.setBounds(n, m);
      std::fill(U.begin(), U.end(), T(0));
      for(int i = 0; i < mn; i++)
        for(int j = i; j < n; j++)
          U(j, i) = A(j, i);
    }

    template<class T>
    DynMatrix<T> DynMatrix<T>::solve(const DynMatrix &b, T zeroThreshold){
      int m = rows(), n = cols(), nrhs = b.cols();
      ICLASSERT_THROW(m == (int)b.rows(), InvalidMatrixDimensionException("DynMatrix::solve: M.rows != b.rows"));

      auto* gelsdImpl = LapackOps<T>::instance()
          .template getSelector<typename LapackOps<T>::GelsdSig>(LapackOp::gelsd)
          .resolveOrThrow();

      // Copy A (gelsd overwrites)
      DynMatrix<T> A(n, m);
      std::copy(begin(), end(), A.begin());

      // B must be max(M,N)×NRHS; copy b into it
      int mx = std::max(m, n);
      DynMatrix<T> B(nrhs, mx, T(0));
      for(int i = 0; i < m; i++)
        for(int j = 0; j < nrhs; j++)
          B(j, i) = b(j, i);

      std::vector<T> S(std::min(m, n));
      int rank;
      T rcond = (zeroThreshold > T(0)) ? zeroThreshold : T(-1);

      int info = gelsdImpl->apply(m, n, nrhs, A.data(), n,
                                   B.data(), nrhs, S.data(), rcond, &rank);
      ICLASSERT_THROW(info == 0, ICLException("solve failed (gelsd info=" + str(info) + ")"));

      // Solution is in first N rows of B
      DynMatrix<T> x(nrhs, n);
      for(int i = 0; i < n; i++)
        for(int j = 0; j < nrhs; j++)
          x(j, i) = B(j, i);

      return x;
    }



    template<class T>
    DynMatrix<T> DynMatrix<T>::pinv(T zeroThreshold) const{
      // Reduced SVD (jobz='S') + BLAS gemm for efficient pseudo-inverse.
      // Uses O(min(m,n)) memory for U/Vt instead of O(m²+n²).
      auto* svdImpl = LapackOps<T>::instance()
          .template getSelector<typename LapackOps<T>::GesddSig>(LapackOp::gesdd)
          .resolveOrThrow();
      auto* gemmImpl = BlasOps<T>::instance()
          .template getSelector<typename BlasOps<T>::GemmSig>(BlasOp::gemm)
          .resolveOrThrow();

      int r = rows();
      int c = cols();
      int mn = std::min(r, c);

      DynMatrix<T> matrixCopy(*this);
      std::vector<T> S(mn);
      DynMatrix<T> U(mn, r);
      DynMatrix<T> Vt(c, mn);

      int info = svdImpl->apply('S', r, c, matrixCopy.begin(), c,
                                 S.data(), U.begin(), mn, Vt.begin(), c);
      ICLASSERT_THROW(info == 0, ICLException("SVD failed in pinv (info=" + str(info) + ")"));

      DynMatrix<T> Sinv(mn, mn, T(0));
      for(int i = 0; i < mn; ++i)
        Sinv(i, i) = (std::fabs(S[i]) > zeroThreshold) ? T(1) / S[i] : T(0);

      // pseudoInverse = Vt^T * Sinv * U^T
      DynMatrix<T> temp(mn, c);
      gemmImpl->apply(true, false, c, mn, mn, T(1),
                       Vt.begin(), c, Sinv.begin(), mn,
                       T(0), temp.begin(), mn);

      DynMatrix<T> pseudoInverse(r, c);
      gemmImpl->apply(false, true, c, r, mn, T(1),
                       temp.begin(), mn, U.begin(), mn,
                       T(0), pseudoInverse.begin(), r);

      return pseudoInverse;
    }


    // Jacobi eigenvalue iteration moved to LapackOps_Cpp.cpp (cpp_syev).
    // Old jacobi_iterate_vtk + find_eigenvectors removed — see git history.

    template<class T>
    void DynMatrix<T>::eigen(DynMatrix<T> &eigenvectors, DynMatrix<T> &eigenvalues) const{
      ICLASSERT_THROW(cols() == rows(), InvalidMatrixDimensionException("find eigenvectors: input matrix a is not a square-matrix"));
      const int n = cols();
      eigenvalues.setBounds(1,n);
      eigenvectors.setBounds(n,n);

      auto* impl = LapackOps<T>::instance()
          .template getSelector<typename LapackOps<T>::SyevSig>(LapackOp::syev)
          .resolveOrThrow();

      // syev overwrites A with eigenvectors — copy input
      DynMatrix<T> A(n, n);
      std::copy(begin(), end(), A.begin());
      int info = impl->apply('V', n, A.data(), n, eigenvalues.begin());
      if(info != 0) {
        throw ICLException("eigenvalue decomposition failed (info=" + str(info) + ")");
      }

      // Copy eigenvectors from A to output
      // (C++ backend stores them as pv[i][j] → A[i*lda+j], matching DynMatrix layout)
      for(int i = 0; i < n; ++i)
        for(int j = 0; j < n; ++j)
          eigenvectors(j, i) = A(i, j);
    }

    template<class T>
    void DynMatrix<T>::svd(DynMatrix &V, DynMatrix &s, DynMatrix &U) const{
      svd_dyn<T>(*this,V,s,U);
    }


// ippmInvert_m / ippmDet_m were removed from modern IPP (ippm module dropped).
// Uses C++ fallback (Gauss-Jordan inv, LU det) from the generic template.
#if 1 // was: #else branch of ICL_HAVE_IPP — now always use C++ fallback
    template ICLMath_API DynMatrix<float> DynMatrix<float>::inv()const;
    template ICLMath_API DynMatrix<double> DynMatrix<double>::inv()const;

    template ICLMath_API float DynMatrix<float>::det()const;
    template ICLMath_API double DynMatrix<double>::det()const;
#endif


    template ICLMath_API void DynMatrix<float>::svd(DynMatrix<float>&, DynMatrix<float>&, DynMatrix<float>&) const;
    template ICLMath_API void DynMatrix<double>::svd(DynMatrix<double>&, DynMatrix<double>&, DynMatrix<double>&) const;

    template ICLMath_API void DynMatrix<float>::eigen(DynMatrix<float>&, DynMatrix<float>&) const;
    template ICLMath_API void DynMatrix<double>::eigen(DynMatrix<double>&, DynMatrix<double>&) const;

    template ICLMath_API void DynMatrix<float>::decompose_QR(DynMatrix<float> &Q, DynMatrix<float> &R) const;
    template ICLMath_API void DynMatrix<double>::decompose_QR(DynMatrix<double> &Q, DynMatrix<double> &R) const;

    template ICLMath_API void DynMatrix<float>::decompose_RQ(DynMatrix<float> &R, DynMatrix<float> &Q) const;
    template ICLMath_API void DynMatrix<double>::decompose_RQ(DynMatrix<double> &R, DynMatrix<double> &Q) const;

    template ICLMath_API void DynMatrix<float>::decompose_LU(DynMatrix<float> &L, DynMatrix<float> &U, float zeroThreshold) const;
    template ICLMath_API void DynMatrix<double>::decompose_LU(DynMatrix<double> &L, DynMatrix<double> &U, double zeroThreshold) const;

    template ICLMath_API DynMatrix<float> DynMatrix<float>::solve(const DynMatrix<float> &b, float zeroThreshold);
    template ICLMath_API DynMatrix<double> DynMatrix<double>::solve(const DynMatrix<double> &b, double zeroThreshold);


    template ICLMath_API DynMatrix<float> DynMatrix<float>::pinv(float) const;
    template ICLMath_API DynMatrix<double> DynMatrix<double>::pinv(double) const;

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
    DynMatrix<T> DynMatrix<T>::loadCSV(const std::string &filename){
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
        int cLen = static_cast<int>(v.size());
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
    void DynMatrix<T>::saveCSV(const std::string &filename){
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
    template ICLMath_API DynMatrix<icl##D> DynMatrix<icl##D>::loadCSV(const std::string &filename); \
    template ICLMath_API void DynMatrix<icl##D>::saveCSV(const std::string&);
    ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
  } // namespace math
}
