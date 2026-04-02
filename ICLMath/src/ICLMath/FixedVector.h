// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/FixedMatrix.h>

namespace icl::math {
    template<class T, int DIM>
    struct FixedColVector : public FixedMatrix<T, 1, DIM>{
      using super = FixedMatrix<T,1,DIM>;
      FixedColVector(){}
      explicit FixedColVector(const T &init):super(init){}
      explicit FixedColVector(const T *srcData):super(srcData){}
      FixedColVector(const FixedMatrix<T,1,DIM> &other):super(other){}
      FixedColVector(const T&v0,const T&v1, const T&v2=0, const T&v3=0, const T&v4=0, const T&v5=0,
                     const T&v6=0,const T&v7=0, const T&v8=0, const T&v9=0, const T&v10=0, const T&v11=0):
      super(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11){}

      template<class Iterator>
      FixedColVector (const FixedMatrixPart<T,DIM,Iterator> &r):super(r){}

      template<class otherT, class Iterator>
      FixedColVector(const FixedMatrixPart<otherT,DIM,Iterator> &r):super(r){}


    };


    template<class T, int DIM>
    struct FixedRowVector : public FixedMatrix<T, DIM, 1>{
      using super = FixedMatrix<T,DIM,1>;
      FixedRowVector(){}
      explicit FixedRowVector(const T &init):super(init){}
      explicit FixedRowVector(const T *srcData):super(srcData){}
      FixedRowVector(const FixedMatrix<T,DIM,1> &other):super(other){}
      FixedRowVector(const T&v0,const T&v1, const T&v2=0, const T&v3=0, const T&v4=0, const T&v5=0,
                     const T&v6=0,const T&v7=0, const T&v8=0, const T&v9=0, const T&v10=0, const T&v11=0):
      super(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11){}

      template<class Iterator>
      FixedRowVector (const FixedMatrixPart<T,DIM,Iterator> &r):super(r){}

      template<class otherT, class Iterator>
      FixedRowVector(const FixedMatrixPart<otherT,DIM,Iterator> &r):super(r){}

    };



  } // namespace icl::math