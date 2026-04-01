/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/DynMatrix.h                        **
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

#pragma once

#include <ICLMath/DynMatrixBase.h>
#include <ICLUtils/CompatMacros.h>
#include <iterator>
#include <functional>

namespace icl{
  namespace math{

    /// Highly flexible and optimized matrix class implementation  \ingroup LINALG
    /** Inherits DynMatrixBase<T> for storage and element access.
        Adds strided column/row iteration, arithmetic operators,
        linear algebra, and I/O. Arithmetic and linear algebra methods
        are out-of-line, explicitly instantiated for float and double.
    */
    template<class T>
    struct DynMatrix : public DynMatrixBase<T>{

      // Inherit all base constructors
      using DynMatrixBase<T>::DynMatrixBase;
      using DynMatrixBase<T>::operator=;

      // Make base members accessible without this-> in templates
      using DynMatrixBase<T>::m_rows;
      using DynMatrixBase<T>::m_cols;
      using DynMatrixBase<T>::m_data;
      using DynMatrixBase<T>::m_ownData;
      using DynMatrixBase<T>::rows;
      using DynMatrixBase<T>::cols;
      using DynMatrixBase<T>::dim;
      using DynMatrixBase<T>::begin;
      using DynMatrixBase<T>::end;
      using DynMatrixBase<T>::data;
      using DynMatrixBase<T>::operator();
      using DynMatrixBase<T>::operator[];
      using DynMatrixBase<T>::row_check;
      using DynMatrixBase<T>::col_check;

      /** \cond */
      class DynMatrixColumn;
      /** \endcond*/

      /// creates a column matrix from given column of other matrix
      DynMatrix(const DynMatrixColumn &column);

      /// creates a new DynMatrix from given csv filename
      /** @see DynMatrix<T>::loadCSV */
      inline DynMatrix(const std::string &filename):DynMatrixBase<T>(){
        *this = loadCSV(filename);
      }

      /// loads a dynmatrix from given CSV file
      static DynMatrix<T> loadCSV(const std::string &filename);

      /// writes the current matrix to a csv file
      void saveCSV(const std::string &filename);

      // ---- Row/column iterator types ----

      /// row_iterator type (just a data-pointer)
      using row_iterator = T*;

      /// const_row_iterator type
      using const_row_iterator = const T*;

      /// Internal column iterator struct (using height-stride) \ingroup LINALG
      struct col_iterator {
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using difference_type = unsigned int;
        mutable T *p;
        unsigned int stride;
        inline col_iterator(T *col_begin,unsigned int stride):p(col_begin),stride(stride){}

        inline col_iterator &operator++(){ p+=stride; return *this; }
        inline const col_iterator &operator++() const{ p+=stride; return *this; }
        inline col_iterator operator++(int){ col_iterator tmp = *this; ++(*this); return tmp; }
        inline const col_iterator operator++(int) const{ col_iterator tmp = *this; ++(*this); return tmp; }
        inline col_iterator &operator--(){ p-=stride; return *this; }
        inline const col_iterator &operator--() const{ p-=stride; return *this; }
        inline col_iterator operator--(int){ col_iterator tmp = *this; --(*this); return tmp; }
        inline const col_iterator operator--(int) const{ col_iterator tmp = *this; --(*this); return tmp; }
        inline col_iterator &operator+=(difference_type n){ p += n * stride; return *this; }
        inline const col_iterator &operator+=(difference_type n) const{ p += n * stride; return *this; }
        inline col_iterator &operator-=(difference_type n){ p -= n * stride; return *this; }
        inline const col_iterator &operator-=(difference_type n) const{ p -= n * stride; return *this; }
        inline col_iterator operator+(difference_type n) { col_iterator tmp = *this; tmp+=n; return tmp; }
        inline const col_iterator operator+(difference_type n) const{ col_iterator tmp = *this; tmp+=n; return tmp; }
        inline col_iterator operator-(difference_type n) { col_iterator tmp = *this; tmp-=n; return tmp; }
        inline const col_iterator operator-(difference_type n) const { col_iterator tmp = *this; tmp-=n; return tmp; }
        inline difference_type operator-(const col_iterator &other) const{ return (p-other.p)/stride; }
        inline T &operator*(){ return *p; }
        inline T operator*() const{ return *p; }
        inline bool operator==(const col_iterator &i) const{ return p == i.p; }
        inline bool operator!=(const col_iterator &i) const{ return p != i.p; }
        inline bool operator<(const col_iterator &i) const{ return p < i.p; }
        inline bool operator<=(const col_iterator &i) const{ return p <= i.p; }
        inline bool operator>=(const col_iterator &i) const{ return p >= i.p; }
        inline bool operator>(const col_iterator &i) const{ return p > i.p; }
      };

      /// const column iterator typedef
      using const_col_iterator = const col_iterator;

      /// Internally used Utility structure referencing a matrix column shallowly
      class ICLMath_API DynMatrixColumn{
        public:
  #ifdef DYN_MATRIX_INDEX_CHECK
  #define DYN_MATRIX_COLUMN_CHECK(C,E) if(C) ERROR_LOG(E)
  #else
  #define DYN_MATRIX_COLUMN_CHECK(C,E)
  #endif
        DynMatrix<T> *matrix;
        unsigned int column;

        inline DynMatrixColumn(const DynMatrix<T> *matrix, unsigned int column):
        matrix(const_cast<DynMatrix<T>*>(matrix)),column(column){
          DYN_MATRIX_COLUMN_CHECK(column >= matrix->cols(),"invalid column index");
        }
        inline DynMatrixColumn(const DynMatrix<T> &matrix):
        matrix(const_cast<DynMatrix<T>*>(&matrix)),column(0){
          DYN_MATRIX_COLUMN_CHECK(matrix.cols() != 1,"source matrix must have exactly ONE column");
        }
        inline DynMatrixColumn(const DynMatrixColumn &c):
        matrix(c.matrix),column(c.column){}

        inline col_iterator begin() { return matrix->col_begin(column); }
        inline col_iterator end() { return matrix->col_end(column); }
        inline const col_iterator begin() const { return matrix->col_begin(column); }
        inline const col_iterator end() const { return matrix->col_end(column); }
        inline unsigned int dim() const { return matrix->rows(); }

        inline DynMatrixColumn &operator=(const DynMatrixColumn &c){
          DYN_MATRIX_COLUMN_CHECK(dim() != c.dim(),"dimension missmatch");
          std::copy(c.begin(),c.end(),begin());
          return *this;
        }
        inline DynMatrixColumn &operator=(const DynMatrix &src){
          DYN_MATRIX_COLUMN_CHECK(dim() != src.dim(),"dimension missmatch");
          std::copy(src.begin(),src.end(),begin());
          return *this;
        }
        inline DynMatrixColumn &operator+=(const DynMatrixColumn &c){
          DYN_MATRIX_COLUMN_CHECK(dim() != c.dim(),"dimension missmatch");
          std::transform(c.begin(),c.end(),begin(),begin(),std::plus<T>());
          return *this;
        }
        inline DynMatrixColumn &operator-=(const DynMatrixColumn &c){
          DYN_MATRIX_COLUMN_CHECK(dim() != c.dim(),"dimension missmatch");
          std::transform(c.begin(),c.end(),begin(),begin(),std::minus<T>());
          return *this;
        }
        inline DynMatrixColumn &operator+=(const DynMatrix &m){
          DYN_MATRIX_COLUMN_CHECK(dim() != m.dim(),"dimension missmatch");
          std::transform(m.begin(),m.end(),begin(),begin(),std::plus<T>());
          return *this;
        }
        inline DynMatrixColumn &operator-=(const DynMatrix &m){
          DYN_MATRIX_COLUMN_CHECK(dim() != m.dim(),"dimension missmatch");
          std::transform(m.begin(),m.end(),begin(),begin(),std::minus<T>());
          return *this;
        }
        inline DynMatrixColumn &operator*=(const T&val){
          std::for_each(begin(),end(),[val](T &v){ v *= val; });
          return *this;
        }
        inline DynMatrixColumn &operator/=(const T&val){
          std::for_each(begin(),end(),[val](T &v){ v /= val; });
          return *this;
        }
      };

      inline DynMatrix &operator=(const DynMatrixColumn &col){
  #ifdef DYN_MATRIX_INDEX_CHECK
        if(dim() != col.dim()) ERROR_LOG("dimension missmatch");
  #endif
        std::copy(col.begin(),col.end(),begin());
        return *this;
      }

  #undef DYN_MATRIX_COLUMN_CHECK

      // ---- Row/column access ----

      inline col_iterator col_begin(unsigned int col) {
        col_check(col);
        return col_iterator(m_data+col,cols());
      }
      inline col_iterator col_end(unsigned int col) {
        col_check(col);
        return col_iterator(m_data+col+dim(),cols());
      }
      inline const_col_iterator col_begin(unsigned int col) const {
        col_check(col);
        return col_iterator(m_data+col,cols());
      }
      inline const_col_iterator col_end(unsigned int col) const {
        col_check(col);
        return col_iterator(m_data+col+dim(),cols());
      }

      inline row_iterator row_begin(unsigned int row) {
        row_check(row);
        return m_data+row*cols();
      }
      inline row_iterator row_end(unsigned int row) {
        row_check(row);
        return m_data+(row+1)*cols();
      }
      inline const_row_iterator row_begin(unsigned int row) const {
        row_check(row);
        return m_data+row*cols();
      }
      inline const_row_iterator row_end(unsigned int row) const {
        row_check(row);
        return m_data+(row+1)*cols();
      }

      /// Extracts a shallow copied matrix row
      inline DynMatrix row(int row){
        row_check(row);
        return DynMatrix(m_cols,1,row_begin(row),false);
      }
      inline const DynMatrix row(int row) const{
        row_check(row);
        return DynMatrix(m_cols,1,const_cast<T*>(row_begin(row)),false);
      }

      /// Extracts a shallow copied matrix column
      inline DynMatrixColumn col(int col){
        return DynMatrixColumn(this,col);
      }
      inline const DynMatrixColumn col(int col) const{
        return DynMatrixColumn(this,col);
      }

      /// returns a shallow transposed copy (dimensions swapped, data not re-arranged)
      inline const DynMatrix<T> shallowTransposed() const{
        return DynMatrix<T>(m_rows,m_cols,const_cast<T*>(m_data),false);
      }
      inline DynMatrix<T> shallowTransposed() {
        return DynMatrix<T>(m_rows,m_cols,const_cast<T*>(m_data),false);
      }

      /// creates a dim-D identity Matrix
      static inline DynMatrix id(unsigned int dim) {
        DynMatrix M(dim,dim,T(0));
        for(unsigned int i=0;i<dim;++i) M(i,i) = 1;
        return M;
      }

      // ================================================================
      // Arithmetic operators — declarations only, defined in DynMatrix.cpp
      // Explicitly instantiated for float and double.
      // ================================================================

      /// Multiply elements with scalar
      DynMatrix operator*(T f) const;
      /// Multiply elements with scalar (in source destination fashion)
      DynMatrix &mult(T f, DynMatrix &dst) const;
      /// Multiply elements with scalar (inplace)
      DynMatrix &operator*=(T f);
      /// Divide elements by scalar
      DynMatrix operator/(T f) const;
      /// Divide elements by scalar (inplace)
      DynMatrix &operator/=(T f);

      /// Matrix multiplication (in source destination fashion)
      DynMatrix &mult(const DynMatrix &m, DynMatrix &dst) const;
      /// Matrix multiplication
      DynMatrix operator*(const DynMatrix &m) const;
      /// Inplace matrix multiplication
      DynMatrix &operator*=(const DynMatrix &m);
      /// Matrix division (this * m.inv())
      DynMatrix operator/(const DynMatrix &m) const;
      /// Inplace matrix division
      DynMatrix &operator/=(const DynMatrix &m);

      /// Elementwise matrix multiplication (in source destination fashion)
      DynMatrix &elementwise_mult(const DynMatrix &m, DynMatrix &dst) const;
      /// Elementwise matrix multiplication
      DynMatrix elementwise_mult(const DynMatrix &m) const;
      /// Elementwise division (in source destination fashion)
      DynMatrix &elementwise_div(const DynMatrix &m, DynMatrix &dst) const;
      /// Elementwise division
      DynMatrix elementwise_div(const DynMatrix &m) const;

      /// adds a scalar to each element
      DynMatrix operator+(const T &t) const;
      /// subtracts a scalar from each element
      DynMatrix operator-(const T &t) const;
      /// adds a scalar to each element (inplace)
      DynMatrix &operator+=(const T &t);
      /// subtracts a scalar from each element (inplace)
      DynMatrix &operator-=(const T &t);

      /// Matrix addition
      DynMatrix operator+(const DynMatrix &m) const;
      /// Matrix subtraction
      DynMatrix operator-(const DynMatrix &m) const;
      /// Matrix addition (inplace)
      DynMatrix &operator+=(const DynMatrix &m);
      /// Matrix subtraction (inplace)
      DynMatrix &operator-=(const DynMatrix &m);

      // ================================================================
      // Norms, distances, and other computed properties
      // ================================================================

      /// applies an L_l norm on the matrix elements
      T norm(double l=2) const;
      /// returns the squared distance of inner data vectors
      T sqrDistanceTo(const DynMatrix &other) const;
      /// returns the distance of inner data vectors
      T distanceTo(const DynMatrix &other) const;

      /// matrix transposed
      DynMatrix transp() const;
      /// inner product of data pointers
      T element_wise_inner_product(const DynMatrix<T> &other) const;
      /// returns the inner product (dot-product): A.transp() * B
      DynMatrix<T> dot(const DynMatrix<T> &M) const;
      /// returns diagonal-elements as column-vector
      DynMatrix<T> diag() const;
      /// computes the sum of all diagonal elements
      T trace() const;
      /// computes the cross product
      static DynMatrix<T> cross(const DynMatrix<T> &x, const DynMatrix<T> &y);
      /// computes the condition of a matrix
      T cond(const double p=2) const;

      // ================================================================
      // Linear algebra — float/double only
      // ================================================================

      /// QR decomposition
      void decompose_QR(DynMatrix &Q, DynMatrix &R) const;
      /// RQ decomposition
      void decompose_RQ(DynMatrix &R, DynMatrix &Q) const;
      /// LU decomposition
      void decompose_LU(DynMatrix &L, DynMatrix &U, T zeroThreshold = T(1E-16)) const;
      /// solves Mx=b via SVD least-squares (gelsd)
      DynMatrix solve(const DynMatrix &b, T zeroThreshold = T(1E-16));
      /// matrix inverse
      DynMatrix inv() const;
      /// eigenvalue decomposition (symmetric matrices only)
      void eigen(DynMatrix &eigenvectors, DynMatrix &eigenvalues) const;
      /// singular value decomposition
      void svd(DynMatrix &U, DynMatrix &S, DynMatrix &V) const;
      /// Moore-Penrose pseudo-inverse
      DynMatrix pinv(T zeroThreshold = T(1E-16)) const;
      /// matrix determinant
      T det() const;
    };

    /** \cond */
    /// creates a dyn-matrix from given matrix column
    template<class T>
    DynMatrix<T>::DynMatrix(const typename DynMatrix<T>::DynMatrixColumn &column) :
    DynMatrixBase<T>(1, column.dim()){
      std::copy(column.begin(),column.end(),this->begin());
    }
    /** \endcond */

    /// ostream operator implemented for uchar, short, int, float and double matrices  \ingroup LINALG
    template<class T> ICLMath_IMP
    std::ostream &operator<<(std::ostream &s, const DynMatrix<T> &m);

    /// istream operator implemented for uchar, short, int, float and double matrices  \ingroup LINALG
    template<class T> ICLMath_IMP
    std::istream &operator>>(std::istream &s, DynMatrix<T> &m);


    /// horizontal concatenation of matrices (missing elements padded with 0)
    template<class T>
    inline DynMatrix<T> operator,(const DynMatrix<T> &left, const DynMatrix<T> &right){
      int w = left.cols() + right.cols();
      int h = iclMax(left.rows(),right.rows());
      DynMatrix<T> result(w,h,T(0));
      for(unsigned int y=0;y<left.rows();++y){
        std::copy(left.row_begin(y), left.row_end(y), result.row_begin(y));
      }
      for(unsigned int y=0;y<right.rows();++y){
        std::copy(right.row_begin(y), right.row_end(y), result.row_begin(y) + left.cols());
      }
      return result;
    }

    /// vertical concatenation of matrices (missing elements padded with 0)
    template<class T>
    inline DynMatrix<T> operator%(const DynMatrix<T> &top, const DynMatrix<T> &bottom){
      int w = iclMax(top.cols(),bottom.cols());
      int h = top.rows() + bottom.rows();
      DynMatrix<T> result(w,h,T(0));
      for(unsigned int y=0;y<top.rows();++y){
        std::copy(top.row_begin(y), top.row_end(y), result.row_begin(y));
      }
      for(unsigned int y=0;y<bottom.rows();++y){
        std::copy(bottom.row_begin(y), bottom.row_end(y), result.row_begin(y+top.rows()));
      }
      return result;
    }
  } // namespace math
}
