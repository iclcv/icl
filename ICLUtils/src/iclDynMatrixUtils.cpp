#include <iclDynMatrixUtils.h>
#include <cmath>
#include <iclMacros.h>

#define CHECK_DIM(m1,m2,RET)                                            \
  ICLASSERT_RETURN_VAL( (m1.cols() != m2.cols()) || (m1.rows() != m2.rows()) , RET)

namespace icl{

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

  struct Pow{
    double e;
    Pow(double e):e(e){}
    inline double operator()(const double &x) const{
      return pow(x,e);
    }
  };
  
  template<class T>
  DynMatrix<T> &matrix_pow(DynMatrix<T> &m, double exponent){
    std::for_each(m.begin(),m.end(),Pow(exponent));
    return m;
  }
  
  template DynMatrix<float> &matrix_pow(DynMatrix<float> &m, double exponent);
  template DynMatrix<double> &matrix_pow(DynMatrix<double> &m, double exponent);
  
  template<class T>
  DynMatrix<T> &matrix_pow(const DynMatrix<T> &m, double exponent, DynMatrix<T> &dst){
    dst.setBounds(m.cols(),m.rows());
    std::transform(m.begin(),m.end(),dst.begin(),Pow(exponent));
    return dst;
  }

  template DynMatrix<float> &matrix_pow(const DynMatrix<float> &m, double exponent, DynMatrix<float> &dst);
  template DynMatrix<double> &matrix_pow(const DynMatrix<double> &m, double exponent, DynMatrix<double> &dst);
  
  
  template<class T>
  DynMatrix<T> &matrix_arctan2(const DynMatrix<T> &my, const DynMatrix<T> &mx, DynMatrix<T> &dst){
    CHECK_DIM(my,mx,dst);
    dst.setBounds(mx.cols(),mx.rows());
    std::transform(my.begin(),my.end(),mx.begin(),dst.begin(),::atan2);
    return dst;
  }

  template DynMatrix<float> &matrix_arctan2(const DynMatrix<float> &my, const DynMatrix<float> &mx, DynMatrix<float> &dst);
  template DynMatrix<double> &matrix_arctan2(const DynMatrix<double> &my, const DynMatrix<double> &mx, DynMatrix<double> &dst);

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
 
}


