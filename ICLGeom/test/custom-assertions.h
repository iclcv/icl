#include <gtest/gtest.h>
#include <ICLGeom/Camera.h>
#include <ICLUtils/DynMatrixUtils.h>
#include <ICLUtils/FixedMatrixUtils.h>

/** Checks whether all elements of two matrices are pairwise almost equal using
* a given error bound 'delta'. Use this function in your test by calling, e.g.,
* EXPECT_TRUE(isNear(A, B, delta));
* Important: If type T is float, you must explicitely tell the compiler that
*            delta is a float, e.g., isNear(A, B, 1e-6f).
*/
template<class T, unsigned int WIDTH, unsigned int HEIGHT>
testing::AssertionResult isNear(const icl::FixedMatrix<T,WIDTH,HEIGHT> &matrix_a,
                                const icl::FixedMatrix<T,WIDTH,HEIGHT> &matrix_b,
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
}

/// Checks whether all elements of two vectors are pairwise almost equal using
/// a given error bound. Use this function in your test by calling, e.g.,
/// EXPECT_TRUE(isNear(A, B, error_bound));
testing::AssertionResult isNear(const icl::Vec &vector_a, const icl::Vec &vector_b, float delta) {
  for (unsigned int i=0; i<4; i++) {
    if (abs(vector_a[i]-vector_b[i]) > delta) {
      return testing::AssertionFailure()
        << "The difference of vector a[" << i << "] and vector b[" << i << "] is "
        << vector_a[i]-vector_b[i] << ", which exceeds delta, where\n"
        << "a=" << vector_a.transp() << "\nb=" << vector_b.transp()
        << "\ndelta=" << delta << ".";
        
    }
  } 
  return testing::AssertionSuccess();
}

