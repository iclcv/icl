/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/DynMatrixUtils.cpp                 **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter, Daniel Dornbusch,                 **
**          Sergius Gaulik                                         **
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

#include <ICLUtils/Macros.h>
#include <ICLMath/DynMatrixUtils.h>
#include <ICLMath/MathOps.h>
#include <ICLUtils/StringUtils.h>
#include <cmath>

#ifdef ICL_HAVE_MKL
  #include "mkl_types.h"
  #include "mkl_cblas.h"
#endif


#ifdef ICL_HAVE_EIGEN3
  #include <Eigen/SVD>
#endif

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


namespace icl{
  namespace math{

    ///////////////////////////////////////////////////////////////////////////
    // C++ backend implementations for MathOps dispatch                    //
    ///////////////////////////////////////////////////////////////////////////

    // --- Helpers ---

    static inline double reciprocal_helper(double x) { return x ? 1.0/x : 0; }
    static inline float reciprocal_helper(float x) { return x ? 1.0f/x : 0; }

    // --- Stats backends ---

    static float cpp_mean_f(const float* data, int len) {
      return std::accumulate(data, data+len, 0.0f) / len;
    }
    static double cpp_mean_d(const double* data, int len) {
      return std::accumulate(data, data+len, 0.0) / len;
    }

    static float cpp_var_f(const float* data, int len) {
      if(len < 2) return 0;
      float m = cpp_mean_f(data, len);
      float accu = 0;
      for(int i = 0; i < len; ++i) { float d = m - data[i]; accu += d*d; }
      return accu / (len - 1);
    }
    static double cpp_var_d(const double* data, int len) {
      if(len < 2) return 0;
      double m = cpp_mean_d(data, len);
      double accu = 0;
      for(int i = 0; i < len; ++i) { double d = m - data[i]; accu += d*d; }
      return accu / (len - 1);
    }

    static void cpp_meanvar_f(const float* data, int len, float* mean, float* var) {
      float m = cpp_mean_f(data, len);
      if(mean) *mean = m;
      if(var) {
        float accu = 0;
        for(int i = 0; i < len; ++i) { float d = m - data[i]; accu += d*d; }
        *var = (len > 1) ? accu / (len - 1) : 0.0f;
      }
    }
    static void cpp_meanvar_d(const double* data, int len, double* mean, double* var) {
      double m = cpp_mean_d(data, len);
      if(mean) *mean = m;
      if(var) {
        double accu = 0;
        for(int i = 0; i < len; ++i) { double d = m - data[i]; accu += d*d; }
        *var = (len > 1) ? accu / (len - 1) : 0.0;
      }
    }

    // --- Min/Max/MinMax backends ---

    static float cpp_min_f(const float* data, int len, int cols, int* x, int* y) {
      const float* a = std::min_element(data, data+len);
      int idx = static_cast<int>(a - data);
      if(x) *x = idx % cols;
      if(y) *y = idx / cols;
      return *a;
    }
    static double cpp_min_d(const double* data, int len, int cols, int* x, int* y) {
      const double* a = std::min_element(data, data+len);
      int idx = static_cast<int>(a - data);
      if(x) *x = idx % cols;
      if(y) *y = idx / cols;
      return *a;
    }

    static float cpp_max_f(const float* data, int len, int cols, int* x, int* y) {
      const float* a = std::max_element(data, data+len);
      int idx = static_cast<int>(a - data);
      if(x) *x = idx % cols;
      if(y) *y = idx / cols;
      return *a;
    }
    static double cpp_max_d(const double* data, int len, int cols, int* x, int* y) {
      const double* a = std::max_element(data, data+len);
      int idx = static_cast<int>(a - data);
      if(x) *x = idx % cols;
      if(y) *y = idx / cols;
      return *a;
    }

    static void cpp_minmax_f(const float* data, int len, int cols, float* dst,
                             int* minx, int* miny, int* maxx, int* maxy) {
      dst[0] = cpp_min_f(data, len, cols, minx, miny);
      dst[1] = cpp_max_f(data, len, cols, maxx, maxy);
    }
    static void cpp_minmax_d(const double* data, int len, int cols, double* dst,
                             int* minx, int* miny, int* maxx, int* maxy) {
      dst[0] = cpp_min_d(data, len, cols, minx, miny);
      dst[1] = cpp_max_d(data, len, cols, maxx, maxy);
    }

    // --- Unary element-wise backends ---

    template<class T>
    static void cpp_unaryInplace(UnaryMathFunc func, T* data, int len) {
      switch(func) {
        case UnaryMathFunc::abs:        std::for_each(data, data+len, [](T& v){ v = std::fabs(v); }); break;
        case UnaryMathFunc::log:        std::for_each(data, data+len, [](T& v){ v = std::log(v); }); break;
        case UnaryMathFunc::exp:        std::for_each(data, data+len, [](T& v){ v = std::exp(v); }); break;
        case UnaryMathFunc::sqrt:       std::for_each(data, data+len, [](T& v){ v = std::sqrt(v); }); break;
        case UnaryMathFunc::sqr:        std::for_each(data, data+len, [](T& v){ v = v*v; }); break;
        case UnaryMathFunc::sin:        std::for_each(data, data+len, [](T& v){ v = std::sin(v); }); break;
        case UnaryMathFunc::cos:        std::for_each(data, data+len, [](T& v){ v = std::cos(v); }); break;
        case UnaryMathFunc::tan:        std::for_each(data, data+len, [](T& v){ v = std::tan(v); }); break;
        case UnaryMathFunc::arcsin:     std::for_each(data, data+len, [](T& v){ v = std::asin(v); }); break;
        case UnaryMathFunc::arccos:     std::for_each(data, data+len, [](T& v){ v = std::acos(v); }); break;
        case UnaryMathFunc::arctan:     std::for_each(data, data+len, [](T& v){ v = std::atan(v); }); break;
        case UnaryMathFunc::reciprocal: std::for_each(data, data+len, [](T& v){ v = reciprocal_helper(v); }); break;
      }
    }
    static void cpp_unaryInplace_f(UnaryMathFunc func, float* data, int len) {
      cpp_unaryInplace<float>(func, data, len);
    }
    static void cpp_unaryInplace_d(UnaryMathFunc func, double* data, int len) {
      cpp_unaryInplace<double>(func, data, len);
    }

    template<class T>
    static void cpp_unaryCopy(UnaryMathFunc func, const T* src, T* dst, int len) {
      switch(func) {
        case UnaryMathFunc::abs:        std::transform(src, src+len, dst, [](T v){ return std::fabs(v); }); break;
        case UnaryMathFunc::log:        std::transform(src, src+len, dst, [](T v){ return std::log(v); }); break;
        case UnaryMathFunc::exp:        std::transform(src, src+len, dst, [](T v){ return std::exp(v); }); break;
        case UnaryMathFunc::sqrt:       std::transform(src, src+len, dst, [](T v){ return std::sqrt(v); }); break;
        case UnaryMathFunc::sqr:        std::transform(src, src+len, dst, [](T v){ return v*v; }); break;
        case UnaryMathFunc::sin:        std::transform(src, src+len, dst, [](T v){ return std::sin(v); }); break;
        case UnaryMathFunc::cos:        std::transform(src, src+len, dst, [](T v){ return std::cos(v); }); break;
        case UnaryMathFunc::tan:        std::transform(src, src+len, dst, [](T v){ return std::tan(v); }); break;
        case UnaryMathFunc::arcsin:     std::transform(src, src+len, dst, [](T v){ return std::asin(v); }); break;
        case UnaryMathFunc::arccos:     std::transform(src, src+len, dst, [](T v){ return std::acos(v); }); break;
        case UnaryMathFunc::arctan:     std::transform(src, src+len, dst, [](T v){ return std::atan(v); }); break;
        case UnaryMathFunc::reciprocal: std::transform(src, src+len, dst, [](T v){ return reciprocal_helper(v); }); break;
      }
    }
    static void cpp_unaryCopy_f(UnaryMathFunc func, const float* src, float* dst, int len) {
      cpp_unaryCopy<float>(func, src, dst, len);
    }
    static void cpp_unaryCopy_d(UnaryMathFunc func, const double* src, double* dst, int len) {
      cpp_unaryCopy<double>(func, src, dst, len);
    }

    // --- Unary-const backends ---

    template<class T>
    static void cpp_unaryConstInplace(UnaryConstFunc func, T* data, T val, int len) {
      switch(func) {
        case UnaryConstFunc::powc: std::for_each(data, data+len, [val](T& v){ v = std::pow(v, val); }); break;
        case UnaryConstFunc::addc: std::for_each(data, data+len, [val](T& v){ v = v + val; }); break;
        case UnaryConstFunc::subc: std::for_each(data, data+len, [val](T& v){ v = v - val; }); break;
        case UnaryConstFunc::mulc: std::for_each(data, data+len, [val](T& v){ v = v * val; }); break;
        case UnaryConstFunc::divc: std::for_each(data, data+len, [val](T& v){ v = v / val; }); break;
      }
    }
    static void cpp_unaryConstInplace_f(UnaryConstFunc func, float* data, float val, int len) {
      cpp_unaryConstInplace<float>(func, data, val, len);
    }
    static void cpp_unaryConstInplace_d(UnaryConstFunc func, double* data, double val, int len) {
      cpp_unaryConstInplace<double>(func, data, val, len);
    }

    template<class T>
    static void cpp_unaryConstCopy(UnaryConstFunc func, const T* src, T val, T* dst, int len) {
      switch(func) {
        case UnaryConstFunc::powc: std::transform(src, src+len, dst, [val](T v){ return std::pow(v, val); }); break;
        case UnaryConstFunc::addc: std::transform(src, src+len, dst, [val](T v){ return v + val; }); break;
        case UnaryConstFunc::subc: std::transform(src, src+len, dst, [val](T v){ return v - val; }); break;
        case UnaryConstFunc::mulc: std::transform(src, src+len, dst, [val](T v){ return v * val; }); break;
        case UnaryConstFunc::divc: std::transform(src, src+len, dst, [val](T v){ return v / val; }); break;
      }
    }
    static void cpp_unaryConstCopy_f(UnaryConstFunc func, const float* src, float val, float* dst, int len) {
      cpp_unaryConstCopy<float>(func, src, val, dst, len);
    }
    static void cpp_unaryConstCopy_d(UnaryConstFunc func, const double* src, double val, double* dst, int len) {
      cpp_unaryConstCopy<double>(func, src, val, dst, len);
    }

    // --- Binary element-wise backends ---

    template<class T>
    static void cpp_binaryCopy(BinaryMathFunc func, const T* a, const T* b, T* dst, int len) {
      switch(func) {
        case BinaryMathFunc::add:     std::transform(a, a+len, b, dst, [](T x, T y){ return x + y; }); break;
        case BinaryMathFunc::sub:     std::transform(a, a+len, b, dst, [](T x, T y){ return x - y; }); break;
        case BinaryMathFunc::mul:     std::transform(a, a+len, b, dst, [](T x, T y){ return x * y; }); break;
        case BinaryMathFunc::div:     std::transform(a, a+len, b, dst, [](T x, T y){ return x / y; }); break;
        case BinaryMathFunc::pow:     std::transform(a, a+len, b, dst, [](T x, T y){ return std::pow(x, y); }); break;
        case BinaryMathFunc::arctan2: std::transform(a, a+len, b, dst, [](T x, T y){ return std::atan2(x, y); }); break;
      }
    }
    static void cpp_binaryCopy_f(BinaryMathFunc func, const float* a, const float* b, float* dst, int len) {
      cpp_binaryCopy<float>(func, a, b, dst, len);
    }
    static void cpp_binaryCopy_d(BinaryMathFunc func, const double* a, const double* b, double* dst, int len) {
      cpp_binaryCopy<double>(func, a, b, dst, len);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Backend registration via static init                                  //
    ///////////////////////////////////////////////////////////////////////////

    static int _mathops_reg = [] {
      using namespace icl::utils;

      // --- float ---
      auto cpp_f = MathOps<float>::instance().backends(Backend::Cpp);
      cpp_f.add<MathOps<float>::MeanSig>(MathOp::mean, cpp_mean_f);
      cpp_f.add<MathOps<float>::VarSig>(MathOp::var, cpp_var_f);
      cpp_f.add<MathOps<float>::MeanVarSig>(MathOp::meanvar, cpp_meanvar_f);
      cpp_f.add<MathOps<float>::MinSig>(MathOp::min, cpp_min_f);
      cpp_f.add<MathOps<float>::MaxSig>(MathOp::max, cpp_max_f);
      cpp_f.add<MathOps<float>::MinMaxSig>(MathOp::minmax, cpp_minmax_f);
      cpp_f.add<MathOps<float>::UnaryInplaceSig>(MathOp::unaryInplace, cpp_unaryInplace_f);
      cpp_f.add<MathOps<float>::UnaryCopySig>(MathOp::unaryCopy, cpp_unaryCopy_f);
      cpp_f.add<MathOps<float>::UnaryConstInplaceSig>(MathOp::unaryConstInplace, cpp_unaryConstInplace_f);
      cpp_f.add<MathOps<float>::UnaryConstCopySig>(MathOp::unaryConstCopy, cpp_unaryConstCopy_f);
      cpp_f.add<MathOps<float>::BinaryCopySig>(MathOp::binaryCopy, cpp_binaryCopy_f);

      // --- double ---
      auto cpp_d = MathOps<double>::instance().backends(Backend::Cpp);
      cpp_d.add<MathOps<double>::MeanSig>(MathOp::mean, cpp_mean_d);
      cpp_d.add<MathOps<double>::VarSig>(MathOp::var, cpp_var_d);
      cpp_d.add<MathOps<double>::MeanVarSig>(MathOp::meanvar, cpp_meanvar_d);
      cpp_d.add<MathOps<double>::MinSig>(MathOp::min, cpp_min_d);
      cpp_d.add<MathOps<double>::MaxSig>(MathOp::max, cpp_max_d);
      cpp_d.add<MathOps<double>::MinMaxSig>(MathOp::minmax, cpp_minmax_d);
      cpp_d.add<MathOps<double>::UnaryInplaceSig>(MathOp::unaryInplace, cpp_unaryInplace_d);
      cpp_d.add<MathOps<double>::UnaryCopySig>(MathOp::unaryCopy, cpp_unaryCopy_d);
      cpp_d.add<MathOps<double>::UnaryConstInplaceSig>(MathOp::unaryConstInplace, cpp_unaryConstInplace_d);
      cpp_d.add<MathOps<double>::UnaryConstCopySig>(MathOp::unaryConstCopy, cpp_unaryConstCopy_d);
      cpp_d.add<MathOps<double>::BinaryCopySig>(MathOp::binaryCopy, cpp_binaryCopy_d);

      return 0;
    }();

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
      switch(transpDef){
        case NONE_T: return src1.mult(src2,dst);
        case SRC1_T: return src1.transp().mult(src2,dst);
        case SRC2_T: return src1.mult(src2.transp(),dst);
        case BOTH_T: return src1.transp().mult(src2.transp(),dst);
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }

    template<class T>
    DynMatrix<T> &big_matrix_mult_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef){
      return matrix_mult_t( src1, src2, dst, transpDef );
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


  // ippm matrix add/sub/mul were removed from modern IPP (ippm module dropped).
  // TODO: re-add via BackendDispatching framework if needed for performance.
  #if 0 // was: ICL_HAVE_IPP (ippm functions no longer available)
    template<class T, typename func>
    DynMatrix<T> &ipp_func_t_call(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, func f){
      IppStatus status = f(src1.begin(),src1.stride1(),src1.stride2(),src1.cols(),src1.rows(),
                           src2.begin(),src2.stride1(),src2.stride2(),src2.cols(),src2.rows(),
                           dst.begin(),dst.stride1(), dst.stride2());
      if(status != ippStsNoErr){
        throw IncompatibleMatrixDimensionException(ippGetStatusString(status));
      }
      return dst;
    }

  template<class T, typename func>
    DynMatrix<T> &ipp_func_t_call_2(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, func f){
      IppStatus status = f(src1.begin(),src1.stride1(),src1.stride2(),
                           src2.begin(),src2.stride1(),src2.stride2(),
                           dst.begin(),dst.stride1(), dst.stride2(),
                           dst.cols(),dst.rows());
      if(status != ippStsNoErr){
        throw IncompatibleMatrixDimensionException(ippGetStatusString(status));
      }
      return dst;
    }


    template<> ICLMath_API DynMatrix<float> &matrix_add_t(const DynMatrix<float> &src1, const DynMatrix<float> &src2, DynMatrix<float> &dst, int transpDef){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          return ipp_func_t_call_2(src1,src2,dst,ippmAdd_mm_32f);
        case SRC1_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src2.cols(),src2.rows());
          return ipp_func_t_call_2(src1,src2,dst,ippmAdd_tm_32f);
        case SRC2_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          return ipp_func_t_call_2(src2,src1,dst,ippmAdd_tm_32f);
        case BOTH_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.rows(),src1.cols());
          return ipp_func_t_call_2(src1,src2,dst,ippmAdd_tt_32f);
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }
    template<> ICLMath_API DynMatrix<double> &matrix_add_t(const DynMatrix<double> &src1, const DynMatrix<double> &src2, DynMatrix<double> &dst, int transpDef){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          return ipp_func_t_call_2(src1,src2,dst,ippmAdd_mm_64f);
        case SRC1_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src2.cols(),src2.rows());
          return ipp_func_t_call_2(src1,src2,dst,ippmAdd_tm_64f);
        case SRC2_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          return ipp_func_t_call_2(src2,src1,dst,ippmAdd_tm_64f);
        case BOTH_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.rows(),src1.cols());
          return ipp_func_t_call_2(src1,src2,dst,ippmAdd_tt_64f);
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }

    template<> ICLMath_API DynMatrix<float> &matrix_sub_t(const DynMatrix<float> &src1, const DynMatrix<float> &src2, DynMatrix<float> &dst, int transpDef){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          return ipp_func_t_call_2(src1,src2,dst,ippmSub_mm_32f);
        case SRC1_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src2.cols(),src2.rows());
          return ipp_func_t_call_2(src1,src2,dst,ippmSub_tm_32f);
        case SRC2_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          return ipp_func_t_call_2(src2,src1,dst,ippmSub_tm_32f);
        case BOTH_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.rows(),src1.cols());
          return ipp_func_t_call_2(src1,src2,dst,ippmSub_tt_32f);
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }
    template<> ICLMath_API DynMatrix<double> &matrix_sub_t(const DynMatrix<double> &src1, const DynMatrix<double> &src2, DynMatrix<double> &dst, int transpDef){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          return ipp_func_t_call_2(src1,src2,dst,ippmSub_mm_64f);
        case SRC1_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src2.cols(),src2.rows());
          return ipp_func_t_call_2(src1,src2,dst,ippmSub_tm_64f);
        case SRC2_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          return ipp_func_t_call_2(src2,src1,dst,ippmSub_tm_64f);
        case BOTH_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.rows(),src1.cols());
          return ipp_func_t_call_2(src1,src2,dst,ippmSub_tt_64f);
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }


    template<> ICLMath_API DynMatrix<float> &matrix_mult_t(const DynMatrix<float> &src1, const DynMatrix<float> &src2, DynMatrix<float> &dst, int transpDef){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM_CR(src1,src2,dst);
          dst.setBounds(src2.cols(),src1.rows());
          return ipp_func_t_call(src1,src2,dst,ippmMul_mm_32f);
        case SRC1_T:
          CHECK_DIM_RR(src1,src2,dst);
          dst.setBounds(src2.cols(),src1.cols());
          return ipp_func_t_call(src1,src2,dst,ippmMul_tm_32f);
        case SRC2_T:
          CHECK_DIM_CC(src1,src2,dst);
          dst.setBounds(src2.rows(),src1.rows());
          return ipp_func_t_call(src1,src2,dst,ippmMul_mt_32f);
        case BOTH_T:
          CHECK_DIM_RC(src1,src2,dst);
          dst.setBounds(src2.rows(),src1.cols());
          return ipp_func_t_call(src1,src2,dst,ippmMul_tt_32f);
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }

    template<> ICLMath_API DynMatrix<double> &matrix_mult_t(const DynMatrix<double> &src1, const DynMatrix<double> &src2, DynMatrix<double> &dst, int transpDef){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM_CR(src1,src2,dst);
          dst.setBounds(src2.cols(),src1.rows());
          return ipp_func_t_call(src1,src2,dst,ippmMul_mm_64f);
        case SRC1_T:
          CHECK_DIM_RR(src1,src2,dst);
          dst.setBounds(src2.cols(),src1.cols());
          return ipp_func_t_call(src1,src2,dst,ippmMul_tm_64f);
        case SRC2_T:
          CHECK_DIM_CC(src1,src2,dst);
          dst.setBounds(src2.rows(),src1.rows());
          return ipp_func_t_call(src1,src2,dst,ippmMul_mt_64f);
        case BOTH_T:
          CHECK_DIM_RC(src1,src2,dst);
          dst.setBounds(src2.rows(),src1.cols());
          return ipp_func_t_call(src1,src2,dst,ippmMul_tt_64f);
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }

  #else

    template ICLMath_API DynMatrix<double> &matrix_mult_t(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&, int);
    template ICLMath_API DynMatrix<float> &matrix_mult_t(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&, int);

    template ICLMath_API DynMatrix<double> &matrix_add_t(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&, int);
    template ICLMath_API DynMatrix<float> &matrix_add_t(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&, int);


    template ICLMath_API DynMatrix<double> &matrix_sub_t(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&, int);
    template ICLMath_API DynMatrix<float> &matrix_sub_t(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&, int);

  #endif // ICL_HAVE_IPP


    // optimized specialization only if MKL was found
  #ifdef ICL_HAVE_MKL
    template<> ICLMath_API DynMatrix<float> &big_matrix_mult_t(const DynMatrix<float> &src1, const DynMatrix<float> &src2, DynMatrix<float> &dst, int transpDef){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM_CR(src1,src2,dst);
          dst.setBounds(src2.cols(),src1.rows());
          cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, src1.rows(), src2.cols(), src1.cols(), 1.0, src1.begin(), src1.cols(),
                      src2.begin(), src2.cols(), 0.0, dst.begin(), src2.cols());
          return dst;
        case SRC1_T:
          CHECK_DIM_RR(src1,src2,dst);
          dst.setBounds(src2.cols(),src1.cols());
          cblas_sgemm(CblasRowMajor, CblasTrans, CblasNoTrans, src1.cols(), src2.cols(), src1.rows(), 1.0, src1.begin(), src1.cols(),
                      src2.begin(), src2.cols(), 0.0, dst.begin(), src2.cols());
          return dst;
        case SRC2_T:
          CHECK_DIM_CC(src1,src2,dst);
          dst.setBounds(src2.rows(),src1.rows());
          cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, src1.rows(), src2.rows(), src1.cols(), 1.0, src1.begin(), src1.cols(),
                      src2.begin(), src2.cols(), 0.0, dst.begin(), src2.rows());
          return dst;
        case BOTH_T:
          CHECK_DIM_RC(src1,src2,dst);
          dst.setBounds(src2.rows(),src1.cols());
          cblas_sgemm(CblasRowMajor, CblasTrans, CblasTrans, src1.cols(), src2.rows(), src1.rows(), 1.0, src1.begin(), src1.cols(),
                      src2.begin(), src2.cols(), 0.0, dst.begin(), src2.rows());
          return dst;
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }
    template<> ICLMath_API DynMatrix<double> &big_matrix_mult_t(const DynMatrix<double> &src1, const DynMatrix<double> &src2, DynMatrix<double> &dst, int transpDef){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM_CR(src1,src2,dst);
          dst.setBounds(src2.cols(),src1.rows());
          cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, src1.rows(), src2.cols(), src1.cols(), 1.0, src1.begin(), src1.cols(),
                      src2.begin(), src2.cols(), 0.0, dst.begin(), src2.cols());
          return dst;
        case SRC1_T:
          CHECK_DIM_RR(src1,src2,dst);
          dst.setBounds(src2.cols(),src1.cols());
          cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans, src1.cols(), src2.cols(), src1.rows(), 1.0, src1.begin(), src1.cols(),
                      src2.begin(), src2.cols(), 0.0, dst.begin(), src2.cols());
          return dst;
        case SRC2_T:
          CHECK_DIM_CC(src1,src2,dst);
          dst.setBounds(src2.rows(),src1.rows());
          cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, src1.rows(), src2.rows(), src1.cols(), 1.0, src1.begin(), src1.cols(),
                      src2.begin(), src2.cols(), 0.0, dst.begin(), src2.rows());
          return dst;
        case BOTH_T:
          CHECK_DIM_RC(src1,src2,dst);
          dst.setBounds(src2.rows(),src1.cols());
          cblas_dgemm(CblasRowMajor, CblasTrans, CblasTrans, src1.cols(), src2.rows(), src1.rows(), 1.0, src1.begin(), src1.cols(),
                      src2.begin(), src2.cols(), 0.0, dst.begin(), src2.rows());
          return dst;
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }

  #endif


    template ICLMath_API DynMatrix<double> &big_matrix_mult_t(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&, int);
    template ICLMath_API DynMatrix<float> &big_matrix_mult_t(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&, int);


  // undefine macros
  #undef CHECK_DIM
  #undef CHECK_DIM_T
  #undef CHECK_DIM_CC
  #undef CHECK_DIM_CR
  #undef CHECK_DIM_RC
  #undef CHECK_DIM_RR

  #ifndef ICL_HAVE_EIGEN3
    // C++ fallback for SVD
    static int svd_internal(int m,int n,int withu,int withv,icl64f eps,icl64f tol,
                            const DynMatrix<icl64f> &a,icl64f *q, DynMatrix<icl64f> &u, DynMatrix<icl64f> &v){
      /** found here: http://www.crbond.com/download/misc/svd.c*/
      /*  svd.c -- Singular value decomposition. Translated to 'C' from the
          *           original Algol code in "Handbook for Automatic Computation,
          *           vol. II, Linear Algebra", Springer-Verlag.
          *
          *  (C) 2000, C. Bond. All rights reserved.
          *
          *  This is almost an exact translation from the original, except that
          *  an iteration counter is added to prevent stalls. This corresponds
          *  to similar changes in other translations.
          *
          *  Returns an error code = 0, if no errors and 'k' if a failure to
          *  converge at the 'kth' singular value.
          *
          */
      /** this function was modified to match the internal type for matrices */
      int i,j,k,l(0),l1,iter,retval;
      icl64f c,f,g,h,s,x,y,z;
      std::vector<icl64f> e(n);
      retval = 0;

      /* Copy 'a' to 'u' */
      for (i=0;i<m;i++) {
        for (j=0;j<n;j++)
            u(j,i) = a(j,i);
      }

      /* Householder's reduction to bidiagonal form. */
      g = x = 0.0;
      for (i=0;i<n;i++) {
        e[i] = g;
        s = 0.0;
        l = i+1;
        for (j=i;j<m;j++)
          s += u(i,j)*u(i,j);
        if (s < tol)
          g = 0.0;
        else {
          f = u(i,i);
          g = (f < 0) ? sqrt(s) : -sqrt(s);
          h = f * g - s;
          u(i,i) = f - g;
          for (j=l;j<n;j++) {
            s = 0.0;
            for (k=i;k<m;k++)
              s += u(i,k) * u(j,k);
            f = s / h;
            for (k=i;k<m;k++)
              u(j,k) += f * u(i,k);
          } /* end j */
        } /* end s */
        q[i] = g;
        s = 0.0;
        for (j=l;j<n;j++)
          s += u(j,i) * u(j,i);
        if (s < tol)
          g = 0.0;
        else {
          f = u(i+1,i);
          g = (f < 0) ? sqrt(s) : -sqrt(s);
          h = f * g - s;
          u(i+1,i) = f - g;
          for (j=l;j<n;j++)
            e[j] = u(j,i)/h;
          for (j=l;j<m;j++) {
            s = 0.0;
            for (k=l;k<n;k++)
              s += u(k,j) * u(k,i);
            for (k=l;k<n;k++)
              u(k,j) += s * e[k];
          } /* end j */
        } /* end s */
        /**/
        y = fabs(q[i]) + fabs(e[i]);
        if (y > x)
          x = y;
      } /* end i */

      /* accumulation of right-hand transformations */
      if (withv) {
        for (i=n-1;i>=0;i--) {
          if (g != 0.0) {
            h = u(i+1,i) * g;
            for (j=l;j<n;j++)
              v(i,j) = u(j,i)/h;
            for (j=l;j<n;j++) {
              s = 0.0;
              for (k=l;k<n;k++)
                s += u(k,i) * v(j,k);
              for (k=l;k<n;k++)
                v(j,k) += s * v(i,k);

            } /* end j */
          } /* end g */
          for (j=l;j<n;j++)
            v(j,i) = v(i,j) = 0.0;
          v(i,i) = 1.0;
          g = e[i];
          l = i;
        } /* end i */

      } /* end withv, parens added for clarity */



      /* accumulation of left-hand transformations */
      if (withu) {
        for (i=n;i<m;i++) {
          for (j=n;j<m;j++)
            u(j,i) = 0.0;
          u(i,i) = 1.0;
        }
      }

      if (withu) {
        for (i=n-1;i>=0;i--) {
          l = i + 1;
          g = q[i];
          for (j=l;j<m;j++){  /* upper limit was 'n' */
            u(j,i) = 0.0;
          }
          if (g != 0.0) {
            h = u(i,i) * g;
            for (j=l;j<m;j++) { /* upper limit was 'n' */
              s = 0.0;
              for (k=l;k<m;k++){
                s += u(i,k) * u(j,k);
              }
              f = s / h;
              for (k=i;k<m;k++){
                u(j,k) += f * u(i,k);
              }
            } /* end j */
            for (j=i;j<m;j++){
              u(i,j) /= g;
            }
          } /* end g */
          else {
            for (j=i;j<m;j++){
              u(i,j) = 0.0;
            }
          }
          u(i,i) += 1.0;
        } /* end i*/
      } /* end withu, parens added for clarity */


      /* diagonalization of the bidiagonal form */
      eps *= x;
      for (k=n-1;k>=0;k--) {
        iter = 0;
        test_f_splitting:
        for (l=k;l>=0;l--) {
          if (fabs(e[l]) <= eps) goto test_f_convergence;
          if (fabs(q[l-1]) <= eps) goto cancellation;
        } /* end l */

        /* cancellation of e[l] if l > 0 */
      cancellation:
        c = 0.0;
        s = 1.0;
        l1 = l - 1;
        for (i=l;i<=k;i++) {
          f = s * e[i];
          e[i] *= c;
          if (fabs(f) <= eps) goto test_f_convergence;
          g = q[i];
          h = q[i] = sqrt(f*f + g*g);
          c = g / h;
          s = -f / h;
          if (withu) {
            for (j=0;j<m;j++) {
              y = u(l1,j);
              z = u(i,j);
              u(l1,j) = y * c + z * s;
              u(i,j) = -y *s + z * c;
            } /* end j */
          } /* end withu, parens added for clarity */
        } /* end i */
        test_f_convergence:
        /**/
        z = q[k];
        if (l == k) goto convergence;

        /* shift from bottom 2x2 minor */
        iter++;
        if (iter > 30) {
          retval = k;
          break;
        }
        x = q[l];
        y = q[k-1];
        g = e[k-1];
        h = e[k];
        f = ((y-z)*(y+z) + (g-h)*(g+h)) / (2*h*y);
        g = sqrt(f*f + 1.0);
        f = ((x-z)*(x+z) + h*(y/((f<0)?(f-g):(f+g))-h))/x;
        /* next QR transformation */
        c = s = 1.0;
        for (i=l+1;i<=k;i++) {
          g = e[i];
          y = q[i];
          h = s * g;
          g *= c;
          e[i-1] = z = sqrt(f*f+h*h);
          c = f / z;
          s = h / z;
          f = x * c + g * s;
          g = -x * s + g * c;
          h = y * s;
          y *= c;
          if (withv) {
            for (j=0;j<n;j++) {
              x = v(i-1,j);
              z = v(i,j);
              v(i-1,j) = x * c + z * s;
              v(i,j) = -x * s + z * c;
            } /* end j */
          } /* end withv, parens added for clarity */
          /**/
          q[i-1] = z = sqrt(f*f + h*h);
          c = f/z;
          s = h/z;
          f = c * g + s * y;
          x = -s * g + c * y;
          if (withu) {
            for (j=0;j<m;j++) {
              y = u(i-1,j);
              z = u(i,j);
              u(i-1,j) = y * c + z * s;
              u(i,j) = -y * s + z * c;
            } /* end j */
          } /* end withu, parens added for clarity */
        } /* end i */
        e[l] = 0.0;
        e[k] = f;
        q[k] = x;
        goto test_f_splitting;
        convergence:
        if (z < 0.0) {
          /* q[k] is made non-negative */
          q[k] = - z;
          if (withv) {
            for (j=0;j<n;j++)
              v(k,j) = -v(k,j);
          } /* end withv, parens added for clarity */
        } /* end z */
      } /* end k */
      return retval;
    }

    namespace{
      struct SVD_IdxEV{
        icl64f ev;
        int idx;
      SVD_IdxEV(){}
        SVD_IdxEV(icl64f ev, int idx):ev(ev),idx(idx){}
        bool operator<(const SVD_IdxEV &i) const { return ev > i.ev; }
      };
    }

    static void svd_copy_mat_sort(DynMatrix<icl64f> &m, DynMatrix<icl64f> &M, const std::vector<SVD_IdxEV> &idxlut){
      for(unsigned int j=0;j<M.cols();++j){
        int jidx = idxlut[j].idx;
        for(unsigned int i=0;i<M.rows();++i){
          //M(j,i) = m[i][j];
          M(j,i) = m(jidx,i); // or M(jidx,i) = m[i][j]; ??
        }
      }
    }

    static void svd_copy_vec_sort(icl64f *m, DynMatrix<icl64f> &M, const std::vector<SVD_IdxEV> &idxlut){
      for(unsigned int j=0;j<M.rows();++j){
        int jidx = idxlut[j].idx;
        M[j] = m[jidx]; // or M[jidx] = m[j]??
      }
    }
  #else // use eigen
    template<class T>
    void svd_eigen(const DynMatrix<T> &M, DynMatrix<T> &U, DynMatrix<T> &s, DynMatrix<T> &Vt){
      typedef Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> EigenMat;

      int sdim = iclMin(M.rows(),M.cols());

      Eigen::Map<EigenMat> m(const_cast<T*>(M.begin()),M.rows(),M.cols());
      Eigen::JacobiSVD<EigenMat> svd(m, Eigen::ComputeFullU | Eigen::ComputeFullV);
      Eigen::Map<EigenMat> ms(s.begin(),sdim,s.cols());
      std::fill(s.begin()+sdim, s.end(), 0);
      Eigen::Map<EigenMat> mV(Vt.begin(),Vt.rows(),Vt.cols());

      ms = svd.singularValues();

      if(M.cols() > M.rows()){
        // wrap eigen matrix around the left  part of the ICL-Matrix
        std::fill(U.begin(),U.end(),0);
        Eigen::Map<EigenMat,0,Eigen::OuterStride<> > mU(U.begin(),U.rows(),sdim, Eigen::OuterStride<>(U.cols()));
        mU = svd.matrixU();
      }else{
        Eigen::Map<EigenMat> mU(U.begin(),U.rows(),U.cols());
        mU = svd.matrixU();
      }
      mV = svd.matrixV();
    }
  #endif


    template<class T>
    void svd_dyn(const DynMatrix<T> &A, DynMatrix<T> &U, DynMatrix<T> &s, DynMatrix<T> &V){
      (void)A; (void)U; (void)s; (void)V;
    }

    template<>
    ICLMath_API void svd_dyn(const DynMatrix<icl64f> &M, DynMatrix<icl64f> &U, DynMatrix<icl64f> &s, DynMatrix<icl64f> &Vt){
      unsigned int rows = M.rows(), cols = M.cols();
      U.setBounds(rows, rows);
      Vt.setBounds(cols, cols);
      s.setBounds(1,iclMin(rows,cols));

/*
  #if defined ICL_HAVE_IPP_SVD
      int niter = M.rows();
      while (true) {
        IppStatus status = ippsSVDSort_64f_D2(M.begin(), U.begin(), M.rows(), s.begin(), Vt.begin(), M.cols(), M.cols(), niter);
        switch (status) {
          case ippStsNoErr:
            return;
          case ippStsSVDCnvgErr: // indicates that the algorithm did not converge in niter steps.
            niter *= 2;
            break;
          default:
            throw ICLException(ippGetStatusString(status));
        }
      }
  #elif defined ICL_HAVE_EIGEN3
*/
  #ifdef ICL_HAVE_EIGEN3
      svd_eigen(M,U,s,Vt);
  #else
      DynMatrix<icl64f> _U(rows, rows);
      DynMatrix<icl64f> _V(cols, cols);
      icl64f * _s = new icl64f[iclMax(rows,cols)];

      int r;
      if (cols > rows)
        r = svd_internal(cols, rows, 1, 1,
                         10e-18, 10e-18,
                         M.transp(), _s, _V, _U);
      else
        r = svd_internal(rows, cols, 1, 1,
                         10e-18, 10e-18,
                         M, _s, _U, _V);
      if(r) {
        throw ICLException("error in svd (c++): no convergence for singular value '" + str(r) +"'");
      }

      std::vector<SVD_IdxEV> idxlut(iclMax(rows,cols));
      for(unsigned int i=0;i<iclMax(rows,cols);++i){
        if (i < cols)
          idxlut[i] = SVD_IdxEV(_s[i],i);
        else idxlut[i] = SVD_IdxEV(0.0,i);
      }
      std::sort(idxlut.begin(),idxlut.end());

      svd_copy_mat_sort(_U, U, idxlut);
      svd_copy_mat_sort(_V, Vt, idxlut);
      svd_copy_vec_sort(_s, s, idxlut);

      delete [] _s;
  #endif // ICL_HAVE_EIGEN3
    }

    template<>
    ICLMath_API void svd_dyn(const DynMatrix<icl32f> &A, DynMatrix<icl32f> &U, DynMatrix<icl32f> &s, DynMatrix<icl32f> &V){
      U.setBounds(A.rows(), A.rows());
      V.setBounds(A.cols(), A.cols());
      s.setBounds(1, iclMin(A.rows(), A.cols()));

  #if defined ICL_HAVE_EIGEN3 && !defined ICL_HAVE_IPP
      svd_eigen(A,U,s,V);
  #else
      DynMatrix<icl64f> A64f(A.cols(),A.rows()),U64f(U.cols(),U.rows()),s64f(1,s.rows()),V64f(V.cols(),V.rows());
      std::copy(A.begin(),A.end(),A64f.begin());

      svd_dyn(A64f,U64f,s64f,V64f);

      std::copy(U64f.begin(),U64f.end(),U.begin());
      std::copy(V64f.begin(),V64f.end(),V.begin());
      std::copy(s64f.begin(),s64f.end(),s.begin());
  #endif
    }


  } // namespace math
}
