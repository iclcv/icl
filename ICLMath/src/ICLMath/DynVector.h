/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/DynVector.h                        **
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
