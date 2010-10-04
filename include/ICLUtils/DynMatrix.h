/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/DynMatrix.h                           **
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

#ifndef ICL_DYN_MATRIX_H
#define ICL_DYN_MATRIX_H
#include <iterator>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iostream>
#include <vector>
#include <cmath>

#include <ICLUtils/Exception.h>
#include <ICLUtils/Macros.h>

#ifdef HAVE_IPP
#include <ipp.h>
#endif


namespace icl{

  /// Special linear algebra exception type  \ingroup LINALG \ingroup EXCEPT
  struct InvalidMatrixDimensionException :public ICLException{
    InvalidMatrixDimensionException(const std::string &msg):ICLException(msg){}
  };

  /// Special linear algebra exception type  \ingroup LINALG \ingroup EXCEPT
  struct IncompatibleMatrixDimensionException :public ICLException{
    IncompatibleMatrixDimensionException(const std::string &msg):ICLException(msg){}
  };

  /// Special linear algebra exception type  \ingroup LINALG \ingroup EXCEPT
  struct  InvalidIndexException : public ICLException{
    InvalidIndexException(const std::string &msg):ICLException(msg){}
  };

  /// Special linear algebra exception type  \ingroup LINALG \ingroup EXCEPT
  struct SingularMatrixException : public ICLException{
    SingularMatrixException(const std::string &msg):ICLException(msg){}
  };

  /// Highly flexible and optimized matrix class implementation  \ingroup LINALG
  /** In contrast to the FixedMatrix template class, the DynMatrix instances are dynamically sized at runtime
      The template class is instantiated for the common ICL types
      uint8_t, int16_t, int32_t, float and double
  */
  template<class T>
  struct DynMatrix{

    /** \cond */
    class DynMatrixColumn;
    /** \endcond*/
    
    /// creates a column matrix from given column of other matrix
    DynMatrix(const DynMatrixColumn &column);
    
    /// Default empty constructor creates a null-matrix
    inline DynMatrix():m_rows(0),m_cols(0),m_data(0),m_ownData(true){}
    
    /// Create a dyn matrix with given dimensions (and optional initialValue)
    inline DynMatrix(unsigned int cols,unsigned int rows,const  T &initValue=0) throw (InvalidMatrixDimensionException) : 
    m_rows(rows),m_cols(cols),m_ownData(true){
      if(!dim()) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      m_data = new T[cols*rows];
      std::fill(begin(),end(),initValue);
    }

    /// Create a matrix with given data
    /** Data can be wrapped deeply or shallowly. If the latter is true, given data pointer
        will not be released in the destructor*/
    inline DynMatrix(unsigned int cols,unsigned int rows, T *data, bool deepCopy=true) throw (InvalidMatrixDimensionException) : 
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
    inline DynMatrix(unsigned int cols,unsigned int rows,const T *data) throw (InvalidMatrixDimensionException) : 
      m_rows(rows),m_cols(cols),m_ownData(true){
      if(!dim()) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      m_data = new T[dim()];
      std::copy(data,data+dim(),begin());
    }

    /// Default copy constructor
    inline DynMatrix(const DynMatrix &other):
    m_rows(other.m_rows),m_cols(other.m_cols),m_data(new T[dim()]),m_ownData(true){
      std::copy(other.begin(),other.end(),begin());
    }
    
    /// returns with this matrix has a valid data pointer
    inline bool isNull() const { return !m_data; }

    /// Destructor (deletes data if no wrapped shallowly)
    inline ~DynMatrix(){
      if(m_data && m_ownData) delete [] m_data;
    }

    /// Assignment operator (using deep-copy)
    inline DynMatrix &operator=(const DynMatrix &other){
      if(dim() != other.dim()){
        delete m_data;
        m_data = new T[other.dim()];
      }
      m_cols = other.m_cols;
      m_rows = other.m_rows;

      std::copy(other.begin(),other.end(),begin());
      return *this;
    }

    /// resets matrix dimensions  
    inline void setBounds(unsigned int cols, unsigned int rows, bool holdContent=false, const T &initializer=0) throw (InvalidMatrixDimensionException){
      if((int)cols == m_cols && (int)rows==m_rows) return;
      if(!(cols*rows)) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      DynMatrix M(cols,rows,initializer);
      if(holdContent){
        unsigned int min_cols = iclMin(cols,(unsigned int)m_cols);
        unsigned int min_rows = iclMin(rows,(unsigned int)m_rows);
        for(unsigned int i=0;i<min_cols;++i){
          for(unsigned int j=0;j<min_rows;++j){
            M(i,j) = (*this)(i,j);
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
    inline bool isSimilar(const DynMatrix &other, T tollerance=0.0001) const{
      if(other.cols() != cols() || other.rows() != rows()) return false;
      for(unsigned int i=0;i<dim();++i){
        T diff = m_data[i] - other.m_data[i];
        if((diff>0?diff:-diff) > tollerance) return false;
      }
      return true;
    }
    
    /// elementwise comparison (==)
    inline bool operator==(const DynMatrix &other) const{
      if(other.cols() != cols() || other.rows() != rows()) return false;
      for(unsigned int i=0;i<dim();++i){
        if(m_data[i] !=  other.m_data[i]) return false;
      }
      return true;
    }

    /// elementwise comparison (!=)
    inline bool operator!=(const DynMatrix &other) const{
      if(other.cols() != cols() || other.rows() != rows()) return false;
      for(unsigned int i=0;i<dim();++i){
        if(m_data[i] !=  other.m_data[i]) return true;
      }
      return false;
    }


    /// Multiply elements with scalar
    inline DynMatrix operator*(T f) const{
      DynMatrix dst(cols(),rows());
      return mult(f,dst);
    }

    /// Multiply elements with scalar (in source destination fashion)
    inline DynMatrix &mult(T f, DynMatrix &dst) const{
      dst.setBounds(cols(),rows());
      std::transform(begin(),end(),dst.begin(),std::bind2nd(std::multiplies<T>(),f));
      return dst;      
    }

    /// Multiply elements with scalar (inplace)
    inline DynMatrix &operator*=(T f){
      std::transform(begin(),end(),begin(),std::bind2nd(std::multiplies<T>(),f));
      return *this;
    }

    /// Device elements by scalar
    inline DynMatrix operator/(T f) const{
      return this->operator*(1/f);
    }

    /// Device elements by scalar (inplace)
    inline DynMatrix &operator/=(T f){
      return this->operator*=(1/f);
    }

    /// Matrix multiplication (in source destination fashion) [IPP-Supported]
    inline DynMatrix &mult(const DynMatrix &m, DynMatrix &dst) const throw (IncompatibleMatrixDimensionException){
      if(m.rows() != cols()) throw IncompatibleMatrixDimensionException("A*B : rows(A) must be cols(B)");
      dst.setBounds(m.cols(),rows());
      for(unsigned int c=0;c<dst.cols();++c){
        for(unsigned int r=0;r<dst.rows();++r){
          dst(c,r) = std::inner_product(row_begin(r),row_end(r),m.col_begin(c),T(0));
        }
      }
      return dst;
    }

    /// Elementwise matrix multiplication (in source destination fashion) [IPP-Supported]
    inline DynMatrix &elementwise_mult(const DynMatrix &m, DynMatrix &dst) const throw (IncompatibleMatrixDimensionException){
      if((m.cols() != cols()) || (m.rows() != rows())) throw IncompatibleMatrixDimensionException("A.*B dimension mismatch");
      dst.setBounds(cols(),rows());
      for(int i=0;i<dim();++i){
	dst[i] = m_data[i] * m[i];
      }
      return dst;
    }

    /// Elementwise matrix multiplication (without destination matrix) [IPP-Supported]
    inline DynMatrix elementwise_mult(const DynMatrix &m) const throw (IncompatibleMatrixDimensionException){
      DynMatrix dst(cols(),rows());
      return elementwise_mult(m,dst);
    }

    /// Elementwise division (in source destination fashion) [IPP-Supported]
    inline DynMatrix &elementwise_div(const DynMatrix &m, DynMatrix &dst) const throw (IncompatibleMatrixDimensionException){
      if((m.cols() != cols()) || (m.rows() != rows())) throw IncompatibleMatrixDimensionException("A./B dimension mismatch");
      dst.setBounds(cols(),rows());
      for(int i=0;i<dim();++i){
	dst[i] = m_data[i] / m[i];
      }
      return dst;
    }

    /// Elementwise matrix multiplication (without destination matrix) [IPP-Supported]
    inline DynMatrix elementwise_div(const DynMatrix &m) const throw (IncompatibleMatrixDimensionException){
      DynMatrix dst(cols(),rows());
      return elementwise_div(m,dst);
    }

    
    
    
    /// Essential matrix multiplication [IPP-Supported]
    inline DynMatrix operator*(const DynMatrix &m) const throw (IncompatibleMatrixDimensionException){
      DynMatrix d(m.cols(),rows());
      return mult(m,d);
    }
    
    /// inplace matrix multiplication applying this = this*m [IPP-Supported]
    inline DynMatrix &operator*=(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      return *this=((*this)*m);
    }
    
    /// inplace matrix devision (calling this/m.inv()) [IPP-Supported]
    inline DynMatrix operator/(const DynMatrix &m) const 
      throw (IncompatibleMatrixDimensionException,
             InvalidMatrixDimensionException,
             SingularMatrixException){
      return this->operator*(m.inv());
    }

    /// inplace matrix devision (calling this/m.inv()) (inplace)
    inline DynMatrix &operator/=(const DynMatrix &m) const 
      throw (IncompatibleMatrixDimensionException,
             InvalidMatrixDimensionException,
             SingularMatrixException){
      return *this = this->operator*(m.inv());
    }

    /// adds a scalar to each element
    inline DynMatrix operator+(const T &t){
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::plus<T>(),t));
      return d;
    }

    /// substacts a scalar from each element
    inline DynMatrix operator-(const T &t){
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::minus<T>(),t));
      return d;
    }

    /// adds a scalar to each element (inplace)
    inline DynMatrix &operator+=(const T &t){
      std::transform(begin(),end(),begin(),std::bind2nd(std::plus<T>(),t));
      return *this;
    }

    /// substacts a scalar from each element (inplace)
    inline DynMatrix &operator-=(const T &t){
      std::transform(begin(),end(),begin(),std::bind2nd(std::minus<T>(),t));
      return *this;
    }

    /// Matrix addition
    inline DynMatrix operator+(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      if(cols() != m.cols() || rows() != m.rows()) throw IncompatibleMatrixDimensionException("A+B size(A) must be size(B)");
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),m.begin(),d.begin(),std::plus<T>());
      return d;
    }

    /// Matrix substraction
    inline DynMatrix operator-(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      if(cols() != m.cols() || rows() != m.rows()) throw IncompatibleMatrixDimensionException("A+B size(A) must be size(B)");
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),m.begin(),d.begin(),std::minus<T>());
      return d;
    }

    /// Matrix addition (inplace)
    inline DynMatrix &operator+=(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      if(cols() != m.cols() || rows() != m.rows()) throw IncompatibleMatrixDimensionException("A+B size(A) must be size(B)");
      std::transform(begin(),end(),m.begin(),begin(),std::plus<T>());
      return *this;
    }

    /// Matrix substraction (inplace)
    inline DynMatrix &operator-=(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      if(cols() != m.cols() || rows() != m.rows()) throw IncompatibleMatrixDimensionException("A+B size(A) must be size(B)");
      std::transform(begin(),end(),m.begin(),begin(),std::minus<T>());
      return *this;
    }
  
    /// element access operator (x,y)-access index begin 0!
    inline T &operator()(unsigned int col,unsigned int row){
#ifdef DYN_MATRIX_INDEX_CHECK
      if((int)col >= m_cols || (int)row >= m_rows) ERROR_LOG("access to "<<m_cols<<'x'<<m_rows<<"-matrix index (" << col << "," << row << ")");
#endif
      return m_data[col+cols()*row];
    }

    /// element access operator (x,y)-access index begin 0! (const)
    inline const T &operator() (unsigned int col,unsigned int row) const{
#ifdef DYN_MATRIX_INDEX_CHECK
      if((int)col >= m_cols || (int)row >= m_rows) ERROR_LOG("access to "<<m_cols<<'x'<<m_rows<<"-matrix index (" << col << "," << row << ")");
#endif
      return m_data[col+cols()*row];
    }

    /// element access with index check
    inline T &at(unsigned int col,unsigned int row) throw (InvalidIndexException){
      if(col>=cols() || row >= rows()) throw InvalidIndexException("row or col index too large");
      return m_data[col+cols()*row];
    }

    /// element access with index check (const)
    inline const T &at(unsigned int col,unsigned int row) const throw (InvalidIndexException){
      return const_cast<DynMatrix*>(this)->at(col,row);
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
    
    /// applies an L_l norm on the matrix elements (all elements are treated as vector)
    inline T norm(double l=2) const{
      double accu = 0;
      for(unsigned int i=0;i<dim();++i){
        accu += ::pow(double(m_data[i]),l);
      }
      return ::pow(double(accu),1.0/l);
    }

    

    /// default iterator type (just a data-pointer) 
    typedef T* iterator;
    
    /// dafault const_iterator type (just a data-pointer)
    typedef const T* const_iterator;

    /// comples row_iterator type
    typedef T* row_iterator;
    
    /// complex const_row_iterator type
    typedef const T* const_row_iterator;

    /// height of the matrix (number of rows)
    unsigned int rows() const { return m_rows; }
  
    /// width of the matrix (number of columns)
    unsigned int cols() const { return m_cols; }

    /// internal data pointer
    T *data() { return m_data; }

    /// internal data pointer (const)
    const T *data() const { return m_data; }

    /// matrix dimension (width*height) or (cols*rows)
    unsigned int dim() const { return m_rows*m_cols; }

    /// returns sizeof (T)*dim()
    int stride0() const { return sizeof(T) * dim(); }
    
    /// returns sizeof(T)*cols()
    int stride1() const { return sizeof(T) * cols(); }

    /// returns sizeof (T)
    int stride2() const { return sizeof(T); }
  
    /// Internal column iterator struct (using height-stride) \ingroup LINALG
    struct col_iterator : public std::iterator<std::random_access_iterator_tag,T>{
      typedef unsigned int difference_type;
      mutable T *p;
      unsigned int stride;
      inline col_iterator(T *col_begin,unsigned int stride):p(col_begin),stride(stride){}


    /// prefix increment operator
      inline col_iterator &operator++(){
        p+=stride;
        return *this;
      }
      /// prefix increment operator (const)
      inline const col_iterator &operator++() const{
        p+=stride;
        return *this;
      }
      /// postfix increment operator
      inline col_iterator operator++(int){
        col_iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      /// postfix increment operator (const)
      inline const col_iterator operator++(int) const{
        col_iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      /// prefix decrement operator
      inline col_iterator &operator--(){
        p-=stride;
        return *this;
      }

      /// prefix decrement operator (const)
      inline const col_iterator &operator--() const{
        p-=stride;
        return *this;
      }

      /// postfix decrement operator
      inline col_iterator operator--(int){
        col_iterator tmp = *this;
        --(*this);
        return tmp;
      }

      /// postfix decrement operator (const)
      inline const col_iterator operator--(int) const{
        col_iterator tmp = *this;
        --(*this);
        return tmp;
      }

      /// jump next n elements (inplace)
      inline col_iterator &operator+=(difference_type n){
        p += n * stride;
        return *this;
      }

      /// jump next n elements (inplace) (const)
      inline const col_iterator &operator+=(difference_type n) const{
        p += n * stride;
        return *this;
      }


      /// jump backward next n elements (inplace)
      inline col_iterator &operator-=(difference_type n){
        p -= n * stride;
        return *this;
      }

      /// jump backward next n elements (inplace) (const)
      inline const col_iterator &operator-=(difference_type n) const{
        p -= n * stride;
        return *this;
      }


      /// jump next n elements
      inline col_iterator operator+(difference_type n) {
        col_iterator tmp = *this;
        tmp+=n;
        return tmp;
      }

      /// jump next n elements (const)
      inline const col_iterator operator+(difference_type n) const{
        col_iterator tmp = *this;
        tmp+=n;
        return tmp;
      }

      /// jump backward next n elements
      inline col_iterator operator-(difference_type n) {
        col_iterator tmp = *this;
        tmp-=n;
        return tmp;
      }

      /// jump backward next n elements (const)
      inline const col_iterator operator-(difference_type n) const {
        col_iterator tmp = *this;
        tmp-=n;
        return tmp;
      }

      inline difference_type operator-(const col_iterator &other) const{
        return (p-other.p)/stride;
      }

      
      /// Dereference operator
      inline T &operator*(){
        return *p;
      }

      /// const Dereference operator
      inline T operator*() const{
        return *p;
      }

      /// comparison operator ==
      inline bool operator==(const col_iterator &i) const{ return p == i.p; }

      /// comparison operator !=
      inline bool operator!=(const col_iterator &i) const{ return p != i.p; }

      /// comparison operator <
      inline bool operator<(const col_iterator &i) const{ return p < i.p; }

      /// comparison operator <=
      inline bool operator<=(const col_iterator &i) const{ return p <= i.p; }

      /// comparison operator >=
      inline bool operator>=(const col_iterator &i) const{ return p >= i.p; }

      /// comparison operator >
      inline bool operator>(const col_iterator &i) const{ return p > i.p; }
    };
    
    /// const column iterator typedef
    typedef const col_iterator const_col_iterator;

    /// Internally used Utility structure referencing a matrix column shallowly
    struct DynMatrixColumn{
#ifdef DYN_MATRIX_INDEX_CHECK
#define DYN_MATRIX_COLUMN_CHECK(C,E) if(C) ERROR_LOG(E)
#else
#define DYN_MATRIX_COLUMN_CHECK(C,E)
#endif
      /// Matrix reference
      DynMatrix<T> *matrix;
      
      /// referenced column in matrix
      unsigned int column;
      
      /// create from source matrix and column index
      inline DynMatrixColumn(const DynMatrix<T> *matrix, unsigned int column):
      matrix(const_cast<DynMatrix<T>*>(matrix)),column(column){
        DYN_MATRIX_COLUMN_CHECK(column >= matrix->cols(),"invalid column index");
      }
      
      /// Create from source matrix (only works if matrix has only single column = column-vector)
      inline DynMatrixColumn(const DynMatrix<T> &matrix):
      matrix(const_cast<DynMatrix<T>*>(&matrix)),column(0){
        DYN_MATRIX_COLUMN_CHECK(matrix->cols() != 1,"source matrix must have exactly ONE column");
      }
      /// Shallow copy from another matrix column reference
      inline DynMatrixColumn(const DynMatrixColumn &c):
      matrix(c.matrix),column(c.column){}
      
      /// returns column begin
      inline col_iterator begin() { return matrix->col_begin(column); }
      
      /// returns column end
      inline col_iterator end() { return matrix->col_end(column); }

      /// returns column begin (const)
      inline const col_iterator begin() const { return matrix->col_begin(column); }

      /// returns column end (const)
      inline const col_iterator end() const { return matrix->col_end(column); }

      /// returns column length (matrix->rows())
      inline unsigned int dim() const { return matrix->rows(); }
      
      /// assignment by another column
      inline DynMatrixColumn &operator=(const DynMatrixColumn &c){
        DYN_MATRIX_COLUMN_CHECK(dim() != c.dim(),"dimension missmatch");
        std::copy(c.begin(),c.end(),begin());
        return *this;
      }
        
      /// assigne dyn matrix to matrix columns
      inline DynMatrixColumn &operator=(const DynMatrix &src){
        DYN_MATRIX_COLUMN_CHECK(dim() != src.dim(),"dimension missmatch");
        std::copy(src.begin(),src.end(),begin());
        return *this;
      }   

      /// operator += for other columns
      inline DynMatrixColumn &operator+=(const DynMatrixColumn &c){
        DYN_MATRIX_COLUMN_CHECK(dim() != c.dim(),"dimension missmatch");
        std::transform(c.begin(),c.end(),begin(),begin(),std::plus<T>());
        return *this;
      }   
      
      /// operator += for other columns
      inline DynMatrixColumn &operator-=(const DynMatrixColumn &c){
        DYN_MATRIX_COLUMN_CHECK(dim() != c.dim(),"dimension missmatch");
        std::transform(c.begin(),c.end(),begin(),begin(),std::minus<T>());
        return *this;
      }   

      /// operator += for DynMatrices
      inline DynMatrixColumn &operator+=(const DynMatrix &m){
        DYN_MATRIX_COLUMN_CHECK(dim() != m.dim(),"dimension missmatch");
        std::transform(m.begin(),m.end(),begin(),begin(),std::plus<T>());
        return *this;
      }   
      /// operator -= for DynMatrices
      inline DynMatrixColumn &operator-=(const DynMatrix &m){
        DYN_MATRIX_COLUMN_CHECK(dim() != m.dim(),"dimension missmatch");
        std::transform(m.begin(),m.end(),begin(),begin(),std::minus<T>());
        return *this;
      }   
      
      /// operator *= for scalars
      inline DynMatrixColumn &operator*=(const T&val){
        std::for_each(begin(),end(),std::bind2nd(std::multiplies<T>(),val));
        return *this;
      }
      /// operator /= for scalars
      inline DynMatrixColumn &operator/=(const T&val){
        std::for_each(begin(),end(),std::bind2nd(std::divides<T>(),val));
        return *this;
      }
      
    };

    inline DynMatrix &operator=(const DynMatrixColumn &col){
      DYN_MATRIX_COLUMN_CHECK(dim() != col.dim(),"dimension missmatch");
      std::copy(col.begin(),col.end(),begin());
      return *this;
    }

#undef DYN_MATRIX_COLUMN_CHECK
    
    

    /// returns an iterator to the begin of internal data array
    inline iterator begin() { return m_data; }

    /// returns an iterator to the end of internal data array
    inline iterator end() { return m_data+dim(); }
  
    /// returns an iterator to the begin of internal data array (const)
    inline const_iterator begin() const { return m_data; }

    /// returns an iterator to the end of internal data array (const)
    inline const_iterator end() const { return m_data+dim(); }
  
    /// returns an iterator running through a certain matrix column 
    inline col_iterator col_begin(unsigned int col) {       
      col_check(col);
      return col_iterator(m_data+col,cols()); 
    }

    /// returns an iterator end of a certain matrix column 
    inline col_iterator col_end(unsigned int col) {
      col_check(col);
      return col_iterator(m_data+col+dim(),cols()); 
    }
  
    /// returns an iterator running through a certain matrix column (const)
    inline const_col_iterator col_begin(unsigned int col) const { 
      col_check(col);
      return col_iterator(m_data+col,cols()); 
    }

    /// returns an iterator end of a certain matrix column (const)
    inline const_col_iterator col_end(unsigned int col) const { 
      col_check(col);
      return col_iterator(m_data+col+dim(),cols()); 
    }

    /// returns an iterator running through a certain matrix row 
    inline row_iterator row_begin(unsigned int row) { 
      row_check(row);
      return m_data+row*cols(); 
    }

    /// returns an iterator of a certains row's end
    inline row_iterator row_end(unsigned int row) { 
      row_check(row);
      return m_data+(row+1)*cols(); 
    }

    /// returns an iterator running through a certain matrix row  (const)
    inline const_row_iterator row_begin(unsigned int row) const { 
      row_check(row);
      return m_data+row*cols(); 
    }

    /// returns an iterator of a certains row's end (const)
    inline const_row_iterator row_end(unsigned int row) const {
      row_check(row);
      return m_data+(row+1)*cols(); 
    }
    
    /// Extracts a shallow copied matrix row
    inline DynMatrix row(int row){
      row_check(row);
      return DynMatrix(m_cols,1,row_begin(row),false);
    }

    /// Extracts a shallow copied matrix row (const)
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

    /// applies QR-decomposition using stabilized Gram-Schmidt orthonormalization (only for icl32f and icl64f)
    void decompose_QR(DynMatrix &Q, DynMatrix &R) const;
    
    /// applies RQ-decomposition (by exploiting implemnetation of QR-decomposition) (only for icl32f, and icl64f)
    void decompose_RQ(DynMatrix &R, DynMatrix &Q) const;
    
    /// applies LU-decomposition (without using partial pivoting) (only for icl32f and icl64f)
    /** Even though, implementation also works for non-sqared matrices, it's not recommended to
        apply this function on non-sqared matrices */
    void decompose_LU(DynMatrix &L, DynMatrix &U, T zeroThreshold=1E-16) const;
    
    /// solves Mx=b for M=*this (only if M is a squared upper triangular matrix) (only for icl32f and icl64f)
    DynMatrix solve_upper_triangular(const DynMatrix &b) const throw(InvalidMatrixDimensionException);

    /// solves Mx=b for M=*this (only if M is a squared lower triangular matrix) (only for icl32f and icl64f)
    DynMatrix solve_lower_triangular(const DynMatrix &b) const throw(InvalidMatrixDimensionException);
    
    /// solves Mx=b for M=*this (only for icl32f and icl64f)
    /** solves Mx=b using one of the following algorithms
        @param method "lu" (default) using LU-decomposition
                      "svd" (using svd-based pseudo-inverse)
                      "qr" (using QR-decomposition based pseudo-inverse)
                      "inv" (using matrix inverse)
        
        \section BENCHM Benchmarks
        While LU decomposition based solving provides the worst results, it is also
        the fastest method in general. Only in case of having very small matrices (e.g. 4x4),
        other methods are faster. A double precision random N by N system is solved up to 
        an accuracy of about 10e-5 if LU decomposition is used. All other methods provide
        accuracies of about 10e-14 in case of double precision.
        
        Here are some benchmarks for double precision:
        * 10.000 times 4x4 matrix:
          * inv 16.2 ms 
          * lu 26.7 ms
          * qr 105 ms
          * svd 142 ms
        * 10.000 times 5x5 matrix:
          * inv 20.2 ms 
          * lu 30.2 ms
          * qr 148 ms
          * svd 131 ms
        * 10.000 times 6x6 matrix:
          * inv 26.9 ms 
          * lu 35.6 ms
          * qr 206 ms
          * svd 192 ms
        * 10.000 times 4x4 matrix:
          * inv 448 ms 
          * lu 42 ms
          * qr 642 ms
          * svd 237 ms
        * 10.000 times 10x10 matrix:
          * inv 2200 ms 
          * lu 75 ms
          * qr 3000 ms
          * svd 495 ms
        * 10 times 50x50 matrix: <b>note: here we have inv and qr in seconds and only 10 trials!</b>
          * inv 5.7 s
          * lu 2.5 ms
          * qr 4.6 s
          * svd 23.4 ms
    */
    DynMatrix solve(const DynMatrix &b, const std::string &method="lu",T zeroThreshold=1E-16) 
      throw(InvalidMatrixDimensionException,  ICLException, SingularMatrixException);
    

    /// invert the matrix (only for icl32f and icl64f)
    DynMatrix inv() const throw (InvalidMatrixDimensionException,SingularMatrixException);
    
    /// Extracts the matrix's eigenvalues and eigenvectors
    /** This function only works on squared symmetric matrices. 
        Resulting eigenvalues are ordered in descending order. The destination matrices' sizes are adapted automatically.

        The function is only available for icl32f and icl64f and it is IPP-accelerated in case of having Intel-IPP-Support.
        The Fallback implementation was basically taken from the Visualization Toolkit VTK (Version 5.6.0)

        Note: There is no internal check if the matrix is really symmetric. If it is not symmetric, the behaviour of 
              this function is not predictable
        
        @param eigenvectors contains the resulting eigenvectors in it's columns
        @param eigenvalues becomes a N-dimensional column vector which ith element is the eigenvalue that corresponds
                           to the ith column of eigenvectors 
    */
    void eigen(DynMatrix &eigenvectors, DynMatrix &eigenvalues) const throw(InvalidMatrixDimensionException, ICLException);
    
    /// Computes Singular Value Decomposition of a matrix - decomposes A into USV'
    /** Internaly, this function will always use double values. Other types are converted internally. 
        This funciton is only instantiated for icl32f and icl64f. 
        @param U is filled column-wise with the eigenvectors of AA'
        @param S is filled with the singular values of A (s is a ColumnVector and not diagonal matrix)
        @param V is filled column-wise with the eigenvectors of A'A (in V, V is stored not V')
        @see icl::svd_dyn 
    */
    void svd(DynMatrix &U, DynMatrix &s,  DynMatrix &V) const throw (ICLException);

    /// calculates the Moore-Penrose pseudo-inverse (only implemented for icl32f and icl64f)
    /** Internally, this functions can use either a QR-decomposition based approach, or it can use
        SVD. 
        QR-Decomposition is already much more stable than
        the naiv approach pinv(X) = X*(X*X')^(-1)
        \code
        DynMatrix Q,R;
        decompose_QR(Q,R);
        return R.inv() * Q.transp();
        \endcode
        The QR-decomposition based approach does not use the zeroThreshold variable.
        
        If useSVD is set to true, internally an SVD based approach is used:

        <code>
        DynMatrix S,v,D;
        svd_dyn(*this,U,s,V);
        
        DynMatrix S(s.rows(),s.rows(),0.0f);
        for(unsigned int i=0;i<s.rows();++i){
          S(i,i) = (fabs(s[i]) > zeroThreshold) ? 1.0/s[i] : 0; 
        }
        return V * S * U.transp();
        </code>
        
    */
    DynMatrix pinv(bool useSVD=false, T zeroThreshold=1E-16) const 
      throw (InvalidMatrixDimensionException,SingularMatrixException,ICLException);


    /// matrix determinant (only for icl32f and icl64f)
    T det() const throw (InvalidMatrixDimensionException);

    /// matrix transposed
    inline DynMatrix transp() const{
      DynMatrix d(rows(),cols());
      for(unsigned int x=0;x<cols();++x){
        for(unsigned int y=0;y<rows();++y){
          d(y,x) = (*this)(x,y);
        }
      }
      return d;
    }

    /// inner product of data pointers (not matrix-mulitiplication)
    /** computes the inner-product of data vectors */
    T element_wise_inner_product(const DynMatrix<T> &other) const {
      return std::inner_product(begin(),end(),other.begin(),T(0));
    }


    /// returns the inner product of two matrices (i.e. dot-product)
    /** A.dot(B) is equivalent to A.transp() * B 
        TODO: optimize implementation (current implementation _is_ A.transp() * B)
    */
    DynMatrix<T> dot(const DynMatrix<T> &M) const throw(InvalidMatrixDimensionException){
      return this->transp() * M;
    }
    
    
    /// returns diagonal-elements as column-vector
    DynMatrix<T> diag() const{
      ICLASSERT_RETURN_VAL(cols()==rows(),DynMatrix<T>());
      DynMatrix<T> d(1,rows());
      for(int i=0;i<rows();++i){
        d[i] = (*this)(i,i);
      }
      return d;
    }
    
    /// computes the sum of all diagonal elements
    T trace() const{
      ICLASSERT_RETURN_VAL(cols()==rows(),0);
      double accu = 0;
      for(int i=0;i<dim();i+=cols()+1){
        accu += m_data[i];
      }
      return accu;
    }
    
    /// sets new data internally and returns old data pointer (for experts only!)
    inline T *set_data(T *newData){
      T *old_data = m_data;
      m_data = newData;
      return old_data;
    }
    
    /// creates a dim-D identity Matrix
    static inline DynMatrix id(unsigned int dim) {
      DynMatrix M(dim,dim);
      for(unsigned int i=0;i<dim;++i) M(i,i) = 1;
      return M;
    }
    
  private:
    inline void row_check(unsigned int row) const{
#ifdef DYN_MATRIX_INDEX_CHECK
      if((int)row >= m_rows) ERROR_LOG("access to row index " << row << " on a "<<m_cols<<'x'<<m_rows<<"-matrix");
#else
      (void)row;
#endif
    }
    inline void col_check(unsigned int col) const{
#ifdef DYN_MATRIX_INDEX_CHECK
      if((int)col >= m_cols) ERROR_LOG("access to column index " << col << " on a "<<m_cols<<'x'<<m_rows<<"-matrix");
#else
      (void)col;
#endif
    }
    inline void idx_check(unsigned int col, unsigned int row) const{
      col_check(col);
      row_check(row);
    }
    
    inline void idx_check(unsigned int idx) const{
#ifdef DYN_MATRIX_INDEX_CHECK
      if(idx >= dim()) ERROR_LOG("access to linear index " << idx << " on a "<<m_cols<<'x'<<m_rows<<"-matrix");
#else
      (void)idx;
#endif
    }

    int m_rows;
    int m_cols;
    T *m_data;
    bool m_ownData;
  };

  /// creates a dyn-matrix from given matrix column
  template<class T>
  DynMatrix<T>::DynMatrix(const DynMatrix<T>::DynMatrixColumn &column):
  m_rows(column.dim()),m_cols(1),m_data(new T[column.dim()]),m_ownData(true){
    std::copy(column.begin(),column.end(),begin());
  }

  /// ostream operator implemented for uchar, short, int, float and double matrices  \ingroup LINALG
  template<class T>
  std::ostream &operator<<(std::ostream &s,const DynMatrix<T> &m);

  /// istream operator implemented for uchar, short, int, float and double matrices  \ingroup LINALG
  template<class T>
  std::istream &operator>>(std::istream &s,DynMatrix<T> &m);


#ifdef HAVE_IPP
  /** \cond */
  
#define DYN_MATRIX_MULT_SPECIALIZE(IPPT)                                                                \
  template<>	    							                                \
    inline DynMatrix<Ipp##IPPT> &DynMatrix<Ipp##IPPT>::mult(                                            \
      const DynMatrix<Ipp##IPPT> &m, DynMatrix<Ipp##IPPT> &dst) const	                                \
    throw (IncompatibleMatrixDimensionException){			                                \
    if(m.rows() != cols()) throw IncompatibleMatrixDimensionException("A*B : rows(A) must be cols(B)");	\
    dst.setBounds(m.cols(),rows());					                                \
    ippmMul_mm_##IPPT(data(),sizeof(Ipp##IPPT)*cols(),sizeof(Ipp##IPPT),cols(),rows(),                  \
		      m.data(),sizeof(Ipp##IPPT)*m.cols(),sizeof(Ipp##IPPT),m.cols(),m.rows(),          \
		      dst.data(),m.cols()*sizeof(Ipp##IPPT),sizeof(Ipp##IPPT));	                        \
    return dst;					\
  }

  DYN_MATRIX_MULT_SPECIALIZE(32f)
  DYN_MATRIX_MULT_SPECIALIZE(64f)
#undef DYN_MATRIX_MULT_SPECIALIZE


#define DYN_MATRIX_ELEMENT_WISE_DIV_SPECIALIZE(IPPT)	                      \
    template<>								      \
    inline DynMatrix<Ipp##IPPT> &DynMatrix<Ipp##IPPT>::elementwise_div(      \
          const DynMatrix<Ipp##IPPT> &m, DynMatrix<Ipp##IPPT> &dst) const     \
      throw (IncompatibleMatrixDimensionException){                           \
    if((m.cols() != cols()) || (m.rows() != rows())){                         \
      throw IncompatibleMatrixDimensionException("A./B dimension mismatch");  \
    }                                                                         \
    dst.setBounds(cols(),rows());                                             \
    ippsDiv_##IPPT(data(),m.data(),dst.data(),dim());                         \
    return dst;                                                               \
  }

  DYN_MATRIX_ELEMENT_WISE_DIV_SPECIALIZE(32f)
  DYN_MATRIX_ELEMENT_WISE_DIV_SPECIALIZE(64f)

#undef DYN_MATRIX_ELEMENT_WISE_DIV_SPECIALIZE






#define DYN_MATRIX_MULT_BY_CONSTANT(IPPT)	             \
    template<>						     \
    inline DynMatrix<Ipp##IPPT> &DynMatrix<Ipp##IPPT>::mult( \
      Ipp##IPPT f, DynMatrix<Ipp##IPPT> &dst) const{         \
    dst.setBounds(cols(),rows());                            \
    ippsMulC_##IPPT(data(),f, dst.data(),dim());             \
    return dst;                                              \
  }

DYN_MATRIX_MULT_BY_CONSTANT(32f)
DYN_MATRIX_MULT_BY_CONSTANT(64f)

#undef DYN_MATRIX_MULT_BY_CONSTANT
 
#define DYN_MATRIX_NORM_SPECIALZE(T,IPPT)                  \
  template<>                                               \
  inline T DynMatrix<T> ::norm(double l) const{            \
    if(l==1){                                              \
      T val;                                               \
      ippsNorm_L1_##IPPT(m_data,dim(),&val);               \
      return val;                                          \
    }else if(l==2){                                        \
      T val;                                               \
      ippsNorm_L2_##IPPT(m_data,dim(),&val);               \
      return val;                                          \
    }                                                      \
    double accu = 0;                                       \
    for(unsigned int i=0;i<dim();++i){                     \
      accu += ::pow(double(m_data[i]),l);                  \
    }                                                      \
    return ::pow(accu,1.0/l);                              \
  }
 
 DYN_MATRIX_NORM_SPECIALZE(float,32f)
 // DYN_MATRIX_NORM_SPECIALZE(double,64f)
 
#undef DYN_MATRIX_NORM_SPECIALZE

 /** \endcond */

#endif // HAVE_IPP

}

#endif
