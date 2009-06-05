#ifndef ICL_DYN_MATRIX_UTILS_H
#define ICL_DYN_MATRIX_UTILS_H

#include <iclDynMatrix.h>
#include <algorithm>

namespace icl{

  template<class T, class Init>
  inline DynMatrix<T> &matrix_init(DynMatrix<T> &m, Init init){
    std::fill(m.begin(),m.end(),init);
    return m;
  }

  template<class T>
  DynMatrix<T> &matrix_abs(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_abs(const DynMatrix<T> &m, DynMatrix<T> &dst);


  template<class T>
  DynMatrix<T> &matrix_log(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_log(const DynMatrix<T> &m, DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_exp(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_exp(const DynMatrix<T> &m, DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_sqrt(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_sqrt(const DynMatrix<T> &m, DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_sqr(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_sqr(const DynMatrix<T> &m, DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_pow(DynMatrix<T> &m, double exponent);
  template<class T>
  DynMatrix<T> &matrix_pow(const DynMatrix<T> &m, double exponent, 
                           DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_sin(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_sin(const DynMatrix<T> &m, DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_cos(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_cos(const DynMatrix<T> &m, DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_tan(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_tan(const DynMatrix<T> &m, DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_arcsin(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_arcsin(const DynMatrix<T> &m, DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_arccos(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_arccos(const DynMatrix<T> &m, DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_arctan(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_arctan(const DynMatrix<T> &m, DynMatrix<T> &dst);

  template<class T>
  DynMatrix<T> &matrix_arctan2(const DynMatrix<T> &my, const DynMatrix<T> &mx,
                               DynMatrix<T> &dst);

  template<class T>
  T matrix_min(const DynMatrix<T> &m, int *x=0, int *y=0);

  template<class T>
  T matrix_max(const DynMatrix<T> &m, int *x=0, int *y=0);

  template<class T>
  void matrix_minmax(const DynMatrix<T> &m, T dst[2],
                     int *minx=0, int *miny=0,
                     int *maxx=0, int *maxy=0);
  
  template<class T>
  DynMatrix<T> &matrix_reciprocal(DynMatrix<T> &m);
  template<class T>
  DynMatrix<T> &matrix_reciprocal(const DynMatrix<T> &m, DynMatrix<T> &dst);
}


#endif
