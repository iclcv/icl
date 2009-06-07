#ifndef ICL_DYN_MATRIX_UTILS_H
#define ICL_DYN_MATRIX_UTILS_H

#include <iclDynMatrix.h>
#include <algorithm>

namespace icl{

  /** @} @{ @name unary functions */

  /// Matrix initialization template
  /** This function can e.g. be used to initialize a matrix with random values
      \code
      #include <iclDynMatrixUtils.h>
      #include <iclRandom.h>
      
      int main(){
         DynMatrix<float> M(10,10);
      
         // initialize all entries with a uniform random number
         matrix_init(M,URand(0,1));
      }
      \endcode
      @param m matrix to initialize
      @param init initializer function value or functor
      @return m
  */
  template<class T, class Init>
  inline DynMatrix<T> &matrix_init(DynMatrix<T> &m, Init init){
    std::fill(m.begin(),m.end(),init);
    return m;
  }

  /// element-wise absolute value (inplace) [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_abs(DynMatrix<T> &m);

  /// element-wise absolute value  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_abs(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise logarith (basis E) (inplace)  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_log(DynMatrix<T> &m);

  /// element-wise logarith (basis E)  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_log(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise exp-function (inplace)  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_exp(DynMatrix<T> &m);

  /// element-wise exp-function  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_exp(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise square-root-function (inplace)  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sqrt(DynMatrix<T> &m);

  /// element-wise square-root-function  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sqrt(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise square-function (x*x) (inplace)  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sqr(DynMatrix<T> &m);

  /// element-wise square-function (x*x)  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sqr(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise sinus-function (x*x) (inplace) 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sin(DynMatrix<T> &m);

  /// element-wise sinus-function (x*x) 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sin(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise cosinus-function (inplace)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_cos(DynMatrix<T> &m);

  /// element-wise cosinus-function
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_cos(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise tangent-function (inplace)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_tan(DynMatrix<T> &m);

  /// element-wise tangent-function
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_tan(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise arcus sinus-function (inplace)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arcsin(DynMatrix<T> &m);

  /// element-wise arcus sinus-function
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arcsin(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise arcus cosinus-function (inplace)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arccos(DynMatrix<T> &m);

  /// element-wise arcus cosinus-function
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arccos(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise arcus tangent-function (inplace)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arctan(DynMatrix<T> &m);

  /// element-wise arcus tangent-function
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arctan(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise reciprocal-function (1/x) (inplace)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_reciprocal(DynMatrix<T> &m);

  /// element-wise reciprocal-function (1/x)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_reciprocal(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /** @} @{ @name unary functions with scalar argument */
  
  /// element-wise power-function (x^exponent) (inplace)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_powc(DynMatrix<T> &m, T exponent);

  /// element-wise power-function (x^exponent)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_powc(const DynMatrix<T> &m, T exponent, DynMatrix<T> &dst);

  /// element-wise addition of constant value (inplace)  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_addc(DynMatrix<T> &m, T val);

  /// element-wise addition of constant value  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_addc(const DynMatrix<T> &m, T val, DynMatrix<T> &dst);

  /// element-wise substraction of constant value (inplace)  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_subc(DynMatrix<T> &m, T val);

  /// element-wise substraction of constant value  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_subc(const DynMatrix<T> &m, T val, DynMatrix<T> &dst);

  /// element-wise division by constant value (inplace)  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_divc(DynMatrix<T> &m, T  val);

  /// element-wise division by constant value  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_divc(const DynMatrix<T> &m, T val, DynMatrix<T> &dst);

  /// element-wise multiplication with constant value (inplace)  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_mulc(DynMatrix<T> &m, T val);

  /// element-wise multiplication with constant value  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_mulc(const DynMatrix<T> &m, T val, DynMatrix<T> &dst);

  /** @} @{ @name binary functions */

  /// element-wise atan2 function atan2(y,x)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arctan2(const DynMatrix<T> &my, const DynMatrix<T> &mx, DynMatrix<T> &dst);

  /// element-wise addition  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_add(const DynMatrix<T> &m1, const DynMatrix<T> &m2, DynMatrix<T> &dst);

  /// element-wise substraction  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sub(const DynMatrix<T> &m1, const DynMatrix<T> &m2, DynMatrix<T> &dst);

  /// element-wise multiplication  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_mul(const DynMatrix<T> &m1, const DynMatrix<T> &m2, DynMatrix<T> &dst);

  /// element-wise division  [IPP-optimized]
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_div(const DynMatrix<T> &m1, const DynMatrix<T> &m2, DynMatrix<T> &dst);

  /// element-wise power
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_pow(const DynMatrix<T> &m1, const DynMatrix<T> &m2, DynMatrix<T> &dst);


  /** @ } */ 

  /// find minimum element of matrix (optionally find location too)  [IPP-optimized]
  /** For float and double only
      @param m source matrix
      @param x if no NULL, minimum x-location (column-index) is written to *x 
      @param y if no NULL, minimum y-location (row-index) is written to *y 
  */
  template<class T>
  T matrix_min(const DynMatrix<T> &m, int *x=0, int *y=0);

  /// find maximum element of matrix (optionally find location too)  [IPP-optimized]
  /** For float and double only
      @param m source matrix
      @param x if no NULL, maximum x-location (column-index) is written to *x 
      @param y if no NULL, maximum y-location (row-index) is written to *y 
  */
  template<class T>
  T matrix_max(const DynMatrix<T> &m, int *x=0, int *y=0);

  /// find min- and maxinim element at once (optionally with locations)  [IPP-optimized]
  /** For float and double only
      @param m source matrix
      @param dst found min and max value are written to dst\n
                       (dst[0]<-min, dst[1]<-max)
      @param minx if no NULL, minimum x-location (column-index) is written to *minx 
      @param miny if no NULL, minimum y-location (row-index) is written to *miny 
      @param maxx if no NULL, maximum x-location (column-index) is written to *maxx 
      @param maxy if no NULL, maximum y-location (row-index) is written to *maxy 
  */
  template<class T>
  void matrix_minmax(const DynMatrix<T> &m, T dst[2],
                     int *minx=0, int *miny=0,
                     int *maxx=0, int *maxy=0);
  
 
}

#endif
