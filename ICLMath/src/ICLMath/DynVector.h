// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/DynMatrix.h>

namespace icl{
  namespace math{

    /// Extension class for DynMatrix<T> that restricts column count to one
    template<class T>
    struct ICLMath_API DynColVector : public DynMatrix<T>{
      DynColVector(const typename DynMatrix<T>::DynMatrixColumn &column);
      DynColVector();
      DynColVector(unsigned int dim, const T &initValue=0);
      DynColVector(unsigned int dim, T *data, bool deepCopy=true);
      DynColVector(unsigned int dim, const T *data);
      DynColVector(const DynMatrix<T> &other);
      DynColVector<T> &operator=(const DynMatrix<T> &other);
      void setBounds(unsigned int dim, bool holdContent=false, const T &initializer=0);
      void setDim(unsigned int dim, bool holdContent=false, const T &initializer=0);
    };

    /// Extension class for DynMatrix<T> that restricts row count to one
    template<class T>
    struct ICLMath_API DynRowVector : public DynMatrix<T>{
      DynRowVector();
      DynRowVector(unsigned int dim, const T &initValue=0);
      DynRowVector(unsigned int dim, T *data, bool deepCopy=true);
      DynRowVector(unsigned int dim, const T *data);
      DynRowVector(const DynMatrix<T> &other);
      DynRowVector<T> &operator=(const DynMatrix<T> &other);
      void setBounds(unsigned int dim, bool holdContent=false, const T &initializer=0);
      void setDim(unsigned int dim, bool holdContent=false, const T &initializer=0);
    };

  } // namespace math
}
