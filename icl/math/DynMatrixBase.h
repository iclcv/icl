// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/Macros.h>
#include <icl/utils/Exception.h>
#include <icl/utils/CompatMacros.h>
#include <algorithm>
#include <cmath>
#include <iosfwd>

namespace icl::math {
  /// Special linear algebra exception type  \ingroup LINALG \ingroup EXCEPT
  struct InvalidMatrixDimensionException :public utils::ICLException{
    InvalidMatrixDimensionException(const std::string &msg):utils::ICLException(msg){}
  };

  /// Special linear algebra exception type  \ingroup LINALG \ingroup EXCEPT
  struct IncompatibleMatrixDimensionException :public utils::ICLException{
    IncompatibleMatrixDimensionException(const std::string &msg):utils::ICLException(msg){}
  };

  /// Special linear algebra exception type  \ingroup LINALG \ingroup EXCEPT
  struct InvalidIndexException : public utils::ICLException{
    InvalidIndexException(const std::string &msg):utils::ICLException(msg){}
  };

  /// Special linear algebra exception type  \ingroup LINALG \ingroup EXCEPT
  struct SingularMatrixException : public utils::ICLException{
    SingularMatrixException(const std::string &msg):utils::ICLException(msg){}
  };

  /// Lightweight base class for DynMatrix — storage, element access, and properties  \ingroup LINALG
  /** DynMatrixBase provides the minimal interface for 2D matrix storage:
      construction, destruction, element access, flat iteration, and basic
      properties. It is fully inlined (header-only) to ensure zero overhead.

      All arithmetic, linear algebra, strided column/row iteration, and
      I/O live in the derived DynMatrix<T> class.
  */
  template<class T>
  struct DynMatrixBase {

    /// default iterator type (just a data-pointer)
    using iterator = T*;

    /// default const_iterator type (just a data-pointer)
    using const_iterator = const T*;

    /// Default empty constructor creates a null-matrix
    inline DynMatrixBase():m_rows(0),m_cols(0),m_data(0),m_ownData(true){}

    /// Create a dyn matrix with given dimensions (and optional initialValue)
    inline DynMatrixBase(unsigned int cols,unsigned int rows,const  T &initValue=0) :
    m_rows(rows),m_cols(cols),m_ownData(true){
      if(!dim()) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      m_data = new T[cols*rows];
      std::fill(begin(),end(),initValue);
    }

    /// Create a matrix with given data
    /** Data can be wrapped deeply or shallowly. If the latter is true, given data pointer
        will not be released in the destructor*/
    inline DynMatrixBase(unsigned int cols,unsigned int rows, T *data, bool deepCopy=true) :
      m_rows(rows),m_cols(cols),m_ownData(deepCopy){
      if(!dim()) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      if(deepCopy){
        m_data = new T[dim()];
        std::copy(data,data+dim(),begin());
      }else{
        m_data = data;
      }
    }

    /// Create a matrix with given data (const version: deepCopy only)
    inline DynMatrixBase(unsigned int cols,unsigned int rows,const T *data) :
      m_rows(rows),m_cols(cols),m_ownData(true){
      if(!dim()) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      m_data = new T[dim()];
      std::copy(data,data+dim(),begin());
    }

    /// Default copy constructor
    inline DynMatrixBase(const DynMatrixBase &other):
      m_rows(other.m_rows),m_cols(other.m_cols),m_data(dim() ? new T[dim()] : 0),m_ownData(true){
      std::copy(other.begin(),other.end(),begin());
    }

    /// returns with this matrix has a valid data pointer
    inline bool isNull() const { return !m_data; }

    /// Destructor (deletes data if not wrapped shallowly)
    inline ~DynMatrixBase(){
      if(m_data && m_ownData) delete [] m_data;
    }

    /// Assignment operator (using deep/shallow-copy)
    /** In general, the assignment operator applies a deep copy.
        Only in case of (*this) is not initialized and other
        is a shallow copy, (*this) will also become a shallow
        copy of the data referenced by other */
    inline DynMatrixBase &operator=(const DynMatrixBase &other){
      if(!m_data && !other.m_ownData){
        m_data = other.m_data;
        m_ownData = false;
        m_rows = other.m_rows;
        m_cols = other.m_cols;
      }else{
        if(dim() != other.dim()){
          delete[] m_data;
          m_data = other.dim() ? new T[other.dim()] : 0;
        }
        m_cols = other.m_cols;
        m_rows = other.m_rows;
        std::copy(other.begin(),other.end(),begin());
      }
      return *this;
    }

    /// resets matrix dimensions
    inline void setBounds(unsigned int cols, unsigned int rows, bool holdContent=false, const T &initializer=0){
      if(static_cast<int>(cols) == m_cols && static_cast<int>(rows)==m_rows) return;
      if(cols*rows == 0) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      DynMatrixBase M(cols,rows,initializer);
      if(holdContent){
        unsigned int min_cols = iclMin(cols,static_cast<unsigned int>(m_cols));
        unsigned int min_rows = iclMin(rows,static_cast<unsigned int>(m_rows));
        for(unsigned int i=0;i<min_cols;++i){
          for(unsigned int j=0;j<min_rows;++j){
            M(j, i) = (*this)(j, i);
          }
        }
      }
      m_cols = cols;
      m_rows = rows;
      if(m_data && m_ownData) delete [] m_data;
      m_data = M.begin();
      m_ownData = true;
      M.set_data(0);
    }

    /// tests weather a matrix is enough similar to another matrix
    inline bool isSimilar(const DynMatrixBase &other, T tollerance=T(0.0001)) const{
      if(other.cols() != cols() || other.rows() != rows()) return false;
      for(unsigned int i=0;i<dim();++i){
        T diff = m_data[i] - other.m_data[i];
        if((diff>0?diff:-diff) > tollerance) return false;
      }
      return true;
    }

    /// elementwise comparison (==)
    inline bool operator==(const DynMatrixBase &other) const{
      if(other.cols() != cols() || other.rows() != rows()) return false;
      for(unsigned int i=0;i<dim();++i){
        if(m_data[i] !=  other.m_data[i]) return false;
      }
      return true;
    }

    /// elementwise comparison (!=)
    inline bool operator!=(const DynMatrixBase &other) const{
      if(other.cols() != cols() || other.rows() != rows()) return false;
      for(unsigned int i=0;i<dim();++i){
        if(m_data[i] !=  other.m_data[i]) return true;
      }
      return false;
    }

    /// Element access with (row, col) convention — standard math indexing
    inline T &operator()(unsigned int row, unsigned int col){
#ifdef DYN_MATRIX_INDEX_CHECK
      if(static_cast<int>(col) >= m_cols || static_cast<int>(row) >= m_rows) ERROR_LOG("access to "<<m_cols<<'x'<<m_rows<<"-matrix index (" << col << "," << row << ")");
#endif
      return m_data[col+cols()*row];
    }

    /// Element access with (row, col) convention — standard math indexing (const)
    inline const T &operator()(unsigned int row, unsigned int col) const{
#ifdef DYN_MATRIX_INDEX_CHECK
      if(static_cast<int>(col) >= m_cols || static_cast<int>(row) >= m_rows) ERROR_LOG("access to "<<m_cols<<'x'<<m_rows<<"-matrix index (" << col << "," << row << ")");
#endif
      return m_data[col+cols()*row];
    }

    /// Bounds-checked element access with (row, col) convention
    inline T &at(unsigned int row,unsigned int col){
#ifdef DYN_MATRIX_INDEX_CHECK
      if(static_cast<int>(col) >= m_cols || static_cast<int>(row) >= m_rows) ERROR_LOG("access to "<<m_cols<<'x'<<m_rows<<"-matrix index (" << col << "," << row << ")");
#endif
      if(col>=cols() || row >= rows()) throw InvalidIndexException("row or col index too large");
      return m_data[col+cols()*row];
    }

    /// Bounds-checked element access with (row, col) convention (const)
    inline const T &at(unsigned int row,unsigned int col) const{
      return const_cast<DynMatrixBase*>(this)->at(row,col);
    }

    /// linear access to actual data array
    inline T &operator[](unsigned int idx) {
      idx_check(idx);
      if(idx >= dim()) ERROR_LOG("access to "<<m_cols<<'x'<<m_rows<<"-matrix index [" << idx<< "]");
      return m_data[idx];
    }

    /// linear access to actual data array (const)
    inline const T &operator[](unsigned int idx) const {
      idx_check(idx);
      return m_data[idx];
    }

    /// height of the matrix (number of rows)
    inline unsigned int rows() const { return m_rows; }

    /// width of the matrix (number of columns)
    inline unsigned int cols() const { return m_cols; }

    /// internal data pointer
    inline T *data() { return m_data; }

    /// internal data pointer (const)
    inline const T *data() const { return m_data; }

    /// matrix dimension (width*height) or (cols*rows)
    inline unsigned int dim() const { return m_rows*m_cols; }

    /// returns sizeof (T)*dim()
    inline int stride0() const { return sizeof(T) * dim(); }

    /// returns sizeof(T)*cols()
    inline int stride1() const { return sizeof(T) * cols(); }

    /// returns sizeof (T)
    inline int stride2() const { return sizeof(T); }

    /// returns an iterator to the begin of internal data array
    inline iterator begin() { return m_data; }

    /// returns an iterator to the end of internal data array
    inline iterator end() { return m_data+dim(); }

    /// returns an iterator to the begin of internal data array (const)
    inline const_iterator begin() const { return m_data; }

    /// returns an iterator to the end of internal data array (const)
    inline const_iterator end() const { return m_data+dim(); }

    /// resets the matrix dimensions without changing the content
    inline void reshape(int newCols, int newRows){
      if((cols() * rows()) != (newCols * newRows)){
        throw InvalidMatrixDimensionException("DynMatrixBase<T>::reshape: source dimension and destination dimension differs!");
      }
      m_cols = newCols;
      m_rows = newRows;
    }

    /// sets new data internally and returns old data pointer (for experts only!)
    inline T *set_data(T *newData){
      T *old_data = m_data;
      m_data = newData;
      return old_data;
    }

    // id() factory is in DynMatrix<T> (out-of-line)

  protected:
    inline void row_check([[maybe_unused]] unsigned int row) const{
#ifdef DYN_MATRIX_INDEX_CHECK
      if(static_cast<int>(row) >= m_rows) ERROR_LOG("access to row index " << row << " on a "<<m_cols<<'x'<<m_rows<<"-matrix");
#endif
    }
    inline void col_check([[maybe_unused]] unsigned int col) const{
#ifdef DYN_MATRIX_INDEX_CHECK
      if(static_cast<int>(col) >= m_cols) ERROR_LOG("access to column index " << col << " on a "<<m_cols<<'x'<<m_rows<<"-matrix");
#endif
    }
    inline void idx_check(unsigned int col, unsigned int row) const{
      col_check(col);
      row_check(row);
    }
    inline void idx_check([[maybe_unused]] unsigned int idx) const{
#ifdef DYN_MATRIX_INDEX_CHECK
      if(idx >= dim()) ERROR_LOG("access to linear index " << idx << " on a "<<m_cols<<'x'<<m_rows<<"-matrix");
#endif
    }

    int m_rows;
    int m_cols;
    T *m_data;
    bool m_ownData;
  };

  /// ostream operator for DynMatrixBase (and DynMatrix via inheritance) \ingroup LINALG
  template<class T> ICLMath_IMP
  std::ostream &operator<<(std::ostream &s, const DynMatrixBase<T> &m);

  /// istream operator for DynMatrixBase (and DynMatrix via inheritance) \ingroup LINALG
  template<class T> ICLMath_IMP
  std::istream &operator>>(std::istream &s, DynMatrixBase<T> &m);

  } // namespace icl::math