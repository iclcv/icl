/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
** University of Bielefeld                                                **
** Contact: nivision@techfak.uni-bielefeld.de                             **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_DYN_MATRIX_UTILS_H
#define ICL_DYN_MATRIX_UTILS_H

#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/BasicTypes.h>
#include <algorithm>

namespace icl{

  /** @} @{ @name unary functions */

  /// Matrix initialization template \ingroup LINALG 
  /** This function can e.g. be used to initialize a matrix with random values
      \code
      #include <ICLUtils/DynMatrixUtils.h>
      #include <ICLCore/Random.h>

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

  /// element-wise absolute value (inplace) [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_abs(DynMatrix<T> &m);

  /// element-wise absolute value  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_abs(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise logarith (basis E) (inplace)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_log(DynMatrix<T> &m);

  /// element-wise logarith (basis E)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_log(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise exp-function (inplace)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_exp(DynMatrix<T> &m);

  /// element-wise exp-function  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_exp(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise square-root-function (inplace)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sqrt(DynMatrix<T> &m);

  /// element-wise square-root-function  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sqrt(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise square-function (x*x) (inplace)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sqr(DynMatrix<T> &m);

  /// element-wise square-function (x*x)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sqr(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise sinus-function (x*x) (inplace) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sin(DynMatrix<T> &m);

  /// element-wise sinus-function (x*x) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sin(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise cosinus-function (inplace) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_cos(DynMatrix<T> &m);

  /// element-wise cosinus-function \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_cos(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise tangent-function (inplace) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_tan(DynMatrix<T> &m);

  /// element-wise tangent-function \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_tan(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise arcus sinus-function (inplace) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arcsin(DynMatrix<T> &m);

  /// element-wise arcus sinus-function \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arcsin(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise arcus cosinus-function (inplace) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arccos(DynMatrix<T> &m);

  /// element-wise arcus cosinus-function \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arccos(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise arcus tangent-function (inplace) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arctan(DynMatrix<T> &m);

  /// element-wise arcus tangent-function \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arctan(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /// element-wise reciprocal-function (1/x) (inplace) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_reciprocal(DynMatrix<T> &m);

  /// element-wise reciprocal-function (1/x) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_reciprocal(const DynMatrix<T> &m, DynMatrix<T> &dst);

  /** @} @{ @name unary functions with scalar argument */

  /// element-wise power-function (x^exponent) (inplace) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_powc(DynMatrix<T> &m, T exponent);

  /// element-wise power-function (x^exponent)
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_powc(const DynMatrix<T> &m, T exponent, DynMatrix<T> &dst);

  /// element-wise addition of constant value (inplace)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_addc(DynMatrix<T> &m, T val);

  /// element-wise addition of constant value  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_addc(const DynMatrix<T> &m, T val, DynMatrix<T> &dst);

  /// element-wise substraction of constant value (inplace)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_subc(DynMatrix<T> &m, T val);

  /// element-wise substraction of constant value  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_subc(const DynMatrix<T> &m, T val, DynMatrix<T> &dst);

  /// element-wise division by constant value (inplace)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_divc(DynMatrix<T> &m, T  val);

  /// element-wise division by constant value  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_divc(const DynMatrix<T> &m, T val, DynMatrix<T> &dst);

  /// element-wise multiplication with constant value (inplace)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_mulc(DynMatrix<T> &m, T val);

  /// element-wise multiplication with constant value  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_mulc(const DynMatrix<T> &m, T val, DynMatrix<T> &dst);

  /** @} @{ @name binary functions */

  /// element-wise atan2 function atan2(y,x) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_arctan2(const DynMatrix<T> &my, const DynMatrix<T> &mx, DynMatrix<T> &dst)
	throw (IncompatibleMatrixDimensionException);

  /// element-wise addition  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_add(const DynMatrix<T> &m1, const DynMatrix<T> &m2, DynMatrix<T> &dst)
	throw (IncompatibleMatrixDimensionException);

  /// element-wise substraction  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_sub(const DynMatrix<T> &m1, const DynMatrix<T> &m2, DynMatrix<T> &dst)
	throw (IncompatibleMatrixDimensionException);

  /// element-wise multiplication  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_mul(const DynMatrix<T> &m1, const DynMatrix<T> &m2, DynMatrix<T> &dst)
	throw (IncompatibleMatrixDimensionException);

  /// element-wise division  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_div(const DynMatrix<T> &m1, const DynMatrix<T> &m2, DynMatrix<T> &dst)
	throw (IncompatibleMatrixDimensionException);

  /// element-wise power \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_pow(const DynMatrix<T> &m1, const DynMatrix<T> &m2, DynMatrix<T> &dst)
	throw (IncompatibleMatrixDimensionException);

  /** @} */
  /** @{ @name matrix distance measurement  */

  /// computes norm between matrix vectors \ingroup LINALG 
  /** For float and double only
      \f[ D = \left(\sum\limits_{i,j} (A_{ij}-B_{ij})^n\right)^{\frac{1}{n}} \f]
  */
  template<class T>
  T matrix_distance(const DynMatrix<T> &m1, const DynMatrix<T> &m2, T norm=2)
	throw (IncompatibleMatrixDimensionException);

  /// computes generalized Kullback-Leibler-divergence between matrix vectors \ingroup LINALG 
  /** For float and double only
      \f[ \mbox{div} = \sum\limits_{i,j} A_{ij} \cdot \log{\frac{A_{ij}}{B_{ij}}} - A_{ij} + B_{ij} \f]
  */
  template<class T>
  T matrix_divergence(const DynMatrix<T> &m1, const DynMatrix<T> &m2)
	throw (IncompatibleMatrixDimensionException);


  /** @} */
  /** @{ @name statistical functions */

  /// find minimum element of matrix (optionally find location too)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only
      @param m source matrix
      @param x if no NULL, minimum x-location (column-index) is written to *x
      @param y if no NULL, minimum y-location (row-index) is written to *y
  */
  template<class T>
  T matrix_min(const DynMatrix<T> &m, int *x=0, int *y=0);

  /// find maximum element of matrix (optionally find location too)  [IPP-optimized] \ingroup LINALG 
  /** For float and double only
      @param m source matrix
      @param x if no NULL, maximum x-location (column-index) is written to *x
      @param y if no NULL, maximum y-location (row-index) is written to *y
  */
  template<class T>
  T matrix_max(const DynMatrix<T> &m, int *x=0, int *y=0);

  /// find min- and maxinim element at once (optionally with locations)  [IPP-optimized] \ingroup LINALG 
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


  /// calculate matrix mean value  [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  T matrix_mean(const DynMatrix<T> &m);

  /// calculate matrix variance  [IPP-optimized] \ingroup LINALG 
 /** For float and double only */
  template<class T>
  T matrix_var(const DynMatrix<T> &m);

  /// calculate matrix variance with given mean \ingroup LINALG 
  /** For float and double only
      @param m source matrix
      @param mean given sample mean
      @param empiricalMean if true, then the given mean was computed from the data, too, and
                           therefore, the intermediate result 'sum of square distances' has
                           to be normalized with m.dim()-1. The reason is that, the the empirical
                           mean minimizes the variance 'per-definition', so we substract that
                           degree of freedom here. If empiricalMean is false, intermediate
                           sum-of-squares is normalized by m.dim() only.
      */
  template<class T>
  T matrix_var(const DynMatrix<T> &m, T mean, bool empiricalMean=true);

  /// computes matrix mean and variance at once  [IPP-optimized] \ingroup LINALG 
  /** For float and double only
      note thatn mean and var must not be null
  */
  template<class T>
  void matrix_meanvar(const DynMatrix<T> &m, T *mean, T*var);

  /// computes matrix standard deviation (sqrt(var)) [IPP-optimized] \ingroup LINALG 
  /** For float and double only */
  template<class T>
  T matrix_stddev(const DynMatrix<T> &m);

  /// calculate matrix standard deviation with given mean \ingroup LINALG 
  /** For float and double only
      @param m source matrix
      @param mean given sample mean
      @param empiricalMean if true, then the given mean was computed from the data, too, and
                           therefore, the intermediate result 'sum of square distances' has
                           to be normalized with m.dim()-1. The reason is that, the the empirical
                           mean minimizes the variance 'per-definition', so we substract that
                           degree of freedom here. If empiricalMean is false, intermediate
                           sum-of-squares is normalized by m.dim() only.
  */
  template<class T>
  T matrix_stddev(const DynMatrix<T> &m, T mean, bool empiricalMean=true);

  /** @} */

  /** @{  @name other functions ...*/
  /// computes alpha*a + beta*b + gamma \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_muladd(const DynMatrix<T> &a,T alpha, const DynMatrix<T> &b, T beta, T gamma, DynMatrix<T> &dst)
	throw (IncompatibleMatrixDimensionException);

  /// computes alpha*a + gamma \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_muladd(const DynMatrix<T> &a,T alpha, T gamma, DynMatrix<T> &dst);

  /// applies masking operation (m(i,j) is set to 0 if mask(i,j) is 0) (inplace) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_mask(const DynMatrix<unsigned char> &mask, DynMatrix<T> &m)
	throw (IncompatibleMatrixDimensionException);

  /// applies masking operation (m(i,j) is set to 0 if mask(i,j) is 0) \ingroup LINALG 
  /** For float and double only */
  template<class T>
  DynMatrix<T> &matrix_mask(const DynMatrix<unsigned char> &mask, const DynMatrix<T> &m, DynMatrix<T> &dst)
   	throw (IncompatibleMatrixDimensionException);

   /** @} */


  /** @{  @name special functions for transposed matrices ...*/

  /// special utility type for definition of transposed states for matrices \ingroup LINALG 
  enum transposedDef{
    NONE_T=0,
    SRC1_T=1<<0,
    SRC2_T=1<<1,
    BOTH_T=SRC1_T | SRC2_T,
  };

  /// applies matrix mutliplication on optionally transposed matrices \ingroup LINALG 
  /** sometimes, it might be more efficient to call matrix multiplication on imaginary transposed source matrices, to
      avoid having to apply an additional transposing step.

      This function is IPP accelerated. In case of having no IPP support, a trivial fallback implementation is
      provided (something like
      \code src1.transp().mult( src2.transp(),dst) ) \endcode in case of having transpDef == BOTH_T
      @param src1 left operand
      @param src2 right operand
      @param dst1 destination matrix (adapted on demand)
      @param transpDef or-ed list of transposedDef values e.g. (SRC1_T | SRC2_T) mean both matrices are transposed.
  */
  template<class T>
  DynMatrix<T> &matrix_mult_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef)
  throw (IncompatibleMatrixDimensionException);

  /// applies matrix mutliplication on optionally transposed matrices (specialized for big matrices) \ingroup LINALG 
  /** sometimes, it might be more efficient to call matrix multiplication on imaginary transposed source matrices, to
      avoid having to apply an additional transposing step.

      This function is accelerated using Intel MKL. If Intel MKL is not available, function matrix_mult_t is used as fallback.
      @param src1 left operand
      @param src2 right operand
      @param dst1 destination matrix (adapted on demand)
      @param transpDef or-ed list of transposedDef values e.g. (SRC1_T | SRC2_T) mean both matrices are transposed.
  */
  template<class T>
  DynMatrix<T> &big_matrix_mult_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef)
  throw (IncompatibleMatrixDimensionException);

  /// applies matrix addition on optionally transposed matrices \ingroup LINALG 
  template<class T>
  DynMatrix<T> &matrix_add_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef)
  throw (IncompatibleMatrixDimensionException);

  /// applies matrix substraction on optionally transposed matrices \ingroup LINALG 
  template<class T>
  DynMatrix<T> &matrix_sub_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef)
  throw (IncompatibleMatrixDimensionException);

  /** @} */


  /** @{ @name SVD functions (currently only with IPP-support)*/
#ifdef HAVE_IPP
  /// IPP based svd implementation (only available if Intel IPP Support is enabled)
  /** <b>don't use this function directly: please use icl::svd_dyn instead </b> */
  void svd_ipp_64f(const DynMatrix<icl64f> &A, DynMatrix<icl64f> &U, DynMatrix<icl64f> &s, DynMatrix<icl64f> &V) throw (ICLException);
#endif

  /// C++ fallback implementation for computing SVD
  /** The sourcecode for the internal implementation was found here:
      http://www.crbond.com/download/misc/svd.c \n
      <b>don't use this function directly: please use icl::svd_dyn instead</b>
  */
  void svd_cpp_64f(const DynMatrix<icl64f> &M, DynMatrix<icl64f> &U, DynMatrix<icl64f> &s, DynMatrix<icl64f> &Vt) throw (ICLException);
  
  /// SVD function - decomposes A into USV'
  /** Internaly, this function will always use double values. Other types are converted internally.*/
  template<class T>
  inline void svd_dyn(const DynMatrix<T> &A, DynMatrix<T> &U, DynMatrix<T> &s, DynMatrix<T> &V) throw (ICLException){
    U.setBounds(A.cols(), A.rows());
    V.setBounds(A.cols(), A.cols());
    s.setBounds(1,A.cols());
    DynMatrix<icl64f> A64f(A.cols(),A.rows()),U64f(U.cols(),U.rows()),s64f(1,s.rows()),V64f(V.cols(),V.rows());
    std::copy(A.begin(),A.end(),A64f.begin());
#ifdef HAVE_IPP
    svd_ipp_64f(A64f,U64f,s64f,V64f);
#else
    svd_cpp_64f(A64f,U64f,s64f,V64f);
#endif
    std::copy(U64f.begin(),U64f.end(),U.begin());
    std::copy(V64f.begin(),V64f.end(),V.begin());
    std::copy(s64f.begin(),s64f.end(),s.begin());
  }
  
  /** \cond */
  // SVD specialization for direct call to 64f function in order to avoid unnecessary double to double conversion before hand
  template<> inline void svd_dyn<icl64f>(const DynMatrix<icl64f> &A, DynMatrix<icl64f> &U, DynMatrix<icl64f> &s, DynMatrix<icl64f> &V) {
    U.setBounds(A.cols(), A.rows());
    V.setBounds(A.cols(), A.cols());
    s.setBounds(1,A.cols());
#ifdef HAVE_IPP
    svd_ipp_64f(A,U,s,V);
#else
    svd_cpp_64f(A,U,s,V);
#endif
  }
  /** \endcond */

  /** @}*/

}

#endif

