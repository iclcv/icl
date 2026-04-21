// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/FixedMatrix.h>
#include <icl/utils/Point32f.h>

namespace icl::math {
  /// Utility structure that represents a 2D homography (implemented for float and double)
  /** Given two sets of at least 4 corresponding 2D points \f$\{a_i\}\f$ and
      \f$\{b_i\}\f$ (\f$i \in [0,n[\f$), this class computes the 3x3
      homography matrix \f$H\f$ such that \f$H\,a_i = b_i\f$ (in homogeneous
      coordinates).

      @section ALG Algorithm

      The homography has 8 degrees of freedom (a 3x3 matrix defined up to
      scale). For each point pair \f$(a, b)\f$ we get two linear equations
      in the 8 unknowns \f$h = (X^T Y^T L_x L_y)^T\f$, where \f$H\f$'s rows
      are \f$X, Y, (L_x\, L_y\, 1)\f$. Stacking the equations for all
      \f$n\f$ pairs gives a \f$2n \times 8\f$ system \f$M h = r\f$. For
      \f$n = 4\f$ this is exactly determined; for \f$n > 4\f$ the system
      is over-determined and solved in a least-squares sense via
      DynMatrix::solve (LAPACK gelsd / SVD).

      For each pair \f$(a, b)\f$, two rows of \f$M\f$ are:

      \f[
      M = \left(\begin{array}{cccccccc}
      a_x & a_y & 1 & 0   & 0   & 0 & -a_x b_x & -a_y b_x \\
      0   & 0   & 0 & a_x & a_y & 1 & -a_x b_y & -a_y b_y \\
      \vdots
      \end{array}\right), \quad r = (b_x\; b_y\; \ldots)^T
      \f]

      @section NORM Hartley Normalization

      Solving the DLT system directly in pixel coordinates is numerically
      unstable: entries span several orders of magnitude (products of two
      pixel coordinates appear in the last two columns), which ill-conditions
      \f$M\f$ and produces errors of tens of pixels even for exactly-determined
      systems. This class applies the standard Hartley normalization
      (Hartley &amp; Zisserman, Algorithm 4.2): each point set is translated
      so its centroid is at the origin and scaled so the mean distance from
      the origin is \f$\sqrt{2}\f$; the homography is fitted in this
      well-conditioned coordinate system and then un-normalized
      (\f$H = T_b^{-1}\,\tilde{H}\,T_a\f$).
  */
  template<class T>
  struct ICLMath_IMP GenericHomography2D : public FixedMatrix<T, 3, 3>{
    /// super class typedef for shorter super-class references
    using Super = FixedMatrix<T,3,3>;

    /// Empty constructor
    GenericHomography2D(){}

    /// Constructor from given two point sets of size n>=4
    GenericHomography2D(const utils::Point32f *pAs, const utils::Point32f *pBs, int n=4);


    /// applies a given homography matrix
    static inline utils::Point32f apply_homography(const FixedMatrix<float,3,3> &H, const utils::Point32f &p){
      float az = H(2, 0)*p.x + H(2, 1) * p.y + H(2, 2);
      return utils::Point32f(( H(0, 0)*p.x + H(0, 1) * p.y + H(0, 2) ) / az,
                             ( H(1, 0)*p.x + H(1, 1) * p.y + H(1, 2) ) / az );
    }

    /// applies the homography
    inline utils::Point32f apply(const utils::Point32f &p) const{
      return apply_homography(*this,p);
    }

    /// applies a given homography matrix
    static inline utils::Point apply_homography_int(const FixedMatrix<float,3,3> &H, const utils::Point &p){
      float az = H(2, 0)*p.x + H(2, 1) * p.y + H(2, 2);
      return utils::Point(round(( H(0, 0)*p.x + H(0, 1) * p.y + H(0, 2) ) / az),
                   round(( H(1, 0)*p.x + H(1, 1) * p.y + H(1, 2) ) / az) );
    }

    /// applies the homography
    inline utils::Point32f apply_int(const utils::Point32f &p) const{
      return apply_homography_int(*this, p);
    }
  };

  /// default homography 2D type definition (usually float depth is enough)
  using Homography2D = GenericHomography2D<float>;


  } // namespace icl::math