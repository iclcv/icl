// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Range.h>

namespace icl::math {
    /// A standard linear mapping class for the 1D case f(x) = m * x + b
    struct LinearTransform1D{
      /// slope
      float m;

      /// offset
      float b;

      /// base constructor with given parameters m and b
      inline LinearTransform1D(float m=0, float b=0):m(m),b(b){}

      /// spedical constructor template with given source and destination range
      template<class T>
      inline LinearTransform1D(const utils::Range<T> &s, const utils::Range<T> &d):
        m(d.getLength()/s.getLength()), b(-m*s.minVal + d.minVal){
      }

      /// applies the mapping to a given x -> f(x) = m * x + b
      float operator()(float x) const { return m*x+b; }
    };

  } // namespace icl::math