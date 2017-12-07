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
#include <ICLUtils/StringUtils.h>
#include <cmath>

#ifdef ICL_HAVE_MKL
  #include "mkl_types.h"
  #include "mkl_cblas.h"
  #include "mkl_vml.h"
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

  #ifdef ICL_HAVE_IPP

    ///////////////////////////////////////////////////////////////////////////
    // Unary functions ////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    double reciprocal(const double x) { return 1.0/x; }

  #ifdef WIN32
    float reciprocal(const float x) { return 1.0 / x; }

    #define ICL_UNARY_HELP_FUNC(name,func)                                    \
      IppStatus ipps##name##_32f_I(float *p, int len){                        \
        std::transform(p,p+len,p,static_cast<float(*)(const float)>(func));   \
        return ippStsNoErr;                                                   \
      }                                                                       \
      IppStatus ipps##name##_64f_I(double *p, int len){                       \
        std::transform(p,p+len,p,static_cast<double(*)(const double)>(func)); \
        return ippStsNoErr;                                                   \
      }                                                                       \
      IppStatus ipps##name##_32f(const float *s, float *d, int len){          \
        std::transform(s,s+len,d,static_cast<float(*)(const float)>(func));   \
        return ippStsNoErr;                                                   \
      }                                                                       \
      IppStatus ipps##name##_64f(const double *s, double *d, int len){        \
        std::transform(s,s+len,d,static_cast<double(*)(const double)>(func)); \
        return ippStsNoErr;                                                   \
    }
  #else
    #define ICL_UNARY_HELP_FUNC(name,func)                                    \
      IppStatus ipps##name##_32f_I(float *p, int len){                        \
        std::transform(p,p+len,p,func);                                       \
        return ippStsNoErr;                                                   \
      }                                                                       \
      IppStatus ipps##name##_64f_I(double *p, int len){                       \
        std::transform(p,p+len,p,func);                                       \
        return ippStsNoErr;                                                   \
      }                                                                       \
      IppStatus ipps##name##_32f(const float *s, float *d, int len){          \
        std::transform(s,s+len,d,func);                                       \
        return ippStsNoErr;                                                   \
      }                                                                       \
      IppStatus ipps##name##_64f(const double *s, double *d, int len){        \
        std::transform(s,s+len,d,func);                                       \
        return ippStsNoErr;                                                   \
    }
  #endif

    ICL_UNARY_HELP_FUNC(Sin_icl,::sin)
    ICL_UNARY_HELP_FUNC(Cos_icl,::cos)
    ICL_UNARY_HELP_FUNC(Tan_icl,::tan)
    ICL_UNARY_HELP_FUNC(Arcsin_icl,::asin)
    ICL_UNARY_HELP_FUNC(Arccos_icl,::acos)
    ICL_UNARY_HELP_FUNC(Reciprocal_icl,icl::math::reciprocal)

  #undef ICL_UNARY_HELP_FUNC

  #define INSTANTIATE_DYN_MATRIX_MATH_OP(op,func)                                                              \
    template<class T>                                                                                          \
    DynMatrix<T> &matrix_##op(DynMatrix<T> &m){                                                                \
      ERROR_LOG("not implemented for this type!");                                                             \
      return m;                                                                                                \
    }                                                                                                          \
    template<> ICLMath_API DynMatrix<float> &matrix_##op(DynMatrix<float> &m){                                 \
      ipps##func##_32f_I(m.begin(), m.dim());                                                                  \
      return m;                                                                                                \
    }                                                                                                          \
    template<> ICLMath_API DynMatrix<double> &matrix_##op(DynMatrix<double> &m){                               \
      ipps##func##_64f_I(m.begin(), m.dim());                                                                  \
      return m;                                                                                                \
    }                                                                                                          \
    template<class T>                                                                                          \
    DynMatrix<T> &matrix_##op(const DynMatrix<T> &m, DynMatrix<T> &dst){                                       \
      ERROR_LOG("not implemented for this type!");                                                             \
      return dst;                                                                                              \
    }                                                                                                          \
    template<> ICLMath_API DynMatrix<float> &matrix_##op(const DynMatrix<float> &m, DynMatrix<float> &dst){    \
      dst.setBounds(m.cols(),m.rows());                                                                        \
      ipps##func##_32f(m.begin(), dst.begin(), m.dim());                                                       \
      return dst;                                                                                              \
    }                                                                                                          \
    template<> ICLMath_API DynMatrix<double> &matrix_##op(const DynMatrix<double> &m, DynMatrix<double> &dst){ \
      dst.setBounds(m.cols(),m.rows());                                                                        \
      ipps##func##_64f(m.begin(), dst.begin(), m.dim());                                                       \
      return dst;                                                                                              \
    }

    INSTANTIATE_DYN_MATRIX_MATH_OP(abs,Abs)
    INSTANTIATE_DYN_MATRIX_MATH_OP(log,Ln)
    INSTANTIATE_DYN_MATRIX_MATH_OP(exp,Exp)
    INSTANTIATE_DYN_MATRIX_MATH_OP(sqrt,Sqrt)
    INSTANTIATE_DYN_MATRIX_MATH_OP(sqr,Sqr)

    INSTANTIATE_DYN_MATRIX_MATH_OP(sin,Sin_icl)
    INSTANTIATE_DYN_MATRIX_MATH_OP(cos,Cos_icl)
    INSTANTIATE_DYN_MATRIX_MATH_OP(tan,Tan_icl)

    INSTANTIATE_DYN_MATRIX_MATH_OP(arcsin,Arcsin_icl)
    INSTANTIATE_DYN_MATRIX_MATH_OP(arccos,Arccos_icl)
    INSTANTIATE_DYN_MATRIX_MATH_OP(arctan,Arctan)
    INSTANTIATE_DYN_MATRIX_MATH_OP(reciprocal,Reciprocal_icl)

  #undef INSTANTIATE_DYN_MATRIX_MATH_OP

    template<class T> struct pow_functor{
      const T val;
      inline pow_functor(const T val):val(val){}
      inline double operator()(const T &x) const{
        return pow(x,val);
      }
    };

    IppStatus ippsPow_icl_32f_I(float e, float *p, int len){
      std::transform(p,p+len,p,pow_functor<float>(e));
      return ippStsNoErr;
    }
    IppStatus ippsPow_icl_64f_I(double e, double *p, int len){
      std::transform(p,p+len,p,pow_functor<double>(e));
      return ippStsNoErr;
    }
    IppStatus ippsPow_icl_32f(const float *s,float e, float *d, int len){
      std::transform(s,s+len,d,pow_functor<float>(e));
      return ippStsNoErr;
    }
    IppStatus ippsPow_icl_64f(const double *s,double e, double *d, int len){
      std::transform(s,s+len,d,pow_functor<double>(e));
      return ippStsNoErr;
    }


  #define INSTANTIATE_DYN_MATRIX_MATH_OP(op,func)                                                                         \
    template<class T> DynMatrix<T> &matrix_##op(DynMatrix<T> &m, T val){                                                  \
      ERROR_LOG("not implemented for this type!");                                                                        \
      return m;                                                                                                           \
    }                                                                                                                     \
    template<> ICLMath_API DynMatrix<float> &matrix_##op(DynMatrix<float> &m, float val){                                 \
      ipps##func##_32f_I(val,m.begin(),m.dim());                                                                          \
      return m;                                                                                                           \
    }                                                                                                                     \
    template<> ICLMath_API DynMatrix<double> &matrix_##op(DynMatrix<double> &m, double val){                              \
      ipps##func##_64f_I(val,m.begin(),m.dim());                                                                          \
      return m;                                                                                                           \
    }                                                                                                                     \
    template<class T> DynMatrix<T> &matrix_##op(const DynMatrix<T> &,T, DynMatrix<T> &m){                                 \
      ERROR_LOG("not implemented for this type!");                                                                        \
      return m;                                                                                                           \
    }                                                                                                                     \
    template<> ICLMath_API DynMatrix<float> &matrix_##op(const DynMatrix<float> &m,float val, DynMatrix<float> &dst){     \
      dst.setBounds(m.cols(),m.rows());                                                                                   \
      ipps##func##_32f(m.begin(), val, dst.begin(), m.dim());                                                             \
      return dst;                                                                                                         \
    }                                                                                                                     \
    template<> ICLMath_API DynMatrix<double> &matrix_##op(const DynMatrix<double> &m,double val, DynMatrix<double> &dst){ \
      dst.setBounds(m.cols(),m.rows());                                                                                   \
      ipps##func##_64f(m.begin(), val, dst.begin(), m.dim());                                                             \
      return dst;                                                                                                         \
    }

    INSTANTIATE_DYN_MATRIX_MATH_OP(powc,Pow_icl)
    INSTANTIATE_DYN_MATRIX_MATH_OP(addc,AddC)
    INSTANTIATE_DYN_MATRIX_MATH_OP(subc,SubC)
    INSTANTIATE_DYN_MATRIX_MATH_OP(mulc,MulC)
    INSTANTIATE_DYN_MATRIX_MATH_OP(divc,DivC)

  #undef INSTANTIATE_DYN_MATRIX_MATH_OP

    ///////////////////////////////////////////////////////////////////////////
    // Binary functions ///////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

  #ifdef WIN32
    IppStatus ippsArctan2_icl_32f(const float *a, const float *b, float *dst, int len){
      std::transform(b,b+len,a,dst,static_cast<float(*)(const float, const float)>(::atan2));
      return ippStsNoErr;
    }
    IppStatus ippsArctan2_icl_64f(const double *a, const double *b, double *dst, int len){
      std::transform(b,b+len,a,dst,static_cast<double(*)(const double, const double)>(::atan2));
      return ippStsNoErr;
    }
    IppStatus ippsPow_icl_32f(const float *a, const float *b, float *dst, int len){
      std::transform(a,a+len,b,dst,static_cast<float(*)(const float, const float)>(::pow));
      return ippStsNoErr;
    }
    IppStatus ippsPow_icl_64f(const double *a, const double *b, double *dst, int len){
      std::transform(a,a+len,b,dst,static_cast<double(*)(const double, const double)>(::pow));
      return ippStsNoErr;
    }
  #else
    IppStatus ippsArctan2_icl_32f(const float *a, const float *b, float *dst, int len){
      std::transform(b,b+len,a,dst,::atan2);
      return ippStsNoErr;
    }
    IppStatus ippsArctan2_icl_64f(const double *a, const double *b, double *dst, int len){
      std::transform(b,b+len,a,dst,::atan2);
      return ippStsNoErr;
    }
    IppStatus ippsPow_icl_32f(const float *a, const float *b, float *dst, int len){
      std::transform(a,a+len,b,dst,::pow);
      return ippStsNoErr;
    }
    IppStatus ippsPow_icl_64f(const double *a, const double *b, double *dst, int len){
      std::transform(a,a+len,b,dst,::pow);
      return ippStsNoErr;
    }
  #endif

  #define INSTANTIATE_DYN_MATRIX_MATH_OP(op,func)                                                                                         \
    template<class T>                                                                                                                     \
    DynMatrix<T> &matrix_##op(const DynMatrix<T> &a, const DynMatrix<T> &b, DynMatrix<T> &dst)                                            \
      throw (IncompatibleMatrixDimensionException){                                                                                       \
      ERROR_LOG("not implemented for this type!");                                                                                        \
      return dst;                                                                                                                         \
    }                                                                                                                                     \
    template<> ICLMath_API DynMatrix<float> &matrix_##op(const DynMatrix<float> &a, const DynMatrix<float> &b, DynMatrix<float> &dst)     \
      throw (IncompatibleMatrixDimensionException){                                                                                       \
      CHECK_DIM(a,b,dst);                                                                                                                 \
      dst.setBounds(a.cols(), a.rows());                                                                                                  \
      ipps##func##_32f(b.begin(), a.begin(), dst.begin(), a.dim());                                                                       \
      return dst;                                                                                                                         \
    }                                                                                                                                     \
    template<> ICLMath_API DynMatrix<double> &matrix_##op(const DynMatrix<double> &a, const DynMatrix<double> &b, DynMatrix<double> &dst) \
      throw (IncompatibleMatrixDimensionException){                                                                                       \
      CHECK_DIM(a,b,dst);                                                                                                                 \
      dst.setBounds(a.cols(), a.rows());                                                                                                  \
      ipps##func##_64f(b.begin(), a.begin(), dst.begin(), a.dim());                                                                       \
      return dst;                                                                                                                         \
    }

    INSTANTIATE_DYN_MATRIX_MATH_OP(arctan2,Arctan2_icl)
    INSTANTIATE_DYN_MATRIX_MATH_OP(add,Add)
    INSTANTIATE_DYN_MATRIX_MATH_OP(sub,Sub)
    INSTANTIATE_DYN_MATRIX_MATH_OP(mul,Mul)
    INSTANTIATE_DYN_MATRIX_MATH_OP(div,Div)
    INSTANTIATE_DYN_MATRIX_MATH_OP(pow,Pow_icl)

  #undef INSTANTIATE_DYN_MATRIX_MATH_OP
    ///////////////////////////////////////////////////////////////////////////
    // Other functions ////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    template<class T>
    T matrix_min(const DynMatrix<T> &m, int *x, int *y){
      ERROR_LOG("not implemented for this type!");
      return 0;
    }
    template<class T>
    T matrix_max(const DynMatrix<T> &m, int *x, int *y){
      ERROR_LOG("not implemented for this type!");
      return 0;
    }

  #define INSTANTIATE_DYN_MATRIX_MATH_OP(op,type,suffix,ippf)             \
    template<> ICLMath_API type matrix_##op(const DynMatrix<type> &m, int *x, int *y){ \
      type mVal = 0;                                                      \
      if(x || y){                                                         \
        int mIdx = 0;                                                     \
        ipps##ippf##Indx_##suffix(m.begin(),m.dim(),&mVal,&mIdx);         \
        if(x) *x = mIdx%m.cols();                                         \
        if(y) *y = mIdx/m.cols();                                         \
      }else{                                                              \
        ipps##ippf##_##suffix(m.begin(),m.dim(),&mVal);                   \
      }                                                                   \
      return mVal;                                                        \
    }
    INSTANTIATE_DYN_MATRIX_MATH_OP(min,float,32f,Min)
    INSTANTIATE_DYN_MATRIX_MATH_OP(max,float,32f,Max)
    INSTANTIATE_DYN_MATRIX_MATH_OP(min,double,64f,Min)
    INSTANTIATE_DYN_MATRIX_MATH_OP(max,double,64f,Max)


  #undef INSTANTIATE_DYN_MATRIX_MATH_OP

    template<class T>
    void matrix_minmax(const DynMatrix<T> &m, T dst[2],
                       int *minx, int *miny,
                       int *maxx, int *maxy){
      ERROR_LOG("not implemented for this type!");
    }
    template<> ICLMath_API void matrix_minmax(const DynMatrix<float> &m, float dst[2],
                                  int *minx, int *miny,
                                  int *maxx, int *maxy){
      if(minx || miny || maxx || maxy){
        int minIdx = 0;
        int maxIdx = 0;
        ippsMinMaxIndx_32f(m.begin(),m.dim(),dst+0,&minIdx,dst+1,&maxIdx);
        if(minx) *minx = minIdx%m.cols();
        if(miny) *miny = minIdx/m.cols();
        if(maxx) *maxx = maxIdx%m.cols();
        if(maxy) *maxy = maxIdx/m.cols();
      }else{
        ippsMinMax_32f(m.begin(),m.dim(),dst+0,dst+1);
      }
    }
    template<> ICLMath_API void matrix_minmax(const DynMatrix<double> &m, double dst[2],
                                  int *minx, int *miny,
                                  int *maxx, int *maxy){
      if(minx || miny || maxx || maxy){
        int minIdx = 0;
        int maxIdx = 0;
        ippsMinMaxIndx_64f(m.begin(),m.dim(),dst+0,&minIdx,dst+1,&maxIdx);
        if(minx) *minx = minIdx%m.cols();
        if(miny) *miny = minIdx/m.cols();
        if(maxx) *maxx = maxIdx%m.cols();
        if(maxy) *maxy = maxIdx/m.cols();
      }else{
        ippsMinMax_64f(m.begin(),m.dim(),dst+0,dst+1);
      }
    }






  #else
    // **************************************************************************
    // No IPP fallbacks
    // **************************************************************************


  #define INSTANTIATE_DYN_MATRIX_MATH_OP(op,func)                         \
    template<class T>                                                     \
    void elem_i_##op(T &t){                                               \
      t = func(t);                                                        \
    }                                                                     \
    template<class T>                                                     \
    T elem_##op(const T &t){                                              \
      return func(t);                                                     \
    }                                                                     \
    template<class T>                                                     \
    DynMatrix<T> &matrix_##op(DynMatrix<T> &m){                           \
      std::for_each(m.begin(),m.end(),elem_i_##op<T>);                    \
      return m;                                                           \
    }                                                                     \
    template<class T>                                                     \
    DynMatrix<T> &matrix_##op(const DynMatrix<T> &m, DynMatrix<T> &dst){  \
      dst.setBounds(m.cols(),m.rows());                                   \
      std::transform(m.begin(),m.end(),dst.begin(),elem_##op<T>);         \
      return dst;                                                         \
    }                                                                     \
    template ICLMath_API DynMatrix<float> &matrix_##op(DynMatrix<float> &m);          \
    template ICLMath_API DynMatrix<double> &matrix_##op(DynMatrix<double> &m);

    static inline double sqr(double x) { return x*x; }
    static inline double reciprocal(double x) { return x ? 1.0/x : 0 ; }

    INSTANTIATE_DYN_MATRIX_MATH_OP(abs,::fabs)
    INSTANTIATE_DYN_MATRIX_MATH_OP(log,::log)
    INSTANTIATE_DYN_MATRIX_MATH_OP(exp,::exp)
    INSTANTIATE_DYN_MATRIX_MATH_OP(sqrt,::sqrt)
    INSTANTIATE_DYN_MATRIX_MATH_OP(sqr,icl::math::sqr)
    INSTANTIATE_DYN_MATRIX_MATH_OP(reciprocal,icl::math::reciprocal)
    INSTANTIATE_DYN_MATRIX_MATH_OP(sin,::sin)
    INSTANTIATE_DYN_MATRIX_MATH_OP(cos,::cos)
    INSTANTIATE_DYN_MATRIX_MATH_OP(tan,::tan)
    INSTANTIATE_DYN_MATRIX_MATH_OP(arcsin,::asin)
    INSTANTIATE_DYN_MATRIX_MATH_OP(arccos,::acos)
    INSTANTIATE_DYN_MATRIX_MATH_OP(arctan,::atan)

  #undef INSTANTIATE_DYN_MATRIX_MATH_OP

  #define INSTANTIATE_DYN_MATRIX_MATH_OP(op,func)                         \
    template<class T> struct op##_inplace_functor{                        \
      const T val;                                                        \
      inline op##_inplace_functor(const T &val):val(val){}                \
      inline void operator()(T &x) const{                                 \
        x = func(x,val);                                                  \
      }                                                                   \
    };                                                                    \
    template<class T> struct op##_functor{                                \
      const T val;                                                        \
      inline op##_functor(const T val):val(val){}                         \
      inline double operator()(const T &x) const{                         \
        return func(x,val);                                               \
      }                                                                   \
    };                                                                    \
    template<class T>                                                     \
    DynMatrix<T> &matrix_##op(DynMatrix<T> &m, T val){                    \
      std::for_each(m.begin(),m.end(),op##_inplace_functor<T>(val));      \
      return m;                                                           \
    }                                                                     \
    template<class T>                                                     \
    DynMatrix<T> &matrix_##op(const DynMatrix<T> &m, T val, DynMatrix<T> &dst){ \
      std::transform(m.begin(),m.end(),dst.begin(),op##_functor<T>(val)); \
      return dst;                                                         \
    }                                                                     \
    template ICLMath_API DynMatrix<float> &matrix_##op(DynMatrix<float>&, float);     \
    template ICLMath_API DynMatrix<double> &matrix_##op(DynMatrix<double>&, double);  \
    template ICLMath_API DynMatrix<float> &matrix_##op(const DynMatrix<float>&, float, DynMatrix<float>&); \
    template ICLMath_API DynMatrix<double> &matrix_##op(const DynMatrix<double>&, double, DynMatrix<double>&);

    template <class T> static inline T addc(const T &a, const T &b){ return a+b; }
    template <class T> static inline T subc(const T &a, const T &b){ return a-b; }
    template <class T> static inline T divc(const T &a, const T &b){ return a/b; }
    template <class T> static inline T mulc(const T &a, const T &b){ return a*b; }

    INSTANTIATE_DYN_MATRIX_MATH_OP(powc,::pow)
    INSTANTIATE_DYN_MATRIX_MATH_OP(addc,icl::math::addc)
    INSTANTIATE_DYN_MATRIX_MATH_OP(subc,icl::math::subc)
    INSTANTIATE_DYN_MATRIX_MATH_OP(mulc,icl::math::mulc)
    INSTANTIATE_DYN_MATRIX_MATH_OP(divc,icl::math::divc)

  #undef INSTANTIATE_DYN_MATRIX_MATH_OP


  // binary functions
  #define INSTANTIATE_DYN_MATRIX_MATH_OP(op,func)                         \
    template<class T>                                                     \
    DynMatrix<T> &matrix_##op(const DynMatrix<T> &a, const DynMatrix<T> &b, DynMatrix<T> &dst) \
      throw (IncompatibleMatrixDimensionException){                       \
      CHECK_DIM(a,b,dst);                                                 \
      dst.setBounds(a.cols(),a.rows());                                   \
      std::transform(a.begin(),a.end(),b.begin(),dst.begin(),func);       \
      return dst;                                                         \
    }                                                                     \
    template ICLMath_API DynMatrix<float> &matrix_##op(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&)     \
       throw (IncompatibleMatrixDimensionException);                                                                \
    template ICLMath_API DynMatrix<double> &matrix_##op(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&) \
       throw (IncompatibleMatrixDimensionException);

    INSTANTIATE_DYN_MATRIX_MATH_OP(add,icl::math::addc<T>)
    INSTANTIATE_DYN_MATRIX_MATH_OP(sub,icl::math::subc<T>)
    INSTANTIATE_DYN_MATRIX_MATH_OP(mul,icl::math::mulc<T>)
    INSTANTIATE_DYN_MATRIX_MATH_OP(div,icl::math::divc<T>)

    INSTANTIATE_DYN_MATRIX_MATH_OP(pow, static_cast<T(*)(T, T)>(std::pow))
    INSTANTIATE_DYN_MATRIX_MATH_OP(arctan2, static_cast<T(*)(T, T)>(std::atan2))
  #undef INSTANTIATE_DYN_MATRIX_MATH_OP

    // others

    template<class T>
    T matrix_min(const DynMatrix<T> &m, int *x, int *y){
      ICLASSERT_RETURN_VAL(m.cols(),0);
      const T *a = std::min_element(m.begin(),m.end());
      int idx = (int)(a-m.begin());
      if(x) *x = idx%m.cols();
      if(y) *y = idx/m.cols();
      return *a;
    }

    template ICLMath_API float matrix_min(const DynMatrix<float> &m, int *x, int *y);
    template ICLMath_API double matrix_min(const DynMatrix<double> &m, int *x, int *y);

    template<class T>
    T matrix_max(const DynMatrix<T> &m, int *x, int *y){
      ICLASSERT_RETURN_VAL(m.cols(),0);
      const T *a = std::max_element(m.begin(),m.end());
      int idx = (int)(a-m.begin());
      if(x) *x = idx%m.cols();
      if(y) *y = idx/m.cols();
      return *a;
    }

    template ICLMath_API float matrix_max(const DynMatrix<float> &m, int *x, int *y);
    template ICLMath_API double matrix_max(const DynMatrix<double> &m, int *x, int *y);


    template<class T>
    void matrix_minmax(const DynMatrix<T> &m, T dst[2],
                       int *minx, int *miny,
                       int *maxx, int *maxy){
      dst[0] = matrix_min(m,minx,miny);
      dst[1] = matrix_max(m,maxx,maxy);
    }

    template ICLMath_API void matrix_minmax(const DynMatrix<float> &m, float dst[2], int*, int*, int*, int*);
    template ICLMath_API void matrix_minmax(const DynMatrix<double> &m, double dst[2], int*, int*, int*, int*);


  #endif //ICL_HAVE_IPP


    /// ------------------------------------------------------------
    /// ipp and non-ipp mixed ...
    /// ------------------------------------------------------------


    // MEAN ***************************
    template<class T> T matrix_mean(const DynMatrix<T> &m){
      return std::accumulate(m.begin(),m.end(),T(0))/m.dim();
    }

  #ifdef ICL_HAVE_IPP
    template<> ICLMath_API float matrix_mean(const DynMatrix<float> &m){
      float v=0; ippsMean_32f(m.begin(),m.dim(),&v,ippAlgHintNone);  return v;
    }

    template<> ICLMath_API double matrix_mean(const DynMatrix<double> &m){
      double v=0; ippsMean_64f(m.begin(),m.dim(),&v); return v;
    }
  #else
    template ICLMath_API float matrix_mean(const DynMatrix<float>&);
    template ICLMath_API double matrix_mean(const DynMatrix<double>&);
  #endif

    // VAR ***************************
    template<class T> struct var_functor{
      T mean,&accu;
      static inline T util_sqr(const T &t){ return t*t; }
      var_functor(const T &mean, T &accu):mean(mean),accu(accu){}
      void operator()(const T&x) const { accu += util_sqr(mean-x); }
    };

    template<class T>
    T matrix_var(const DynMatrix<T> &m){
      if(m.dim()<2){ return 0; }
      T var = 0;
      std::for_each(m.begin(),m.end(),var_functor<T>(matrix_mean(m),var));
      return var/(m.dim()-1);
    }

  #ifdef ICL_HAVE_IPP
    template<> ICLMath_API float matrix_var(const DynMatrix<float> &m){
      float v=0; ippsStdDev_32f(m.begin(),m.dim(),&v,ippAlgHintNone); return v*v;
    }
    template<> ICLMath_API double matrix_var(const DynMatrix<double> &m){
      double v=0; ippsStdDev_64f(m.begin(),m.dim(),&v); return v*v;
    }
  #else
    template ICLMath_API float matrix_var(const DynMatrix<float>&);
    template ICLMath_API double matrix_var(const DynMatrix<double>&);
  #endif // ICL_HAVE_IPP

     // VAR (2)***************************
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


    // MEANVAR ***************************
    template<class T>
    void matrix_meanvar(const DynMatrix<T> &m, T *mean, T*var){
      T meanVal = matrix_mean(m);
      T varVal = matrix_var(m,meanVal,true);
      if(mean) *mean=meanVal;
      if(var) *var=varVal;
    }

  #ifdef ICL_HAVE_IPP
    template<> ICLMath_API void matrix_meanvar(const DynMatrix<float> &m, float *mean, float *var){
      ICLASSERT_RETURN(mean && var);
      ippsMeanStdDev_32f(m.begin(),m.dim(),mean,var,ippAlgHintNone);
      *var = (*var)*(*var);
    }
    template<> ICLMath_API void matrix_meanvar(const DynMatrix<double> &m, double *mean, double*var){
      ICLASSERT_RETURN(mean && var);
      ippsMeanStdDev_64f(m.begin(),m.dim(),mean,var);
      *var = (*var)*(*var);
    }
  #else
    template ICLMath_API void matrix_meanvar(const DynMatrix<float>&, float*, float*);
    template ICLMath_API void matrix_meanvar(const DynMatrix<double>&, double*, double*);
  #endif // ICL_HAVE_IPP


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
    DynMatrix<T> &matrix_muladd(const DynMatrix<T> &a,T alpha, const DynMatrix<T> &b, T beta, T gamma, DynMatrix<T> &dst)
      throw (IncompatibleMatrixDimensionException){
      CHECK_DIM(a,b,dst);
      if(!alpha) return matrix_muladd(b,beta,gamma,dst);
      if(!beta) return matrix_muladd(a,alpha,gamma,dst);
      dst.setBounds(a.cols(),a.rows());
      std::transform(a.begin(),a.end(),b.begin(),dst.begin(),muladd_functor_2<T>(alpha,beta,gamma));
      return dst;
    }
    template ICLMath_API DynMatrix<float> &matrix_muladd(const DynMatrix<float>&, float, const DynMatrix<float>&, float, float, DynMatrix<float>&)
      throw (IncompatibleMatrixDimensionException);
    template ICLMath_API DynMatrix<double> &matrix_muladd(const DynMatrix<double>&, double, const DynMatrix<double>&, double, double, DynMatrix<double>&)
      throw (IncompatibleMatrixDimensionException);


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
    DynMatrix<T> &matrix_mask(const DynMatrix<unsigned char> &mask, DynMatrix<T> &m)
      throw (IncompatibleMatrixDimensionException){
      CHECK_DIM(mask,m,m);
      std::transform(mask.begin(),mask.end(),m.begin(),m.begin(),mask_functor<T>());
      return m;
    }

    template ICLMath_API DynMatrix<float> &matrix_mask(const DynMatrix<unsigned char> &mask, DynMatrix<float> &m)
      throw (IncompatibleMatrixDimensionException);
    template ICLMath_API DynMatrix<double> &matrix_mask(const DynMatrix<unsigned char> &mask, DynMatrix<double> &m)
      throw (IncompatibleMatrixDimensionException);

      /// applies masking operation (m(i,j) is set to 0 if mask(i,j) is 0)
    template<class T>
    DynMatrix<T> &matrix_mask(const DynMatrix<unsigned char> &mask, const DynMatrix<T> &m, DynMatrix<T> &dst)
      throw (IncompatibleMatrixDimensionException){
      CHECK_DIM(mask,m,dst);
      dst.setBounds(m.cols(),m.rows());
      std::transform(mask.begin(),mask.end(),m.begin(),dst.begin(),mask_functor<T>());
      return dst;
    }

    template ICLMath_API DynMatrix<float> &matrix_mask(const DynMatrix<unsigned char> &mask, const DynMatrix<float> &m, DynMatrix<float> &dst)
      throw (IncompatibleMatrixDimensionException);
    template ICLMath_API DynMatrix<double> &matrix_mask(const DynMatrix<unsigned char> &mask, const DynMatrix<double> &m, DynMatrix<double> &dst)
      throw (IncompatibleMatrixDimensionException);



    template<class T>
    struct matrix_distance_op{
      T e;
      matrix_distance_op(const T &e):e(e){}
      inline T operator()(const T &a, const T&b) const{  return ::pow(a-b,e);   }
    };

    template<class T>
    T matrix_distance(const DynMatrix<T> &m1, const DynMatrix<T> &m2, T norm)
      throw (IncompatibleMatrixDimensionException){
      CHECK_DIM(m1,m2,-1);
      ICLASSERT_RETURN_VAL(norm > 0,-1);
      T result = std::inner_product(m1.begin(),m1.end(),m2.begin(),T(0),std::plus<T>(),matrix_distance_op<T>(norm));
      return pow(result,1/norm);
    }

    template ICLMath_API float matrix_distance(const DynMatrix<float> &m1, const DynMatrix<float> &m2, float)
      throw (IncompatibleMatrixDimensionException);
    template ICLMath_API double matrix_distance(const DynMatrix<double> &, const DynMatrix<double> &, double)
      throw (IncompatibleMatrixDimensionException);



    template<class T>
    struct matrix_divergence_op{
      inline T operator()(const T &a, const T&b) const{ return a * log(a/b) - a + b;  }
    };

    template<class T>
    T matrix_divergence(const DynMatrix<T> &m1, const DynMatrix<T> &m2)
      throw (IncompatibleMatrixDimensionException){
      CHECK_DIM(m1,m2,-1);
      return std::inner_product(m1.begin(),m1.end(),m2.begin(),T(0),std::plus<T>(),matrix_divergence_op<T>());
    }

    template ICLMath_API float matrix_divergence(const DynMatrix<float>&, const DynMatrix<float>&)
      throw (IncompatibleMatrixDimensionException);
    template ICLMath_API double matrix_divergence(const DynMatrix<double>&, const DynMatrix<double>&)
      throw (IncompatibleMatrixDimensionException);

    /// matrix functions for transposed matrices ...



    template<class T>
    DynMatrix<T> &matrix_mult_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
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
    DynMatrix<T> &big_matrix_mult_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
      return matrix_mult_t( src1, src2, dst, transpDef );
    }

    template<class T>
    DynMatrix<T> &matrix_add_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
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
    DynMatrix<T> &matrix_sub_t(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
      switch(transpDef){
        case NONE_T: return matrix_sub(src1,src2,dst);
        case SRC1_T: return matrix_sub(src1.transp(),src2,dst);
        case SRC2_T: return matrix_sub(src1,src2.transp(),dst);
        case BOTH_T: return matrix_sub(src1.transp(),src2.transp(),dst);
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }


  #ifdef ICL_HAVE_MKL
    /*template<class T, typename func>
    DynMatrix<T> &ipp_func_t_call(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, func f)throw (IncompatibleMatrixDimensionException){
      IppStatus status = f(src1.begin(),src1.stride1(),src1.stride2(),src1.cols(),src1.rows(),
                           src2.begin(),src2.stride1(),src2.stride2(),src2.cols(),src2.rows(),
                           dst.begin(),dst.stride1(), dst.stride2());
      if(status != ippStsNoErr){
        throw IncompatibleMatrixDimensionException(ippGetStatusString(status));
      }
      return dst;
    }

    template<class T, typename func>
    DynMatrix<T> &ipp_func_t_call_2(const DynMatrix<T> &src1, const DynMatrix<T> &src2, DynMatrix<T> &dst, func f)throw (IncompatibleMatrixDimensionException){
      IppStatus status = f(src1.begin(),src1.stride1(),src1.stride2(),
                           src2.begin(),src2.stride1(),src2.stride2(),
                           dst.begin(),dst.stride1(), dst.stride2(),
                           dst.cols(),dst.rows());
      if(status != ippStsNoErr){
        throw IncompatibleMatrixDimensionException(ippGetStatusString(status));
      }
      return dst;
    }*/

    template<> ICLMath_API DynMatrix<float> &matrix_add_t(const DynMatrix<float> &src1, const DynMatrix<float> &src2, DynMatrix<float> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          vsAdd(src1.rows()*src1.cols(),src1.begin(),src2.begin(),dst.begin());
          return dst;
        case SRC1_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src2.cols(),src2.rows());
          vsAdd(src1.rows()*src1.cols(),src1.transp().begin(),src2.begin(),dst.begin());
          return dst;
        case SRC2_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          vsAdd(src1.rows()*src1.cols(),src1.begin(),src2.transp().begin(),dst.begin());
          return dst;
        case BOTH_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.rows(),src1.cols());
          vsAdd(src1.rows()*src1.cols(),src1.transp().begin(),src2.transp().begin(),dst.begin());
          return dst;
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }
    template<> ICLMath_API DynMatrix<double> &matrix_add_t(const DynMatrix<double> &src1, const DynMatrix<double> &src2, DynMatrix<double> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          vdAdd(src1.rows()*src1.cols(),src1.begin(),src2.begin(),dst.begin());
          return dst;
        case SRC1_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src2.cols(),src2.rows());
          vdAdd(src1.rows()*src1.cols(),src1.transp().begin(),src2.begin(),dst.begin());
          return dst;
        case SRC2_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          vdAdd(src1.rows()*src1.cols(),src1.begin(),src2.transp().begin(),dst.begin());
          return dst;
        case BOTH_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.rows(),src1.cols());
          vdAdd(src1.rows()*src1.cols(),src1.transp().begin(),src2.transp().begin(),dst.begin());
          return dst;
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }

    template<> ICLMath_API DynMatrix<float> &matrix_sub_t(const DynMatrix<float> &src1, const DynMatrix<float> &src2, DynMatrix<float> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          vsSub(src1.rows()*src1.cols(),src1.begin(),src2.begin(),dst.begin());
          return dst;
        case SRC1_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src2.cols(),src2.rows());
          vsSub(src1.rows()*src1.cols(),src1.transp().begin(),src2.begin(),dst.begin());
          return dst;
        case SRC2_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          vsSub(src1.rows()*src1.cols(),src1.begin(),src2.transp().begin(),dst.begin());
          return dst;
        case BOTH_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.rows(),src1.cols());
          vsSub(src1.rows()*src1.cols(),src1.transp().begin(),src2.transp().begin(),dst.begin());
          return dst;
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }
    template<> ICLMath_API DynMatrix<double> &matrix_sub_t(const DynMatrix<double> &src1, const DynMatrix<double> &src2, DynMatrix<double> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
      switch(transpDef){
        case NONE_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          vdSub(src1.rows()*src1.cols(),src1.begin(),src2.begin(),dst.begin());
          return dst;
        case SRC1_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src2.cols(),src2.rows());
          vdSub(src1.rows()*src1.cols(),src1.transp().begin(),src2.begin(),dst.begin());
          return dst;
        case SRC2_T:
          CHECK_DIM_T(src1,src2,dst);
          dst.setBounds(src1.cols(),src1.rows());
          vdSub(src1.rows()*src1.cols(),src1.begin(),src2.transp().begin(),dst.begin());
          return dst;
        case BOTH_T:
          CHECK_DIM(src1,src2,dst);
          dst.setBounds(src1.rows(),src1.cols());
          vdSub(src1.rows()*src1.cols(),src1.transp().begin(),src2.transp().begin(),dst.begin());
          return dst;
        default: ERROR_LOG("undefined definition of transposed matrices: "<< transpDef);
      }
      return dst;
    }


    template<> ICLMath_API DynMatrix<float> &matrix_mult_t(const DynMatrix<float> &src1, const DynMatrix<float> &src2, DynMatrix<float> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
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

    template<> ICLMath_API DynMatrix<double> &matrix_mult_t(const DynMatrix<double> &src1, const DynMatrix<double> &src2, DynMatrix<double> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
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

  #else

    template ICLMath_API DynMatrix<double> &matrix_mult_t(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&, int)
      throw (IncompatibleMatrixDimensionException);
    template ICLMath_API DynMatrix<float> &matrix_mult_t(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&, int)
      throw (IncompatibleMatrixDimensionException);

    template ICLMath_API DynMatrix<double> &matrix_add_t(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&, int)
      throw (IncompatibleMatrixDimensionException);
    template ICLMath_API DynMatrix<float> &matrix_add_t(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&, int)
      throw (IncompatibleMatrixDimensionException);


    template ICLMath_API DynMatrix<double> &matrix_sub_t(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&, int)
      throw (IncompatibleMatrixDimensionException);
    template ICLMath_API DynMatrix<float> &matrix_sub_t(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&, int)
      throw (IncompatibleMatrixDimensionException);

  #endif // ICL_HAVE_MKL


    // optimized specialization only if MKL was found
  #ifdef ICL_HAVE_MKL
    template<> ICLMath_API DynMatrix<float> &big_matrix_mult_t(const DynMatrix<float> &src1, const DynMatrix<float> &src2, DynMatrix<float> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
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
    template<> ICLMath_API DynMatrix<double> &big_matrix_mult_t(const DynMatrix<double> &src1, const DynMatrix<double> &src2, DynMatrix<double> &dst, int transpDef)
      throw (IncompatibleMatrixDimensionException){
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


    template ICLMath_API DynMatrix<double> &big_matrix_mult_t(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&, int)
      throw (IncompatibleMatrixDimensionException);
    template ICLMath_API DynMatrix<float> &big_matrix_mult_t(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&, int)
      throw (IncompatibleMatrixDimensionException);


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
    void svd_eigen(const DynMatrix<T> &M, DynMatrix<T> &U, DynMatrix<T> &s, DynMatrix<T> &Vt) throw (ICLException){
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
    void svd_dyn(const DynMatrix<T> &A, DynMatrix<T> &U, DynMatrix<T> &s, DynMatrix<T> &V) throw (ICLException){
      (void)A; (void)U; (void)s; (void)V;
    }

    template<>
    ICLMath_API void svd_dyn(const DynMatrix<icl64f> &M, DynMatrix<icl64f> &U, DynMatrix<icl64f> &s, DynMatrix<icl64f> &Vt) throw (ICLException){
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
    ICLMath_API void svd_dyn(const DynMatrix<icl32f> &A, DynMatrix<icl32f> &U, DynMatrix<icl32f> &s, DynMatrix<icl32f> &V) throw (ICLException){
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
