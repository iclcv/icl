#include <iclDynMatrixUtils.h>
#include <cmath>
#include <iclMacros.h>

#define CHECK_DIM(m1,m2,RET)                                            \
  ICLASSERT_RETURN_VAL( (m1.cols() == m2.cols()) && (m1.rows() == m2.rows()) , RET)


#undef HAVE_IPP

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


#endif //HAVE_IPP 


  /// ------------------------------------------------------------
  /// ipp and non-ipp mixed ...
  /// ------------------------------------------------------------


  // MEAN ***************************
  template<class T> T matrix_mean(const DynMatrix<T> &m){
    return std::accumulate(m.begin(),m.end(),T(0))/m.dim();
  }

#ifdef HAVE_IPP
  template<> float matrix_mean(const DynMatrix<float> &m){
    float v=0; ippsMean_32f(m.begin(),m.dim(),&v,ippAlgHintNone);  return v;
  }

  template<> double matrix_mean(const DynMatrix<double> &m){
    double v=0; ippsMean_64f(m.begin(),m.dim(),&v); return v;
  }
#endif // HAVE_IPP

  template float matrix_mean(const DynMatrix<float>&);
  template double matrix_mean(const DynMatrix<double>&);


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

#ifdef HAVE_IPP
  template<> float matrix_var(const DynMatrix<float> &m){
    float v=0; ippsStdDev_32f(m.begin(),m.dim(),&v,ippAlgHintNone); return v*v; 
  }
  template<> double matrix_var(const DynMatrix<double> &m){
    double v=0; ippsStdDev_64f(m.begin(),m.dim(),&v); return v*v; 
  }
#endif // HAVE_IPP

  template float matrix_var(const DynMatrix<float>&);
  template double matrix_var(const DynMatrix<double>&);

   // VAR (2)***************************
  template<class T>
  T matrix_var(const DynMatrix<T> &m, T mean, bool empiricalMean){
    if(m.dim()<2){ return 0; }
    T var = 0;
    std::for_each(m.begin(),m.end(),var_functor<T>(mean,var));
    int norm = empiricalMean ? (m.dim()-1) : m.dim();
    return var/norm;
  }
  template float matrix_var(const DynMatrix<float>&, float, bool);
  template double matrix_var(const DynMatrix<double>&, double, bool);


  // MEANVAR ***************************
  template<class T>
  void matrix_meanvar(const DynMatrix<T> &m, T *mean, T*var){
    T meanVal = matrix_mean(m);
    T varVal = matrix_var(m,meanVal,true);
    if(mean) *mean=meanVal;
    if(var) *var=varVal;
  }

#ifdef HAVE_IPP
  template<> void matrix_meanvar(const DynMatrix<float> &m, float *mean, float *var){
    ICLASSERT_RETURN(mean && var);
    ippsMeanStdDev_32f(m.begin(),m.dim(),mean,var,ippAlgHintNone);
    *var = (*var)*(*var);
  }
  template<> void matrix_meanvar(const DynMatrix<double> &m, double *mean, double*var){
    ICLASSERT_RETURN(mean && var);
    ippsMeanStdDev_64f(m.begin(),m.dim(),mean,var);
    *var = (*var)*(*var);
  }
#endif // HAVE_IPP

  template void matrix_meanvar(const DynMatrix<float>&,float*,float*);
  template void matrix_meanvar(const DynMatrix<double>&,double*,double*);
  

  // STDDEV ***************************
  template<class T>
  T matrix_stddev(const DynMatrix<T> &m){
    return ::sqrt(matrix_var(m));
  }

  template float matrix_stddev(const DynMatrix<float>&);
  template double matrix_stddev(const DynMatrix<double>&);

  template<class T>
  T matrix_stddev(const DynMatrix<T> &m, T mean, bool empiricalMean){
    return ::sqrt(matrix_var(m,mean,empiricalMean));
  }

  template float matrix_stddev(const DynMatrix<float>&,float,bool);
  template double matrix_stddev(const DynMatrix<double>&,double,bool);


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
  template DynMatrix<float> &matrix_muladd(const DynMatrix<float>&,float,const DynMatrix<float>&,float,float,DynMatrix<float>&);
  template DynMatrix<double> &matrix_muladd(const DynMatrix<double>&,double,const DynMatrix<double>&,double,double,DynMatrix<double>&);


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
  template DynMatrix<float> &matrix_muladd(const DynMatrix<float>&,float,float,DynMatrix<float>&);
  template DynMatrix<double> &matrix_muladd(const DynMatrix<double>&,double,double,DynMatrix<double>&);

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

  template DynMatrix<float> &matrix_mask(const DynMatrix<unsigned char> &mask, DynMatrix<float> &m);
  template DynMatrix<double> &matrix_mask(const DynMatrix<unsigned char> &mask, DynMatrix<double> &m);

    /// applies masking operation (m(i,j) is set to 0 if mask(i,j) is 0)
  template<class T>
  DynMatrix<T> &matrix_mask(const DynMatrix<unsigned char> &mask, const DynMatrix<T> &m, DynMatrix<T> &dst){
    CHECK_DIM(mask,m,dst);
    dst.setBounds(m.cols(),m.rows());
    std::transform(mask.begin(),mask.end(),m.begin(),dst.begin(),mask_functor<T>());
    return dst;
  }

  template DynMatrix<float> &matrix_mask(const DynMatrix<unsigned char> &mask, const DynMatrix<float> &m, DynMatrix<float> &dst);
  template DynMatrix<double> &matrix_mask(const DynMatrix<unsigned char> &mask, const DynMatrix<double> &m, DynMatrix<double> &dst);

  

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
  
  template float matrix_distance(const DynMatrix<float> &m1, const DynMatrix<float> &m2, float);
  template double matrix_distance(const DynMatrix<double> &, const DynMatrix<double> &, double);



  template<class T>
  struct matrix_divergence_op{
    inline T operator()(const T &a, const T&b) const{ return a * log(a/b) - a + b;  }
  };

  template<class T>
  T matrix_divergence(const DynMatrix<T> &m1, const DynMatrix<T> &m2){
    CHECK_DIM(m1,m2,-1);
    return std::inner_product(m1.begin(),m1.end(),m2.begin(),T(0),std::plus<T>(),matrix_divergence_op<T>());
  }

  template float matrix_divergence(const DynMatrix<float>&, const DynMatrix<float>&);
  template double matrix_divergence(const DynMatrix<double>&, const DynMatrix<double>&);

#undef CHECK_DIM 

}



