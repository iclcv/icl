#ifndef ICL_MAT_4D_H
#define ICL_MAT_4D_H

#include "iclVec4D.h"
#include <string>

namespace icl{
  
  /// Template based 4x4 Matrix implementation
  /** This complex 4x4 matrix implementation can be used to handle homogeneous
      3D matrix operations like transformations and projections. The
      matrix wraps 4 column-vectors of the corresponding Vec4D template class.
      The Mat4D template class is typedef'ed for:
      \code
      typedef Mat4D<icl32f> Mat4D32f;
      typedef Mat4D<icl64f> Mat4D64f;
      typedef Mat4D<icl32s> Mat4D32s;
      \endcode
      
      Matrix indexes are M(column,row)
  */
  template<class T=icl32f>
  class Mat4D{
    /// Internal data storage (horizontal array of 4 column vectors)
    Vec4D<T> cols[4];
    
    public:
    /// Access operator to the i-th column of the matrix (x-index)
    inline Vec4D<T> &operator[](int colIdx){ return cols[colIdx];  }  

    /// Access operator to the i-th column of the matrix (x-index) (const)
    inline const Vec4D<T> &operator[](int colIdx) const { return cols[colIdx];  }
    
   
    /// Utility class to provide e.g. m.row(2) = Vec4D(1,2,3,4)
    class MatRow{
      /// parent matrix reference
      Mat4D<T> *m;
      
      /// corresponding row
      int row;
      
      public:
      /// Creation function
      /** @param m parent matrix
          @param row row of this matrix 
      */
      inline MatRow(Mat4D *m, int row):m(m),row(row){}
    
      /// Implicit cast to column vector operator
      /** Via this implicitly called function, a MatRow<T> can be
          used wherever a Vec4D<T> is needed 
          @return a new Vec4D<T> object 
      */
      inline operator Vec4D<T>() const{
        return Vec4D<T>((*m)[0][row],(*m)[1][row],(*m)[2][row],(*m)[3][row]);
      }
      
      /// access- operator
      /** @param idx column index
          @return reference to the matrix element (*m)[idx][row] 
      */
      inline T &operator[](int idx){ return (*m)[idx][row]; }

      /// access- operator (const)
      /** @param idx column index
          @return reference to the matrix element m[idx][row] 
      */
      inline const T &operator[](int idx) const{ return (*m)[idx][row]; }
      
      /// Inner (scalar) product with a column vector
      /** @param v 2nd argument for "*"
          @return \f$this^{\tau}*\vec{v}\f$*/
      inline T operator*(const Vec4D<T> &v) const{
        return (*this)[0]*v[0]+(*this)[1]*v[1]+(*this)[2]*v[2]+(*this)[3]*v[3];
      }
     

      /// Assign operator 
      /** e.g. to copy one row of a matrix into all other rows:
          \code
          Mat4D32f m = ...;
          /// set all rows equal to the first
          m(1) = m(2) = m(3) = m(0);
          \endcode
          @param r other row
          @return *this
      */
      inline MatRow &operator=(const MatRow &r){
        (*this)[0] = r[0];
        (*this)[1] = r[1];
        (*this)[2] = r[2];
        (*this)[3] = r[3];
        return *this;
      }
      
      /// Assign operator assigning a row to a column vector
      /** E.g. transpose a matrix
          \code
          Mat4D32f a;
          Mat4D32f aTransposed;
          for(int i=0;i<4;i++){
            aTransposed[i] = a(i);  // []=column-access, ()=row-access
          }
          \endcode
          <b>Note:</b> to transpose a matrix it is faster to call the transpose() function
          @param v new value
          @return *this
      */
      inline MatRow &operator=(const Vec4D<T> &v){
        (*this)[0] = v[0];
        (*this)[1] = v[1];
        (*this)[2] = v[2];
        (*this)[3] = v[3];
        return *this;
      }
      
      /// Assign operator assigning a row of a matrix to a const value
      /** E.g. to clear the matrix's last to rows:
          \code
          Mat4D32f m = ...;
          m(2)=m(3)=0;
          \endcode
          @param val new value for all row elements
          @return *this
      */
      inline MatRow &operator=(T val){
        (*this)[0] = (*this)[1] = (*this)[2] = (*this)[3] = val;
        return *this;
      }
    };
    
    /// extract a row from this matrix (identical to the ()-operator)
    /** Note: MatRow struct references to it's parent matrix, so it is
        only valid as long as the parent-matrix is valid.\n
        E.g. transpose a matrix:
        \code
        Mat4D32f a;
        Mat4D32f aTransposed;
        for(int i=0;i<4;i++){
          aTransposed[i] = a.row(i);  // []=column-access
        }
        \endcode
        <b>Note:</b> to transpose a matrix it is faster to call the transpose() function
        @param idx row index 
        @return a new MatRow struct
    */
    inline MatRow row(int idx){
      return MatRow(this,idx);
    }
    
    /// () operator extracts the idx-th <b>row</b> of this matrix (identical to row(int) function)
    /** @param idx row index 
        @return a new MatRow struct
    */
    inline MatRow operator() (int idx) { return row(idx); }
    
    
    /// returns a 16 element data array of this matrix (data must be copied internally!)
    /** This function internally allocates a T* of 16 elements and fills it with the 
        current data. The data array is not updated implicitly when other Mat4D<T>-element
        functions e.g. transpose() are called. 
        @param dst optional destination T-pointer
        @return internal temporary data array or dst (if given) 
    */
    void getData(T *dst) const{
      for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
          dst[i+4*j] = (*this)[i][j];
        }
      }
    }
  
    /// Creates an empty matrix m[i][j]=0
    inline Mat4D(){}
    
    /// explicit copy constructor
    inline Mat4D(const Mat4D &m){
      cols[0] = m[0];
      cols[1] = m[1];
      cols[2] = m[2];
      cols[3] = m[3];
    }
    
    /// create a matrix by given 4 column vectors
    /** @param col0 vector for column 0 
        @param col1 vector for column 1 
        @param col2 vector for column 2 
        @param col3 vector for column 3 
    */
    inline Mat4D(const Vec4D<T>& col0,const Vec4D<T>& col1,const Vec4D<T>& col2,const Vec4D<T>& col3){
      cols[0] = col0;
      cols[1] = col1;
      cols[2] = col2;
      cols[3] = col3;
    }
    
    /// creates a new matrix from given matrix of (another) depth
    /** @param m other matrix*/
    template<class OtherT>
    inline  Mat4D(const Mat4D<OtherT> m){
      cols[0] = m[0];
      cols[1] = m[1];
      cols[2] = m[2];
      cols[3] = m[3];
    }
    /// creates a matrix by given data array with 16 elements
    /** The given data is copied into the matrix
        @param data data array layout:
        \f[
        m = \left(\begin{array}{cccc}
        data[0]&data[1]&data[2]&data[3]\\
        data[4]&data[5]&data[6]&data[7]\\       
        data[8]&data[9]&data[10]&data[11]\\
        data[12]&data[13]&data[14]&data[15]\\
        \end{array}\right)
        \f]
    */
    inline Mat4D(const T data[16]){
      cols[0] = Vec4D<T>(data[0],data[4],data[8],data[12]);
      cols[1] = Vec4D<T>(data[1],data[5],data[9],data[13]);
      cols[2] = Vec4D<T>(data[2],data[6],data[10],data[14]);
      cols[3] = Vec4D<T>(data[3],data[7],data[11],data[15]);
    }
    /// creates a matrix by given 16 values
    inline Mat4D(T m00, T m10, T m20, T m30,
                 T m01, T m11, T m21, T m31,
                 T m02, T m12, T m22, T m32,
                 T m03, T m13, T m23, T m33){
      cols[0] = Vec4D<T>(m00,m01,m02,m03);
      cols[1] = Vec4D<T>(m10,m11,m12,m13);
      cols[2] = Vec4D<T>(m20,m21,m22,m23);
      cols[3] = Vec4D<T>(m30,m31,m32,m33);
    }

    /// Create a matrix as the outer product of two vectors
    /** The result matrix is 
        \f[
        m = \vec{a}\vec{b}^{\tau} = 
        \left(\begin{array}{cccc}
        m_{00}&m_{10}&m_{20}&m_{30}\\
        m_{01}&m_{11}&m_{21}&m_{31}\\       
        m_{02}&m_{12}&m_{22}&m_{32}\\
        m_{03}&m_{13}&m_{23}&m_{33}\\
        \end{array}\right)\;\;\; with \;\;m_{ij} = a_j\, b_i
        \f]
        @param a first value
        @param b second value first value
    */
    inline Mat4D(const Vec4D<T> &a, const Vec4D<T> &b){
      for(int i=0;i<4;++i){
        for(int j=0;j<4;++j){
          (*this)[i][j] = a[j]*b[i];
        }
      }
    }

    /// Destructor
    inline ~Mat4D(){}
    
    /// Assign operator assigns each element to a constant value f
    /** @param f value for all m[i][j] 
        @return *this
    */
    inline Mat4D<T> &operator=(T f){
      cols[0] = cols[1] = cols[2] = cols[3] = f; return *this; 
    }
    
    /// Assign operator assigns elements from another matrix
    /** @param m other matrix
        @return *this
    */
    template<class OtherT>
    inline Mat4D<T> &operator=(const Mat4D<OtherT> &m){
      cols[0] = m[0];
      cols[1] = m[1];
      cols[2] = m[2];
      cols[3] = m[3];
      return *this;
    }

    /// matrix * vector multiplication
    /** @returns \f$\vec{result}=this*\vec{v} \f$*/
    inline Vec4D<T> operator*(const Vec4D<T> &v) const{
      const Mat4D<T> &t = *this;
      return Vec4D<T>( t[0][0] * v[0] + t[1][0] * v[1] + t[2][0] * v[2] + t[3][0] * v[3] ,
                       t[0][1] * v[0] + t[1][1] * v[1] + t[2][1] * v[2] + t[3][1] * v[3] ,
                       t[0][2] * v[0] + t[1][2] * v[1] + t[2][2] * v[2] + t[3][2] * v[3] ,
                       t[0][3] * v[0] + t[1][3] * v[1] + t[2][3] * v[2] + t[3][3] * v[3] );
    }
    
    /// matrix * matrix operation
    /** @return default matrix product \f$this*m\f$*/
   inline  Mat4D<T> operator*(const Mat4D<T> &m) const{
      Mat4D<T> E;
      const Mat4D<T> &t = *this;
      for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
          E[i][j] = t[0][j]*m[i][0] + t[1][j]*m[i][1] + t[2][j]*m[i][2] + t[3][j]*m[i][3];
        }
      }
      return E;
    }
    /// matrix division (equivalent to (*this)*m.inverted();
    inline Mat4D<T> operator/(const Mat4D<T> &m) const{
      return (*this)*m.inverted();
    }
    
    /// matrix addition (adds another matrix element-wise)
    inline Mat4D<T> operator+(const Mat4D<T> &m) const{
      return Mat4D<T>( cols[0]+m[0], cols[1]+m[1], cols[2]+m[2], cols[3]+m[3] );
    }

    /// matrix subtraction (subtracts another matrix element-wise)
    inline Mat4D<T> operator-(const Mat4D<T> &m) const{
      return Mat4D<T>( cols[0]-m[0], cols[1]-m[1], cols[2]-m[2], cols[3]-m[3] );
    }
    
    /// in-place matrix*matrix multiplication 
    /** \f[
        this =m*this 
        \f]
    */
    inline Mat4D<T> &operator*=(const Mat4D<T> &m){
      (*this) = m*(*this);
      return *this;
    }

    /// in-place matrix*matrix division (equivalent to (*this)*=m.inverted())
    /** \f[
        this = this*m^(-1)
        \f]
    */
    inline Mat4D<T> &operator/=(const Mat4D<T> &m){
      return (*this)*=(m.inverted());
    }
    
    /// in-place matrix addition (adds a constant to each matrix element)
    inline Mat4D<T> &operator+=(T f){
      cols[0]+=f;
      cols[1]+=f;
      cols[2]+=f;
      cols[3]+=f;
      return *this;
    }
    /// in-place matrix subtraction(subtracts a constant to each matrix element)
    inline Mat4D<T> &operator-=(T f){
      cols[0]-=f;
      cols[1]-=f;
      cols[2]-=f;
      cols[3]-=f;
      return *this;
    }
    /// in-place matrix element multiplication (multiplies each element by a constant)
    /** e.g. to create a matrix with value "5" at each position:
        \code 
        Mat4D32f m = 1;
        m*=5;
        \endcode
    */
    inline Mat4D<T> &operator*=(T f){
      cols[0]*=f;
      cols[1]*=f;
      cols[2]*=f;
      cols[3]*=f;
      return *this;
    } 
    
    /// in-place matrix element division (divides each element by a constant)
    inline Mat4D<T> &operator/=(T f){
      cols[0]/=f;
      cols[1]/=f;
      cols[2]/=f;
      cols[3]/=f;
      return *this;
    }

    /// outplace scalar matrix addition
    inline Mat4D<T> operator+(T f){
      return Mat4D<T>(cols[0]+f,cols[1]+f,cols[2]+f,cols[3]+f);
    }

    /// outplace scalar matrix subtraction
    inline Mat4D<T> operator-(T f){
      return Mat4D<T>(cols[0]-f,cols[1]-f,cols[2]-f,cols[3]-f);
    }

    /// outplace scalar matrix multiplication
    inline Mat4D<T> operator*(T f){
      return Mat4D<T>(cols[0]*f,cols[1]*f,cols[2]*f,cols[3]*f);
    }
    /// outplace scalar matrix division
    inline Mat4D<T> operator/(T f){
      return Mat4D<T>(cols[0]/f,cols[1]/f,cols[2]/f,cols[3]/f);
    }
    
    /// returns (this)*(-1)
    inline Mat4D<T> operator-() const { return Mat4D(-cols[0],-cols[1],-cols[2],-cols[3]); }

    /// returns the trace of the matrix
    /** \f$ trace(m) = (m[0][0],m[1][1],m[2][2],m[3][3])^{\tau} \f$*/
    inline Vec4D<T> trace() const {
      return Vec4D<T>(cols[0][0],cols[1][1],cols[2][2],cols[3][3]);
    }
    
    /// transposes this matrix (in-place)
    /** \f$ m[i][j] = m[j][i]\f$*/
    inline Mat4D<T> &transpose(){
      Mat4D<T> &t = *this;
      t[1][0] = t[0][1]; t[2][0] = t[0][2]; t[3][0] = t[0][3];
      t[0][1] = t[1][0]; t[2][1] = t[1][2]; t[3][1] = t[1][3];
      t[0][2] = t[2][0]; t[1][2] = t[2][1]; t[3][2] = t[2][3];
      t[0][3] = t[3][0]; t[1][3] = t[3][1]; t[2][3] = t[3][2];
      return *this;
    }

    /// returns a transposed version of this matrix
    inline Mat4D<T> transposed() const{
      return Mat4D<T>(*this).transpose();
    }
    
    /// inverts this matrix using a dedicated formula for 4x4 matrices (fast)
    Mat4D<T> &invert();
    
    /// returns an inverted version of this matrix
    inline Mat4D<T> inverted() const{
      return Mat4D<T>(*this).invert();
    }
    
    /// calculates the matrix determinant (using a special 4x4-matrix formula -> fast)
    T det() const;
    
    /// shows the matrix to std::out
    void show(const std::string &title) const;
    
    /// creates an identity matrix
    static Mat4D<T> id();
    
    /// creates a 4x4 homogeneous rotation matrix
    /** with 
        \f$rx = alpha, \;\; ry = \beta \;\; and \;\; rz=\gamma \f$ 
        and
        \f$ c\alpha= \cos(\alpha) \;\; s\beta = \sin(\beta)\;\; \f$ and so on ... 
        the rotation matrix can is:
        \f[ 
        R = \left( \begin{array}{cccc}
        c\beta\cdot c\gamma-s\alpha\cdot s\beta\cdot s\gamma & -s\gamma\cdot c\alpha & c\gamma\cdot s\beta+s\gamma\cdot s\alpha\cdot c\beta & 0 \\
        c\beta\cdot s\gamma+c\gamma\cdot s\alpha\cdot s\beta &  c\gamma\cdot c\alpha & s\gamma\cdot s\beta-s\alpha\cdot c\beta\cdot c\gamma & 0 \\
        -s\beta\cdot c\alpha         &  s\alpha    & c\alpha\cdot c\beta          & 0 \\
        0              &  0     & 0              & 1 \\
        \end{array}\right)
        
        
        \f]
        */
    static Mat4D<T> rot(double rx, double ry, double rz);
    
    /// Crates a homogeneous translation matrix
    /** The layout of the transformation matrix is as follows:
        \f[
        T=\left(
        \begin{array}{cc}
        E & \vec{t} \\
        \vec{0}^{\tau} & 1 \\
        \end{array}
        \right)
        \f]
        where \f$E\f$ is a 3x3 identity matrix,
        \f$\vec{t}\f$ is a 3D translation vector, and  \f$\vec{v}^{\tau}\f$ is the projection vector.
    */
    static Mat4D<T> trans(T dx, T dy, T dz);
  
    /// static function to create a homogeneous transformation matrix
    /** The layout of the transformation matrix is as follows:
        \f[
        T=\left(
        \begin{array}{cc}
        R & \vec{t} \\
        \vec{v}^{\tau} & 1 \\
        \end{array}
        \right)
        \f]
        where \f$R\f$ is a 3x3 rotation matrix created as in the static function "rot()",
        \f$\vec{t}\f$ is a 3D translation vector, and  \f$\vec{v}^{\tau}\f$ is the projection vector.
    */
    static Mat4D<T> hom(double rx, double ry, double rz, T dx, T dy, T dz, double v1=0, double v2=0, double v3=0);
    
    /// optimized function for matrix multiplication if destination matrix is already allocated
    /** calling 
        \code
        Mat4D32f A,B,R;
        // begin benchmark
        Mat4D32f::mult(A,B,R);
        // end benchmark
        \endcode
        is more than 50% faster than calling
        \code
        Mat4D32f A,B,R;
        // begin benchmark
        R = A*B;
        // end benchmark
        \endcode
        The first version can be called approx. 4500 Times per msec on a 1.6GHz pentium M
        The second on reaches about 10300 calls per msec. (Using -O4)
    */
    static void mult(const Mat4D<T> &A, const Mat4D<T> &B, Mat4D<T> &RESULT);
    
    /// in-place version version (DO NOT USE!)
    /** The in-place function is not faster than the outplace function! 
        As it has been shown, that an in-place implementation was nearly 50% slower
        the the default implementation using:
        \code
        Mat4D32f M;
        Vec4D32f x,b;
        b = M*x;
        \endcode
        It has been replace by an inline function, which calls 
        \code
        RESULT=A*B;
        \endcode
    */
    static void mult(const Mat4D<T> &A, const Vec4D<T> &B, Vec4D<T> &RESULT){
      RESULT=A*B;
    }
  };
  
 
  /// Typedef of float matrices
  typedef Mat4D<icl32f> Mat4D32f;

  /// Typedef of double matrices
  typedef Mat4D<icl64f> Mat4D64f;

  /// Typedef of int matrices
  typedef Mat4D<icl32s> Mat4D32s;

  /// Extern operator '*': outer product of column vector "c" and a row vector 'r' (depth32f)
  /** <b>Note:</b> This function could not be implemented as template (...)
      @param c column vector
      @param r row vector (taken from a matrix)
      @return outer product \f$ \vec{c}*\vec{r}^{\tau} \f$
  */
  inline Mat4D32f operator*(const Vec4D32f& c, const Mat4D32f::MatRow &r){
    Mat4D32f m;
    for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
        m[i][j]=c[j]*r[i];
      }
    }
    return m;
  }

  /// Extern operator '*': outer product of column vector "c" and a row vector 'r' (depth64f)
  /** <b>Note:</b> This function could not be implemented as template (...)
      @param c column vector
      @param r row vector (taken from a matrix)
      @return outer product \f$ \vec{c}*\vec{r}^{\tau} \f$
  */
  inline Mat4D64f operator*(const Vec4D64f& c, const Mat4D64f::MatRow &r){
    Mat4D64f m;
    for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
        m[i][j]=c[j]*r[i];
      }
    }
    return m;
  }
  
  /// Extern operator '*': outer product of column vector "c" and a row vector 'r' (depth32s)
  /** <b>Note:</b> This function could not be implemented as template (...)
      @param c column vector
      @param r row vector (taken from a matrix)
      @return outer product \f$ \vec{c}*\vec{r}^{\tau} \f$
  */
  inline Mat4D32s operator*(const Vec4D32s& c, const Mat4D32s::MatRow &r){
    Mat4D32s m;
    for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
        m[i][j]=c[j]*r[i];
      }
    }
    return m;
  }
  
  /// Extern operator to transform vectors (V*=m  -> V=M*V) (depths32s)
  inline Vec4D32s &operator*=(Vec4D32s& c, const Mat4D32s &m){
    c=m*c;
    return c;
  }
  /// Extern operator to transform vectors (V*=m  -> V=M*V) (depths32f)
  inline Vec4D32f &operator*=(Vec4D32f& c, const Mat4D32f &m){
    c=m*c;
    return c;
  }
  /// Extern operator to transform vectors (V*=m  -> V=M*V) (depths64f)
  inline Vec4D64f &operator*=(Vec4D64f& c, const Mat4D64f &m){
    c=m*c;
    return c;
  }


}

#endif
