// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/MathFunctions.h>
#include <icl/math/la/DynMatrix.h>

namespace icl::math {

  std::vector<std::complex<double> >
  polynomialRoots(const std::vector<double> &coeffs){
    // trim leading (highest-degree) zeros so the leading coefficient is non-zero
    size_t s = 0;
    while(s + 1 < coeffs.size() && coeffs[s] == 0.0) ++s;
    const int n = int(coeffs.size()) - int(s) - 1;      // polynomial degree
    std::vector<std::complex<double> > roots;
    if(n < 1) return roots;

    // The roots of a polynomial are the eigenvalues of its companion matrix.
    // Monic form p/c0:  row 0 = -a1..-an, sub-diagonal = 1.
    DynMatrix<double> C(n, n, 0.0);
    const double c0 = coeffs[s];
    for(int j = 0; j < n; ++j) C(0, j)   = -coeffs[s + 1 + j] / c0;
    for(int i = 1; i < n; ++i) C(i, i-1) = 1.0;

    const DynMatrix<double>::GeneralEigenResult e = C.eigenGeneral();
    roots.reserve(n);
    for(int i = 0; i < n; ++i)
      roots.emplace_back(e.valuesReal[i], e.valuesImag[i]);
    return roots;
  }

} // namespace icl::math
