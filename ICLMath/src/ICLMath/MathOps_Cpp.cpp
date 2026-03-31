/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/MathOps_Cpp.cpp                    **
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

// C++ fallback backends for MathOps dispatch.

#include <ICLMath/MathOps.h>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <functional>

using namespace icl::utils;

namespace icl {
  namespace math {

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

  } // namespace math
} // namespace icl
