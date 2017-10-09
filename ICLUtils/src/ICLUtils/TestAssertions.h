/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/TestAssertions.h                 **
** Module : ICLUtils                                               **
** Authors: Erik Weitnauer                                         **
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

/// Provide several functions for conviently testing if two matrices / vectors
/// are (almost) equal with GTest.

#pragma once

#ifdef ICL_HAVE_GTEST

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Point32f.h>
#include <ICLMath/FixedVector.h>
#include <ICLMath/DynMatrixUtils.h>
#include <gtest/gtest.h>

namespace icl {
  namespace utils{

  /** Checks whether all elements of two matrices are pairwise almost equal using
  * a given error bound 'delta'. Use this function in your test by calling, e.g.,
  * EXPECT_TRUE(isNear(A, B, delta));
  * Important: If type T is float, you must explicitely tell the compiler that
  *            delta is a float, e.g., isNear(A, B, 1e-6f).
  */
  template<class T, unsigned int WIDTH, unsigned int HEIGHT>
  testing::AssertionResult isNear(const FixedMatrix<T,WIDTH,HEIGHT> &matrix_a,
                                  const FixedMatrix<T,WIDTH,HEIGHT> &matrix_b,
                                  T delta) {
    for (unsigned int x=0; x<WIDTH; x++) for (unsigned int y=0; y<HEIGHT; y++) {
      if ((matrix_a(x,y)-matrix_b(x,y) > delta) ||
          (matrix_b(x,y)-matrix_a(x,y) > delta)) {
        return testing::AssertionFailure()
          << "The difference of matrix A(" << x << ", " << y
          << ") and matrix B(" << x << ", " << y << ") is " << matrix_a(x,y)-matrix_b(x,y)
          << ", which exceeds delta, where\nA=\n" << matrix_a << "\nB=\n" << matrix_b
          << "\ndelta=" << delta << ".";
      }
    }
    return testing::AssertionSuccess();
  } // namespace utils
}

/** Checks whether all elements of two column vectors are pairwise almost equal
* using a given error bound. Use this function in your test by calling, e.g.,
* EXPECT_TRUE(isNear(A, B, error_bound));
* Important: If type T is float, you must explicitely tell the compiler that
*            delta is a float, e.g., isNear(A, B, 1e-6f).
*/
template<class T, unsigned int DIM>
testing::AssertionResult isNear(const FixedColVector<T,DIM> &vector_a,
                                const FixedColVector<T,DIM> &vector_b,
                                T delta) {
  for (unsigned int i=0; i<DIM; i++) {
    if (abs(vector_a[i]-vector_b[i]) > delta) {
      return testing::AssertionFailure()
        << "The difference of col vector a[" << i << "] and col vector b["
        << i << "] is " << vector_a[i]-vector_b[i] << ", which exceeds delta, where\n"
        << "a=" << vector_a.transp() << "\nb=" << vector_b.transp()
        << "\ndelta=" << delta << ".";

    }
  }
  return testing::AssertionSuccess();
}

/** Checks whether all elements of two row vectors are pairwise almost equal
* using a given error bound. Use this function in your test by calling, e.g.,
* EXPECT_TRUE(isNear(A, B, error_bound));
* Important: If type T is float, you must explicitely tell the compiler that
*            delta is a float, e.g., isNear(A, B, 1e-6f).
*/
template<class T, unsigned int DIM>
testing::AssertionResult isNear(const FixedRowVector<T,DIM> &vector_a,
                                const FixedRowVector<T,DIM> &vector_b,
                                T delta) {
  for (unsigned int i=0; i<DIM; i++) {
    if (abs(vector_a[i]-vector_b[i]) > delta) {
      return testing::AssertionFailure()
        << "The difference of row vector a[" << i << "] and row vector b[" << i
        << "] is " << vector_a[i]-vector_b[i] << ", which exceeds delta, where\n"
        << "a=" << vector_a << "\nb=" << vector_b
        << "\ndelta=" << delta << ".";

    }
  }
  return testing::AssertionSuccess();
}

/** Checks whether the distance between two point is lower or equal to a given
* error bound delta. Use this function in your test by calling, e.g.,
* EXPECT_TRUE(isNear(p, q, error_bound));
*/
testing::AssertionResult isNear(const Point32f &p,
                                const Point32f &q,
                                float delta) {
  float dist = sqrt((p.x-q.x)*(p.x-q.x) + (p.y-q.y)*(p.y-q.y));
  if (dist > delta) {
    return testing::AssertionFailure()
      << "The distance of points " << p << " and " << q << " is "
      << dist << ", which exceeds delta=" << delta << ".";
  }
  return testing::AssertionSuccess();
}
}

#endif
