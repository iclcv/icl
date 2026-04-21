// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/Homography2D.h>
#include <icl/math/DynMatrix.h>

#include <icl/math/FixedVector.h>

using namespace icl::utils;

namespace icl::math {
  template<class T>
  GenericHomography2D<T>::GenericHomography2D(const Point32f *x, const Point32f *y, int n,typename GenericHomography2D<T>::Algorithm alg){
    if(alg == Simple){
      DynMatrix<T> X(n,3),Y(n,3);
      for(int i=0;i<n;++i){
        X(i,0) = x[i].x;
        X(i,1) = x[i].y;
        X(i,2) = 1;
        Y(i,0) = y[i].x;
        Y(i,1) = y[i].y;
        Y(i,2) = 1;
      }
      DynMatrix<T> H = X*Y.pinv(); // A * B.pinv()

      std::copy(H.begin(),H.end(),Super::begin());
    }else{
      // actually we use the homography backwards
      std::swap(x,y);

      DynMatrix<T> M(8,2*n),r(1,2*n);
      for(int i=0;i<n;++i){
        T xx = x[i].x, xy=x[i].y, yx=y[i].x, yy=y[i].y;
        T *m = &M(0,2*i);
        m[0] = xx;
        m[1] = xy;
        m[2] = 1;
        m[5] = m[4] = m[3] = 0;
        m[6] = - xx * yx;
        m[7] = - xy * yx;
        m+=8;
        m[2] = m[1] = m[0] = 0;
        m[3] = xx;
        m[4] = xy;
        m[5] = 1;
        m[6] = - xx * yy;
        m[7] = - xy * yy;

        r[2*i] = yx;
        r[2*i+1] = yy;
      }



      DynMatrix<T> h  = M.solve(r);


      std::copy(h.begin(),h.end(),Super::begin());
      this->operator[](8) = 1;
    }
  }

  template struct ICLMath_API GenericHomography2D<float>;
  template struct ICLMath_API GenericHomography2D<double>;

  } // namespace icl::math