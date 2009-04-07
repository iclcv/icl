#ifndef ICL_DYN_MATRIX_H
#define ICL_DYN_MATRIX_H
#include <iterator>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iostream>
#include <vector>
#include <cmath>

#include <iclException.h>
#include <iclMacros.h>

namespace icl{
  struct InvalidMatrixDimensionException :public ICLException{
    InvalidMatrixDimensionException(const std::string &msg):ICLException(msg){}
  };
  struct IncompatibleMatrixDimensionException :public ICLException{
    IncompatibleMatrixDimensionException(const std::string &msg):ICLException(msg){}
  };
  struct  InvalidIndexException : public ICLException{
    InvalidIndexException(const std::string &msg):ICLException(msg){}
  };
  struct SingularMatrixException : public ICLException{
    SingularMatrixException(const std::string &msg):ICLException(msg){}
  };
  struct QRDecompException : public ICLException{
    QRDecompException(const std::string &msg):ICLException(msg){}
  };
  
  
  template<class T>
  struct DynMatrix{
    
    /// Default empty constructor creates a null-matrix
    DynMatrix():m_rows(0),m_cols(0),m_ownData(true),m_data(0){}
    
    /// Create a dyn matrix with given dimensions (and optional initialValue)
    DynMatrix(unsigned int cols,unsigned int rows,const  T &initValue=0) throw (InvalidMatrixDimensionException) : 
    m_rows(rows),m_cols(cols),m_ownData(true){
      if(!dim()) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      m_data = new T[cols*rows];
      std::fill(begin(),end(),initValue);
    }

    /// Create a matrix with given data
    /** Data can be wrapped deeply or shallowly. If the latter is true, given data pointer
        will not be released in the destructor*/
    DynMatrix(unsigned int cols,unsigned int rows, T *data, bool deepCopy=true) throw (InvalidMatrixDimensionException) : 
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
    DynMatrix(unsigned int cols,unsigned int rows,const T *data) throw (InvalidMatrixDimensionException) : 
      m_rows(rows),m_cols(cols),m_ownData(true){
      if(!dim()) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      m_data = new T[dim()];
      std::copy(data,data+dim(),begin());
    }

    /// Default copy constructor
    DynMatrix(const DynMatrix &other):
    m_rows(other.m_rows),m_cols(other.m_cols),m_data(new T[dim()]),m_ownData(true){
      std::copy(other.begin(),other.end(),begin());
    }
    
    /// returns with this matrix has a valid data pointer
    bool isNull() const { return !m_data; }

    /// Destructor (deletes data if no wrapped shallowly)
    ~DynMatrix(){
      if(m_data && m_ownData) delete [] m_data;
    }

    /// Assignment operator (using deep-copy)
    DynMatrix &operator=(const DynMatrix &other){
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
    void setBounds(unsigned int cols, unsigned int rows, bool holdContent=false, const T &initializer=0) throw (InvalidMatrixDimensionException){
      if(cols == m_cols && rows==m_rows) return;
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
      ICL_DELETE(m_data);
      m_data = M.begin();
      M.set_data(0);
    }
  
  
    /// Multiply elements with scalar
    DynMatrix operator*(T f) const{
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::multiplies<T>(),f));
      return d;
    }

    /// Multiply elements with scalar (inplace)
    DynMatrix &operator*=(T f){
      std::transform(begin(),end(),begin(),std::bind2nd(std::multiplies<T>(),f));
      return *this;
    }

    /// Device elements by scalar
    DynMatrix operator/(T f) const{
      return this->operator*(1/f);
    }

    /// Device elements by scalar (inplace)
    DynMatrix &operator/=(T f){
      return this->operator*=(1/f);
    }

  
    /// Essential matrix multiplication
    DynMatrix operator*(const DynMatrix &m) const throw (IncompatibleMatrixDimensionException){
      if(m.rows() != cols()) throw IncompatibleMatrixDimensionException("A*B : rows(A) must be cols(B)");
      DynMatrix d(m.cols(),rows());
      for(unsigned int c=0;c<d.cols();++c){
        for(unsigned int r=0;r<d.rows();++r){
          d(c,r) = std::inner_product(row_begin(r),row_end(r),m.col_begin(c),T(0));
        }
      }
      return d;
    }
    
    /// inplace matrix multiplication applying this = this*m
    DynMatrix &operator*=(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      return *this=((*this)*m);
    }
    
    /// inplace matrix devision (calling this/m.inv())
    DynMatrix operator/(const DynMatrix &m) const 
      throw (IncompatibleMatrixDimensionException,
             InvalidMatrixDimensionException,
             SingularMatrixException){
      return this->operator*(m.inv());
    }

    /// inplace matrix devision (calling this/m.inv()) (inplace)
    DynMatrix &operator/=(const DynMatrix &m) const 
      throw (IncompatibleMatrixDimensionException,
             InvalidMatrixDimensionException,
             SingularMatrixException){
      return *this = this->operator*(m.inv());
    }

    /// adds a scalar to each element
    DynMatrix operator+(const T &t){
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::plus<T>(),t));
      return d;
    }

    /// substacts a scalar from each element
    DynMatrix operator-(const T &t){
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::minus<T>(),t));
      return d;
    }

    /// adds a scalar to each element (inplace)
    DynMatrix &operator+=(const T &t){
      std::transform(begin(),end(),begin(),std::bind2nd(std::plus<T>(),t));
      return *this;
    }

    /// substacts a scalar from each element (inplace)
    DynMatrix &operator-=(const T &t){
      std::transform(begin(),end(),begin(),std::bind2nd(std::minus<T>(),t));
      return *this;
    }

    /// Matrix addition
    DynMatrix operator+(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      if(cols() != m.cols() || rows() != m.rows()) throw IncompatibleMatrixDimensionException("A+B size(A) must be size(B)");
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),m.begin(),d.begin(),std::plus<T>());
      return d;
    }

    /// Matrix substraction
    DynMatrix operator-(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      if(cols() != m.cols() || rows() != m.rows()) throw IncompatibleMatrixDimensionException("A+B size(A) must be size(B)");
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),m.begin(),d.begin(),std::minus<T>());
      return d;
    }

    /// Matrix addition (inplace)
    DynMatrix &operator+=(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      if(cols() != m.cols() || rows() != m.rows()) throw IncompatibleMatrixDimensionException("A+B size(A) must be size(B)");
      std::transform(begin(),end(),m.begin(),begin(),std::plus<T>());
      return *this;
    }

    /// Matrix substraction (inplace)
    DynMatrix &operator-=(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      if(cols() != m.cols() || rows() != m.rows()) throw IncompatibleMatrixDimensionException("A+B size(A) must be size(B)");
      std::transform(begin(),end(),m.begin(),begin(),std::minus<T>());
      return *this;
    }
  
    /// element access operator (x,y)-access index begin 0!
    T &operator()(unsigned int col,unsigned int row){
#ifdef DYN_MATRIX_INDEX_CHECK
      if(col >= m_cols || row >= m_rows) ERROR_LOG("access to "<<m_cols<<'x'<<m_rows<<"-matrix index (" << col << "," << row << ")");
#endif
      return m_data[col+cols()*row];
    }

    /// element access operator (x,y)-access index begin 0! (const)
    const T &operator() (unsigned int col,unsigned int row) const{
#ifdef DYN_MATRIX_INDEX_CHECK
      if(col >= m_cols || row >= m_rows) ERROR_LOG("access to "<<m_cols<<'x'<<m_rows<<"-matrix index (" << col << "," << row << ")");
#endif
      return m_data[col+cols()*row];
    }

    /// element access with index check
    T &at(unsigned int col,unsigned int row) throw (InvalidIndexException){
      if(col>=cols() || row >= rows()) throw InvalidIndexException("row or col index too large");
      return m_data[col+cols()*row];
    }

    /// element access with index check (const)
    const T &at(unsigned int col,unsigned int row) const throw (InvalidIndexException){
      return const_cast<DynMatrix*>(this)->at(col,row);
    }
    

    
    /// linear access to actual data array
    T &operator[](unsigned int idx) { 
      idx_check(idx);
      if(idx >= dim()) ERROR_LOG("access to "<<m_cols<<'x'<<m_rows<<"-matrix index [" << idx<< "]");

      return m_data[idx]; 
      
    }
    

    /// linear access to actual data array (const)
    const T &operator[](unsigned int idx) const { 
      idx_check(idx);
      return m_data[idx]; 
    }
    
    /// applies an L_l norm on the matrix elements (all elements are treated as vector)
    T norm(double l=2) const{
      double accu = 0;
      for(int i=0;i<dim();++i){
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
  
    /// Internal column iterator struct (using height-stride)
    struct col_iterator : public std::iterator<std::random_access_iterator_tag,T>{
      typedef unsigned int difference_type;
      mutable T *p;
      const unsigned int stride;
      col_iterator(T *col_begin,unsigned int stride):p(col_begin),stride(stride){}


    /// prefix increment operator
      col_iterator &operator++(){
        p+=stride;
        return *this;
      }
      /// prefix increment operator (const)
      const col_iterator &operator++() const{
        p+=stride;
        return *this;
      }
      /// postfix increment operator
      col_iterator operator++(int){
        col_iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      /// postfix increment operator (const)
      const col_iterator operator++(int) const{
        col_iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      /// prefix decrement operator
      col_iterator &operator--(){
        p-=stride;
        return *this;
      }

      /// prefix decrement operator (const)
      const col_iterator &operator--() const{
        p-=stride;
        return *this;
      }

      /// postfix decrement operator
      col_iterator operator--(int){
        col_iterator tmp = *this;
        --(*this);
        return tmp;
      }

      /// postfix decrement operator (const)
      const col_iterator operator--(int) const{
        col_iterator tmp = *this;
        --(*this);
        return tmp;
      }

      /// jump next n elements (inplace)
      col_iterator &operator+=(difference_type n){
        p += n * stride;
        return *this;
      }

      /// jump next n elements (inplace) (const)
      const col_iterator &operator+=(difference_type n) const{
        p += n * stride;
        return *this;
      }


      /// jump backward next n elements (inplace)
      col_iterator &operator-=(difference_type n){
        p -= n * stride;
        return *this;
      }

      /// jump backward next n elements (inplace) (const)
      const col_iterator &operator-=(difference_type n) const{
        p -= n * stride;
        return *this;
      }


      /// jump next n elements
      col_iterator operator+(difference_type n) {
        col_iterator tmp = *this;
        tmp+=n;
        return tmp;
      }

      /// jump next n elements (const)
      const col_iterator operator+(difference_type n) const{
        col_iterator tmp = *this;
        tmp+=n;
        return tmp;
      }

      /// jump backward next n elements
      col_iterator operator-(difference_type n) {
        col_iterator tmp = *this;
        tmp-=n;
        return tmp;
      }

      /// jump backward next n elements (const)
      const col_iterator operator-(difference_type n) const {
        col_iterator tmp = *this;
        tmp-=n;
        return tmp;
      }

      difference_type operator-(const col_iterator &other) const{
        return (p-other.p)/stride;
      }

      
      /// Dereference operator
      T &operator*(){
        return *p;
      }

      /// const Dereference operator
      T operator*() const{
        return *p;
      }

      /// comparison operator ==
      bool operator==(const col_iterator &i) const{ return p == i.p; }

      /// comparison operator !=
      bool operator!=(const col_iterator &i) const{ return p != i.p; }

      /// comparison operator <
      bool operator<(const col_iterator &i) const{ return p < i.p; }

      /// comparison operator <=
      bool operator<=(const col_iterator &i) const{ return p <= i.p; }

      /// comparison operator >=
      bool operator>=(const col_iterator &i) const{ return p >= i.p; }

      /// comparison operator >
      bool operator>(const col_iterator &i) const{ return p > i.p; }
    };
  
    /// const column iterator typedef
    typedef const col_iterator const_col_iterator;

    /// returns an iterator to the begin of internal data array
    iterator begin() { return m_data; }

    /// returns an iterator to the end of internal data array
    iterator end() { return m_data+dim(); }
  
    /// returns an iterator to the begin of internal data array (const)
    const_iterator begin() const { return m_data; }

    /// returns an iterator to the end of internal data array (const)
    const_iterator end() const { return m_data+dim(); }
  
    /// returns an iterator running through a certain matrix column 
    col_iterator col_begin(unsigned int col) {       
      col_check(col);
      return col_iterator(m_data+col,cols()); 
    }

    /// returns an iterator end of a certain matrix column 
    col_iterator col_end(unsigned int col) {
      col_check(col);
      return col_iterator(m_data+col+dim(),cols()); 
    }
  
    /// returns an iterator running through a certain matrix column (const)
    const_col_iterator col_begin(unsigned int col) const { 
      col_check(col);
      return col_iterator(m_data+col,cols()); 
    }

    /// returns an iterator end of a certain matrix column (const)
    const_col_iterator col_end(unsigned int col) const { 
      col_check(col);
      return col_iterator(m_data+col+dim(),cols()); 
    }

    /// returns an iterator running through a certain matrix row 
    row_iterator row_begin(unsigned int row) { 
      row_check(row);
      return m_data+row*cols(); 
    }

    /// returns an iterator of a certains row's end
    row_iterator row_end(unsigned int row) { 
      row_check(row);
      return m_data+(row+1)*cols(); 
    }

    /// returns an iterator running through a certain matrix row  (const)
    const_row_iterator row_begin(unsigned int row) const { 
      row_check(row);
      return m_data+row*cols(); 
    }

    /// returns an iterator of a certains row's end (const)
    const_row_iterator row_end(unsigned int row) const {
      row_check(row);
      return m_data+(row+1)*cols(); 
    }
    
    /// used to visualize matrix entries
    friend inline std::ostream &operator<<(std::ostream &s,const DynMatrix &m){
      for(unsigned int i=0;i<m.rows();++i){
        s << "| ";
        std::copy(m.row_begin(i),m.row_end(i),std::ostream_iterator<T>(s," "));
        s << "|" << std::endl;
      }
      return s;
    }
    
    /// Extracts a shallow copied matrix row
    DynMatrix row(int row){
      row_check(row);
      return DynMatrix(m_cols,1,row_begin(row),false);
    }

    /// Extracts a shallow copied matrix row (const)
    const DynMatrix row(int row) const{
      row_check(row);
      return DynMatrix(m_cols,1,const_cast<T*>(row_begin(row)),false);
    }

    /* here we need a helper struct!
        /// Extracts a shallow copied matrix column
        DynMatrix col(int col){
        col_check(col);
        return DynMatrix(1,m_rows,m_data+col,false);
        }
        
        /// Extracts a shallow copied matrix column (const)
        const DynMatrix col(int col) const{
        col_check(col);
        return DynMatrix(1,m_rows,const_cast<T*>(m_data+col),false);
        }
    */

    /// applies QR-decomposition (only for icl32f and icl64f)
    void decompose_QR(DynMatrix &Q, DynMatrix &R) const throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException);
    
    /// invert the matrix (only implemented with IPP_OPTIMIZATION and only for icl32f and icl64f)
    DynMatrix inv() const throw (InvalidMatrixDimensionException,SingularMatrixException);
    
    /// calculates the Moore-Penrose pseudo-inverse (only implemented with IPP_OPTIMIZATION and only for icl32f and icl64f)
    /** Internally, this functions uses an QR-decomposition based approach, which is much more stable than
        the naiv approach pinv(X) * X*(X*X')^(-1)
        \code
        DynMatrix Q,R;
        decompose_QR(Q,R);
        return R.inv() * Q.transp();
        \endcode
    */
    DynMatrix pinv() const throw (InvalidMatrixDimensionException,SingularMatrixException,QRDecompException);


    /// invert the matrix (only implemented with IPP_OPTIMIZATION and only for icl32f and icl64f)
    T det() const throw (InvalidMatrixDimensionException);

    /// matrix transposed
    DynMatrix transp() const{
      DynMatrix d(rows(),cols());
      for(unsigned int x=0;x<cols();++x){
        for(unsigned int y=0;y<rows();++y){
          d(y,x) = (*this)(x,y);
        }
      }
      return d;
    }
    
    /// sets new data internally and returns old data pointer (for experts only!)
    T *set_data(T *newData){
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
      if(row >= m_rows) ERROR_LOG("access to row index " << row << " on a "<<m_cols<<'x'<<m_rows<<"-matrix");
#else
      (void)row;
#endif
    }
    inline void col_check(unsigned int col) const{
#ifdef DYN_MATRIX_INDEX_CHECK
      if(col >= m_cols) ERROR_LOG("access to column index " << col << " on a "<<m_cols<<'x'<<m_rows<<"-matrix");
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

}

#endif
