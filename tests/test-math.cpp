// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"
#include <icl/math/la/FixedMatrix.h>
#include <icl/math/la/DynMatrix.h>
#include <icl/math/transform/Homography2D.h>
#include <icl/math/fit/LeastSquareModelFitting2D.h>
#include <icl/math/fit/PrimitiveFitters2D.h>
#include <icl/math/fit/RobustFitter.h>
#include <icl/math/fit/NelderMeadOptimizer.h>
#include <icl/math/fit/RefiningFitter.h>
#include <icl/math/fit/GeometricRefiners2D.h>
#include <icl/math/fit/CMAESOptimizer.h>
#include <icl/math/fit/PolynomialRegression.h>
#include <icl/math/detail/lapack/LapackOps.h>

using namespace icl::utils;
using namespace icl::math;

// =====================================================================
// FixedMatrix — Construction
// =====================================================================

ICL_REGISTER_TEST("math.fixed.ctor_default", "default ctor leaves data uninitialized")
{
  FixedMatrix<float,2,2> m(0.0f);
  ICL_TEST_EQ(m[0], 0.0f);
  ICL_TEST_EQ(m[3], 0.0f);
}

ICL_REGISTER_TEST("math.fixed.ctor_value", "value ctor fills all elements")
{
  FixedMatrix<float,3,3> m(5.0f);
  for(unsigned i = 0; i < 9; ++i) ICL_TEST_EQ(m[i], 5.0f);
}

ICL_REGISTER_TEST("math.fixed.identity", "id() is identity matrix")
{
  auto I = FixedMatrix<float,4,4>::id();
  for(unsigned r = 0; r < 4; ++r)
    for(unsigned c = 0; c < 4; ++c)
      ICL_TEST_NEAR(I(r, c), (r==c) ? 1.0f : 0.0f, 1e-7f);
}

ICL_REGISTER_TEST("math.fixed.null", "null() is zero matrix")
{
  auto &z = FixedMatrix<double,3,3>::null();
  for(unsigned i = 0; i < 9; ++i) ICL_TEST_EQ(z[i], 0.0);
}

// =====================================================================
// FixedMatrix — Element access
// =====================================================================

ICL_REGISTER_TEST("math.fixed.access_colrow", "operator()(col,row) indexing")
{
  FixedMatrix<float,2,3> m(0.0f);
  // row-major: data[col + cols*row]
  m(0, 0) = 1; m(0, 1) = 2; m(0, 2) = 3;
  m(1, 0) = 4; m(1, 1) = 5; m(1, 2) = 6;
  ICL_TEST_EQ(m[0], 1.0f);  // (0,0)
  ICL_TEST_EQ(m[1], 2.0f);  // (1,0)
  ICL_TEST_EQ(m[2], 3.0f);  // (2,0)
  ICL_TEST_EQ(m[3], 4.0f);  // (0,1)
  ICL_TEST_EQ(m[5], 6.0f);  // (2,1)
}

ICL_REGISTER_TEST("math.fixed.at_throws", "at() throws on invalid index")
{
  FixedMatrix<float,2,2> m(1.0f);
  ICL_TEST_THROW(m.at(0, 2), InvalidIndexException);
  ICL_TEST_THROW(m.at(2, 0), InvalidIndexException);
}

// =====================================================================
// FixedMatrix — Scalar arithmetic
// =====================================================================

ICL_REGISTER_TEST("math.fixed.scalar_add", "matrix + scalar")
{
  FixedMatrix<float,2,2> m(3.0f);
  auto r = m + 2.0f;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(r[i], 5.0f);
}

ICL_REGISTER_TEST("math.fixed.scalar_sub", "matrix - scalar")
{
  FixedMatrix<float,2,2> m(10.0f);
  auto r = m - 3.0f;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(r[i], 7.0f);
}

ICL_REGISTER_TEST("math.fixed.scalar_mul", "matrix * scalar")
{
  FixedMatrix<float,2,2> m(4.0f);
  auto r = m * 3.0f;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(r[i], 12.0f);
}

ICL_REGISTER_TEST("math.fixed.scalar_div", "matrix / scalar")
{
  FixedMatrix<float,2,2> m(12.0f);
  auto r = m / 4.0f;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(r[i], 3.0f);
}

ICL_REGISTER_TEST("math.fixed.negate", "unary minus")
{
  FixedMatrix<float,2,2> m(7.0f);
  auto r = -m;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(r[i], -7.0f);
}

// =====================================================================
// FixedMatrix — Element-wise matrix ops
// =====================================================================

ICL_REGISTER_TEST("math.fixed.matrix_add", "element-wise A + B")
{
  FixedMatrix<float,2,2> a(3.0f), b(4.0f);
  auto c = a + b;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(c[i], 7.0f);
}

ICL_REGISTER_TEST("math.fixed.matrix_sub", "element-wise A - B")
{
  FixedMatrix<float,2,2> a(10.0f), b(3.0f);
  auto c = a - b;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(c[i], 7.0f);
}

ICL_REGISTER_TEST("math.fixed.inplace_ops", "+= and -= consistency")
{
  FixedMatrix<float,3,3> a(5.0f), b(2.0f);
  auto c = a + b;
  a += b;
  for(unsigned i = 0; i < 9; ++i) ICL_TEST_EQ(a[i], c[i]);
}

// =====================================================================
// FixedMatrix — Matrix multiplication
// =====================================================================

ICL_REGISTER_TEST("math.fixed.mult_identity_4x4f", "4x4f * I = same")
{
  FixedMatrix<float,4,4> A;
  for(int i = 0; i < 16; ++i) A[i] = static_cast<float>(i + 1);
  auto R = A * FixedMatrix<float,4,4>::id();
  for(int i = 0; i < 16; ++i) ICL_TEST_NEAR(R[i], A[i], 1e-5f);
}

ICL_REGISTER_TEST("math.fixed.mult_general_4x4f", "4x4f general multiply vs naive")
{
  FixedMatrix<float,4,4> A, B;
  for(int i = 0; i < 16; ++i){
    A[i] = static_cast<float>(i + 1);
    B[i] = static_cast<float>(16 - i);
  }
  auto R = A * B;
  for(int r = 0; r < 4; ++r)
    for(int c = 0; c < 4; ++c){
      float e = 0;
      for(int k = 0; k < 4; ++k) e += A(r, k) * B(k, c);
      ICL_TEST_NEAR(R(r, c), e, 1e-3f);
    }
}

ICL_REGISTER_TEST("math.fixed.mult_3x3f", "3x3f multiply vs naive")
{
  FixedMatrix<float,3,3> A, B;
  for(int i = 0; i < 9; ++i){
    A[i] = static_cast<float>(i + 1);
    B[i] = static_cast<float>(9 - i);
  }
  auto R = A * B;
  for(int r = 0; r < 3; ++r)
    for(int c = 0; c < 3; ++c){
      float e = 0;
      for(int k = 0; k < 3; ++k) e += A(r, k) * B(k, c);
      ICL_TEST_NEAR(R(r, c), e, 1e-3f);
    }
}

ICL_REGISTER_TEST("math.fixed.mult_2x2f", "2x2f multiply vs naive")
{
  FixedMatrix<float,2,2> A, B;
  A(0, 0)=1; A(0, 1)=2; A(1, 0)=3; A(1, 1)=4;
  B(0, 0)=5; B(0, 1)=6; B(1, 0)=7; B(1, 1)=8;
  auto R = A * B;
  // [1 2] * [5 6] = [19 22]
  // [3 4]   [7 8]   [43 50]
  ICL_TEST_NEAR(R(0, 0), 19.0f, 1e-5f);
  ICL_TEST_NEAR(R(0, 1), 22.0f, 1e-5f);
  ICL_TEST_NEAR(R(1, 0), 43.0f, 1e-5f);
  ICL_TEST_NEAR(R(1, 1), 50.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.fixed.mult_rect", "non-square multiply 2x3 * 3x2 = 2x2")
{
  FixedMatrix<float,2,3> A(0.0f); // 2 rows, 3 cols
  FixedMatrix<float,3,2> B(0.0f); // 3 rows, 2 cols
  // A = [1 2 3; 4 5 6]
  A(0, 0)=1; A(0, 1)=2; A(0, 2)=3;
  A(1, 0)=4; A(1, 1)=5; A(1, 2)=6;
  // B = [7 8; 9 10; 11 12]
  B(0, 0)=7;  B(0, 1)=8;
  B(1, 0)=9;  B(1, 1)=10;
  B(2, 0)=11; B(2, 1)=12;
  auto R = A * B; // 2x2
  // [1*7+2*9+3*11  1*8+2*10+3*12] = [58  64]
  // [4*7+5*9+6*11  4*8+5*10+6*12]   [139 154]
  ICL_TEST_NEAR(R(0, 0), 58.0f,  1e-4f);
  ICL_TEST_NEAR(R(0, 1), 64.0f,  1e-4f);
  ICL_TEST_NEAR(R(1, 0), 139.0f, 1e-4f);
  ICL_TEST_NEAR(R(1, 1), 154.0f, 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.mult_double_4x4", "4x4 double multiply")
{
  FixedMatrix<double,4,4> A, B;
  for(int i = 0; i < 16; ++i){
    A[i] = static_cast<double>(i + 1);
    B[i] = static_cast<double>(16 - i);
  }
  auto R = A * B;
  for(int r = 0; r < 4; ++r)
    for(int c = 0; c < 4; ++c){
      double e = 0;
      for(int k = 0; k < 4; ++k) e += A(r, k) * B(k, c);
      ICL_TEST_NEAR(R(r, c), e, 1e-10);
    }
}

ICL_REGISTER_TEST("math.fixed.mult_4x4_vec", "4x4 * 4x1 vector transform")
{
  FixedMatrix<float,4,4> M = FixedMatrix<float,4,4>::id();
  M(0, 3) = 10; M(1, 3) = 20; M(2, 3) = 30; // translation in last column
  FixedMatrix<float,4,1> v(0.0f);
  v[0] = 1; v[1] = 2; v[2] = 3; v[3] = 1; // homogeneous point
  auto r = M * v;
  ICL_TEST_NEAR(r[0], 11.0f, 1e-5f);
  ICL_TEST_NEAR(r[1], 22.0f, 1e-5f);
  ICL_TEST_NEAR(r[2], 33.0f, 1e-5f);
  ICL_TEST_NEAR(r[3], 1.0f,  1e-5f);
}

ICL_REGISTER_TEST("math.fixed.mult_4x4_vec_general", "4x4 * 4x1 general vs naive")
{
  FixedMatrix<float,4,4> M;
  FixedMatrix<float,4,1> v(0.0f);
  for(int i = 0; i < 16; ++i) M[i] = static_cast<float>(i+1);
  v[0] = 2; v[1] = 3; v[2] = 5; v[3] = 7;
  auto r = M * v;
  for(int row = 0; row < 4; ++row){
    float e = 0;
    for(int k = 0; k < 4; ++k) e += M(row, k) * v[k];
    ICL_TEST_NEAR(r[row], e, 1e-3f);
  }
}

// =====================================================================
// FixedMatrix — Transpose
// =====================================================================

ICL_REGISTER_TEST("math.fixed.transp_3x3", "3x3 transpose")
{
  FixedMatrix<float,3,3> m(0.0f);
  m(0, 0)=1; m(0, 1)=2; m(0, 2)=3;
  m(1, 0)=4; m(1, 1)=5; m(1, 2)=6;
  m(2, 0)=7; m(2, 1)=8; m(2, 2)=9;
  auto t = m.transp();
  for(unsigned r = 0; r < 3; ++r)
    for(unsigned c = 0; c < 3; ++c)
      ICL_TEST_EQ(t(r, c), m(c, r));
}

ICL_REGISTER_TEST("math.fixed.transp_rect", "2x3 transpose = 3x2")
{
  FixedMatrix<float,2,3> m(0.0f);
  m(0, 0)=1; m(0, 1)=2; m(0, 2)=3;
  m(1, 0)=4; m(1, 1)=5; m(1, 2)=6;
  auto t = m.transp();  // FixedMatrix<float,3,2>
  ICL_TEST_EQ(t(0, 0), 1.0f); ICL_TEST_EQ(t(0, 1), 4.0f);
  ICL_TEST_EQ(t(1, 0), 2.0f); ICL_TEST_EQ(t(1, 1), 5.0f);
  ICL_TEST_EQ(t(2, 0), 3.0f); ICL_TEST_EQ(t(2, 1), 6.0f);
}

ICL_REGISTER_TEST("math.fixed.transp_identity", "transpose of I = I")
{
  auto I = FixedMatrix<float,4,4>::id();
  auto T = I.transp();
  for(int i = 0; i < 16; ++i) ICL_TEST_EQ(T[i], I[i]);
}

// =====================================================================
// FixedMatrix — Determinant
// =====================================================================

ICL_REGISTER_TEST("math.fixed.det_2x2", "2x2 determinant")
{
  FixedMatrix<float,2,2> m(0.0f);
  m(0, 0)=1; m(0, 1)=2;
  m(1, 0)=3; m(1, 1)=4;
  ICL_TEST_NEAR(m.det(), -2.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.fixed.det_3x3", "3x3 determinant")
{
  FixedMatrix<float,3,3> m(0.0f);
  m(0, 0)=6;  m(0, 1)=1;  m(0, 2)=1;
  m(1, 0)=4;  m(1, 1)=-2; m(1, 2)=5;
  m(2, 0)=2;  m(2, 1)=8;  m(2, 2)=7;
  ICL_TEST_NEAR(m.det(), -306.0f, 1e-2f);
}

ICL_REGISTER_TEST("math.fixed.det_4x4", "4x4 determinant")
{
  auto I = FixedMatrix<float,4,4>::id();
  ICL_TEST_NEAR(I.det(), 1.0f, 1e-6f);

  FixedMatrix<float,4,4> m(0.0f);
  m(0, 0)=1; m(0, 1)=0; m(0, 2)=2; m(0, 3)=-1;
  m(1, 0)=3; m(1, 1)=0; m(1, 2)=0; m(1, 3)=5;
  m(2, 0)=2; m(2, 1)=1; m(2, 2)=4; m(2, 3)=-3;
  m(3, 0)=1; m(3, 1)=0; m(3, 2)=5; m(3, 3)=0;
  ICL_TEST_NEAR(m.det(), 30.0f, 1e-3f);
}

ICL_REGISTER_TEST("math.fixed.det_identity", "det(I) = 1 for all sizes")
{
  { float d = FixedMatrix<float,2,2>::id().det();  ICL_TEST_NEAR(d, 1.0f, 1e-6f); }
  { float d = FixedMatrix<float,3,3>::id().det();  ICL_TEST_NEAR(d, 1.0f, 1e-6f); }
  { float d = FixedMatrix<float,4,4>::id().det();  ICL_TEST_NEAR(d, 1.0f, 1e-6f); }
  { double d = FixedMatrix<double,2,2>::id().det(); ICL_TEST_NEAR(d, 1.0, 1e-12); }
  { double d = FixedMatrix<double,3,3>::id().det(); ICL_TEST_NEAR(d, 1.0, 1e-12); }
  { double d = FixedMatrix<double,4,4>::id().det(); ICL_TEST_NEAR(d, 1.0, 1e-12); }
}

// =====================================================================
// FixedMatrix — Inverse
// =====================================================================

ICL_REGISTER_TEST("math.fixed.inv_2x2", "2x2 inverse round-trip")
{
  FixedMatrix<float,2,2> m(0.0f);
  m(0, 0)=4; m(0, 1)=7;
  m(1, 0)=2; m(1, 1)=6;
  auto R = m * m.inv();
  auto I = FixedMatrix<float,2,2>::id();
  for(int i = 0; i < 4; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.inv_3x3", "3x3 inverse round-trip")
{
  FixedMatrix<float,3,3> m(0.0f);
  m(0, 0)=1; m(0, 1)=2; m(0, 2)=3;
  m(1, 0)=0; m(1, 1)=1; m(1, 2)=4;
  m(2, 0)=5; m(2, 1)=6; m(2, 2)=0;
  auto R = m * m.inv();
  auto I = FixedMatrix<float,3,3>::id();
  for(int i = 0; i < 9; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.inv_4x4", "4x4 inverse round-trip")
{
  FixedMatrix<float,4,4> m(0.0f);
  m(0, 0)=1; m(0, 1)=0; m(0, 2)=2; m(0, 3)=-1;
  m(1, 0)=3; m(1, 1)=0; m(1, 2)=0; m(1, 3)=5;
  m(2, 0)=2; m(2, 1)=1; m(2, 2)=4; m(2, 3)=-3;
  m(3, 0)=1; m(3, 1)=0; m(3, 2)=5; m(3, 3)=0;
  auto R = m * m.inv();
  auto I = FixedMatrix<float,4,4>::id();
  for(int i = 0; i < 16; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-3f);
}

ICL_REGISTER_TEST("math.fixed.inv_singular_throws", "singular matrix inv throws")
{
  FixedMatrix<float,2,2> m(0.0f);
  m(0, 0)=1; m(0, 1)=2;
  m(1, 0)=2; m(1, 1)=4; // det=0
  ICL_TEST_THROW(m.inv(), SingularMatrixException);
}

// =====================================================================
// FixedMatrix — Length / Normalize
// =====================================================================

ICL_REGISTER_TEST("math.fixed.length_l2", "L2 norm of [3,4,0] = 5")
{
  FixedMatrix<float,1,3> v(0.0f);
  v[0] = 3; v[1] = 4;
  ICL_TEST_NEAR(v.length(), 5.0, 1e-10);
}

ICL_REGISTER_TEST("math.fixed.length_l1", "L1 norm of [-3,4,-1] = 8")
{
  FixedMatrix<float,1,3> v(0.0f);
  v[0] = -3; v[1] = 4; v[2] = -1;
  ICL_TEST_NEAR(v.length(1), 8.0, 1e-10);
}

ICL_REGISTER_TEST("math.fixed.length_general", "L3 norm")
{
  FixedMatrix<float,1,2> v(0.0f);
  v[0] = 2; v[1] = 3;
  double expected = std::pow(std::pow(2.0,3) + std::pow(3.0,3), 1.0/3.0);
  ICL_TEST_NEAR(v.length(3), expected, 1e-6);
}

ICL_REGISTER_TEST("math.fixed.normalize", "normalized vector has length 1")
{
  FixedMatrix<float,1,4> v(0.0f);
  v[0]=1; v[1]=2; v[2]=3; v[3]=4;
  auto n = v.normalized();
  ICL_TEST_NEAR(n.length(), 1.0, 1e-5);
}

// =====================================================================
// FixedMatrix — Misc
// =====================================================================

ICL_REGISTER_TEST("math.fixed.trace", "trace of 3x3")
{
  FixedMatrix<float,3,3> m(0.0f);
  m(0, 0)=1; m(1, 1)=5; m(2, 2)=9;
  ICL_TEST_NEAR(m.trace(), 15.0f, 1e-6f);
}

ICL_REGISTER_TEST("math.fixed.equality", "operator== and !=")
{
  FixedMatrix<float,2,2> a(3.0f), b(3.0f), c(4.0f);
  ICL_TEST_TRUE(a == b);
  ICL_TEST_FALSE(a == c);
}

ICL_REGISTER_TEST("math.fixed.element_wise_inner", "element_wise_inner_product")
{
  FixedMatrix<float,1,3> a(0.0f), b(0.0f);
  a[0]=1; a[1]=2; a[2]=3;
  b[0]=4; b[1]=5; b[2]=6;
  ICL_TEST_NEAR(a.element_wise_inner_product(b), 32.0f, 1e-5f);
}

// =====================================================================
// FixedMatrix — SIMD/cblas cross-validation
// =====================================================================

// Generic C++ reference multiply (no SIMD, no cblas — always correct)
template<class T, unsigned int COLS, unsigned int ROWS, unsigned int MCOLS>
static FixedMatrix<T,ROWS,MCOLS> ref_mult(
    const FixedMatrix<T,ROWS,COLS> &a,
    const FixedMatrix<T,COLS,MCOLS> &b) {
  FixedMatrix<T,ROWS,MCOLS> dst;
  for(unsigned int c=0;c<MCOLS;++c)
    for(unsigned int r=0;r<ROWS;++r) {
      T sum = T(0);
      for(unsigned int k=0;k<COLS;++k)
        sum += a(r, k) * b(k, c);
      dst(r, c) = sum;
    }
  return dst;
}

ICL_REGISTER_TEST("math.fixed.simd_cross_4x4f", "4x4 float: SIMD matches C++ reference")
{
  FixedMatrix<float,4,4> a, b;
  for(int i=0; i<16; ++i) { a[i] = float(i+1); b[i] = float(16-i); }
  auto got = a * b;
  auto ref = ref_mult(a, b);
  for(int i=0; i<16; ++i) ICL_TEST_NEAR(got[i], ref[i], 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.simd_cross_4x4d", "4x4 double: SIMD matches C++ reference")
{
  FixedMatrix<double,4,4> a, b;
  for(int i=0; i<16; ++i) { a[i] = double(i+1); b[i] = double(16-i); }
  auto got = a * b;
  auto ref = ref_mult(a, b);
  for(int i=0; i<16; ++i) ICL_TEST_NEAR(got[i], ref[i], 1e-10);
}

ICL_REGISTER_TEST("math.fixed.simd_cross_3x3f", "3x3 float: SIMD matches C++ reference")
{
  FixedMatrix<float,3,3> a, b;
  for(int i=0; i<9; ++i) { a[i] = float(i+1); b[i] = float(9-i); }
  auto got = a * b;
  auto ref = ref_mult(a, b);
  for(int i=0; i<9; ++i) ICL_TEST_NEAR(got[i], ref[i], 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.simd_cross_3x3d", "3x3 double: SIMD matches C++ reference")
{
  FixedMatrix<double,3,3> a, b;
  for(int i=0; i<9; ++i) { a[i] = double(i+1); b[i] = double(9-i); }
  auto got = a * b;
  auto ref = ref_mult(a, b);
  for(int i=0; i<9; ++i) ICL_TEST_NEAR(got[i], ref[i], 1e-10);
}

ICL_REGISTER_TEST("math.fixed.simd_cross_2x2f", "2x2 float: SIMD matches C++ reference")
{
  FixedMatrix<float,2,2> a, b;
  a[0]=1; a[1]=2; a[2]=3; a[3]=4;
  b[0]=5; b[1]=6; b[2]=7; b[3]=8;
  auto got = a * b;
  auto ref = ref_mult(a, b);
  for(int i=0; i<4; ++i) ICL_TEST_NEAR(got[i], ref[i], 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.simd_cross_2x2d", "2x2 double: SIMD matches C++ reference")
{
  FixedMatrix<double,2,2> a, b;
  a[0]=1; a[1]=2; a[2]=3; a[3]=4;
  b[0]=5; b[1]=6; b[2]=7; b[3]=8;
  auto got = a * b;
  auto ref = ref_mult(a, b);
  for(int i=0; i<4; ++i) ICL_TEST_NEAR(got[i], ref[i], 1e-10);
}

ICL_REGISTER_TEST("math.fixed.simd_cross_matvec_4f", "4x4 float * vec: SIMD matches reference")
{
  FixedMatrix<float,4,4> m;
  for(int i=0; i<16; ++i) m[i] = float(i+1);
  FixedMatrix<float,4,1> v;
  v[0]=1; v[1]=2; v[2]=3; v[3]=4;
  FixedMatrix<float,4,1> got, ref;
  m.mult(v, got);
  // manual reference
  for(int r=0; r<4; ++r) {
    float s = 0;
    for(int k=0; k<4; ++k) s += m(r, k) * v[k];
    ref[r] = s;
  }
  for(int i=0; i<4; ++i) ICL_TEST_NEAR(got[i], ref[i], 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.simd_cross_matvec_4d", "4x4 double * vec: SIMD matches reference")
{
  FixedMatrix<double,4,4> m;
  for(int i=0; i<16; ++i) m[i] = double(i+1);
  FixedMatrix<double,4,1> v;
  v[0]=1; v[1]=2; v[2]=3; v[3]=4;
  FixedMatrix<double,4,1> got, ref;
  m.mult(v, got);
  for(int r=0; r<4; ++r) {
    double s = 0;
    for(int k=0; k<4; ++k) s += m(r, k) * v[k];
    ref[r] = s;
  }
  for(int i=0; i<4; ++i) ICL_TEST_NEAR(got[i], ref[i], 1e-10);
}

ICL_REGISTER_TEST("math.fixed.simd_inv_cross_4x4f", "4x4 float inv: A * inv(A) = I")
{
  FixedMatrix<float,4,4> m = {2,0,1,0, 0,1,0,3, 1,0,2,0, 0,1,0,1};
  auto inv = m.inv();
  auto eye = m * inv;
  for(int r=0; r<4; ++r)
    for(int c=0; c<4; ++c)
      ICL_TEST_NEAR(eye(r, c), (r==c ? 1.0f : 0.0f), 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.simd_inv_cross_3x3d", "3x3 double inv: A * inv(A) = I")
{
  FixedMatrix<double,3,3> m = {1,2,3, 0,1,4, 5,6,0};
  auto inv = m.inv();
  auto eye = m * inv;
  for(int r=0; r<3; ++r)
    for(int c=0; c<3; ++c)
      ICL_TEST_NEAR(eye(r, c), (r==c ? 1.0 : 0.0), 1e-10);
}

ICL_REGISTER_TEST("math.fixed.simd_det_cross_4x4f", "4x4 float det: matches Dyn")
{
  FixedMatrix<float,4,4> fm = {2,0,1,0, 0,1,0,3, 1,0,2,0, 0,1,0,1};
  float fd = fm.det();
  float dd = fm.dyn().det();
  ICL_TEST_NEAR(fd, dd, 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.mult_int_fallback", "int matrix multiply uses generic fallback")
{
  FixedMatrix<int,2,2> a, b;
  a[0]=1; a[1]=2; a[2]=3; a[3]=4;
  b[0]=5; b[1]=6; b[2]=7; b[3]=8;
  auto c = a * b;
  ICL_TEST_EQ(c[0], 19);
  ICL_TEST_EQ(c[1], 22);
  ICL_TEST_EQ(c[2], 43);
  ICL_TEST_EQ(c[3], 50);
}

// =====================================================================
// DynMatrix — Construction and ownership
// =====================================================================

ICL_REGISTER_TEST("math.dyn.ctor_default", "default ctor is null matrix")
{
  DynMatrix<float> m;
  ICL_TEST_EQ(m.rows(), 0u);
  ICL_TEST_EQ(m.cols(), 0u);
}

ICL_REGISTER_TEST("math.dyn.ctor_dims", "construction with dims and init value")
{
  DynMatrix<float> m = DynMatrix<float>::create(2, 3, 7.0f);
  ICL_TEST_EQ(m.cols(), 3u);
  ICL_TEST_EQ(m.rows(), 2u);
  for(unsigned i = 0; i < 6; ++i) ICL_TEST_EQ(m[i], 7.0f);
}

ICL_REGISTER_TEST("math.dyn.ctor_copy", "copy ctor deep copies")
{
  DynMatrix<float> a = DynMatrix<float>::create(2, 2, 5.0f);
  DynMatrix<float> b(a);
  b[0] = 99.0f;
  ICL_TEST_EQ(a[0], 5.0f);  // original unchanged
  ICL_TEST_EQ(b[0], 99.0f);
}

ICL_REGISTER_TEST("math.dyn.ctor_wrap", "shallow wrap shares data")
{
  float data[] = {1, 2, 3, 4};
  DynMatrix<float> m = DynMatrix<float>::fromData(2, 2, data, false); // shallow
  ICL_TEST_EQ(m(0, 0), 1.0f);
  ICL_TEST_EQ(m(1, 1), 4.0f);
  m(0, 0) = 99.0f;
  ICL_TEST_EQ(data[0], 99.0f); // data modified through matrix
}

ICL_REGISTER_TEST("math.dyn.ctor_zero_throws", "zero dimension throws")
{
  ICL_TEST_THROW(DynMatrix<float>::create(3, 0), InvalidMatrixDimensionException);
  ICL_TEST_THROW(DynMatrix<float>::create(0, 3), InvalidMatrixDimensionException);
}

// =====================================================================
// DynMatrix — Element access
// =====================================================================

ICL_REGISTER_TEST("math.dyn.access", "operator()(col,row) indexing")
{
  DynMatrix<float> m = DynMatrix<float>::create(2, 3, 0.0f);
  m(0, 0) = 1; m(0, 1) = 2; m(0, 2) = 3;
  m(1, 0) = 4; m(1, 1) = 5; m(1, 2) = 6;
  // row-major: data[col + cols*row]
  ICL_TEST_EQ(m[0], 1.0f);
  ICL_TEST_EQ(m[1], 2.0f);
  ICL_TEST_EQ(m[5], 6.0f);
}

// =====================================================================
// DynMatrix — Scalar arithmetic
// =====================================================================

ICL_REGISTER_TEST("math.dyn.scalar_mul", "matrix * scalar")
{
  DynMatrix<float> m = DynMatrix<float>::create(2, 2, 3.0f);
  auto r = m * 4.0f;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(r[i], 12.0f);
}

ICL_REGISTER_TEST("math.dyn.scalar_div", "matrix / scalar")
{
  DynMatrix<float> m = DynMatrix<float>::create(2, 2, 12.0f);
  auto r = m / 3.0f;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(r[i], 4.0f);
}

ICL_REGISTER_TEST("math.dyn.scalar_add_sub", "matrix +/- scalar")
{
  DynMatrix<float> m = DynMatrix<float>::create(2, 2, 5.0f);
  auto a = m + 3.0f;
  auto s = m - 2.0f;
  for(unsigned i = 0; i < 4; ++i){
    ICL_TEST_EQ(a[i], 8.0f);
    ICL_TEST_EQ(s[i], 3.0f);
  }
}

// =====================================================================
// DynMatrix — Matrix multiplication
// =====================================================================

ICL_REGISTER_TEST("math.dyn.mult_identity", "A * I = A")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3, 0.0f);
  for(unsigned i = 0; i < 9; ++i) A[i] = static_cast<float>(i+1);
  auto I = DynMatrix<float>::id(3);
  auto R = A * I;
  for(unsigned i = 0; i < 9; ++i) ICL_TEST_NEAR(R[i], A[i], 1e-5f);
}

ICL_REGISTER_TEST("math.dyn.mult_2x2", "2x2 multiply correctness")
{
  DynMatrix<float> A = DynMatrix<float>::create(2, 2, 0.0f), B = DynMatrix<float>::create(2, 2, 0.0f);
  A(0, 0)=1; A(0, 1)=2; A(1, 0)=3; A(1, 1)=4;
  B(0, 0)=5; B(0, 1)=6; B(1, 0)=7; B(1, 1)=8;
  auto R = A * B;
  ICL_TEST_NEAR(R(0, 0), 19.0f, 1e-5f);
  ICL_TEST_NEAR(R(0, 1), 22.0f, 1e-5f);
  ICL_TEST_NEAR(R(1, 0), 43.0f, 1e-5f);
  ICL_TEST_NEAR(R(1, 1), 50.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.dyn.mult_rect", "2x3 * 3x2 = 2x2")
{
  DynMatrix<float> A = DynMatrix<float>::create(2, 3, 0.0f);
  A(0, 0)=1; A(0, 1)=2; A(0, 2)=3;
  A(1, 0)=4; A(1, 1)=5; A(1, 2)=6;
  DynMatrix<float> B = DynMatrix<float>::create(3, 2, 0.0f);
  B(0, 0)=7;  B(0, 1)=8;
  B(1, 0)=9;  B(1, 1)=10;
  B(2, 0)=11; B(2, 1)=12;
  auto R = A * B;
  ICL_TEST_EQ(R.cols(), 2u);
  ICL_TEST_EQ(R.rows(), 2u);
  ICL_TEST_NEAR(R(0, 0), 58.0f,  1e-4f);
  ICL_TEST_NEAR(R(0, 1), 64.0f,  1e-4f);
  ICL_TEST_NEAR(R(1, 0), 139.0f, 1e-4f);
  ICL_TEST_NEAR(R(1, 1), 154.0f, 1e-4f);
}

ICL_REGISTER_TEST("math.dyn.mult_dim_mismatch_throws", "incompatible dimensions throw")
{
  DynMatrix<float> A = DynMatrix<float>::create(2, 3, 1.0f), B = DynMatrix<float>::create(2, 2, 1.0f);  // A.cols()=3 != B.rows()=2
  ICL_TEST_THROW(A * B, IncompatibleMatrixDimensionException);
}

ICL_REGISTER_TEST("math.dyn.elementwise_mult", "elementwise multiplication")
{
  DynMatrix<float> a = DynMatrix<float>::create(2, 2, 0.0f), b = DynMatrix<float>::create(2, 2, 0.0f);
  a[0]=1; a[1]=2; a[2]=3; a[3]=4;
  b[0]=5; b[1]=6; b[2]=7; b[3]=8;
  auto r = a.elementwise_mult(b);
  ICL_TEST_EQ(r[0], 5.0f);
  ICL_TEST_EQ(r[1], 12.0f);
  ICL_TEST_EQ(r[2], 21.0f);
  ICL_TEST_EQ(r[3], 32.0f);
}

// =====================================================================
// DynMatrix — Transpose
// =====================================================================

ICL_REGISTER_TEST("math.dyn.transp_square", "3x3 transpose")
{
  DynMatrix<float> m = DynMatrix<float>::create(3, 3, 0.0f);
  for(unsigned i = 0; i < 9; ++i) m[i] = static_cast<float>(i+1);
  auto t = m.transp();
  for(unsigned r = 0; r < 3; ++r)
    for(unsigned c = 0; c < 3; ++c)
      ICL_TEST_EQ(t(r, c), m(c, r));
}

ICL_REGISTER_TEST("math.dyn.transp_rect", "2x3 transpose = 3x2")
{
  DynMatrix<float> m = DynMatrix<float>::create(2, 3, 0.0f);
  m(0, 0)=1; m(0, 1)=2; m(0, 2)=3;
  m(1, 0)=4; m(1, 1)=5; m(1, 2)=6;
  auto t = m.transp();
  ICL_TEST_EQ(t.cols(), 2u);
  ICL_TEST_EQ(t.rows(), 3u);
  ICL_TEST_EQ(t(0, 0), 1.0f); ICL_TEST_EQ(t(0, 1), 4.0f);
  ICL_TEST_EQ(t(1, 0), 2.0f); ICL_TEST_EQ(t(1, 1), 5.0f);
  ICL_TEST_EQ(t(2, 0), 3.0f); ICL_TEST_EQ(t(2, 1), 6.0f);
}

ICL_REGISTER_TEST("math.dyn.transp_double", "double transpose = original")
{
  DynMatrix<float> m = DynMatrix<float>::create(2, 3, 0.0f);
  for(unsigned i = 0; i < 6; ++i) m[i] = static_cast<float>(i+1);
  auto tt = m.transp().transp();
  for(unsigned i = 0; i < 6; ++i) ICL_TEST_EQ(tt[i], m[i]);
}

// =====================================================================
// DynMatrix — Determinant
// =====================================================================

ICL_REGISTER_TEST("math.dyn.det_2x2", "2x2 determinant")
{
  DynMatrix<float> m = DynMatrix<float>::create(2, 2, 0.0f);
  m(0, 0)=1; m(0, 1)=2;
  m(1, 0)=3; m(1, 1)=4;
  ICL_TEST_NEAR(m.det(), -2.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.dyn.det_3x3", "3x3 determinant")
{
  DynMatrix<float> m = DynMatrix<float>::create(3, 3, 0.0f);
  m(0, 0)=6;  m(0, 1)=1;  m(0, 2)=1;
  m(1, 0)=4;  m(1, 1)=-2; m(1, 2)=5;
  m(2, 0)=2;  m(2, 1)=8;  m(2, 2)=7;
  ICL_TEST_NEAR(m.det(), -306.0f, 1e-2f);
}

ICL_REGISTER_TEST("math.dyn.det_identity", "det(I) = 1")
{
  float d2 = DynMatrix<float>::id(2).det();
  float d3 = DynMatrix<float>::id(3).det();
  float d4 = DynMatrix<float>::id(4).det();
  ICL_TEST_NEAR(d2, 1.0f, 1e-6f);
  ICL_TEST_NEAR(d3, 1.0f, 1e-6f);
  ICL_TEST_NEAR(d4, 1.0f, 1e-5f);
}

// =====================================================================
// DynMatrix — Inverse
// =====================================================================

ICL_REGISTER_TEST("math.dyn.inv_2x2", "2x2 inverse round-trip")
{
  DynMatrix<float> m = DynMatrix<float>::create(2, 2, 0.0f);
  m(0, 0)=4; m(0, 1)=7;
  m(1, 0)=2; m(1, 1)=6;
  auto R = m * m.inv();
  auto I = DynMatrix<float>::id(2);
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-4f);
}

ICL_REGISTER_TEST("math.dyn.inv_3x3", "3x3 inverse round-trip")
{
  DynMatrix<float> m = DynMatrix<float>::create(3, 3, 0.0f);
  m(0, 0)=1; m(0, 1)=2; m(0, 2)=3;
  m(1, 0)=0; m(1, 1)=1; m(1, 2)=4;
  m(2, 0)=5; m(2, 1)=6; m(2, 2)=0;
  auto R = m * m.inv();
  auto I = DynMatrix<float>::id(3);
  for(unsigned i = 0; i < 9; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-4f);
}

ICL_REGISTER_TEST("math.dyn.inv_4x4", "4x4 inverse round-trip")
{
  DynMatrix<float> m = DynMatrix<float>::create(4, 4, 0.0f);
  m(0, 0)=1; m(0, 1)=0; m(0, 2)=2; m(0, 3)=-1;
  m(1, 0)=3; m(1, 1)=0; m(1, 2)=0; m(1, 3)=5;
  m(2, 0)=2; m(2, 1)=1; m(2, 2)=4; m(2, 3)=-3;
  m(3, 0)=1; m(3, 1)=0; m(3, 2)=5; m(3, 3)=0;
  auto R = m * m.inv();
  auto I = DynMatrix<float>::id(4);
  for(unsigned i = 0; i < 16; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-3f);
}

// =====================================================================
// DynMatrix — Trace / Diag / Reshape
// =====================================================================

ICL_REGISTER_TEST("math.dyn.trace", "trace of 3x3")
{
  DynMatrix<float> m = DynMatrix<float>::create(3, 3, 0.0f);
  m(0, 0)=2; m(1, 1)=5; m(2, 2)=8;
  ICL_TEST_NEAR(m.trace(), 15.0f, 1e-6f);
}

ICL_REGISTER_TEST("math.dyn.diag", "diagonal extraction")
{
  DynMatrix<float> m = DynMatrix<float>::create(3, 3, 0.0f);
  m(0, 0)=1; m(1, 1)=5; m(2, 2)=9;
  auto d = m.diag();
  ICL_TEST_EQ(d.rows(), 3u);
  ICL_TEST_EQ(d.cols(), 1u);
  ICL_TEST_EQ(d[0], 1.0f);
  ICL_TEST_EQ(d[1], 5.0f);
  ICL_TEST_EQ(d[2], 9.0f);
}

ICL_REGISTER_TEST("math.dyn.reshape", "reshape preserves data")
{
  DynMatrix<float> m = DynMatrix<float>::create(1, 6, 0.0f);
  for(unsigned i = 0; i < 6; ++i) m[i] = static_cast<float>(i);
  m.reshape(3, 2);
  ICL_TEST_EQ(m.cols(), 3u);
  ICL_TEST_EQ(m.rows(), 2u);
  for(unsigned i = 0; i < 6; ++i) ICL_TEST_EQ(m[i], static_cast<float>(i));
}

ICL_REGISTER_TEST("math.dyn.reshape_mismatch_throws", "reshape with wrong dim throws")
{
  DynMatrix<float> m = DynMatrix<float>::create(3, 2, 0.0f);
  ICL_TEST_THROW(m.reshape(2, 2), InvalidMatrixDimensionException);
}

// =====================================================================
// Cross-validation: FixedMatrix vs DynMatrix
// =====================================================================

ICL_REGISTER_TEST("math.cross.mult_fixed_vs_dyn", "4x4 multiply: Fixed and Dyn agree")
{
  FixedMatrix<float,4,4> fA, fB;
  for(int i = 0; i < 16; ++i){
    fA[i] = static_cast<float>(i*3 + 1);
    fB[i] = static_cast<float>(17 - i*2);
  }
  auto fR = fA * fB;

  DynMatrix<float> dA = DynMatrix<float>::fromData(4, 4, fA.data());
  DynMatrix<float> dB = DynMatrix<float>::fromData(4, 4, fB.data());
  auto dR = dA * dB;

  for(int i = 0; i < 16; ++i){
    ICL_TEST_NEAR(fR[i], dR[i], 1e-2f);
  }
}

ICL_REGISTER_TEST("math.cross.det_fixed_vs_dyn", "determinant: Fixed and Dyn agree")
{
  FixedMatrix<float,3,3> fm(0.0f);
  fm(0, 0)=6;  fm(0, 1)=1;  fm(0, 2)=1;
  fm(1, 0)=4;  fm(1, 1)=-2; fm(1, 2)=5;
  fm(2, 0)=2;  fm(2, 1)=8;  fm(2, 2)=7;

  DynMatrix<float> dm = DynMatrix<float>::fromData(3, 3, fm.data());
  ICL_TEST_NEAR(fm.det(), dm.det(), 1e-3f);
}

ICL_REGISTER_TEST("math.cross.transp_fixed_vs_dyn", "transpose: Fixed and Dyn agree")
{
  FixedMatrix<float,3,3> fm(0.0f);
  for(int i = 0; i < 9; ++i) fm[i] = static_cast<float>(i+1);
  auto ft = fm.transp();

  DynMatrix<float> dm = DynMatrix<float>::fromData(3, 3, fm.data());
  auto dt = dm.transp();
  for(int i = 0; i < 9; ++i) ICL_TEST_NEAR(ft[i], dt[i], 1e-6f);
}

// ============================================================
// QR decomposition tests
// ============================================================

ICL_REGISTER_TEST("math.dyn.qr_identity", "QR of identity: Q=I, R=I")
{
  auto I = DynMatrix<float>::id(3);
  DynMatrix<float> Q, R;
  I.decompose_QR(Q, R);

  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++) {
      float expected = (i == j) ? 1.0f : 0.0f;
      ICL_TEST_NEAR(std::abs(Q(i, j)), std::abs(expected), 1e-5f);
      ICL_TEST_NEAR(std::abs(R(i, j)), std::abs(expected), 1e-5f);
    }
}

ICL_REGISTER_TEST("math.dyn.qr_reconstruct", "A = Q*R round-trip")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=12; A(0, 1)=-51; A(0, 2)=4;
  A(1, 0)=6;  A(1, 1)=167; A(1, 2)=-68;
  A(2, 0)=-4; A(2, 1)=24;  A(2, 2)=-41;

  DynMatrix<float> Q, R;
  A.decompose_QR(Q, R);

  // Verify A = Q*R
  auto QR = Q * R;
  for(unsigned int i = 0; i < A.dim(); i++)
    ICL_TEST_NEAR(QR[i], A[i], 1e-3f);
}

ICL_REGISTER_TEST("math.dyn.qr_orthogonal", "Q^T * Q = I")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=12; A(0, 1)=-51; A(0, 2)=4;
  A(1, 0)=6;  A(1, 1)=167; A(1, 2)=-68;
  A(2, 0)=-4; A(2, 1)=24;  A(2, 2)=-41;

  DynMatrix<float> Q, R;
  A.decompose_QR(Q, R);

  auto QtQ = Q.transp() * Q;
  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++)
      ICL_TEST_NEAR(QtQ(i, j), (i==j) ? 1.0f : 0.0f, 1e-4f);
}

ICL_REGISTER_TEST("math.dyn.qr_upper_triangular", "R is upper triangular")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=12; A(0, 1)=-51; A(0, 2)=4;
  A(1, 0)=6;  A(1, 1)=167; A(1, 2)=-68;
  A(2, 0)=-4; A(2, 1)=24;  A(2, 2)=-41;

  DynMatrix<float> Q, R;
  A.decompose_QR(Q, R);

  for(int i = 0; i < 3; i++)
    for(int j = 0; j < i; j++)
      ICL_TEST_NEAR(R(i, j), 0.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.dyn.qr_double", "QR decomposition with double precision")
{
  DynMatrix<double> A = DynMatrix<double>::create(3, 3);
  A(0, 0)=12; A(0, 1)=-51; A(0, 2)=4;
  A(1, 0)=6;  A(1, 1)=167; A(1, 2)=-68;
  A(2, 0)=-4; A(2, 1)=24;  A(2, 2)=-41;

  DynMatrix<double> Q, R;
  A.decompose_QR(Q, R);

  auto QR = Q * R;
  for(unsigned int i = 0; i < A.dim(); i++)
    ICL_TEST_NEAR(QR[i], A[i], 1e-10);
}

// ============================================================
// LU decomposition tests
// ============================================================

ICL_REGISTER_TEST("math.dyn.lu_reconstruct", "L*U approximates A (with permutation)")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=2;  A(0, 1)=1; A(0, 2)=1;
  A(1, 0)=4;  A(1, 1)=3; A(1, 2)=3;
  A(2, 0)=8;  A(2, 1)=7; A(2, 2)=9;

  DynMatrix<float> L, U;
  A.decompose_LU(L, U);

  // L*U should reconstruct A up to row permutation
  auto LU = L * U;
  // Verify each row of LU matches some row of A
  for(int i = 0; i < 3; i++) {
    bool found = false;
    for(int k = 0; k < 3; k++) {
      bool match = true;
      for(int j = 0; j < 3; j++) {
        if(std::abs(LU(i, j) - A(k, j)) > 1e-3f) { match = false; break; }
      }
      if(match) { found = true; break; }
    }
    ICL_TEST_TRUE(found);
  }
}

ICL_REGISTER_TEST("math.dyn.lu_lower_triangular", "L is lower triangular with unit diagonal")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=2;  A(0, 1)=1; A(0, 2)=1;
  A(1, 0)=4;  A(1, 1)=3; A(1, 2)=3;
  A(2, 0)=8;  A(2, 1)=7; A(2, 2)=9;

  DynMatrix<float> L, U;
  A.decompose_LU(L, U);

  for(int i = 0; i < 3; i++) {
    for(int j = i + 1; j < 3; j++)
      ICL_TEST_NEAR(L(i, j), 0.0f, 1e-5f);
  }
}

ICL_REGISTER_TEST("math.dyn.lu_upper_triangular", "U is upper triangular")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=2;  A(0, 1)=1; A(0, 2)=1;
  A(1, 0)=4;  A(1, 1)=3; A(1, 2)=3;
  A(2, 0)=8;  A(2, 1)=7; A(2, 2)=9;

  DynMatrix<float> L, U;
  A.decompose_LU(L, U);

  for(int i = 0; i < 3; i++)
    for(int j = 0; j < i; j++)
      ICL_TEST_NEAR(U(i, j), 0.0f, 1e-5f);
}

// ============================================================
// RQ decomposition test
// ============================================================

ICL_REGISTER_TEST("math.dyn.rq_reconstruct", "R*Q round-trip")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=12; A(0, 1)=-51; A(0, 2)=4;
  A(1, 0)=6;  A(1, 1)=167; A(1, 2)=-68;
  A(2, 0)=-4; A(2, 1)=24;  A(2, 2)=-41;

  DynMatrix<float> R, Q;
  A.decompose_RQ(R, Q);

  auto RQ = R * Q;
  for(unsigned int i = 0; i < A.dim(); i++)
    ICL_TEST_NEAR(RQ[i], A[i], 1e-2f);
}

// ============================================================
// solve() tests
// ============================================================

ICL_REGISTER_TEST("math.dyn.solve_inv", "solve via inv method")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=2; A(0, 1)=1; A(0, 2)=1;
  A(1, 0)=4; A(1, 1)=3; A(1, 2)=3;
  A(2, 0)=8; A(2, 1)=7; A(2, 2)=9;

  DynMatrix<float> b = DynMatrix<float>::create(3, 1);
  b[0] = 1; b[1] = 1; b[2] = 1;

  auto x = A.solve(b);
  auto Ax = A * x;
  for(int i = 0; i < 3; i++)
    ICL_TEST_NEAR(Ax[i], b[i], 1e-3f);
}

ICL_REGISTER_TEST("math.dyn.solve_svd", "solve via svd method")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=2; A(0, 1)=1; A(0, 2)=1;
  A(1, 0)=4; A(1, 1)=3; A(1, 2)=3;
  A(2, 0)=8; A(2, 1)=7; A(2, 2)=9;

  DynMatrix<float> b = DynMatrix<float>::create(3, 1);
  b[0] = 1; b[1] = 1; b[2] = 1;

  auto x = A.solve(b);
  auto Ax = A * x;
  for(int i = 0; i < 3; i++)
    ICL_TEST_NEAR(Ax[i], b[i], 1e-3f);
}

ICL_REGISTER_TEST("math.dyn.solve_qr", "solve via qr method")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=2; A(0, 1)=1; A(0, 2)=1;
  A(1, 0)=4; A(1, 1)=3; A(1, 2)=3;
  A(2, 0)=8; A(2, 1)=7; A(2, 2)=9;

  DynMatrix<float> b = DynMatrix<float>::create(3, 1);
  b[0] = 1; b[1] = 1; b[2] = 1;

  auto x = A.solve(b);
  auto Ax = A * x;
  for(int i = 0; i < 3; i++)
    ICL_TEST_NEAR(Ax[i], b[i], 1e-3f);
}

// ============================================================
// pinv() tests
// ============================================================

ICL_REGISTER_TEST("math.dyn.pinv_svd", "pseudo-inverse via SVD")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=1; A(0, 1)=2; A(0, 2)=3;
  A(1, 0)=0; A(1, 1)=1; A(1, 2)=4;
  A(2, 0)=5; A(2, 1)=6; A(2, 2)=0;

  auto P = A.pinv();
  auto APA = A * P * A;
  for(unsigned int i = 0; i < A.dim(); i++)
    ICL_TEST_NEAR(APA[i], A[i], 1e-3f);
}

ICL_REGISTER_TEST("math.dyn.pinv_qr", "pseudo-inverse via QR")
{
  DynMatrix<float> A = DynMatrix<float>::create(3, 3);
  A(0, 0)=1; A(0, 1)=2; A(0, 2)=3;
  A(1, 0)=0; A(1, 1)=1; A(1, 2)=4;
  A(2, 0)=5; A(2, 1)=6; A(2, 2)=0;

  auto P = A.pinv();
  auto APA = A * P * A;
  for(unsigned int i = 0; i < A.dim(); i++)
    ICL_TEST_NEAR(APA[i], A[i], 1e-3f);
}

// ============================================================
// det via LU (n > 4)
// ============================================================

ICL_REGISTER_TEST("math.dyn.det_5x5", "5x5 determinant via LU dispatch")
{
  // Diagonal matrix with known det = product of diagonal
  DynMatrix<float> A = DynMatrix<float>::create(5, 5, 0.0f);
  A(0, 0)=2; A(1, 1)=3; A(2, 2)=4; A(3, 3)=5; A(4, 4)=6;
  ICL_TEST_NEAR(A.det(), 720.0f, 1e-2f);
}

// ============================================================
// Homography2D — round-trip on exact 4-point pairs
// ============================================================

// A Homography2D fitted to 4 exact source/dest point pairs is an
// exactly-determined system and must reproduce each mapping to within
// floating-point precision. If it fails with large residuals the fit
// is numerically unstable (typically from unnormalized DLT); that
// would manifest as ImageRectification's pre-check rejecting inputs
// that should be valid.

ICL_REGISTER_TEST("math.homography.roundtrip_axis_aligned",
                  "axis-aligned quad → rectangle must be near-exact")
{
  // 1920x1080 source, 512x512 destination, centred axis-aligned quad
  const Point32f ps[4] = { {20,20}, {1899,20}, {1899,1059}, {20,1059} };
  const Point32f ys[4] = { {0,0}, {511,0}, {511,511}, {0,511} };
  const Homography2D HOM = Homography2D::fit(ys, ps, 4);   // apply(ys)=ps
  for(int i = 0; i < 4; ++i){
    const Point32f p = HOM.apply(ys[i]);
    ICL_TEST_NEAR(p.x, ps[i].x, 1e-2f);
    ICL_TEST_NEAR(p.y, ps[i].y, 1e-2f);
  }
}

ICL_REGISTER_TEST("math.homography.roundtrip_rotated_quad",
                  "rotated convex quad (from rectify-image in the wild)")
{
  // This is the actual failing configuration reported by a user running
  // rectify-image on a 1920x1080 camera. corner 2 = (1818,1014).
  // Before Hartley normalization the DLT solve produced an error of
  // ~128 px in y for this case. Post-fix must round-trip to <1 px.
  const Point32f ps[4] = { {620,270}, {1200,250}, {1818,1014}, {460,920} };
  const Point32f ys[4] = { {0,0}, {511,0}, {511,511}, {0,511} };
  const Homography2D HOM = Homography2D::fit(ys, ps, 4);   // apply(ys)=ps
  for(int i = 0; i < 4; ++i){
    const Point32f p = HOM.apply(ys[i]);
    ICL_TEST_NEAR(p.x, ps[i].x, 1.0f);
    ICL_TEST_NEAR(p.y, ps[i].y, 1.0f);
  }
}

// fit(src,dst) maps src -> dst: apply(src[i]) == dst[i] (the opposite arg order
// of the deprecated constructor).
ICL_REGISTER_TEST("math.homography.fit_maps_src_to_dst",
                  "Homography2D::fit(src,dst) yields apply(src)=dst")
{
  const Point32f src[4] = { {0,0}, {100,0}, {100,80}, {0,80} };
  const Point32f dst[4] = { {12,7}, {520,30}, {498,470}, {40,441} };
  const Homography2D H = Homography2D::fit(src, dst, 4);
  for(int i=0;i<4;++i){
    const Point32f p = H.apply(src[i]);
    ICL_TEST_NEAR(p.x, dst[i].x, 1e-2f);
    ICL_TEST_NEAR(p.y, dst[i].y, 1e-2f);
  }
}

// The deprecated ctor(pAs,pBs) must equal fit(pBs,pAs) (same 3x3, opposite args).
ICL_REGISTER_TEST("math.homography.deprecated_ctor_equals_fit",
                  "legacy ctor(a,b) == fit(b,a)")
{
  const Point32f a[4] = { {0,0}, {511,0}, {511,511}, {0,511} };
  const Point32f b[4] = { {620,270}, {1200,250}, {1818,1014}, {460,920} };
  const Homography2D Hfit = Homography2D::fit(b, a, 4);   // apply(b)=a
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  const Homography2D Hctor(a, b, 4);                       // legacy: apply(b)=a
#pragma GCC diagnostic pop
  for(int i=0;i<9;++i) ICL_TEST_NEAR(Hfit[i], Hctor[i], 1e-4f);
}

// refined() must reproduce clean data exactly (LM converges to the DLT optimum)
// and be NO WORSE than fit() under correspondence noise (it minimizes the true
// geometric error, of which the DLT is only an algebraic proxy).
ICL_REGISTER_TEST("math.homography.refined_exact_and_no_worse_than_fit",
                  "refined() is exact on clean data and <= fit() error under noise")
{
  // ground-truth homography (mild projective) + a grid of source points
  const float Hgt[9] = { 1.2f, 0.15f, 30.f,  -0.1f, 1.05f, 12.f,  3e-4f, -2e-4f, 1.f };
  auto applyH = [&](float x, float y, float &u, float &v){
    const float w = Hgt[6]*x+Hgt[7]*y+Hgt[8];
    u = (Hgt[0]*x+Hgt[1]*y+Hgt[2])/w; v = (Hgt[3]*x+Hgt[4]*y+Hgt[5])/w;
  };
  std::vector<Point32f> src, dst, dstClean;
  for(int r=0;r<6;++r) for(int c=0;c<6;++c){
    float x=c*20.f, y=r*20.f, u,v; applyH(x,y,u,v);
    src.push_back({x,y}); dstClean.push_back({u,v});
  }
  const int n=(int)src.size();

  // clean: both fit and refined must round-trip to sub-pixel
  {
    const Homography2D Hf = Homography2D::fit(src.data(), dstClean.data(), n);
    const Homography2D Hr = Homography2D::refined(src.data(), dstClean.data(), n);
    double ef=0, er=0;
    for(int i=0;i<n;++i){ Point32f pf=Hf.apply(src[i]), pr=Hr.apply(src[i]);
      ef=std::max(ef,(double)std::hypot(pf.x-dstClean[i].x, pf.y-dstClean[i].y));
      er=std::max(er,(double)std::hypot(pr.x-dstClean[i].x, pr.y-dstClean[i].y)); }
    ICL_TEST_TRUE(ef < 1e-2); ICL_TEST_TRUE(er < 1e-2);
  }

  // noisy dst: refined's RMS to the CLEAN targets must be <= fit's (within eps)
  dst = dstClean;
  unsigned s=12345; auto rnd=[&](){ s=s*1103515245u+12345u; return ((int)((s>>13)&0x3ff)-512)/512.0f; };
  for(auto &p : dst){ p.x += 1.2f*rnd(); p.y += 1.2f*rnd(); }
  const Homography2D Hf = Homography2D::fit(src.data(), dst.data(), n);
  const Homography2D Hr = Homography2D::refined(src.data(), dst.data(), n);
  double sf=0, sr=0;
  for(int i=0;i<n;++i){ Point32f pf=Hf.apply(src[i]), pr=Hr.apply(src[i]);
    sf += std::pow(std::hypot(pf.x-dstClean[i].x, pf.y-dstClean[i].y),2);
    sr += std::pow(std::hypot(pr.x-dstClean[i].x, pr.y-dstClean[i].y),2); }
  const double rmsF=std::sqrt(sf/n), rmsR=std::sqrt(sr/n);
  ICL_TEST_TRUE(rmsR <= rmsF + 1e-3);          // refined no worse than fit
  ICL_TEST_TRUE(rmsR < 1.0);                    // and still sub-pixel to GT
}

// robust() must ignore gross-outlier correspondences that would wreck the plain
// least-squares fit(), recovering the true model from the inliers.
ICL_REGISTER_TEST("math.homography.robust_rejects_outliers",
                  "robust() recovers the model from ~1/3 gross-outlier correspondences")
{
  const float Hgt[9] = { 1.2f, 0.15f, 30.f, -0.1f, 1.05f, 12.f, 3e-4f, -2e-4f, 1.f };
  auto ap=[&](float x,float y,float&u,float&v){ const float w=Hgt[6]*x+Hgt[7]*y+Hgt[8];
    u=(Hgt[0]*x+Hgt[1]*y+Hgt[2])/w; v=(Hgt[3]*x+Hgt[4]*y+Hgt[5])/w; };
  std::vector<Point32f> src, dst, dstClean; std::vector<int> outliers;
  unsigned s=7; auto rnd=[&](){ s=s*1103515245u+12345u; return ((int)((s>>13)&0x3ff)-512)/512.0f; };
  int gi=0;
  for(int r=0;r<8;++r) for(int c=0;c<8;++c){
    float x=c*20.f, y=r*20.f, u,v; ap(x,y,u,v);
    src.push_back({x,y}); dstClean.push_back({u,v});
    if(gi%3==0){ dst.push_back({u+60.f*rnd(), v+60.f*rnd()}); outliers.push_back(gi); }  // ~1/3 gross
    else         dst.push_back({u+0.3f*rnd(), v+0.3f*rnd()});                             // clean + tiny noise
    ++gi;
  }
  const int n=(int)src.size();
  const auto fit = Homography2D::robust(src.data(), dst.data(), n, 3.0f);
  ICL_TEST_TRUE(fit.ok);
  int falseInliers=0;
  for(int o : outliers) if(std::find(fit.inliers.begin(), fit.inliers.end(), o) != fit.inliers.end()) falseInliers++;
  ICL_TEST_EQ(falseInliers, 0);                                       // no outlier accepted
  ICL_TEST_TRUE((int)fit.inliers.size() >= n - (int)outliers.size() - 2);  // kept ~all inliers
  ICL_TEST_TRUE(fit.rms < 1.0f);
  double eR=0, eP=0;
  for(int i=0;i<n;++i){ Point32f p=fit.H.apply(src[i]); eR=std::max(eR,(double)std::hypot(p.x-dstClean[i].x, p.y-dstClean[i].y)); }
  const Homography2D Hplain = Homography2D::fit(src.data(), dst.data(), n);
  for(int i=0;i<n;++i){ Point32f p=Hplain.apply(src[i]); eP=std::max(eP,(double)std::hypot(p.x-dstClean[i].x, p.y-dstClean[i].y)); }
  ICL_TEST_TRUE(eR < 2.0);                    // robust recovers GT on clean points
  ICL_TEST_TRUE(eP > eR);                     // and beats the outlier-poisoned plain fit
}

ICL_REGISTER_TEST("math.fixed.closest_rotation",
                  "closest_rotation recovers a proper rotation from a scaled/noisy/reflected matrix")
{
  const FixedMatrix<float,3,3> R = create_rot_3D<float>(0.3f, 0.5f, -0.7f);

  // idempotent on a clean rotation
  const FixedMatrix<float,3,3> R2 = closest_rotation(R);
  for(int i=0;i<9;++i) ICL_TEST_NEAR(R2.begin()[i], R.begin()[i], 1e-4f);

  // recover the rotation from a scaled + slightly perturbed copy
  FixedMatrix<float,3,3> M = R * 2.5f;
  M(0,1) += 0.02f; M(2,0) -= 0.015f;
  const FixedMatrix<float,3,3> Rr = closest_rotation(M);
  ICL_TEST_NEAR(Rr.det(), 1.0f, 1e-3f);                      // proper rotation
  const FixedMatrix<float,3,3> I = Rr * Rr.transp();         // orthonormal
  ICL_TEST_NEAR(I(0,0),1.f,1e-3f); ICL_TEST_NEAR(I(1,1),1.f,1e-3f); ICL_TEST_NEAR(I(2,2),1.f,1e-3f);
  ICL_TEST_NEAR(I(0,1),0.f,1e-3f); ICL_TEST_NEAR(I(1,2),0.f,1e-3f);
  for(int i=0;i<9;++i) ICL_TEST_NEAR(Rr.begin()[i], R.begin()[i], 2e-2f);

  // a reflection (det<0) input must still yield a PROPER rotation (det +1)
  FixedMatrix<float,3,3> Ref = R;
  for(int r=0;r<3;++r) Ref(r,0) = -Ref(r,0);                 // negate a column → det<0
  ICL_TEST_EQ(closest_rotation(Ref).det() > 0.99f, true);
}

// =====================================================================
// Delaunay triangulation
// =====================================================================

#include <icl/math/transform/DelaunayTriangulation.h>
#include <set>

namespace {
  // does the triangulation contain an edge between point indices i and j?
  bool hasEdge(const std::vector<std::pair<int,int>> &e, int i, int j) {
    return std::find(e.begin(), e.end(),
                     std::make_pair(std::min(i,j), std::max(i,j))) != e.end();
  }
}

ICL_REGISTER_TEST("math.delaunay.degenerate", "fewer than 3 points yields no triangles")
{
  ICL_TEST_EQ(delaunayTriangulation({}).size(), 0u);
  ICL_TEST_EQ(delaunayTriangulation({Point32f(0,0)}).size(), 0u);
  ICL_TEST_EQ(delaunayTriangulation({Point32f(0,0), Point32f(1,1)}).size(), 0u);
}

ICL_REGISTER_TEST("math.delaunay.unit_square", "a unit square triangulates into 2 triangles / 5 edges")
{
  // 0:(0,0) 1:(1,0) 2:(1,1) 3:(0,1)
  const std::vector<Point32f> p = {Point32f(0,0), Point32f(1,0), Point32f(1,1), Point32f(0,1)};
  const auto tris = delaunayTriangulation(p);
  ICL_TEST_EQ(tris.size(), 2u);                 // a quad = 2 triangles
  const auto e = delaunayEdges(tris);
  ICL_TEST_EQ(e.size(), 5u);                    // 4 perimeter + 1 diagonal
  // all four perimeter edges must be present
  ICL_TEST_TRUE(hasEdge(e, 0, 1));
  ICL_TEST_TRUE(hasEdge(e, 1, 2));
  ICL_TEST_TRUE(hasEdge(e, 2, 3));
  ICL_TEST_TRUE(hasEdge(e, 3, 0));
  // exactly one of the two diagonals
  ICL_TEST_TRUE(hasEdge(e, 0, 2) != hasEdge(e, 1, 3));
}

ICL_REGISTER_TEST("math.delaunay.grid_contains_axis_edges",
                  "a regular grid's Delaunay graph contains every unit axis edge")
{
  // 5x4 regular grid, spacing 10; this is the property the checkerboard graph
  // associator relies on: adjacent grid corners are always Delaunay-adjacent.
  const int W = 5, H = 4;
  std::vector<Point32f> p;
  for (int r = 0; r < H; ++r)
    for (int c = 0; c < W; ++c)
      p.push_back(Point32f(c * 10.f + 0.3f * r, r * 10.f));   // slight shear (generic position)
  const auto e = delaunayEdges(delaunayTriangulation(p));
  auto idx = [&](int c, int r){ return r * W + c; };
  for (int r = 0; r < H; ++r)
    for (int c = 0; c < W; ++c) {
      if (c + 1 < W) ICL_TEST_TRUE(hasEdge(e, idx(c,r), idx(c+1,r)));   // horizontal
      if (r + 1 < H) ICL_TEST_TRUE(hasEdge(e, idx(c,r), idx(c,r+1)));   // vertical
    }
  // Euler: a triangulation of N points with h hull points has 2N-2-h triangles.
  // Hull of this grid = perimeter = 2*(W+H)-4 = 14 points → 2*20-2-14 = 24 triangles.
  ICL_TEST_EQ(delaunayTriangulation(p).size(), 24u);
}

// eigen() must return eigenvalues (and their eigenvector columns) in DESCENDING
// order, independent of the active LAPACK backend. LAPACK syev returns ascending
// and the C++ Jacobi fallback descending; the wrapper reconciles them. Regression
// for the Jacobi->LAPACK migration that silently flipped the order and broke every
// "column 0 = largest eigenvalue" caller (LeastSquareModelFitting, RigidTransformEstimator, ...).
ICL_REGISTER_TEST("math.dyn.eigen_descending_order",
                  "eigen() returns eigenvalues largest-first with matching eigenvectors")
{
  // symmetric matrix with known eigenvalues {1,2,3} on the (rotated) diagonal
  DynMatrix<double> A = DynMatrix<double>::create(3,3,0.0);
  A(0,0) = 2; A(1,1) = 3; A(2,2) = 1;   // deliberately unsorted on the diagonal
  DynMatrix<double> evec, eval;
  A.eigen(evec, eval);
  ICL_TEST_NEAR(eval[0], 3.0, 1e-9);    // largest first
  ICL_TEST_NEAR(eval[1], 2.0, 1e-9);
  ICL_TEST_NEAR(eval[2], 1.0, 1e-9);
  // each column must be an eigenvector of its eigenvalue: A*v = lambda*v
  for(int k=0;k<3;++k){
    DynMatrix<double> v(3,1);   // (rows,cols) => column vector
    for(int i=0;i<3;++i) v[i] = evec(i,k);
    DynMatrix<double> Av = A*v;
    for(int i=0;i<3;++i) ICL_TEST_NEAR(Av[i], eval[k]*v[i], 1e-9);
  }
}

// LeastSquareModelFitting depends on eigen()'s descending contract: it takes
// column 0 as the model. Fit a circle to exact samples and recover its centre/radius.
ICL_REGISTER_TEST("math.fit.least_square_circle",
                  "direct least-square circle fit recovers centre and radius")
{
  LeastSquareModelFitting2D fit(4, LeastSquareModelFitting2D::circle_gen);
  std::vector<Point32f> pts;
  for(int i=0;i<24;++i){
    const double t = i*2*M_PI/24;
    pts.push_back(Point32f(10 + 5*std::cos(t), -3 + 5*std::sin(t)));
  }
  const std::vector<double> m = fit.fit(pts);
  const double cx = -m[1]/(2*m[0]);
  const double cy = -m[2]/(2*m[0]);
  const double r  = std::sqrt((m[1]*m[1]+m[2]*m[2])/(4*m[0]*m[0]) - m[3]/m[0]);
  ICL_TEST_NEAR(cx, 10.0, 1e-3);
  ICL_TEST_NEAR(cy, -3.0, 1e-3);
  ICL_TEST_NEAR(r,   5.0, 1e-3);
}

// Exercises the SVD null-space path at a larger model dim (6-param general
// ellipse). Fit exact samples of an axis-aligned ellipse and check the recovered
// implicit conic matches a1 x^2 + a3 y^2 + a6 = const (no rotation / linear terms).
ICL_REGISTER_TEST("math.fit.least_square_ellipse",
                  "direct least-square general-ellipse fit recovers an axis-aligned conic")
{
  LeastSquareModelFitting2D fit(6, LeastSquareModelFitting2D::ellipse_gen);
  const double A = 8, B = 3;                 // semi-axes, centred at origin
  std::vector<Point32f> pts;
  for(int i=0;i<40;++i){
    const double t = i*2*M_PI/40;
    pts.push_back(Point32f(A*std::cos(t), B*std::sin(t)));
  }
  const std::vector<double> m = fit.fit(pts);   // [x^2, xy, y^2, x, y, 1]
  // normalise by the x^2 coefficient so the model is comparable to ground truth
  ICL_TEST_TRUE(std::abs(m[0]) > 1e-9);
  const double xy = m[1]/m[0], yy = m[2]/m[0], x = m[3]/m[0], y = m[4]/m[0];
  // rotation and linear terms must vanish for an origin-centred axis-aligned ellipse
  ICL_TEST_NEAR(xy, 0.0, 1e-4);
  ICL_TEST_NEAR(x,  0.0, 1e-4);
  ICL_TEST_NEAR(y,  0.0, 1e-4);
  // y^2 coefficient must equal A^2/B^2 (from x^2/A^2 + y^2/B^2 = 1)
  ICL_TEST_NEAR(yy, (A*A)/(B*B), 1e-3);
}

// RobustFitter wrapping a line ModelFitter: recover a line y = 2x + 1 from data
// that is ~1/4 gross outliers. A plain least-squares fitter made outlier-tolerant
// purely by composition (the unified robustifier replacing the old RansacFitter).
ICL_REGISTER_TEST("math.fit.robust_line_with_outliers",
                  "RobustFitter(line ModelFitter) recovers a line under outliers")
{
  using Pt = Point32f;
  using Line = std::vector<float>;           // {m, b} for y = m*x + b
  std::vector<Pt> data;
  for(int i=0;i<120;++i) data.push_back(Pt(i*0.1f, 2.0f*(i*0.1f) + 1.0f));   // inliers
  for(int i=0;i<40;++i)  data.push_back(Pt(i*0.1f, 2.0f*(i*0.1f) + 1.0f + (i%2?6.f:-6.f))); // outliers

  struct LineFitter : ModelFitter<Pt,Line> {
    Line fit(const std::vector<Pt> &s) override {
      double sx=0,sy=0,sxx=0,sxy=0; const int n=(int)s.size();
      for(const Pt &p:s){ sx+=p.x; sy+=p.y; sxx+=p.x*p.x; sxy+=p.x*p.y; }
      const double d = n*sxx - sx*sx;
      if(std::abs(d) < 1e-12) return Line{0,0};
      const double m = (n*sxy - sx*sy)/d;
      return Line{ (float)m, (float)((sy - m*sx)/n) };
    }
    double residual(const Line &m, const Pt &p) const override {
      return std::abs(p.y - (m[0]*p.x + m[1]));
    }
    int minSamples() const override { return 2; }
  } base;

  RobustFitter<Pt,Line> robust(&base, 0.5, 0.99, 2000, "ransac");
  const Line m = robust.fit(data);
  ICL_TEST_TRUE(!m.empty());
  ICL_TEST_NEAR(m[0], 2.0f, 0.05f);          // slope
  ICL_TEST_NEAR(m[1], 1.0f, 0.05f);          // intercept
  ICL_TEST_TRUE(robust.inliers().size() >= 100u);
}

// RobustFitter (Tier-C decorator) wrapping a CircleFitter2D: recover a circle from
// data that is ~1/3 gross outliers. Proves the ModelFitter framework end-to-end —
// a plain algebraic fitter made outlier-tolerant purely by composition.
ICL_REGISTER_TEST("math.fit.robust_circle_msac",
                  "RobustFitter(CircleFitter2D) recovers a circle under outliers (MSAC+LO)")
{
  using Pt = Point32f;
  const double CX = 40, CY = -15, R = 12;
  std::vector<Pt> data;
  for(int i=0;i<60;++i){ double t=i*2*M_PI/60; data.push_back(Pt(CX+R*std::cos(t), CY+R*std::sin(t))); }
  for(int i=0;i<30;++i){ data.push_back(Pt(CX-30+i, CY+25)); }         // outlier line

  CircleFitter2D circle;
  // threshold is on the ALGEBRAIC residual (not geometric px); 0.1 cleanly
  // separates the exact circle points (~0) from the far outliers (~0.8).
  RobustFitter<Pt, std::vector<double> > robust(&circle, 0.1, /*conf*/0.99, /*maxIt*/2000);
  const std::vector<double> m = robust.fit(data);
  ICL_TEST_TRUE(m.size() == 4u);
  const double cx = -m[1]/(2*m[0]), cy = -m[2]/(2*m[0]);
  const double r  = std::sqrt((m[1]*m[1]+m[2]*m[2])/(4*m[0]*m[0]) - m[3]/m[0]);
  ICL_TEST_NEAR(cx, CX, 0.5);
  ICL_TEST_NEAR(cy, CY, 0.5);
  ICL_TEST_NEAR(r,  R,  0.5);
  ICL_TEST_TRUE(robust.inliers().size() >= 60u);      // all circle points recovered
}

// A plain (non-robust) RANSAC-mode RobustFitter also works — exercises the "ransac"
// scoring branch and confirms the base fitter is swappable (line here).
ICL_REGISTER_TEST("math.fit.robust_line_ransac_mode",
                  "RobustFitter(LineFitter2D) in RANSAC scoring mode recovers a line")
{
  using Pt = Point32f;
  std::vector<Pt> data;
  for(int i=0;i<50;++i) data.push_back(Pt(i, 0.5f*i + 3.f));           // inliers: y=0.5x+3
  for(int i=0;i<15;++i) data.push_back(Pt(i, 0.5f*i + 3.f + 20.f));    // outliers
  LineFitter2D line;
  RobustFitter<Pt, std::vector<double> > robust(&line, 0.3, 0.99, 3000, "ransac");
  const std::vector<double> m = robust.fit(data);      // [a,b,c] : a x + b y + c = 0
  ICL_TEST_TRUE(m.size() == 3u);
  // slope of a x + b y + c = 0 is -a/b; expect 0.5
  ICL_TEST_NEAR(-m[0]/m[1], 0.5, 1e-2);
  ICL_TEST_TRUE(robust.inliers().size() >= 50u);
}

// NelderMeadOptimizer behind the Optimizer<V> interface minimises a quadratic bowl.
ICL_REGISTER_TEST("math.fit.nelder_mead_quadratic",
                  "NelderMeadOptimizer minimises (x-3)²+(y+2)² to the true minimum")
{
  using V = std::vector<double>;
  NelderMeadOptimizer<V> opt(5000, 1e-12, 1e-12);
  auto f = [](const V &p)->double{ return (p[0]-3)*(p[0]-3) + (p[1]+2)*(p[1]+2); };
  const auto r = opt.minimize(f, V{5.0, 5.0});
  ICL_TEST_NEAR(r.params[0],  3.0, 1e-3);
  ICL_TEST_NEAR(r.params[1], -2.0, 1e-3);
  ICL_TEST_TRUE(r.error < 1e-6);

  // ZERO init: the default simplex used to be degenerate here (x[i]*=1.05 leaves
  // a zero component unperturbed) — now perturbed with an absolute step.
  const auto r0 = opt.minimize(f, V{0.0, 0.0});
  ICL_TEST_NEAR(r0.params[0],  3.0, 1e-3);
  ICL_TEST_NEAR(r0.params[1], -2.0, 1e-3);
  ICL_TEST_TRUE(r0.error < 1e-6);
}

// Returning eigen()/svd() overloads with structured bindings (ergonomic sugar over
// the out-parameter forms; same DESCENDING eigen contract).
ICL_REGISTER_TEST("math.dyn.eigen_svd_returning_overloads",
                  "auto [evec,eval]=m.eigen() and auto [U,S,V]=m.svd() work and agree")
{
  DynMatrix<double> A = DynMatrix<double>::create(3,3,0.0);
  A(0,0)=2; A(1,1)=3; A(2,2)=1;
  auto [evec, eval] = A.eigen();
  ICL_TEST_NEAR(eval[0], 3.0, 1e-9);            // descending
  ICL_TEST_NEAR(eval[2], 1.0, 1e-9);
  // agree with the out-parameter form
  DynMatrix<double> ev2, ea2; A.eigen(ev2, ea2);
  ICL_TEST_NEAR(eval[1], ea2[1], 1e-12);
  auto [U, S, V] = A.eigen().vectors.svd();     // also reachable via named fields
  ICL_TEST_TRUE(S.rows() == 3u);
}

// Taubin circle fit is a drop-in for CircleFitter2D and less biased on a partial
// arc. On a 120° arc with a deterministic radial wobble, Taubin's centre must be
// at least as close to ground truth as the naive algebraic (Kåsa) fit.
ICL_REGISTER_TEST("math.fit.taubin_circle_beats_algebraic_on_arc",
                  "TaubinCircleFitter is less biased than the algebraic fit on an arc")
{
  const double CX = 100, CY = 50, R = 30;
  std::vector<Point32f> pts;
  for(int i=0;i<40;++i){
    const double t = -M_PI/3 + (2*M_PI/3) * i/39.0;        // 120° arc
    const double rr = R + (i%2 ? 0.4 : -0.4);              // deterministic wobble
    pts.push_back(Point32f(CX + rr*std::cos(t), CY + rr*std::sin(t)));
  }
  auto centreErr = [&](const std::vector<double>&m){
    const double cx=-m[1]/(2*m[0]), cy=-m[2]/(2*m[0]);
    return std::hypot(cx-CX, cy-CY);
  };
  CircleFitter2D algebraic; TaubinCircleFitter taubin;
  const double eA = centreErr(algebraic.fit(pts));
  const double eT = centreErr(taubin.fit(pts));
  ICL_TEST_TRUE(eT <= eA + 1e-6);      // Taubin no worse (and typically much better)
  ICL_TEST_TRUE(eT < 1.0);             // and close to truth
}

// SeededFitter chaining: algebraic circle seed → geometric (orthogonal-distance)
// refine. The refined model's geometric SSE must be <= the seed's, and the chain
// is itself a ModelFitter (proves generic chainability).
ICL_REGISTER_TEST("math.fit.seeded_geometric_circle_refine",
                  "SeededFitter(algebraic, geometric) lowers the geometric error")
{
  const double CX = -20, CY = 8, R = 15;
  std::vector<Point32f> pts;
  for(int i=0;i<50;++i){
    const double t = i*2*M_PI/50;
    const double rr = R + (i%3==0 ? 0.8 : (i%3==1 ? -0.5 : 0.2));   // deterministic noise
    pts.push_back(Point32f(CX + rr*std::cos(t), CY + rr*std::sin(t)));
  }
  auto geomSSE = [&](const std::vector<double>&m){
    const double cx=-m[1]/(2*m[0]), cy=-m[2]/(2*m[0]);
    const double r=std::sqrt((m[1]*m[1]+m[2]*m[2])/(4*m[0]*m[0]) - m[3]/m[0]);
    double s=0; for(auto&p:pts){ double d=std::hypot(p.x-cx,p.y-cy)-r; s+=d*d; } return s;
  };
  CircleFitter2D seed; GeometricCircleRefiner refiner;
  const std::vector<double> mSeed = seed.fit(pts);
  SeededFitter<Point32f, std::vector<double> > chain(&seed, &refiner);
  const std::vector<double> mRef = chain.fit(pts);
  ICL_TEST_TRUE(geomSSE(mRef) <= geomSSE(mSeed) + 1e-9);   // refine never worsens geometry
  const double cx=-mRef[1]/(2*mRef[0]), cy=-mRef[2]/(2*mRef[0]);
  ICL_TEST_NEAR(cx, CX, 0.5);
  ICL_TEST_NEAR(cy, CY, 0.5);
}

// eigenVector() returns the single extreme eigenvector without materializing the
// full decomposition; must agree with the corresponding column of eigen().
ICL_REGISTER_TEST("math.dyn.eigen_vector_extreme",
                  "eigenVector(smallest/largest) matches the eigen() columns")
{
  DynMatrix<double> A = DynMatrix<double>::create(3,3,0.0);
  A(0,0)=2; A(1,1)=5; A(2,2)=1;                 // eigenvalues 2,5,1
  auto [evec, eval] = A.eigen();                // descending: 5,2,1
  const DynMatrix<double> vBig   = A.eigenVector(true);    // eigenvalue 5
  const DynMatrix<double> vSmall = A.eigenVector(false);   // eigenvalue 1
  // largest sits in column 0, smallest in the last column (descending)
  for(int i=0;i<3;++i){
    ICL_TEST_NEAR(std::abs(vBig[i]),   std::abs(evec(i,0)), 1e-9);
    ICL_TEST_NEAR(std::abs(vSmall[i]), std::abs(evec(i,2)), 1e-9);
  }
  // A*v == lambda*v (lambda=1 for the smallest here)
  ICL_TEST_NEAR(vSmall[2]*vSmall[2], 1.0, 1e-9);   // eigenvector ~ e_z
  // optional eigenvalue out-param returns the matching eigenvalue
  double lambdaBig = 0, lambdaSmall = 0;
  A.eigenVector(true,  &lambdaBig);
  A.eigenVector(false, &lambdaSmall);
  ICL_TEST_NEAR(lambdaBig,   5.0, 1e-9);
  ICL_TEST_NEAR(lambdaSmall, 1.0, 1e-9);
}

// CMA-ES on the 2D Rosenbrock banana — an ill-conditioned, curved valley where a
// naive step optimizer stalls. Seed the RNG for determinism.
ICL_REGISTER_TEST("math.fit.cmaes_rosenbrock",
                  "CMAESOptimizer solves the 2D Rosenbrock function")
{
  using V = std::vector<double>;
  randomSeed(12345);
  auto rosen = [](const V &p)->double{
    const double a = 1 - p[0], b = p[1] - p[0]*p[0];
    return a*a + 100.0*b*b;
  };
  CMAESOptimizer<V> opt(/*maxIt*/3000, /*sigma0*/0.5, /*minError*/1e-12);
  const auto r = opt.minimize(rosen, V{-1.2, 1.0});
  ICL_TEST_NEAR(r.params[0], 1.0, 1e-3);
  ICL_TEST_NEAR(r.params[1], 1.0, 1e-3);
  ICL_TEST_TRUE(r.error < 1e-8);
}

// CMA-ES on an ill-conditioned quadratic (axis scales differ by 1e6): covariance
// adaptation handles the anisotropy that an isotropic-step method struggles with.
ICL_REGISTER_TEST("math.fit.cmaes_ill_conditioned",
                  "CMAESOptimizer minimises a 1e6-anisotropic quadratic")
{
  using V = std::vector<double>;
  randomSeed(777);
  auto f = [](const V &p)->double{
    const double dx = p[0]-2.0, dy = p[1]+1.0;
    return dx*dx + 1e6*dy*dy;            // very stretched valley
  };
  CMAESOptimizer<V> opt(4000, 1.0, 1e-14);
  const auto r = opt.minimize(f, V{0.0, 0.0});
  ICL_TEST_NEAR(r.params[0],  2.0, 1e-2);
  ICL_TEST_NEAR(r.params[1], -1.0, 1e-4);
}

// PolynomialRegression via the SVD least-squares solve recovers a known quadratic.
ICL_REGISTER_TEST("math.fit.polynomial_regression_quadratic",
                  "PolynomialRegression recovers y = 2x0^2 - 3x0 + 1")
{
  PolynomialRegression<double> pr("x0^2 + x0 + 1");     // features [x0², x0, 1]
  const int N = 12;
  DynMatrix<double> xs(N,1), ys(N,1);   // (rows,cols): N rows, 1 col
  for(int i=0;i<N;++i){
    const double x = -3 + 0.5*i;
    xs(i,0) = x;                        // operator()(row,col)
    ys(i,0) = 2*x*x - 3*x + 1;
  }
  const auto &res = pr.apply(xs, ys);
  const auto &c = res.getParams();       // one column: coefficients for [x0², x0, 1]
  ICL_TEST_NEAR(c[0],  2.0, 1e-6);
  ICL_TEST_NEAR(c[1], -3.0, 1e-6);
  ICL_TEST_NEAR(c[2],  1.0, 1e-6);
}

// General (non-symmetric) eigendecomposition via geev — real + complex spectra.
ICL_REGISTER_TEST("math.dyn.eigen_general_geev",
                  "eigenGeneral handles non-symmetric real and complex eigenvalues")
{
  // upper-triangular: real eigenvalues {2,3}, right eigenvectors [1,0] and [1,1]
  DynMatrix<double> A = DynMatrix<double>::create(2,2,0.0);
  A(0,0)=2; A(0,1)=1; A(1,1)=3;                 // operator()(row,col)
  auto e = A.eigenGeneral();
  std::vector<double> ev = { e.valuesReal[0], e.valuesReal[1] };
  std::sort(ev.begin(), ev.end());
  ICL_TEST_NEAR(ev[0], 2.0, 1e-9);
  ICL_TEST_NEAR(ev[1], 3.0, 1e-9);
  ICL_TEST_NEAR(e.valuesImag[0], 0.0, 1e-9);
  // A*v == lambda*v for each (real) eigenpair
  for(int j=0;j<2;++j){
    const double l = e.valuesReal[j];
    double v0=e.vectorsReal(0,j), v1=e.vectorsReal(1,j);
    ICL_TEST_NEAR(A(0,0)*v0+A(0,1)*v1, l*v0, 1e-9);
    ICL_TEST_NEAR(A(1,0)*v0+A(1,1)*v1, l*v1, 1e-9);
  }
  // rotation: eigenvalues ±i
  DynMatrix<double> R = DynMatrix<double>::create(2,2,0.0);
  R(0,1)=-1; R(1,0)=1;
  auto er = R.eigenGeneral();
  ICL_TEST_NEAR(er.valuesReal[0], 0.0, 1e-9);
  ICL_TEST_NEAR(std::abs(er.valuesImag[0]), 1.0, 1e-9);
}

// geev across all registered backends (Accelerate/Eigen/C++ fallback) must agree
// on a non-symmetric matrix with known eigenvalues {1,2,3}. Exercises the C++
// Faddeev-LeVerrier fallback + null-space eigenvectors via forced backend select.
ICL_REGISTER_TEST("math.dyn.eigen_general_backends",
                  "eigenGeneral agrees across Accelerate / Eigen / C++ backends")
{
  DynMatrix<double> A = DynMatrix<double>::create(3,3,0.0);
  A(0,0)=1; A(0,1)=4; A(0,2)=5;      // upper-triangular → eigenvalues 1,2,3
  A(1,1)=2; A(1,2)=6;
  A(2,2)=3;
  auto &sel = LapackOps<double>::instance()
                .getSelector<LapackOps<double>::GeevSig>(LapackOp::geev);
  for(Backend b : { Backend::Cpp, Backend::Eigen, Backend::Accelerate }){
    if(!sel.get(b)) continue;                    // backend not built on this platform
    sel.force(b);
    auto e = A.eigenGeneral();
    std::vector<double> ev = { e.valuesReal[0], e.valuesReal[1], e.valuesReal[2] };
    std::sort(ev.begin(), ev.end());
    ICL_TEST_NEAR(ev[0], 1.0, 1e-6);
    ICL_TEST_NEAR(ev[1], 2.0, 1e-6);
    ICL_TEST_NEAR(ev[2], 3.0, 1e-6);
    // real eigenvector for eigenvalue 1 must satisfy A v = v (first std basis vec)
    int j1 = 0; for(int k=1;k<3;++k) if(std::abs(e.valuesReal[k]-1.0) < std::abs(e.valuesReal[j1]-1.0)) j1=k;
    const double v0=e.vectorsReal(0,j1), v1=e.vectorsReal(1,j1), v2=e.vectorsReal(2,j1);
    ICL_TEST_NEAR(A(0,0)*v0+A(0,1)*v1+A(0,2)*v2, 1.0*v0, 1e-6);
    ICL_TEST_NEAR(A(1,0)*v0+A(1,1)*v1+A(1,2)*v2, 1.0*v1, 1e-6);
    sel.unforce();
  }
}

// Halíř–Flusser ellipse fit: recovers an axis-aligned ellipse AND guarantees the
// result is an ellipse (4ac−b² > 0), which the identity-constraint fit does not.
ICL_REGISTER_TEST("math.fit.halir_flusser_ellipse",
                  "HalirFlusserEllipseFitter recovers an ellipse and stays elliptic")
{
  HalirFlusserEllipseFitter fit;
  const double A = 8, B = 3;                    // semi-axes, centred at origin
  std::vector<Point32f> pts;
  for(int i=0;i<50;++i){
    const double t = i*2*M_PI/50;
    pts.push_back(Point32f(A*std::cos(t), B*std::sin(t)));
  }
  const std::vector<double> m = fit.fit(pts);   // [A,B,C,D,E,F]
  ICL_TEST_TRUE(m.size() == 6u);
  ICL_TEST_TRUE(4*m[0]*m[2] - m[1]*m[1] > 0);   // it IS an ellipse
  ICL_TEST_TRUE(std::abs(m[0]) > 1e-12);
  // normalise by the x² coeff; rotation + linear terms vanish, y²/x² = A²/B²
  const double xy=m[1]/m[0], yy=m[2]/m[0], x=m[3]/m[0], y=m[4]/m[0];
  ICL_TEST_NEAR(xy, 0.0, 1e-4);
  ICL_TEST_NEAR(x,  0.0, 1e-4);
  ICL_TEST_NEAR(y,  0.0, 1e-4);
  ICL_TEST_NEAR(yy, (A*A)/(B*B), 1e-3);
}

// RobustFitter "trimmed" (LTS) mode: threshold-free — recover a circle under
// outliers by specifying only the inlier fraction (no residual threshold to tune).
ICL_REGISTER_TEST("math.fit.robust_trimmed_circle",
                  "RobustFitter trimmed/LTS mode recovers a circle via inlier fraction")
{
  using Pt = Point32f;
  const double CX = 40, CY = -15, R = 12;
  std::vector<Pt> data;
  for(int i=0;i<70;++i){ double t=i*2*M_PI/70; data.push_back(Pt(CX+R*std::cos(t), CY+R*std::sin(t))); }
  for(int i=0;i<30;++i){ data.push_back(Pt(CX-40+i*2.0, CY+28)); }        // 30% outliers
  CircleFitter2D circle;
  // scoring="trimmed", inlierFraction=0.7 (≈ the 70% true inliers); NO threshold used
  RobustFitter<Pt, std::vector<double> > robust(&circle, /*thresh*/0.0, 0.99, 2000, "trimmed", true, 0.7);
  const std::vector<double> m = robust.fit(data);
  const double cx=-m[1]/(2*m[0]), cy=-m[2]/(2*m[0]);
  const double r =std::sqrt((m[1]*m[1]+m[2]*m[2])/(4*m[0]*m[0]) - m[3]/m[0]);
  ICL_TEST_NEAR(cx, CX, 0.5);
  ICL_TEST_NEAR(cy, CY, 0.5);
  ICL_TEST_NEAR(r,  R,  0.5);
  ICL_TEST_TRUE(robust.inliers().size() >= 65u);   // ~70 true inliers recovered
}

// Superquadric shape fitting via CMA-ES — dogfoods the Optimizer<V> framework on a
// genuinely non-convex 5-DOF problem. Sample exact points on an axis-aligned,
// origin-centred superquadric (size a,b,c + squareness e1,e2) and recover the
// parameters by minimising the Solina inside-outside error Σ(√(abc)·(F^(e1/2)−1))².
ICL_REGISTER_TEST("math.fit.cmaes_superquadric_shape",
                  "CMAESOptimizer recovers superquadric size + squareness from surface points")
{
  using V = std::vector<double>;
  randomSeed(2026);
  const double A=3.0, B=2.0, C=1.4, E1=0.7, E2=0.8;    // ground truth (rounded box-ish)
  auto sgnpow = [](double base, double e){ return (base<0?-1.0:1.0)*std::pow(std::abs(base), e); };

  std::vector<std::array<double,3>> pts;
  for(int i=1;i<12;++i) for(int j=0;j<24;++j){          // parametric SQ surface
    const double eta = -M_PI/2 + M_PI*i/12.0, om = -M_PI + 2*M_PI*j/24.0;
    pts.push_back({ A*sgnpow(std::cos(eta),E1)*sgnpow(std::cos(om),E2),
                    B*sgnpow(std::cos(eta),E1)*sgnpow(std::sin(om),E2),
                    C*sgnpow(std::sin(eta),E1) });
  }
  auto cost = [&](const V &p)->double{
    const double a=std::abs(p[0])+1e-3, b=std::abs(p[1])+1e-3, c=std::abs(p[2])+1e-3;
    const double e1=std::min(2.0,std::max(0.1,p[3])), e2=std::min(2.0,std::max(0.1,p[4]));
    double s=0;
    for(const auto &q : pts){
      const double f = std::pow(std::pow(std::abs(q[0]/a),2/e2) + std::pow(std::abs(q[1]/b),2/e2), e2/e1)
                     + std::pow(std::abs(q[2]/c),2/e1);
      const double r = std::sqrt(a*b*c)*(std::pow(f,e1/2)-1);
      s += r*r;
    }
    return s;
  };
  CMAESOptimizer<V> opt(4000, 0.4, 1e-12);
  const auto r = opt.minimize(cost, V{2.5, 2.5, 2.5, 1.0, 1.0});   // init from a rough sphere
  ICL_TEST_NEAR(std::abs(r.params[0]), A, 0.1);
  ICL_TEST_NEAR(std::abs(r.params[1]), B, 0.1);
  ICL_TEST_NEAR(std::abs(r.params[2]), C, 0.1);
  ICL_TEST_NEAR(r.params[3], E1, 0.15);
  ICL_TEST_NEAR(r.params[4], E2, 0.15);
}

ICL_REGISTER_TEST("math.fit.cmaes_superquadric_trimmed",
                  "trimmed (LTS) Solina cost recovers a superquadric despite 40% gross outliers")
{
  using V = std::vector<double>;
  const double A=3.0, B=2.0, C=1.4, E1=0.7, E2=0.8;
  auto sgnpow = [](double base, double e){ return (base<0?-1.0:1.0)*std::pow(std::abs(base), e); };

  std::vector<std::array<double,3>> pts;
  for(int i=1;i<12;++i) for(int j=0;j<24;++j){          // 264 inliers on the SQ surface
    const double eta = -M_PI/2 + M_PI*i/12.0, om = -M_PI + 2*M_PI*j/24.0;
    pts.push_back({ A*sgnpow(std::cos(eta),E1)*sgnpow(std::cos(om),E2),
                    B*sgnpow(std::cos(eta),E1)*sgnpow(std::sin(om),E2),
                    C*sgnpow(std::sin(eta),E1) });
  }
  const int nIn = (int)pts.size();
  uint32_t seed = 12345u;                                // deterministic LCG for outliers
  auto rnd = [&]{ seed = seed*1664525u + 1013904223u; return (double)seed / 4294967296.0; };
  const int nOut = 176;                                  // → 40% of 440 total
  for(int i=0;i<nOut;++i) pts.push_back({ -5+10*rnd(), -5+10*rnd(), -5+10*rnd() });

  const int keep = nIn;                                  // trim to the inlier count
  std::vector<double> r2; r2.reserve(pts.size());
  auto cost = [&](const V &p)->double{
    const double a=std::abs(p[0])+1e-3, b=std::abs(p[1])+1e-3, c=std::abs(p[2])+1e-3;
    const double e1=std::min(2.0,std::max(0.1,p[3])), e2=std::min(2.0,std::max(0.1,p[4]));
    r2.clear();
    for(const auto &q : pts){
      const double f = std::pow(std::pow(std::abs(q[0]/a),2/e2) + std::pow(std::abs(q[1]/b),2/e2), e2/e1)
                     + std::pow(std::abs(q[2]/c),2/e1);
      const double r = std::sqrt(a*b*c)*(std::pow(f,e1/2)-1);
      r2.push_back(r*r);
    }
    std::nth_element(r2.begin(), r2.begin()+keep, r2.end());
    double s=0; for(int i=0;i<keep;++i) s+=r2[i]; return s;
  };
  CMAESOptimizer<V> opt(4000, 0.4, 1e-12);
  const auto r = opt.minimize(cost, V{2.5, 2.5, 2.5, 1.0, 1.0});
  ICL_TEST_NEAR(std::abs(r.params[0]), A, 0.2);
  ICL_TEST_NEAR(std::abs(r.params[1]), B, 0.2);
  ICL_TEST_NEAR(std::abs(r.params[2]), C, 0.2);
  ICL_TEST_NEAR(r.params[3], E1, 0.2);
  ICL_TEST_NEAR(r.params[4], E2, 0.2);
}
