// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/la/FixedMatrix.h>
#include <icl/utils/Point.h>
#include <vector>

namespace icl::math {

  template<class T> struct GenericHomography2D;   // fwd (result type below references it)

  /// Result of a robust (RANSAC) homography fit: the model + its inlier support.
  template<class T>
  struct HomographyFit {
    GenericHomography2D<T> H;      ///< best homography (refit on all inliers)
    std::vector<int> inliers;     ///< indices of the inlier correspondences
    T rms = T(0);                 ///< inlier reprojection RMS [px]
    bool ok = false;              ///< true iff a model with >=4 inliers was found
  };

  /// Utility structure that represents a 2D homography (implemented for float and double)
  /** Given two sets of at least 4 corresponding 2D points passed to the
      constructor as \c pAs (call them \f$\{a_i\}\f$) and \c pBs
      (\f$\{b_i\}\f$), this class computes the 3x3 homography that maps the
      SECOND set onto the FIRST:

      \f[ H\,b_i \;=\; a_i \qquad\Longleftrightarrow\qquad \texttt{apply}(b_i) = a_i \f]

      \warning The direction is `pBs → pAs`, i.e. `apply(pBs[i]) == pAs[i]` — the
      OPPOSITE of what the \f$a,b\f$ naming in the algorithm section below might
      suggest. The constructor swaps the two point sets once and runs the DLT
      derivation (which is written for \f$H\,a = b\f$) on the swapped inputs, so
      the externally observable mapping is \f$b \to a\f$. (Example callers:
      `Homography2D(image, board).apply(boardPt) → imagePt`.)

      @section ALG Algorithm

      (Written in terms of the INTERNAL, post-swap variables — here \f$a,b\f$ are
      the swapped point sets, so the derivation solves \f$H\,a=b\f$ internally.)
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

    /// DEPRECATED — use fit(src,dst,n). NOTE the arg order differs: this maps
    /// pBs → pAs (`apply(pBs[i])==pAs[i]`), whereas fit maps src → dst.
    [[deprecated("use Homography2D::fit(src,dst,n) — fit maps src->dst, the OPPOSITE "
                 "arg order of this constructor")]]
    GenericHomography2D(const utils::Point32f *pAs, const utils::Point32f *pBs, int n=4);

    /// Fit the homography that maps \a src onto \a dst: `apply(src[i]) == dst[i]`.
    /** Normalized (Hartley) DLT — the linear least-squares estimate; \a n >= 4.
        This is the plain, fast default. See refined() for a geometric-error LM
        refinement. */
    static GenericHomography2D fit(const utils::Point32f *src, const utils::Point32f *dst, int n=4);

    /// Like fit(), then Levenberg-Marquardt refinement of the geometric error.
    /** Seeds with the fit() DLT estimate and minimizes the true reprojection
        error \f$\sum_i \|\texttt{apply}(src_i) - dst_i\|^2\f$. More accurate than
        fit() on noisy correspondences (DLT only minimizes an algebraic proxy), at
        a few× the cost. Same src→dst convention as fit(). */
    static GenericHomography2D refined(const utils::Point32f *src, const utils::Point32f *dst, int n=4);

    /// RANSAC robust fit of src → dst, tolerant to outlier correspondences.
    /** Repeatedly fits a 4-point minimal homography and keeps the one with the
        largest inlier set (points whose reprojection residual is
        < \a inlierThreshPx), then refits fit() on all inliers. Deterministic
        (fixed internal RNG) with adaptive early-out. Returns the model plus the
        inlier index set and RMS; \c ok is false if \a n<4 or no >=4-inlier model
        was found. Same src→dst convention as fit(). */
    static HomographyFit<T> robust(const utils::Point32f *src, const utils::Point32f *dst, int n,
                                   T inlierThreshPx, int maxIters=200);


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
      return utils::Point32f(apply_homography_int(*this, p.rounded()));
    }
  };

  /// default homography 2D type definition (usually float depth is enough)
  using Homography2D = GenericHomography2D<float>;


  } // namespace icl::math