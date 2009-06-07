#include <iclDynMatrixUtils.h>
#include <cmath>
#include <iclMacros.h>

#define CHECK_DIM(m1,m2,RET)                                            \
  ICLASSERT_RETURN_VAL( (m1.cols() == m2.cols()) && (m1.rows() == m2.rows()) , RET)


//#undef HAVE_IPP

namespace icl{

#ifdef HAVE_IPP

  ///////////////////////////////////////////////////////////////////////////
  // Caller-functions ///////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  template<class T, IppStatus (*func)(T*,int)>
  static inline DynMatrix<T> &ipp_unary_func_i(DynMatrix<T> &m){
    func(m.begin(),m.dim());
    return m;
  }
  template<class T, IppStatus (*func)(const T*,T*,int)>
  static inline DynMatrix<T> &ipp_unary_func(const DynMatrix<T> &m, DynMatrix<T> &dst){
    dst.setBounds(m.cols(),m.rows());
    func(m.begin(),dst.begin(),m.dim());
    return dst;
  }
  template<class T, IppStatus (*func)(T,T*,int)>
  static inline DynMatrix<T> &ipp_unary_func_c_i(DynMatrix<T> &m,T val){
    func(val,m.begin(),m.dim());
    return m;
  }
  template<class T, IppStatus (*func)(const T*,T,T*,int)>
  static inline DynMatrix<T> &ipp_unary_func_c(const DynMatrix<T> &m,T val, DynMatrix <T> &dst){
    dst.setBounds(m.cols(),m.rows());
    func(m.begin(),val,dst.begin(),m.dim());
    return dst;
  }

  template<class T, IppStatus (*func)(const T*,const T*,T*,int)>
  static inline DynMatrix<T> &ipp_binary_func(const DynMatrix<T> &a, const DynMatrix<T> &b, DynMatrix <T> &dst){
    CHECK_DIM(a,b,dst);
    dst.setBounds(a.cols(),a.rows());
    func(b.begin(),a.begin(),dst.begin(),a.dim()); // direction is reversed (ipps->order)
    return dst;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Unary functions ////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  double reciprocal(const double &x) { return 1.0/x; }

#define ICL_UNARY_HELP_FUNC(name,func)                                  \
  IppStatus ipps##name##_32f_I(float *p, int len){                      \
    std::transform(p,p+len,p,func);                                     \
    return ippStsNoErr;                                                 \
  }                                                                     \
  IppStatus ipps##name##_64f_I(double *p, int len){                     \
    std::transform(p,p+len,p,func);                                     \
    return ippStsNoErr;                                                 \
  }                                                                     \
  IppStatus ipps##name##_32f(const float *s, float *d, int len){        \
    std::transform(s,s+len,d,func);                                     \
    return ippStsNoErr;                                                 \
  }                                                                     \
  IppStatus ipps##name##_64f(const double *s, double *d, int len){      \
    std::transform(s,s+len,d,func);                                     \
    return ippStsNoErr;                                                 \
  }
  
  ICL_UNARY_HELP_FUNC(Sin_icl,::sin)
  ICL_UNARY_HELP_FUNC(Cos_icl,::cos)
  ICL_UNARY_HELP_FUNC(Tan_icl,::tan)
  ICL_UNARY_HELP_FUNC(Arcsin_icl,::asin)
  ICL_UNARY_HELP_FUNC(Arccos_icl,::acos)
  ICL_UNARY_HELP_FUNC(Reciprocal_icl,icl::reciprocal)

#undef ICL_UNARY_HELP_FUNC
  
#define INSTANTIATE_DYN_MATRIX_MATH_OP(op,func)                         \
  template<class T>                                                     \
  DynMatrix<T> &matrix_##op(DynMatrix<T> &m){                           \
    ERROR_LOG("not implemented for this type!");                        \
    return m;                                                           \
  }                                                                     \
  template<> DynMatrix<float> &matrix_##op(DynMatrix<float> &m){        \
    return ipp_unary_func_i<float,ipps##func##_32f_I>(m);               \
  }                                                                     \
  template<> DynMatrix<double> &matrix_##op(DynMatrix<double> &m){      \
    return ipp_unary_func_i<double,ipps##func##_64f_I>(m);              \
  }                                                                     \
  template<> DynMatrix<float> &matrix_##op(DynMatrix<float> &m);        \
  template<> DynMatrix<double> &matrix_##op(DynMatrix<double> &m);      \
                                                                        \
  template<class T>                                                     \
  DynMatrix<T> &matrix_##op(const DynMatrix<T> &m, DynMatrix<T> &dst){  \
    ERROR_LOG("not implemented for this type!");                        \
    return dst;                                                         \
  }                                                                     \
  template<> DynMatrix<float> &matrix_##op(const DynMatrix<float> &m, DynMatrix<float> &dst){ \
    return ipp_unary_func<float,ipps##func##_32f>(m,dst);               \
  }                                                                     \
  template<> DynMatrix<double> &matrix_##op(const DynMatrix<double> &m, DynMatrix<double> &dst){ \
    return ipp_unary_func<double,ipps##func##_64f>(m,dst);              \
  }                                                                     \
  template<> DynMatrix<float> &matrix_##op(const DynMatrix<float> &m, DynMatrix<float> &dst); \
  template<> DynMatrix<double> &matrix_##op(const DynMatrix<double> &m, DynMatrix<double> &dst); 
  
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



#define INSTANTIATE_DYN_MATRIX_MATH_OP(op,func)                         \
  template<class T> DynMatrix<T> &matrix_##op(DynMatrix<T> &m, T val){  \
    ERROR_LOG("not implemented for this type!");                        \
    return m;                                                           \
  }                                                                     \
  template<> DynMatrix<float> &matrix_##op(DynMatrix<float> &m, float val){ \
    return ipp_unary_func_c_i<float,ipps##func##_32f_I>(m,val);         \
  }                                                                     \
  template<> DynMatrix<double> &matrix_##op(DynMatrix<double> &m, double val){ \
    return ipp_unary_func_c_i<double,ipps##func##_64f_I>(m,val);        \
  }                                                                     \
  template<class T> DynMatrix<T> &matrix_##op(const DynMatrix<T> &,T, DynMatrix<T> &m){ \
    ERROR_LOG("not implemented for this type!");                        \
    return m;                                                           \
  }                                                                     \
  template<> DynMatrix<float> &matrix_##op(const DynMatrix<float> &s,float val, DynMatrix<float> &dst){ \
    return ipp_unary_func_c<float,ipps##func##_32f>(s,val,dst);         \
  }                                                                     \
  template<> DynMatrix<double> &matrix_##op(const DynMatrix<double> &s,double val, DynMatrix<double> &dst){ \
    return ipp_unary_func_c<double,ipps##func##_64f>(s,val,dst);        \
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
  
#define INSTANTIATE_DYN_MATRIX_MATH_OP(op,func)                         \
  template<class T>                                                     \
  DynMatrix<T> &matrix_##op(const DynMatrix<T> &a, const DynMatrix<T> &b, DynMatrix<T> &dst){ \
    ERROR_LOG("not implemented for this type!");                        \
    return dst;                                                         \
  }                                                                     \
  template<> DynMatrix<float> &matrix_##op(const DynMatrix<float> &a, const DynMatrix<float> &b, DynMatrix<float> &dst){ \
    return ipp_binary_func<float,ipps##func##_32f>(a,b,dst);            \
  }                                                                     \
  template<> DynMatrix<double> &matrix_##op(const DynMatrix<double> &a, const DynMatrix<double> &b, DynMatrix<double> &dst){ \
    return ipp_binary_func<double,ipps##func##_64f>(a,b,dst);           \
  }                                                                     \
  
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
  T matrix_max(const DynMatrix<T> &m, int *x=0, int *y=0){
    ERROR_LOG("not implemented for this type!");
    return 0;
  }
  
#define INSTANTIATE_DYN_MATRIX_MATH_OP(op,type,suffix,ippf)             \
  template<> type matrix_##op(const DynMatrix<type> &m, int *x, int *y){ \
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
  template<> void matrix_minmax(const DynMatrix<float> &m, float dst[2],
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
  template<> void matrix_minmax(const DynMatrix<double> &m, double dst[2],
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
  template DynMatrix<float> &matrix_##op(DynMatrix<float> &m);          \
  template DynMatrix<double> &matrix_##op(DynMatrix<double> &m);       

  static inline double sqr(double x) { return x*x; }
  static inline double reciprocal(double x) { return x ? 1.0/x : 0 ; }
  
  INSTANTIATE_DYN_MATRIX_MATH_OP(abs,::fabs)
  INSTANTIATE_DYN_MATRIX_MATH_OP(log,::log)
  INSTANTIATE_DYN_MATRIX_MATH_OP(exp,::exp)
  INSTANTIATE_DYN_MATRIX_MATH_OP(sqrt,::sqrt)
  INSTANTIATE_DYN_MATRIX_MATH_OP(sqr,icl::sqr)
  INSTANTIATE_DYN_MATRIX_MATH_OP(reciprocal,icl::reciprocal)
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
  template DynMatrix<float> &matrix_##op(DynMatrix<float>&, float);     \
  template DynMatrix<double> &matrix_##op(DynMatrix<double>&, double);  \
  template DynMatrix<float> &matrix_##op(const DynMatrix<float>&, float, DynMatrix<float>&); \
  template DynMatrix<double> &matrix_##op(const DynMatrix<double>&, double, DynMatrix<double>&);

  template <class T> static inline T addc(const T &a, const T &b){ return a+b; }
  template <class T> static inline T subc(const T &a, const T &b){ return a-b; }
  template <class T> static inline T divc(const T &a, const T &b){ return a/b; }
  template <class T> static inline T mulc(const T &a, const T &b){ return a*b; }
  
  INSTANTIATE_DYN_MATRIX_MATH_OP(powc,::pow)
  INSTANTIATE_DYN_MATRIX_MATH_OP(addc,icl::addc)
  INSTANTIATE_DYN_MATRIX_MATH_OP(subc,icl::subc)
  INSTANTIATE_DYN_MATRIX_MATH_OP(mulc,icl::mulc)
  INSTANTIATE_DYN_MATRIX_MATH_OP(divc,icl::divc)

#undef INSTANTIATE_DYN_MATRIX_MATH_OP
  

  // binary functions 
  
#define INSTANTIATE_DYN_MATRIX_MATH_OP(op,func)                         \
  template<class T>                                                     \
  DynMatrix<T> &matrix_##op(const DynMatrix<T> &a, const DynMatrix<T> &b, DynMatrix<T> &dst){ \
    CHECK_DIM(a,b,dst);                                                 \
    dst.setBounds(a.cols(),a.rows());                                   \
    std::transform(a.begin(),a.end(),b.begin(),dst.begin(),func);       \
    return dst;                                                         \
  }                                                                     \
  template DynMatrix<float> &matrix_##op(const DynMatrix<float>&, const DynMatrix<float>&, DynMatrix<float>&); \
  template DynMatrix<double> &matrix_##op(const DynMatrix<double>&, const DynMatrix<double>&, DynMatrix<double>&);

  INSTANTIATE_DYN_MATRIX_MATH_OP(pow,::pow)
  INSTANTIATE_DYN_MATRIX_MATH_OP(add,icl::addc<T>)
  INSTANTIATE_DYN_MATRIX_MATH_OP(sub,icl::subc<T>)
  INSTANTIATE_DYN_MATRIX_MATH_OP(mul,icl::mulc<T>)
  INSTANTIATE_DYN_MATRIX_MATH_OP(div,icl::divc<T>)
  INSTANTIATE_DYN_MATRIX_MATH_OP(arctan2,::atan2)
  
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
  
  template float matrix_min(const DynMatrix<float> &m, int *x, int *y);
  template double matrix_min(const DynMatrix<double> &m, int *x, int *y);
  
  template<class T>
  T matrix_max(const DynMatrix<T> &m, int *x, int *y){
    ICLASSERT_RETURN_VAL(m.cols(),0);
    const T *a = std::max_element(m.begin(),m.end());
    int idx = (int)(a-m.begin());
    if(x) *x = idx%m.cols();
    if(y) *y = idx/m.cols();
    return *a;
  }

  template float matrix_max(const DynMatrix<float> &m, int *x, int *y);
  template double matrix_max(const DynMatrix<double> &m, int *x, int *y);


  template<class T>
  void matrix_minmax(const DynMatrix<T> &m, T dst[2],
                     int *minx, int *miny,
                     int *maxx, int *maxy){
    dst[0] = matrix_min(m,minx,miny);
    dst[1] = matrix_max(m,maxx,maxy);
  }

  template void matrix_minmax(const DynMatrix<float> &m, float dst[2],int*,int*,int*,int*);
  template void matrix_minmax(const DynMatrix<double> &m, double dst[2],int*,int*,int*,int*);

#undef CHECK_DIM 

#endif //HAVE_IPP 
}



