// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Daniel Dornbusch

#include <icl/utils/Macros.h>
#include <icl/math/DynMatrixUtils.h>
#include <icl/math/MathOps.h>
#include <icl/utils/StringUtils.h>
#include <cmath>
#include <numeric>

#include <icl/math/BlasOps.h>
#include <icl/math/LapackOps.h>


// Eigen/SVD no longer needed — SVD dispatches through BlasOps

using namespace icl::utils;

// check matrix dimensions: (m1.cols == m2.cols) and (m1.rows == m2.rows)
//#define CHECK_DIM(m1,m2,RET)
//  ICLASSERT_RETURN_VAL( (m1.cols() == m2.cols()) && (m1.rows() == m2.rows()), RET)

#define CHECK_DIM( m1, m2, RET )                                               \
  ICLASSERT_THROW( (m1.cols() == m2.cols()) && (m1.rows() == m2.rows()), IncompatibleMatrixDimensionException(__FUNCTION__) )

// check matrix dimensions: (m1.cols == m2.rows) and (m1.rows == m2.cols)
#define CHECK_DIM_T( m1, m2, RET )                                             \
  ICLASSERT_THROW( (m1.cols() == m2.rows()) && (m1.rows() == m2.cols()), IncompatibleMatrixDimensionException(__FUNCTION__) )

// check matrix dimensions: m1.cols == m2.cols
#define CHECK_DIM_CC( m1, m2, RET )                                            \
  ICLASSERT_THROW( m1.cols() == m2.cols(), IncompatibleMatrixDimensionException(__FUNCTION__) )

// check matrix dimensions: m1.cols == m2.rows
#define CHECK_DIM_CR( m1, m2, RET )                                            \
  ICLASSERT_THROW( m1.cols() == m2.rows(), IncompatibleMatrixDimensionException(__FUNCTION__) )

// check matrix dimensions: m1.rows == m2.cols
#define CHECK_DIM_RC( m1, m2, RET )                                            \
  ICLASSERT_THROW( m1.rows() == m2.cols(), IncompatibleMatrixDimensionException(__FUNCTION__) )

// check matrix dimensions: m1.rows == m2.rows
#define CHECK_DIM_RR( m1, m2, RET )                                            \
   ICLASSERT_THROW( m1.rows() == m2.rows(), IncompatibleMatrixDimensionException(__FUNCTION__) )


namespace icl::math {
  ///////////////////////////////////////////////////////////////////////////
  // Dispatch functions: unary element-wise                                //
  ///////////////////////////////////////////////////////////////////////////

#define MATH_UNARY_OP(name, func_enum)                                              \
  template<class T> DynMatrix<T>& matrix_##name(DynMatrix<T> &m) {                \
      MathOps<T>::instance()                                                       \
          .template getSelector<typename MathOps<T>::UnaryInplaceSig>(MathOp::unaryInplace) \
          .resolveOrThrow(0)->apply(UnaryMathFunc::func_enum, m.begin(), m.dim()); \
      return m;                                                                    \
  }                                                                                \
  template<class T> DynMatrix<T>& matrix_##name(const DynMatrix<T> &m, DynMatrix<T> &dst) { \
      dst.setBounds(m.cols(), m.rows());                                           \
      MathOps<T>::instance()                                                       \
          .template getSelector<typename MathOps<T>::UnaryCopySig>(MathOp::unaryCopy) \
          .resolveOrThrow(0)->apply(UnaryMathFunc::func_enum, m.begin(), dst.begin(), m.dim()); \
      return dst;                                                                  \
  }                                                                                \
  template ICLMath_API DynMatrix<float>& matrix_##name(DynMatrix<float>&);         \
  template ICLMath_API DynMatrix<double>& matrix_##name(DynMatrix<double>&);       \
  template ICLMath_API DynMatrix<float>& matrix_##name(const DynMatrix<float>&, DynMatrix<float>&); \
  template ICLMath_API DynMatrix<double>& matrix_##name(const DynMatrix<double>&, DynMatrix<double>&);

  MATH_UNARY_OP(abs, abs)
  MATH_UNARY_OP(log, log)
  MATH_UNARY_OP(exp, exp)
  MATH_UNARY_OP(sqrt, sqrt)
  MATH_UNARY_OP(sqr, sqr)
  MATH_UNARY_OP(sin, sin)
  MATH_UNARY_OP(cos, cos)
  MATH_UNARY_OP(tan, tan)
  MATH_UNARY_OP(arcsin, arcsin)
  MATH_UNARY_OP(arccos, arccos)
  MATH_UNARY_OP(arctan, arctan)
  MATH_UNARY_OP(reciprocal, reciprocal)
#undef MATH_UNARY_OP

  ///////////////////////////////////////////////////////////////////////////
  // Dispatch functions: unary-const ops                                   //
  ///////////////////////////////////////////////////////////////////////////

#define MATH_UNARY_CONST_OP(name, func_enum)                                        \
  template<class T> DynMatrix<T>& matrix_##name(DynMatrix<T> &m, T val) {         \
      MathOps<T>::instance()                                                       \
          .template getSelector<typename MathOps<T>::UnaryConstInplaceSig>(MathOp::unaryConstInplace) \
          .resolveOrThrow(0)->apply(UnaryConstFunc::func_enum, m.begin(), val, m.dim()); \
      return m;                                                                    \
  }                                                                                \
  template<class T> DynMatrix<T>& matrix_##name(const DynMatrix<T> &m, T val, DynMatrix<T> &dst) { \
      dst.setBounds(m.cols(), m.rows());                                           \
      MathOps<T>::instance()                                                       \
          .template getSelector<typename MathOps<T>::UnaryConstCopySig>(MathOp::unaryConstCopy) \
          .resolveOrThrow(0)->apply(UnaryConstFunc::func_enum, m.begin(), val, dst.begin(), m.dim()); \
      return dst;                                                                  \
  }                                                                                \
  template ICLMath_API DynMatrix<float>& matrix_##name(DynMatrix<float>&, float);  \
  template ICLMath_API DynMatrix<double>& matrix_##name(DynMatrix<double>&, double); \
  template ICLMath_API DynMatrix<float>& matrix_##name(const DynMatrix<float>&, float, DynMatrix<float>&); \
  template ICLMath_API DynMatrix<double>& matrix_##name(const DynMatrix<double>&, double, DynMatrix<double>&);

  MATH_UNARY_CONST_OP(powc, powc)
  MATH_UNARY_CONST_OP(addc, addc)
  MATH_UNARY_CONST_OP(subc, subc)
  MATH_UNARY_CONST_OP(mulc, mulc)
  MATH_UNARY_CONST_OP(divc, divc)
#undef MATH_UNARY_CONST_OP

  ///////////////////////////////////////////////////////////////////////////
  // Dispatch functions: binary element-wise                               //
  ///////////////////////////////////////////////////////////////////////////

#define MATH_BINARY_OP(name, func_enum)                                             \
  template<class T>                                                               \
  DynMatrix<T>& matrix_##name(const DynMatrix<T> &a, const DynMatrix<T> &b, DynMatrix<T> &dst) { \
      CHECK_DIM(a, b, dst);                                                       \
      dst.setBounds(a.cols(), a.rows());                                           \
      MathOps<T>::instance()                                                       \
          .template getSelector<typename MathOps<T>::BinaryCopySig>(MathOp::binaryCopy) \
          .resolveOrThrow(0)->apply(BinaryMathFunc::func_enum, a.begin(), b.begin(), dst.begin(), a.dim()); \
      return dst;                                                                  \
  }                                                                                \
  template ICLMath_API DynMatrix<float>& matrix_##name(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&); \
  template ICLMath_API DynMatrix<double>& matrix_##name(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&);

  MATH_BINARY_OP(add, add)
  MATH_BINARY_OP(sub, sub)
  MATH_BINARY_OP(mul, mul)
  MATH_BINARY_OP(div, div)
  MATH_BINARY_OP(pow, pow)
  MATH_BINARY_OP(arctan2, arctan2)
#undef MATH_BINARY_OP

  ///////////////////////////////////////////////////////////////////////////
  // Dispatch functions: min / max / minmax                                //
  ///////////////////////////////////////////////////////////////////////////

  template<class T>
  T matrix_min(const DynMatrix<T> &m, int *x, int *y) {
    ICLASSERT_RETURN_VAL(m.cols(), 0);
    return MathOps<T>::instance()
        .template getSelector<typename MathOps<T>::MinSig>(MathOp::min)
        .resolveOrThrow(0)->apply(m.begin(), m.dim(), m.cols(), x, y);
  }
  template ICLMath_API float matrix_min(const DynMatrix<float>&, int*, int*);
  template ICLMath_API double matrix_min(const DynMatrix<double>&, int*, int*);

  template<class T>
  T matrix_max(const DynMatrix<T> &m, int *x, int *y) {
    ICLASSERT_RETURN_VAL(m.cols(), 0);
    return MathOps<T>::instance()
        .template getSelector<typename MathOps<T>::MaxSig>(MathOp::max)
        .resolveOrThrow(0)->apply(m.begin(), m.dim(), m.cols(), x, y);
  }
  template ICLMath_API float matrix_max(const DynMatrix<float>&, int*, int*);
  template ICLMath_API double matrix_max(const DynMatrix<double>&, int*, int*);

  template<class T>
  void matrix_minmax(const DynMatrix<T> &m, T dst[2],
                     int *minx, int *miny,
                     int *maxx, int *maxy) {
    MathOps<T>::instance()
        .template getSelector<typename MathOps<T>::MinMaxSig>(MathOp::minmax)
        .resolveOrThrow(0)->apply(m.begin(), m.dim(), m.cols(), dst, minx, miny, maxx, maxy);
  }
  template ICLMath_API void matrix_minmax(const DynMatrix<float>&, float[2], int*, int*, int*, int*);
  template ICLMath_API void matrix_minmax(const DynMatrix<double>&, double[2], int*, int*, int*, int*);

  ///////////////////////////////////////////////////////////////////////////
  // Dispatch functions: mean / var / meanvar                              //
  ///////////////////////////////////////////////////////////////////////////

  template<class T> T matrix_mean(const DynMatrix<T> &m) {
    return MathOps<T>::instance()
        .template getSelector<typename MathOps<T>::MeanSig>(MathOp::mean)
        .resolveOrThrow(0)->apply(m.begin(), m.dim());
  }
  template ICLMath_API float matrix_mean(const DynMatrix<float>&);
  template ICLMath_API double matrix_mean(const DynMatrix<double>&);

  template<class T> struct var_functor{
    T mean,&accu;
    static inline T util_sqr(const T &t){ return t*t; }
    var_functor(const T &mean, T &accu):mean(mean),accu(accu){}
    void operator()(const T&x) const { accu += util_sqr(mean-x); }
  };

  template<class T> T matrix_var(const DynMatrix<T> &m) {
    return MathOps<T>::instance()
        .template getSelector<typename MathOps<T>::VarSig>(MathOp::var)
        .resolveOrThrow(0)->apply(m.begin(), m.dim());
  }
  template ICLMath_API float matrix_var(const DynMatrix<float>&);
  template ICLMath_API double matrix_var(const DynMatrix<double>&);

  // VAR (2) -- plain template, not dispatched (no IPP version exists)
  template<class T>
  T matrix_var(const DynMatrix<T> &m, T mean, bool empiricalMean){
    if(m.dim()<2){ return 0; }
    T var = 0;
    std::for_each(m.begin(),m.end(),var_functor<T>(mean,var));
    int norm = empiricalMean ? (m.dim()-1) : m.dim();
    return var/norm;
  }
  template ICLMath_API float matrix_var(const DynMatrix<float>&, float, bool);
  template ICLMath_API double matrix_var(const DynMatrix<double>&, double, bool);

  template<class T>
  void matrix_meanvar(const DynMatrix<T> &m, T *mean, T *var) {
    MathOps<T>::instance()
        .template getSelector<typename MathOps<T>::MeanVarSig>(MathOp::meanvar)
        .resolveOrThrow(0)->apply(m.begin(), m.dim(), mean, var);
  }
  template ICLMath_API void matrix_meanvar(const DynMatrix<float>&, float*, float*);
  template ICLMath_API void matrix_meanvar(const DynMatrix<double>&, double*, double*);


  // STDDEV ***************************
  template<class T>
  T matrix_stddev(const DynMatrix<T> &m){
    return ::sqrt(matrix_var(m));
  }

  template ICLMath_API float matrix_stddev(const DynMatrix<float>&);
  template ICLMath_API double matrix_stddev(const DynMatrix<double>&);

  template<class T>
  T matrix_stddev(const DynMatrix<T> &m, T mean, bool empiricalMean){
    return ::sqrt(matrix_var(m,mean,empiricalMean));
  }

  template ICLMath_API float matrix_stddev(const DynMatrix<float>&, float, bool);
  template ICLMath_API double matrix_stddev(const DynMatrix<double>&, double, bool);


  template<class T>
  struct muladd_functor_1{
    T alpha,gamma;
    inline muladd_functor_1(const T &alpha, const T &gamma):alpha(alpha),gamma(gamma){}
    inline T operator()(const T&a) const{
      return alpha*a + gamma;
    }
  };
  template<class T>
  struct muladd_functor_2{
    T alpha,beta,gamma;
    inline muladd_functor_2(const T &alpha, const T &beta,const T &gamma):alpha(alpha),beta(beta),gamma(gamma){}
    inline T operator()(const T&a, const T&b) const{
      return alpha*a + beta*b + gamma;
    }
  };


  template<class T>
  DynMatrix<T> &matrix_muladd(const DynMatrix<T> &a,T alpha, const DynMatrix<T> &b, T beta, T gamma, DynMatrix<T> &dst){
    CHECK_DIM(a,b,dst);
    if(!alpha) return matrix_muladd(b,beta,gamma,dst);
    if(!beta) return matrix_muladd(a,alpha,gamma,dst);
    dst.setBounds(a.cols(),a.rows());
    std::transform(a.begin(),a.end(),b.begin(),dst.begin(),muladd_functor_2<T>(alpha,beta,gamma));
    return dst;
  }
  template ICLMath_API DynMatrix<float> &matrix_muladd(const DynMatrix<float>&, float, const DynMatrix<float>&, float, float, DynMatrix<float>&);
  template ICLMath_API DynMatrix<double> &matrix_muladd(const DynMatrix<double>&, double, const DynMatrix<double>&, double, double, DynMatrix<double>&);


  template<class T>
  DynMatrix<T> &matrix_muladd(const DynMatrix<T> &a,T alpha, T gamma, DynMatrix<T> &dst){
    dst.setBounds(a.cols(),a.rows());
    if(!alpha){
      std::fill(dst.begin(),dst.end(),gamma);
    }else{
      std::transform(a.begin(),a.end(),dst.begin(),muladd_functor_1<T>(alpha,gamma));
    }
    return dst;
  }
  template ICLMath_API DynMatrix<float> &matrix_muladd(const DynMatrix<float>&, float, float, DynMatrix<float>&);
  template ICLMath_API DynMatrix<double> &matrix_muladd(const DynMatrix<double>&, double, double, DynMatrix<double>&);

  template<class T>
  struct mask_functor{
    T operator()(const unsigned char &mask,const T &m) const{
      return mask ? m : 0;
    }
  };

  template<class T>
  DynMatrix<T> &matrix_mask(const DynMatrix<unsigned char> &mask, DynMatrix<T> &m){
    CHECK_DIM(mask,m,m);
    std::transform(mask.begin(),mask.end(),m.begin(),m.begin(),mask_functor<T>());
    return m;
  }

  template ICLMath_API DynMatrix<float> &matrix_mask(const DynMatrix<unsigned char> &mask, DynMatrix<float> &m);
  template ICLMath_API DynMatrix<double> &matrix_mask(const DynMatrix<unsigned char> &mask, DynMatrix<double> &m);

    /// applies masking operation (m(i,j) is set to 0 if mask(i,j) is 0)
  template<class T>
  DynMatrix<T> &matrix_mask(const DynMatrix<unsigned char> &mask, const DynMatrix<T> &m, DynMatrix<T> &dst){
    CHECK_DIM(mask,m,dst);
    dst.setBounds(m.cols(),m.rows());
    std::transform(mask.begin(),mask.end(),m.begin(),dst.begin(),mask_functor<T>());
    return dst;
  }

  template ICLMath_API DynMatrix<float> &matrix_mask(const DynMatrix<unsigned char> &mask, const DynMatrix<float> &m, DynMatrix<float> &dst);
  template ICLMath_API DynMatrix<double> &matrix_mask(const DynMatrix<unsigned char> &mask, const DynMatrix<double> &m, DynMatrix<double> &dst);



  template<class T>
  struct matrix_distance_op{
    T e;
    matrix_distance_op(const T &e):e(e){}
    inline T operator()(const T &a, const T&b) const{  return ::pow(a-b,e);   }
  };

  template<class T>
  T matrix_distance(const DynMatrix<T> &m1, const DynMatrix<T> &m2, T norm){
    CHECK_DIM(m1,m2,-1);
    ICLASSERT_RETURN_VAL(norm > 0,-1);
    T result = std::inner_product(m1.begin(),m1.end(),m2.begin(),T(0),std::plus<T>(),matrix_distance_op<T>(norm));
    return pow(result,1/norm);
  }

  template ICLMath_API float matrix_distance(const DynMatrix<float> &m1, const DynMatrix<float> &m2, float);
  template ICLMath_API double matrix_distance(const DynMatrix<double> &, const DynMatrix<double> &, double);



  template<class T>
  struct matrix_divergence_op{
    inline T operator()(const T &a, const T&b) const{ return a * log(a/b) - a + b;  }
  };

  template<class T>
  T matrix_divergence(const DynMatrix<T> &m1, const DynMatrix<T> &m2){
    CHECK_DIM(m1,m2,-1);
    return std::inner_product(m1.begin(),m1.end(),m2.begin(),T(0),std::plus<T>(),matrix_divergence_op<T>());
  }

  template ICLMath_API float matrix_divergence(const DynMatrix<float>&, const DynMatrix<float>&);
  template ICLMath_API double matrix_divergence(const DynMatrix<double>&, const DynMatrix<double>&);

  /// matrix functions for transposed matrices ...



  template<class T>
  DynMatrix<T> &matrix_mult_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef){
    // Generic fallback: explicit transpose + loop-based mult
    switch(transpDef){
      case NONE_T: return src1.mult(src2,dst);
      case SRC1_T: return src1.transp().mult(src2,dst);
      case SRC2_T: return src1.mult(src2.transp(),dst);
      case BOTH_T: return src1.transp().mult(src2.transp(),dst);
      default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
    }
    return dst;
  }

  // float/double: use gemm transpose flags directly (no temporary copies)
  namespace {
    template<class T>
    DynMatrix<T> &gemm_mult_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef){
      auto* impl = BlasOps<T>::instance()
          .template getSelector<typename BlasOps<T>::GemmSig>(BlasOp::gemm)
          .resolveOrThrow();
      bool tA = (transpDef == SRC1_T || transpDef == BOTH_T);
      bool tB = (transpDef == SRC2_T || transpDef == BOTH_T);
      int M = tA ? src1.cols() : src1.rows();
      int N = tB ? src2.rows() : src2.cols();
      int K = tA ? src1.rows() : src1.cols();
      dst.setBounds(N, M);
      impl->apply(tA, tB, M, N, K, T(1),
                   src1.begin(), src1.cols(), src2.begin(), src2.cols(),
                   T(0), dst.begin(), N);
      return dst;
    }
  }

  template<>
  DynMatrix<float> &matrix_mult_t(const DynMatrix<float> &src1, const DynMatrix<float> &src2, DynMatrix<float> &dst, int transpDef){
    return gemm_mult_t(src1, src2, dst, transpDef);
  }
  template<>
  DynMatrix<double> &matrix_mult_t(const DynMatrix<double> &src1, const DynMatrix<double> &src2, DynMatrix<double> &dst, int transpDef){
    return gemm_mult_t(src1, src2, dst, transpDef);
  }

  template<class T>
  DynMatrix<T> &matrix_add_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef){
    switch(transpDef){
      case NONE_T: return matrix_add(src1,src2,dst);
      case SRC1_T: return matrix_add(src1.transp(),src2,dst);
      case SRC2_T: return matrix_add(src1,src2.transp(),dst);
      case BOTH_T: return matrix_add(src1.transp(),src2.transp(),dst);
      default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
    }
    return dst;
  }

 template<class T>
  DynMatrix<T> &matrix_sub_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef){
    switch(transpDef){
      case NONE_T: return matrix_sub(src1,src2,dst);
      case SRC1_T: return matrix_sub(src1.transp(),src2,dst);
      case SRC2_T: return matrix_sub(src1,src2.transp(),dst);
      case BOTH_T: return matrix_sub(src1.transp(),src2.transp(),dst);
      default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
    }
    return dst;
  }


  // matrix_mult_t<float/double> are explicit specializations (no instantiation needed).
  template ICLMath_API DynMatrix<double> &matrix_add_t(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&, int);
  template ICLMath_API DynMatrix<float> &matrix_add_t(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&, int);
  template ICLMath_API DynMatrix<double> &matrix_sub_t(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&, int);
  template ICLMath_API DynMatrix<float> &matrix_sub_t(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&, int);

  // MKL cblas_xgemm specializations removed — now dispatched via BlasOps<T>::gemm
  // (BlasOps_Mkl.cpp registers the MKL backend, BlasOps_Cpp.cpp the C++ fallback)




// undefine macros
#undef CHECK_DIM
#undef CHECK_DIM_T
#undef CHECK_DIM_CC
#undef CHECK_DIM_CR
#undef CHECK_DIM_RC
#undef CHECK_DIM_RR

// SVD now dispatches through LapackOps<T>::gesdd.
// Old svd_internal + svd_eigen code removed — preserved in git history.

  template<class T>
  void svd_dyn(const DynMatrix<T> &M, DynMatrix<T> &U, DynMatrix<T> &s, DynMatrix<T> &V) {
    unsigned int rows = M.rows(), cols = M.cols();
    unsigned int sdim = iclMin(rows, cols);
    U.setBounds(rows, rows);
    V.setBounds(cols, cols);
    s.setBounds(1, sdim);

    // gesdd modifies A in-place — copy
    DynMatrix<T> A_copy(cols, rows);
    std::copy(M.begin(), M.end(), A_copy.begin());

    auto* impl = LapackOps<T>::instance()
        .template getSelector<typename LapackOps<T>::GesddSig>(LapackOp::gesdd)
        .resolveOrThrow();

    // gesdd returns V-transpose; svd_dyn convention returns V
    DynMatrix<T> Vt(cols, cols);
    int info = impl->apply('A', rows, cols, A_copy.data(), cols,
                            s.data(), U.data(), rows, Vt.data(), cols);
    if(info != 0) {
      throw ICLException("error in svd: no convergence (info=" + str(info) + ")");
    }

    // Transpose Vt → V
    for(unsigned int r = 0; r < cols; ++r)
      for(unsigned int c = 0; c < cols; ++c)
        V(r, c) = Vt(c, r);
  }
  template ICLMath_API void svd_dyn(const DynMatrix<icl32f>&, DynMatrix<icl32f>&, DynMatrix<icl32f>&, DynMatrix<icl32f>&);
  template ICLMath_API void svd_dyn(const DynMatrix<icl64f>&, DynMatrix<icl64f>&, DynMatrix<icl64f>&, DynMatrix<icl64f>&);


  } // namespace icl::math