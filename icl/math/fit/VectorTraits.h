// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/la/DynVector.h>
#include <icl/math/la/FixedVector.h>
#include <vector>
#include <algorithm>

namespace icl::math {

  /// Uniform vector abstraction for the Tier-B optimizers.
  /** Formalises the ad-hoc free-function layer (vdim/create_zero_vector) that
      SimplexEngine hand-rolled, so derivative-free optimizers can be written
      once against an abstract "vector of scalars" and instantiated for
      std::vector<S>, DynColVector<S> and FixedColVector<S,D>. Primary template is
      intentionally undefined — only the supported vector types specialise it. */
  template<class V> struct VectorTraits;

  template<class S>
  struct VectorTraits<std::vector<S> >{
    using Scalar = S;
    static unsigned dim(const std::vector<S> &v){ return unsigned(v.size()); }
    static std::vector<S> create(unsigned n, S val = S(0)){ return std::vector<S>(n, val); }
    static S    get(const std::vector<S> &v, unsigned i){ return v[i]; }
    static void set(std::vector<S> &v, unsigned i, S x){ v[i] = x; }
  };

  template<class S>
  struct VectorTraits<DynColVector<S> >{
    using Scalar = S;
    static unsigned dim(const DynColVector<S> &v){ return v.dim(); }
    static DynColVector<S> create(unsigned n, S val = S(0)){ return DynColVector<S>(n, val); }
    static S    get(const DynColVector<S> &v, unsigned i){ return v[i]; }
    static void set(DynColVector<S> &v, unsigned i, S x){ v[i] = x; }
  };

  // FixedColVector<S,D> == FixedMatrix<S,D,1>
  template<class S, unsigned int D>
  struct VectorTraits<FixedMatrix<S,D,1> >{
    using Scalar = S;
    static unsigned dim(const FixedMatrix<S,D,1> &){ return D; }
    static FixedMatrix<S,D,1> create(unsigned, S val = S(0)){
      FixedMatrix<S,D,1> v; std::fill(v.begin(), v.end(), val); return v;
    }
    static S    get(const FixedMatrix<S,D,1> &v, unsigned i){ return v[i]; }
    static void set(FixedMatrix<S,D,1> &v, unsigned i, S x){ v[i] = x; }
  };

} // namespace icl::math
