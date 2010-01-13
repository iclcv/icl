#ifndef ICL_FIXED_MATRIX_H
#define ICL_FIXED_MATRIX_H
#include <iterator>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iostream>
#include <vector>
#include <cmath>

#include <ICLUtils/Exception.h>
#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLUtils/MatrixSubRectIterator.h>

#ifdef HAVE_IPP
#include <ippm.h>
#endif

namespace icl{

  /// FixedMatrix base struct defining datamode enum \ingroup LINALG
  struct FixedMatrixBase{
    /// Optimized copy function template (for N>30 using std::copy, otherwise a simple loop is used)
    template<class SrcIterator, class DstIterator, unsigned int N>
    static void optimized_copy(SrcIterator srcBegin, SrcIterator srcEnd, DstIterator dstBegin){
      if(N>30){
        std::copy(srcBegin,srcEnd,dstBegin);
      }else{
        while(srcBegin != srcEnd){
          //          *dstBegin++ = *srcBegin++;
          *dstBegin =
          *srcBegin;

          dstBegin++;
          srcBegin++;
        }
      }
    }
  };
    
  /** \cond */
  /// Forward Declaration fo FixedMatrixPart struct
  template<class T,unsigned int COLS,unsigned int ROWS> class FixedMatrix;
  /** \endcond */
  

  /// Utility struct for FixedMatrix sub-parts \ingroup LINALG
  /** FixedMatrix member functions row and col return an instance of this
      utility structure. FixedMatrixPart instances wrap a range of Iterators
      (begin- and end-Iterator) of template parameter type. Once created, 
      FixedMatrixParts can not be setup with different Iterators. All further 
      Assignments will only copy the ranged data represented by the source
      and destination iterator pairs.\\
      To avoid problems with ranges of different size, FixedMatrixPart instances
      are 'templated' to the range size (template parameter N)
  **/
  template<class T,unsigned int N, class Iterator>
  class FixedMatrixPart : public FixedMatrixBase{

    public:
    /// Start iterator 
    Iterator begin;
    
    /// End iterator
    Iterator end;
  
    /// Creates a new FixedMatrixPart instance with given Iterator Pair
    FixedMatrixPart(Iterator begin, Iterator end):begin(begin),end(end){}
    
    /// Assignment with a new value (all data in range will be assigned with that value)
    FixedMatrixPart& operator=(const T &value){
      std::fill(begin,end,value);
      return *this;
    }
    /// Assignment with another (identical) instance of FixedMatrixPart
    /** Internally std::copy is used */
    FixedMatrixPart& operator=(const FixedMatrixPart &other){
      FixedMatrixBase::optimized_copy<Iterator,Iterator,N>(other.begin,other.end,begin);
      return *this;
    }
    
    /// Assignment with another (compatible) instance of FixedMatrixPart
    /** For compatibility. Iterator type and destination type may differ but
        RangeSize must be equal */
    template<class OtherIterator, class OtherT>
    FixedMatrixPart& operator=(const FixedMatrixPart<OtherT,N,OtherIterator> &other){
      std::transform(other.begin,other.end,begin,clipped_cast<OtherT,T>);
      return *this;
    }
    
    /// Assignment with a compatible FixedMatrix instance (FixedMatrix DIM must be euqal to Part-size)
    /** DIM equality is forced by Argument template parameters <...,COLS,N/COLS> */
    template<unsigned int COLS>
    FixedMatrixPart& operator=(const FixedMatrix<T,COLS,N/COLS> &m);

    /// Assignment with a FixedMatrix instance (FixedMatrix DIM must be euqal to Part-size)
    /** DIM equality is forced by Argument template parameters <...,COLS,N/COLS> */
    template<class T2, unsigned int COLS>
    FixedMatrixPart& operator=(const FixedMatrix<T2,COLS,N/COLS> &m);
    
  };



  /// Powerful and highly flexible matrix class implementation \ingroup LINALG
  /** By using fixed template parameters as Matrix dimensions,
      specializations to e.g. row or column vectors, are also
      as performant as possible.
      
      \section PERF Performance
      Here are some benchmark results (measured on a 2GHz Core2Duo). All results are means of
      10000 measurements using float matrices.
      - <b>invert matrix: inv() </b>\n
        - 1000 x inv 2x2 matrix 74 ns   [optimized C++]
        - 1000 x inv 3x3 matrix 248 ns  [optimized C++]
        - 1000 x inv 4x4 matrix 721 ns  [optimized C++]
        - 1000 x inv 5x5 matrix 1.1 ms  [IPP only]

      - <b>matrix determinant: det() </b>\n
        - 1000 x det 2x2 matrix 11 ns   [optimized C++]
        - 1000 x det 3x3 matrix 48 ns   [optimized C++]
        - 1000 x det 4x4 matrix 138 ns  [optimized C++]
        - 1000 x det 5x5 matrix 604 ns  [IPP only]

      - <b>add two matrices using operator+: A+B </b>\n
        - 1000 x add 2x2 matrices 7 ns  [STL's transform template ]
        - 1000 x add 3x3 matrices 14 ns [STL's transform template ]
        - 1000 x add 4x4 matrices 21 ns [STL's transform template ]
        - 1000 x add 5x5 matrices 58 ns [STL's transform template ]

      - <b>multiply two matrices (matrix multiplication): A*B </b>\n
        - 1000 x multiply 2x2 matrices 55 ns [IPP if available]
        - 1000 x multiply 3x3 matrices 63 ns [IPP if available]
        - 1000 x multiply 4x4 matrices 79 ns [IPP if available]
        - 1000 x multiply 5x5 matrices 238 ns [generic C++ implementation for A*B]
  */
  template<class T,unsigned int COLS,unsigned int ROWS>
  class FixedMatrix : public FixedMatrixBase{
    public:

    /// returning a reference to a null matrix
    static const FixedMatrix &null(){
      static FixedMatrix null_matrix(T(0));
      return null_matrix;
    }

   /// count of matrix elements (COLS x ROWS)
    static const unsigned int DIM = COLS*ROWS;
    
    protected:
    
    /// internal data storage
    T m_data[COLS*ROWS];
    
    public:
    
    /// Default constructor 
    /** New data is created internally but elements are not initialized */
    FixedMatrix(){}
    
    /// Create Matrix and initialize elements with given value
    FixedMatrix(const T &initValue){
      std::fill(begin(),end(),initValue);
    }

    /// Create matrix with given data pointer (const version)
    /** As given data pointer is const, no shallow pointer copies are
        allowed here 
        @params srcdata const source data pointer copied deeply
    */
    FixedMatrix(const T *srcdata){
      FixedMatrixBase::optimized_copy<const T*,T*,DIM>(srcdata,srcdata+dim(),begin());
      //std::copy(srcdata,srcdata+dim(),begin());
    }

    /// Create matrix with given initializer elements (16 values max)
    /** default parameters for unnecessary parameters are not created when 
        compiled with -O4
    **/
    FixedMatrix(const T& v0,const T& v1,const T& v2=0,const T& v3=0,
                const T& v4=0,const T& v5=0,const T& v6=0,const T& v7=0,  
                const T& v8=0,const T& v9=0,const T& v10=0,const T& v11=0,  
                const T& v12=0,const T& v13=0,const T& v14=0,const T& v15=0){
#define C1(N) if(DIM>N) m_data[N]=v##N
#define C4(A,B,C,D) C1(A);C1(B);C1(C);C1(D)
      C4(0,1,2,3);C4(4,5,6,7);C4(8,9,10,11);C4(12,13,14,15);
#undef C1
#undef C2
    } 

    /// Range based constructor for STL compatiblitiy 
    /** Range size must be compatible to the new matrix's dimension */
    template<class OtherIterator>
    FixedMatrix(OtherIterator begin, OtherIterator end){
      FixedMatrixBase::optimized_copy<OtherIterator,T*,DIM>(begin,end,this->begin());
      //      std::copy(begin,end,begin());
    }

    // Explicit Copy template based constructor (deep copy)
    FixedMatrix(const FixedMatrix &other){
      FixedMatrixBase::optimized_copy<const T*,T*,DIM>(other.begin(),other.end(),begin());
      //std::copy(other.begin(),other.end(),begin());
    }

    // Explicit Copy template based constructor (deep copy)
    template<class otherT>
    FixedMatrix(const FixedMatrix<otherT,COLS,ROWS> &other){
      std::transform(other.begin(),other.end(),begin(),clipped_cast<otherT,T>);
    }

    /// Create matrix of a sub-part of another matrix (identical types)
    template<class Iterator>
    FixedMatrix (const FixedMatrixPart<T,DIM,Iterator> &r){ 
      FixedMatrixBase::optimized_copy<Iterator,T*,DIM>(r.begin,r.end,begin());
    }

    /// Create matrix of a sub-part of another matrix (compatible types)
    template<class otherT, class Iterator>
    FixedMatrix(const FixedMatrixPart<otherT,DIM,Iterator> &r){ 
      std::transform(r.begin,r.end,begin(),clipped_cast<otherT,T>);
    }
    
    /// Assignment operator (with compatible data type) (deep copy)
    FixedMatrix &operator=(const FixedMatrix &other){
      if(this == &other) return *this;
      FixedMatrixBase::optimized_copy<const T*,T*,DIM>(other.begin(),other.end(),begin());
      //std::copy(other.begin(),other.end(),begin());
      return *this;
    }
    
    /// Assignment operator (with compatible data type) (deep copy)
    /** Internally using std::transform with icl::clipped_cast<otherT,T> */
    template<class otherT>
    FixedMatrix &operator=(const FixedMatrix<otherT,COLS,ROWS> &other){
      if(this == &other) return *this;
      std::transform(other.begin(),other.end(),begin(),clipped_cast<otherT,T>);
      return *this;
    }
    
    /// Assign all elements with given value
    FixedMatrix &operator=(const T &t){
      std::fill(begin(),end(),t);
      return *this;
    }

    /// Assign matrix elements with sup-part of another matrix (identical types)
    template<class Iterator>
    FixedMatrix &operator=(const FixedMatrixPart<T,DIM,Iterator> &r){ 
      //      std::copy(r.begin,r.end,begin());
      FixedMatrixBase::optimized_copy<Iterator,T*,DIM>(r.begin,r.end,begin());
      return *this;
    }

    /// Assign matrix elements with sup-part of another matrix (compatible types)
    template<class otherT, class Iterator>
    FixedMatrix &operator=(const FixedMatrixPart<otherT,DIM,Iterator> &r){ 
      std::transform(r.begin,r.end,begin(),clipped_cast<otherT,T>);
      return *this;
    }

    /// Matrix devision
    /** A/B is equal to A*inv(B)
        Only allowed form square matrices B
        @param m denominator for division expression
    */
    FixedMatrix operator/(const FixedMatrix &m) const 
      throw (IncompatibleMatrixDimensionException,
             InvalidMatrixDimensionException,
             SingularMatrixException){
      return this->operator*(m.inv());
    }

    /// Matrix devision (inplace)
    FixedMatrix &operator/=(const FixedMatrix &m) const 
      throw (IncompatibleMatrixDimensionException,
             InvalidMatrixDimensionException,
             SingularMatrixException){
      return *this = this->operator*(m.inv());
    }

    /// Multiply all elements by a scalar
    FixedMatrix operator*(T f) const{
      FixedMatrix d;
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::multiplies<T>(),f));
      return d;
    }

    /// moved outside the class  Multiply all elements by a scalar (inplace)
    FixedMatrix &operator*=(T f){
      std::transform(begin(),end(),begin(),std::bind2nd(std::multiplies<T>(),f));
      return *this;
    }

    /// Divide all elements by a scalar
    FixedMatrix operator/(T f) const{
      return this->operator*(1/f);
    }

    /// Divide all elements by a scalar
    FixedMatrix &operator/=(T f){
      return this->operator*=(1/f);
    }

    /// Add a scalar to each element
    FixedMatrix operator+(const T &t) const{
      FixedMatrix d;
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::plus<T>(),t));
      return d;
    }

    /// Add a scalar to each element (inplace)
    FixedMatrix &operator+=(const T &t){
      std::transform(begin(),end(),begin(),std::bind2nd(std::plus<T>(),t));
      return *this;
    }


    /// Substract a scalar from each element
    FixedMatrix operator-(const T &t) const{
      FixedMatrix d;
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::minus<T>(),t));
      return d;
    }

    /// Substract a scalar from each element (inplace)
    FixedMatrix &operator-=(const T &t){
      std::transform(begin(),end(),begin(),std::bind2nd(std::minus<T>(),t));
      return *this;
    }

    /// Element-wise matrix addition
    FixedMatrix operator+(const FixedMatrix &m) const{
      FixedMatrix d;
      std::transform(begin(),end(),m.begin(),d.begin(),std::plus<T>());
      return d;
    }

    /// Element-wise matrix addition (inplace)
    FixedMatrix &operator+=(const FixedMatrix &m){
      std::transform(begin(),end(),m.begin(),begin(),std::plus<T>());
      return *this;
    }

    /// Element-wise matrix subtraction
    FixedMatrix operator-(const FixedMatrix &m) const{
      FixedMatrix d;
      std::transform(begin(),end(),m.begin(),d.begin(),std::minus<T>());
      return d;
    }
    /// Element-wise matrix subtraction (inplace)
    FixedMatrix &operator-=(const FixedMatrix &m){
      std::transform(begin(),end(),m.begin(),begin(),std::minus<T>());
      return *this;
    }

    /// Prefix - operator 
    /** M + (-M) = 0;
    */
    FixedMatrix operator-() const {
      FixedMatrix cpy(*this);
      std::transform(cpy.begin(),cpy.end(),cpy.begin(),std::negate<T>());
      return cpy;
    }


    /// Element access operator
    T &operator()(unsigned int col,unsigned int row){
      return m_data[col+cols()*row];
    }

    /// Element access operator (const)
    const T &operator() (unsigned int col,unsigned int row) const{
      return m_data[col+cols()*row];
    }

    /// Element access index save (with exception if index is invalid)
    T &at(unsigned int col,unsigned int row) throw (InvalidIndexException){
      if(col>=cols() || row >= rows()) throw InvalidIndexException("row or col index too large");
      return m_data[col+cols()*row];
    }

    /// Element access index save (with exception if index is invalid) (const)
    const T &at(unsigned int col,unsigned int row) const throw (InvalidIndexException){
      return const_cast<FixedMatrix*>(this)->at(col,row);
    }
    
    /// linear data view element access
    /** This function is very useful e.g. for derived classes
        FixedRowVector and FixedColVector */
    T &operator[](unsigned int idx){
      return m_data[idx];
    }
    /// linear data view element access (const)
    /** This function is very useful e.g. for derived classes
        FixedRowVector and FixedColVector */
    const T &operator[](unsigned int idx) const{
      return m_data[idx];
    }
    

    /// iterator type
    typedef T* iterator;
    
    /// const iterator type
    typedef const T* const_iterator;

    /// row_iterator
    typedef T* row_iterator;

    /// const row_iterator
    typedef const T* const_row_iterator;
  
    /// compatibility-function returns template parameter ROWS
    static unsigned int rows(){ return ROWS; }
  
    /// compatibility-function returns template parameter COLS
    static unsigned int cols(){ return COLS; }

    /// return internal data pointer
    T *data() { return m_data; }

    /// return internal data pointer (const)
    const T *data() const { return m_data; }

    /// return static member variable DIM (COLS*ROWS)
    static unsigned int dim() { return DIM; }
  
    /// internal struct for row-wise iteration with stride=COLS \ingroup LINALG
    struct col_iterator : public std::iterator<std::random_access_iterator_tag,T>{

      /// just for compatibility with STL
      typedef unsigned int difference_type;
      
      /// wrapped data pointer (held shallowly)
      mutable T *p;
      
      /// the stride is equal to parent Matrix' classes COLS template parameter
      static const unsigned int STRIDE = COLS;
      
      /// Constructor
      col_iterator(T *col_begin):p(col_begin){}
      
      /// prefix increment operator
      col_iterator &operator++(){
        p+=STRIDE;
        return *this;
      }
      /// prefix increment operator (const)
      const col_iterator &operator++() const{
        p+=STRIDE;
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
        p-=STRIDE;
        return *this;
      }

      /// prefix decrement operator (const)
      const col_iterator &operator--() const{
        p-=STRIDE;
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
        p += n * STRIDE;
        return *this;
      }

      /// jump next n elements (inplace) (const)
      const col_iterator &operator+=(difference_type n) const{
        p += n * STRIDE;
        return *this;
      }


      /// jump backward next n elements (inplace)
      col_iterator &operator-=(difference_type n){
        p -= n * STRIDE;
        return *this;
      }

      /// jump backward next n elements (inplace) (const)
      const col_iterator &operator-=(difference_type n) const{
        p -= n * STRIDE;
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

      /// steps between two iterators ... (todo: check!)
      difference_type operator-(const col_iterator &i) const{
        return (p-i.p)/STRIDE;
      }
      
      /// Dereference operator
      T &operator*(){
        return *p;
      }

      /// const Dereference operator
      const T &operator*() const{
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
  
    // const column operator typedef
    typedef const col_iterator const_col_iterator;

    /// returns an iterator to first element iterating over each element (row-major order)
    iterator begin() { return m_data; }

    /// returns an iterator after the last element
    iterator end() { return m_data+dim(); }
  
    /// returns an iterator to first element iterating over each element (row-major order) (const)
    const_iterator begin() const { return m_data; }

    /// returns an iterator after the last element (const)
    const_iterator end() const { return m_data+dim(); }
  
    /// returns an iterator iterating over a certain column 
    col_iterator col_begin(unsigned int col) { return col_iterator(m_data+col); }

    /// row end iterator
    col_iterator col_end(unsigned int col) { return col_iterator(m_data+col+dim()); }
  
    /// returns an iterator iterating over a certain column (const)
    const_col_iterator col_begin(unsigned int col) const { return col_iterator(const_cast<T*>(m_data)+col); }

    /// row end iterator const
    const_col_iterator col_end(unsigned int col) const { return col_iterator(const_cast<T*>(m_data)+col+dim()); }

    /// returns an iterator iterating over a certain row
    row_iterator row_begin(unsigned int row) { return m_data+row*cols(); }

    /// row end iterator
    row_iterator row_end(unsigned int row) { return m_data+(row+1)*cols(); }

    /// returns an iterator iterating over a certain row (const)
    const_row_iterator row_begin(unsigned int row) const { return m_data+row*cols(); }

    /// row end iterator (const)
    const_row_iterator row_end(unsigned int row) const { return m_data+(row+1)*cols(); }

    /// inplace matrix multiplication (dst = (*this)*m)
    /** Inplace matrix multiplication for 4x4-float-matrices (using ipp-optimization)
        is approximately twice as fast as D=A*B operator based multiplication
        \section BM Benchmark 
        1.000.000 Multiplications of 4x4-float matrices (using ipp-optimization) on a 2GHz 
        Core-2-Duo take about 145ms using inplace multiplication and about 290ms using
        not-inplace multiplication. \n
        TODO: These results differ from the general matrix benchmarks ??
        
        @param m right matrix multiplication operand
        @dst destination of matrix multiplication
        @see operator*(const FixedMatrix<T,MCOLS,COLS>&) 
    */
    template<unsigned int MCOLS>
    void mult(const FixedMatrix<T,MCOLS,COLS> &m,  FixedMatrix<T,MCOLS,ROWS> &dst) const{
      for(unsigned int c=0;c<MCOLS;++c){
        for(unsigned int r=0;r<ROWS;++r){
          //          std::cout << "calling inner_product" << std::endl;
          dst(c,r) = std::inner_product(m.col_begin(c),m.col_end(c),row_begin(r),T(0));
        }
      }      
    }
    /// Matrix multiplication (essential)
    /** matrices multiplication A*B is only valid if cols(A) is equal to rows(B).
        Resulting matrix has dimensions cols(B) x rows(A). Matrix mutliplication
        is ipp-optimized for float and double and for (2x2-, 3x3- and 4x4- matrices)
        @param m right operator in matrix multiplication
        @return 

        multiplication sceme
        <pre>
                 __MCOLS__
                |      c  |
                |      c  |
              COLS     c  |  = right operand
                |      c  |
                |______c__|
                
        _COLS__  __________
       |      | |      /\ |
       |      | |      |  |  = result
     ROWS     | |      |  |
       |rrrrrr| |<-----x  |  x is inner product <r,c>
       |______| |_________|
        </pre>
    */
    template<unsigned int MCOLS>
    FixedMatrix<T,MCOLS,ROWS> operator*(const FixedMatrix<T,MCOLS,COLS> &m) const{
      FixedMatrix<T,MCOLS,ROWS> d;
      mult(m,d);
      return d;
    }
 


    /// invert the matrix (only implemented with IPP_OPTIMIZATION and only for icl32f and icl64f)
    /** This function internally uses an instance of DynMatrix<T> 
        Additionally implemented (in closed form) for float and double for 2x2 3x3 and 4x4 matrices
        
        <b>Note:</b> Inv will not compute a pseudo inverse matrix. Include iclFixedMatrixUtils.h
        instead and use the non-member template function pinv() instead for pseudo-inverse calculation
    */
    FixedMatrix inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){
      DynMatrix<T> m(COLS,ROWS,const_cast<T*>(m_data),false);
      DynMatrix<T> mi = m.inv();
      m.set_data(0);
      FixedMatrix r;
      FixedMatrixBase::optimized_copy<T*,T*,DIM>(mi.begin(),mi.end(),r.begin());
      return r;
    }
    
    /// calculate matrix determinant (only implemented with IPP_OPTIMIZATION and only for icl32f and icl64f)
    /** This function internally uses an instance of DynMatrix<T> 
        Additionally implemented (in closed form) for float and double for 2x2 3x3 and 4x4 matrices
    */
    T det() const throw(InvalidMatrixDimensionException){
      DynMatrix<T> m(COLS,ROWS,const_cast<T*>(m_data),false);
      T d = m.det();
      m.set_data(0);
      return d;
    }
  
    /// returns matrix's transposed
    FixedMatrix<T,ROWS,COLS> transp() const{
      FixedMatrix<T,ROWS,COLS> d;
      for(unsigned int i=0;i<cols();++i){
        FixedMatrixBase::optimized_copy<const_col_iterator, typename FixedMatrix<T,ROWS,COLS>::row_iterator,DIM>(col_begin(i),col_end(i),d.row_begin(i));
        //        std::copy(col_begin(i),col_end(i),d.row_begin(i));
      }
      return d;
    }

     /// inner product of data pointers (not matrix-mulitiplication)
    /** computes the inner-product of data vectors */
    template<unsigned int OTHER_COLS>
    T inner_product(const FixedMatrix<T,OTHER_COLS,DIM/OTHER_COLS> &other) const {
      return std::inner_product(begin(),end(),other.begin(),T(0));
    }
    
    /// returns diagonal-elements as column-vector
    FixedMatrix<T,1,COLS> diag() const{
      ICLASSERT_RETURN_VAL(COLS==ROWS,(FixedMatrix<T,1,COLS>()));
      FixedMatrix<T,1,COLS> d;
      for(int i=0;i<COLS;++i){
        d[i] = (*this)(i,i);
      }
      return d;
    }
    
    /// computes the sum of all diagonal elements
    T trace() const{
      ICLASSERT_RETURN_VAL(COLS==ROWS,0);
      double accu = 0;
      for(int i=0;i<DIM;i+=COLS+1){
        accu += m_data[i];
      }
      return accu;
    }
   


    /// returns a matrix row-reference  iterator pair
    FixedMatrixPart<T,COLS,row_iterator> row(unsigned int idx){
      return FixedMatrixPart<T,COLS,row_iterator>(row_begin(idx),row_end(idx));
    }

    /// returns a matrix row-reference  iterator pair (const)
    FixedMatrixPart<T,COLS,const_row_iterator> row(unsigned int idx) const{
      return FixedMatrixPart<T,COLS,const_row_iterator>(row_begin(idx),row_end(idx));
    }

    /// returns a matrix col-reference  iterator pair
    FixedMatrixPart<T,ROWS,col_iterator> col(unsigned int idx){
      return FixedMatrixPart<T,ROWS,col_iterator>(col_begin(idx),col_end(idx));
    }

    /// returns a matrix col-reference  iterator pair (const)
    FixedMatrixPart<T,ROWS,const_col_iterator> col(unsigned int idx) const{
      return FixedMatrixPart<T,ROWS,const_col_iterator>(col_begin(idx),col_end(idx));
    }

    /// extracts a rectangular matrix sub region
    template<unsigned int X,unsigned int Y,unsigned int WIDTH,unsigned int HEIGHT>
    FixedMatrixPart<T,WIDTH*HEIGHT,MatrixSubRectIterator<T> > part(){
      return FixedMatrixPart<T,WIDTH*HEIGHT,MatrixSubRectIterator<T> >( 
                 MatrixSubRectIterator<T>(begin(),COLS,X,Y,WIDTH,HEIGHT),
                 MatrixSubRectIterator<T>::create_end_iterator(begin(),COLS,X,Y,WIDTH,HEIGHT));
    }

    /// extracts a rectangular matrix sub region (const)
    template<unsigned int X,unsigned int Y,unsigned int WIDTH,unsigned int HEIGHT>
    const FixedMatrixPart<T,WIDTH*HEIGHT,MatrixSubRectIterator<T> > part() const{
      return const_cast<FixedMatrix<T,COLS,ROWS>*>(this)->part<X,Y,WIDTH,HEIGHT>();
    }
    
    /// extends/shrinks matrix dimensions while preserving content on remaining elements (without scaling)
    /** This is resizing operation, which preserves contents for all remaining matrix elements.
        If new dimension is smaller than the current dimension, values are deleted.
        If otherwise new dimension gets larger, new allocated value are initialized with
        given init value.
    */
    template<unsigned int NEW_WIDTH,unsigned int NEW_HEIGHT>
    inline FixedMatrix<T,NEW_WIDTH,NEW_HEIGHT> resize(const T &init=T(0)){
      FixedMatrix<T,NEW_WIDTH,NEW_HEIGHT> M(init);
      for(unsigned int x=0;x<COLS && x < NEW_WIDTH; ++x){
        for(unsigned int y=0;y<ROWS && y < NEW_HEIGHT; ++y){
          M(x,y) = (*this)(x,y);
        }
      }
      return M;
    }
    
    /// create identity matrix 
    /** if matrix is not a spare one, upper left square matrix
        is initialized with the fitting identity matrix and other
        elements are initialized with 0
    */
    static FixedMatrix<T,ROWS,COLS> id(){
      FixedMatrix<T,ROWS,COLS> m(T(0));
      for(unsigned int i=0;i<ROWS && i<COLS;++i){
        m(i,i) = 1;
      }
      return m;
    }

    /// Calculates the length of the matrix data vector
    inline double length(T norm=2) const{ 
      double sumSquares = 0;
      for(unsigned int i=0;i<DIM;++i){
        sumSquares += ::pow((*this)[i],(double)norm);
      }
      return ::pow( sumSquares, 1.0/norm);
    }

    /// inplace normalization
    inline void normalize(T norm=2){
      T l = (T)length(norm);
      if(l) (*this) /= l;
    }
    
    /// create a normalized version of this matrix
    inline FixedMatrix<T,COLS,ROWS> normalized(T norm=2) const{
      T l = (T)length(norm);
      return l ?  (*this)/l : *this;
    }

    /// Element-wise comparison with other matrix
    template<class otherT>
    bool operator==(const FixedMatrix<otherT,COLS,ROWS> &m) const{
      for(unsigned int i=0;i<DIM;++i){
        if(m_data[i] != m[i]) return false;
      }
      return true;
    }    
    /// Element-wise comparison with other matrix
    template<class otherT>
    bool operator!=(const FixedMatrix<otherT,COLS,ROWS> &m) const{
      return !this->operator==(m);
    }    

  };

 

  /// Matrix multiplication (inplace)
  /** inplace matrix multiplication does only work for squared source and 
      destination matrices of identical size */
  template<class T, unsigned int M_ROWS_AND_COLS,unsigned int V_COLS>
  inline FixedMatrix<T,V_COLS,M_ROWS_AND_COLS> &operator*=(FixedMatrix<T,V_COLS,M_ROWS_AND_COLS> &v,
                                                           const FixedMatrix<T,M_ROWS_AND_COLS,M_ROWS_AND_COLS> &m){
    return v = (m*v);
  } 

  /// put the matrix into a std::ostream (human readable) 
  /** Internally, this function wraps a DynMatrix<T> shallowly around m*/
  template<class T, unsigned int COLS, unsigned int ROWS>
  inline std::ostream &operator<<(std::ostream &s,const FixedMatrix<T,COLS,ROWS> &m){
    return s << DynMatrix<T>(COLS,ROWS,const_cast<T*>(m.begin()),false);
  }

  /// read matrix from std::istream (human readable) 
  /** Internally, this function wraps a DynMatrix<T> shallowly around m*/
  template<class T, unsigned int COLS, unsigned int ROWS>
  inline std::istream &operator>>(std::istream &s,FixedMatrix<T,COLS,ROWS> &m){
    DynMatrix<T> dynM(COLS,ROWS,const_cast<T*>(m.begin()),false);
    return s >> dynM;
  }


  /// creates a 2D rotation matrix (defined for float and double)
  template<class T>
  FixedMatrix<T,2,2> create_rot_2D(T angle);

  /// creates a 2D homogen matrix (defined for float and double)
  template<class T>
  FixedMatrix<T,3,3> create_hom_3x3(T angle, T dx=0, T dy=0, T v0=0, T v1=0);

  /// creates a 2D homogen matrix with translation part only (defined for float and double)
  template<class T>
  inline FixedMatrix<T,3,3> create_hom_3x3_trans(T dx, T dy){
    FixedMatrix<T,3,3> m = FixedMatrix<T,3,3>::id();
    m(2,0)=dx;
    m(2,1)=dy;
    return m;
  }

  /// create 3D rotation matrix that rotates about given axis by given angle (float and double only)
  template<class T>
  FixedMatrix<T,3,3> create_rot_3D(T axisX, T axisY, T axisZ, T angle);
  
  /// creates a 3D rotation matrix (defined for float and double)
  template<class T>
  FixedMatrix<T,3,3> create_rot_3D(T rx,T ry,T rz);

  /// creates a 3D homogeneous matrix (defined for float and double)
  template<class T>
  FixedMatrix<T,4,4> create_hom_4x4(T rx, T ry, T rz, T dx=0, T dy=0, T dz=0, T v0=0, T v1=0, T v2=0);

  /// create 4D homogeneous matrix that rotates about given axis by given angle float and double only)
  template<class T>
  FixedMatrix<T,4,4> create_rot_4x4(T axisX, T axisY, T axisZ, T angle);


  /// creates a 3D homogen matrix with translation part only (defined for float and double)
  template<class T>
  inline FixedMatrix<T,4,4> create_hom_4x4_trans(T dx, T dy, T dz){
    FixedMatrix<T,4,4> m = FixedMatrix<T,4,4>::id();
    m(3,0)=dx;
    m(3,1)=dy;
    m(3,2)=dy;
    return m;
  }

  /// calculate the trace of a square matrix
  template<class T, unsigned int ROWS_AND_COLS>
  FixedMatrix<T,1,ROWS_AND_COLS> trace(const FixedMatrix<T,ROWS_AND_COLS,ROWS_AND_COLS> &m){
    FixedMatrix<T,1,ROWS_AND_COLS> t;
    for(unsigned int i=0;i<ROWS_AND_COLS;++i){
      t[i] = m(i,i);
    }
    return t;
  }


  /** \cond  declared and documented above */
  template<class T,unsigned int N, class Iterator> template<unsigned int COLS>
  inline FixedMatrixPart<T,N,Iterator>& FixedMatrixPart<T,N,Iterator>::operator=(const FixedMatrix<T,COLS,N/COLS> &m){
    FixedMatrixBase::optimized_copy<const T*,Iterator,N>(m.begin(),m.end(),begin);
    //std::copy(m.begin(),m.end(),begin);
    return *this;
  }
  template<class T,unsigned int N, class Iterator> template<class T2, unsigned int COLS>
  inline FixedMatrixPart<T,N,Iterator>& FixedMatrixPart<T,N,Iterator>::operator=(const FixedMatrix<T2,COLS,N/COLS> &m){
    std::transform(m.begin(),m.end(),begin,clipped_cast<T2,T>);
    return *this;
  }
  /** \endcond */

#ifdef HAVE_IPP
#define OPTIMIZED_MATRIX_MULTIPLICATION(LEFT_COLS,LEFT_ROWS,RIGHT_COLS,TYPE,IPPSUFFIX) \
  template<> template<>                                                                \
  inline void                                                                          \
  FixedMatrix<TYPE,RIGHT_COLS,LEFT_ROWS>::mult                                         \
     (                                                                                 \
        const FixedMatrix<TYPE,RIGHT_COLS,LEFT_COLS> &m,                               \
        FixedMatrix<TYPE,RIGHT_COLS,LEFT_ROWS> &dst                                    \
     ) const {                                                                         \
    static const unsigned int ST=sizeof(TYPE);                                         \
    ippmMul_mm_##IPPSUFFIX(data(),LEFT_COLS*ST,ST,LEFT_COLS,LEFT_ROWS,                 \
                           m.data(),RIGHT_COLS*ST,ST,RIGHT_COLS,LEFT_COLS,             \
                           dst.data(),RIGHT_COLS*ST,ST);                               \
  }

  OPTIMIZED_MATRIX_MULTIPLICATION(2,2,2,float,32f);
  OPTIMIZED_MATRIX_MULTIPLICATION(3,3,3,float,32f);  
  OPTIMIZED_MATRIX_MULTIPLICATION(4,4,4,float,32f);

  OPTIMIZED_MATRIX_MULTIPLICATION(2,2,2,double,64f);
  OPTIMIZED_MATRIX_MULTIPLICATION(3,3,3,double,64f);  
  OPTIMIZED_MATRIX_MULTIPLICATION(4,4,4,double,64f);
#undef OPTIMIZED_MATRIX_MULTIPLICATION
 

#endif

#define USE_OPTIMIZED_INV_AND_DET_FOR_2X2_3X3_AND_4X4_MATRICES

#ifdef USE_OPTIMIZED_INV_AND_DET_FOR_2X2_3X3_AND_4X4_MATRICES

  /** \cond */
  // this functions are implemented in iclFixedMatrix.cpp. All templates are
  // instantiated for float and double

  template<class T> 
  void icl_util_get_fixed_4x4_matrix_inv(const T *src, T*dst);
  template<class T> 
  void icl_util_get_fixed_3x3_matrix_inv(const T *src, T*dst);
  template<class T> 
  void icl_util_get_fixed_2x2_matrix_inv(const T *src, T*dst);

  template<class T> 
  T icl_util_get_fixed_4x4_matrix_det(const T *src);
  template<class T> 
  T icl_util_get_fixed_3x3_matrix_det(const T *src);
  template<class T> 
  T icl_util_get_fixed_2x2_matrix_det(const T *src);

#define SPECIALISED_MATRIX_INV_AND_DET(D,T) \
  template<>                                                            \
  inline FixedMatrix<T,D,D> FixedMatrix<T,D,D>::inv() const             \
  throw (InvalidMatrixDimensionException,SingularMatrixException){      \
    FixedMatrix<T,D,D> r;                                               \
    icl_util_get_fixed_##D##x##D##_matrix_inv<T>(begin(),r.begin());    \
    return r;                                                           \
  }                                                                     \
  template<>                                                            \
  inline T FixedMatrix<T,D,D>::det() const                              \
  throw(InvalidMatrixDimensionException){                               \
    return icl_util_get_fixed_##D##x##D##_matrix_det<T>(begin());       \
  }


  SPECIALISED_MATRIX_INV_AND_DET(2,float);
  SPECIALISED_MATRIX_INV_AND_DET(3,float);
  SPECIALISED_MATRIX_INV_AND_DET(4,float);
  SPECIALISED_MATRIX_INV_AND_DET(2,double);
  SPECIALISED_MATRIX_INV_AND_DET(3,double);
  SPECIALISED_MATRIX_INV_AND_DET(4,double);


#undef SPECIALISED_MATRIX_INV_AND_DET
  
/** \endcond */
#endif


}

#endif
