// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/DynVector.h>

namespace icl::math {
  // ---- DynColVector ----

  template<class T>
  DynColVector<T>::DynColVector(const typename DynMatrix<T>::DynMatrixColumn &column)
    : DynMatrix<T>(column){}

  template<class T>
  DynColVector<T>::DynColVector() : DynMatrix<T>(){}

  template<class T>
  DynColVector<T>::DynColVector(unsigned int dim, const T &initValue)
    : DynMatrix<T>(1, dim, initValue){}

  template<class T>
  DynColVector<T>::DynColVector(unsigned int dim, T *data, bool deepCopy)
    : DynMatrix<T>(1, dim, data, deepCopy){}

  template<class T>
  DynColVector<T>::DynColVector(unsigned int dim, const T *data)
    : DynMatrix<T>(1, dim, data){}

  template<class T>
  DynColVector<T>::DynColVector(const DynMatrix<T> &other) : DynMatrix<T>(other){
    ICLASSERT_THROW(DynMatrix<T>::cols() == 1,
                    InvalidMatrixDimensionException("DynColVector(DynMatrix): source matrix has more than one column"));
  }

  template<class T>
  DynColVector<T>& DynColVector<T>::operator=(const DynMatrix<T> &other){
    DynMatrix<T>::operator=(other);
    ICLASSERT_THROW(DynMatrix<T>::cols() == 1,
                    InvalidMatrixDimensionException("DynColVector = DynMatrix: source matrix has more than one column"));
    return *this;
  }

  template<class T>
  void DynColVector<T>::setBounds(unsigned int dim, bool holdContent, const T &initializer){
    DynMatrix<T>::setBounds(1, dim, holdContent, initializer);
  }

  template<class T>
  void DynColVector<T>::setDim(unsigned int dim, bool holdContent, const T &initializer){
    setBounds(dim, holdContent, initializer);
  }

  // ---- DynRowVector ----

  template<class T>
  DynRowVector<T>::DynRowVector() : DynMatrix<T>(){}

  template<class T>
  DynRowVector<T>::DynRowVector(unsigned int dim, const T &initValue)
    : DynMatrix<T>(dim, 1, initValue){}

  template<class T>
  DynRowVector<T>::DynRowVector(unsigned int dim, T *data, bool deepCopy)
    : DynMatrix<T>(dim, 1, data, deepCopy){}

  template<class T>
  DynRowVector<T>::DynRowVector(unsigned int dim, const T *data)
    : DynMatrix<T>(dim, 1, data){}

  template<class T>
  DynRowVector<T>::DynRowVector(const DynMatrix<T> &other) : DynMatrix<T>(other){
    ICLASSERT_THROW(DynMatrix<T>::rows() == 1,
                    InvalidMatrixDimensionException("DynRowVector(DynMatrix): source matrix has more than one row"));
  }

  template<class T>
  DynRowVector<T>& DynRowVector<T>::operator=(const DynMatrix<T> &other){
    DynMatrix<T>::operator=(other);
    ICLASSERT_THROW(DynMatrix<T>::rows() == 1,
                    InvalidMatrixDimensionException("DynRowVector = DynMatrix: source matrix has more than one row"));
    return *this;
  }

  template<class T>
  void DynRowVector<T>::setBounds(unsigned int dim, bool holdContent, const T &initializer){
    DynMatrix<T>::setBounds(dim, 1, holdContent, initializer);
  }

  template<class T>
  void DynRowVector<T>::setDim(unsigned int dim, bool holdContent, const T &initializer){
    setBounds(dim, holdContent, initializer);
  }

  // Whole-class explicit instantiation
  template struct ICLMath_API DynColVector<float>;
  template struct ICLMath_API DynColVector<double>;
  template struct ICLMath_API DynRowVector<float>;
  template struct ICLMath_API DynRowVector<double>;

  } // namespace icl::math