// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/Test.h>
#include <ICLMath/FixedMatrix.h>
#include <ICLMath/DynMatrix.h>

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
      ICL_TEST_NEAR(I(c,r), (r==c) ? 1.0f : 0.0f, 1e-7f);
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
  FixedMatrix<float,3,2> m(0.0f);
  // row-major: data[col + cols*row]
  m(0,0) = 1; m(1,0) = 2; m(2,0) = 3;
  m(0,1) = 4; m(1,1) = 5; m(2,1) = 6;
  ICL_TEST_EQ(m[0], 1.0f);  // (0,0)
  ICL_TEST_EQ(m[1], 2.0f);  // (1,0)
  ICL_TEST_EQ(m[2], 3.0f);  // (2,0)
  ICL_TEST_EQ(m[3], 4.0f);  // (0,1)
  ICL_TEST_EQ(m[5], 6.0f);  // (2,1)
}

ICL_REGISTER_TEST("math.fixed.at_throws", "at() throws on invalid index")
{
  FixedMatrix<float,2,2> m(1.0f);
  ICL_TEST_THROW(m.at(2,0), InvalidIndexException);
  ICL_TEST_THROW(m.at(0,2), InvalidIndexException);
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
      for(int k = 0; k < 4; ++k) e += A(k,r) * B(c,k);
      ICL_TEST_NEAR(R(c,r), e, 1e-3f);
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
      for(int k = 0; k < 3; ++k) e += A(k,r) * B(c,k);
      ICL_TEST_NEAR(R(c,r), e, 1e-3f);
    }
}

ICL_REGISTER_TEST("math.fixed.mult_2x2f", "2x2f multiply vs naive")
{
  FixedMatrix<float,2,2> A, B;
  A(0,0)=1; A(1,0)=2; A(0,1)=3; A(1,1)=4;
  B(0,0)=5; B(1,0)=6; B(0,1)=7; B(1,1)=8;
  auto R = A * B;
  // [1 2] * [5 6] = [19 22]
  // [3 4]   [7 8]   [43 50]
  ICL_TEST_NEAR(R(0,0), 19.0f, 1e-5f);
  ICL_TEST_NEAR(R(1,0), 22.0f, 1e-5f);
  ICL_TEST_NEAR(R(0,1), 43.0f, 1e-5f);
  ICL_TEST_NEAR(R(1,1), 50.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.fixed.mult_rect", "non-square multiply 2x3 * 3x2 = 2x2")
{
  FixedMatrix<float,3,2> A(0.0f); // 3 cols, 2 rows
  FixedMatrix<float,2,3> B(0.0f); // 2 cols, 3 rows
  // A = [1 2 3; 4 5 6]
  A(0,0)=1; A(1,0)=2; A(2,0)=3;
  A(0,1)=4; A(1,1)=5; A(2,1)=6;
  // B = [7 8; 9 10; 11 12]
  B(0,0)=7;  B(1,0)=8;
  B(0,1)=9;  B(1,1)=10;
  B(0,2)=11; B(1,2)=12;
  auto R = A * B; // 2x2
  // [1*7+2*9+3*11  1*8+2*10+3*12] = [58  64]
  // [4*7+5*9+6*11  4*8+5*10+6*12]   [139 154]
  ICL_TEST_NEAR(R(0,0), 58.0f,  1e-4f);
  ICL_TEST_NEAR(R(1,0), 64.0f,  1e-4f);
  ICL_TEST_NEAR(R(0,1), 139.0f, 1e-4f);
  ICL_TEST_NEAR(R(1,1), 154.0f, 1e-4f);
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
      for(int k = 0; k < 4; ++k) e += A(k,r) * B(c,k);
      ICL_TEST_NEAR(R(c,r), e, 1e-10);
    }
}

ICL_REGISTER_TEST("math.fixed.mult_4x4_vec", "4x4 * 4x1 vector transform")
{
  FixedMatrix<float,4,4> M = FixedMatrix<float,4,4>::id();
  M(3,0) = 10; M(3,1) = 20; M(3,2) = 30; // translation in last column
  FixedMatrix<float,1,4> v(0.0f);
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
  FixedMatrix<float,1,4> v(0.0f);
  for(int i = 0; i < 16; ++i) M[i] = static_cast<float>(i+1);
  v[0] = 2; v[1] = 3; v[2] = 5; v[3] = 7;
  auto r = M * v;
  for(int row = 0; row < 4; ++row){
    float e = 0;
    for(int k = 0; k < 4; ++k) e += M(k, row) * v[k];
    ICL_TEST_NEAR(r[row], e, 1e-3f);
  }
}

// =====================================================================
// FixedMatrix — Transpose
// =====================================================================

ICL_REGISTER_TEST("math.fixed.transp_3x3", "3x3 transpose")
{
  FixedMatrix<float,3,3> m(0.0f);
  m(0,0)=1; m(1,0)=2; m(2,0)=3;
  m(0,1)=4; m(1,1)=5; m(2,1)=6;
  m(0,2)=7; m(1,2)=8; m(2,2)=9;
  auto t = m.transp();
  for(unsigned r = 0; r < 3; ++r)
    for(unsigned c = 0; c < 3; ++c)
      ICL_TEST_EQ(t(c,r), m(r,c));
}

ICL_REGISTER_TEST("math.fixed.transp_rect", "2x3 transpose = 3x2")
{
  FixedMatrix<float,3,2> m(0.0f);
  m(0,0)=1; m(1,0)=2; m(2,0)=3;
  m(0,1)=4; m(1,1)=5; m(2,1)=6;
  auto t = m.transp();  // FixedMatrix<float,2,3>
  ICL_TEST_EQ(t(0,0), 1.0f); ICL_TEST_EQ(t(1,0), 4.0f);
  ICL_TEST_EQ(t(0,1), 2.0f); ICL_TEST_EQ(t(1,1), 5.0f);
  ICL_TEST_EQ(t(0,2), 3.0f); ICL_TEST_EQ(t(1,2), 6.0f);
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
  m(0,0)=1; m(1,0)=2;
  m(0,1)=3; m(1,1)=4;
  ICL_TEST_NEAR(m.det(), -2.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.fixed.det_3x3", "3x3 determinant")
{
  FixedMatrix<float,3,3> m(0.0f);
  m(0,0)=6;  m(1,0)=1;  m(2,0)=1;
  m(0,1)=4;  m(1,1)=-2; m(2,1)=5;
  m(0,2)=2;  m(1,2)=8;  m(2,2)=7;
  ICL_TEST_NEAR(m.det(), -306.0f, 1e-2f);
}

ICL_REGISTER_TEST("math.fixed.det_4x4", "4x4 determinant")
{
  auto I = FixedMatrix<float,4,4>::id();
  ICL_TEST_NEAR(I.det(), 1.0f, 1e-6f);

  FixedMatrix<float,4,4> m(0.0f);
  m(0,0)=1; m(1,0)=0; m(2,0)=2; m(3,0)=-1;
  m(0,1)=3; m(1,1)=0; m(2,1)=0; m(3,1)=5;
  m(0,2)=2; m(1,2)=1; m(2,2)=4; m(3,2)=-3;
  m(0,3)=1; m(1,3)=0; m(2,3)=5; m(3,3)=0;
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
  m(0,0)=4; m(1,0)=7;
  m(0,1)=2; m(1,1)=6;
  auto R = m * m.inv();
  auto I = FixedMatrix<float,2,2>::id();
  for(int i = 0; i < 4; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.inv_3x3", "3x3 inverse round-trip")
{
  FixedMatrix<float,3,3> m(0.0f);
  m(0,0)=1; m(1,0)=2; m(2,0)=3;
  m(0,1)=0; m(1,1)=1; m(2,1)=4;
  m(0,2)=5; m(1,2)=6; m(2,2)=0;
  auto R = m * m.inv();
  auto I = FixedMatrix<float,3,3>::id();
  for(int i = 0; i < 9; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-4f);
}

ICL_REGISTER_TEST("math.fixed.inv_4x4", "4x4 inverse round-trip")
{
  FixedMatrix<float,4,4> m(0.0f);
  m(0,0)=1; m(1,0)=0; m(2,0)=2; m(3,0)=-1;
  m(0,1)=3; m(1,1)=0; m(2,1)=0; m(3,1)=5;
  m(0,2)=2; m(1,2)=1; m(2,2)=4; m(3,2)=-3;
  m(0,3)=1; m(1,3)=0; m(2,3)=5; m(3,3)=0;
  auto R = m * m.inv();
  auto I = FixedMatrix<float,4,4>::id();
  for(int i = 0; i < 16; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-3f);
}

ICL_REGISTER_TEST("math.fixed.inv_singular_throws", "singular matrix inv throws")
{
  FixedMatrix<float,2,2> m(0.0f);
  m(0,0)=1; m(1,0)=2;
  m(0,1)=2; m(1,1)=4; // det=0
  ICL_TEST_THROW(m.inv(), SingularMatrixException);
}

// =====================================================================
// FixedMatrix — Length / Normalize
// =====================================================================

ICL_REGISTER_TEST("math.fixed.length_l2", "L2 norm of [3,4,0] = 5")
{
  FixedMatrix<float,3,1> v(0.0f);
  v[0] = 3; v[1] = 4;
  ICL_TEST_NEAR(v.length(), 5.0, 1e-10);
}

ICL_REGISTER_TEST("math.fixed.length_l1", "L1 norm of [-3,4,-1] = 8")
{
  FixedMatrix<float,3,1> v(0.0f);
  v[0] = -3; v[1] = 4; v[2] = -1;
  ICL_TEST_NEAR(v.length(1), 8.0, 1e-10);
}

ICL_REGISTER_TEST("math.fixed.length_general", "L3 norm")
{
  FixedMatrix<float,2,1> v(0.0f);
  v[0] = 2; v[1] = 3;
  double expected = std::pow(std::pow(2.0,3) + std::pow(3.0,3), 1.0/3.0);
  ICL_TEST_NEAR(v.length(3), expected, 1e-6);
}

ICL_REGISTER_TEST("math.fixed.normalize", "normalized vector has length 1")
{
  FixedMatrix<float,4,1> v(0.0f);
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
  m(0,0)=1; m(1,1)=5; m(2,2)=9;
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
  FixedMatrix<float,3,1> a(0.0f), b(0.0f);
  a[0]=1; a[1]=2; a[2]=3;
  b[0]=4; b[1]=5; b[2]=6;
  ICL_TEST_NEAR(a.element_wise_inner_product(b), 32.0f, 1e-5f);
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
  DynMatrix<float> m(3, 2, 7.0f);
  ICL_TEST_EQ(m.cols(), 3u);
  ICL_TEST_EQ(m.rows(), 2u);
  for(unsigned i = 0; i < 6; ++i) ICL_TEST_EQ(m[i], 7.0f);
}

ICL_REGISTER_TEST("math.dyn.ctor_copy", "copy ctor deep copies")
{
  DynMatrix<float> a(2, 2, 5.0f);
  DynMatrix<float> b(a);
  b[0] = 99.0f;
  ICL_TEST_EQ(a[0], 5.0f);  // original unchanged
  ICL_TEST_EQ(b[0], 99.0f);
}

ICL_REGISTER_TEST("math.dyn.ctor_wrap", "shallow wrap shares data")
{
  float data[] = {1, 2, 3, 4};
  DynMatrix<float> m(2, 2, data, false); // shallow
  ICL_TEST_EQ(m(0,0), 1.0f);
  ICL_TEST_EQ(m(1,1), 4.0f);
  m(0,0) = 99.0f;
  ICL_TEST_EQ(data[0], 99.0f); // data modified through matrix
}

ICL_REGISTER_TEST("math.dyn.ctor_zero_throws", "zero dimension throws")
{
  ICL_TEST_THROW(DynMatrix<float>(0, 3), InvalidMatrixDimensionException);
  ICL_TEST_THROW(DynMatrix<float>(3, 0), InvalidMatrixDimensionException);
}

// =====================================================================
// DynMatrix — Element access
// =====================================================================

ICL_REGISTER_TEST("math.dyn.access", "operator()(col,row) indexing")
{
  DynMatrix<float> m(3, 2, 0.0f);
  m(0,0) = 1; m(1,0) = 2; m(2,0) = 3;
  m(0,1) = 4; m(1,1) = 5; m(2,1) = 6;
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
  DynMatrix<float> m(2, 2, 3.0f);
  auto r = m * 4.0f;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(r[i], 12.0f);
}

ICL_REGISTER_TEST("math.dyn.scalar_div", "matrix / scalar")
{
  DynMatrix<float> m(2, 2, 12.0f);
  auto r = m / 3.0f;
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_EQ(r[i], 4.0f);
}

ICL_REGISTER_TEST("math.dyn.scalar_add_sub", "matrix +/- scalar")
{
  DynMatrix<float> m(2, 2, 5.0f);
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
  DynMatrix<float> A(3, 3, 0.0f);
  for(unsigned i = 0; i < 9; ++i) A[i] = static_cast<float>(i+1);
  auto I = DynMatrix<float>::id(3);
  auto R = A * I;
  for(unsigned i = 0; i < 9; ++i) ICL_TEST_NEAR(R[i], A[i], 1e-5f);
}

ICL_REGISTER_TEST("math.dyn.mult_2x2", "2x2 multiply correctness")
{
  DynMatrix<float> A(2, 2, 0.0f), B(2, 2, 0.0f);
  A(0,0)=1; A(1,0)=2; A(0,1)=3; A(1,1)=4;
  B(0,0)=5; B(1,0)=6; B(0,1)=7; B(1,1)=8;
  auto R = A * B;
  ICL_TEST_NEAR(R(0,0), 19.0f, 1e-5f);
  ICL_TEST_NEAR(R(1,0), 22.0f, 1e-5f);
  ICL_TEST_NEAR(R(0,1), 43.0f, 1e-5f);
  ICL_TEST_NEAR(R(1,1), 50.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.dyn.mult_rect", "2x3 * 3x2 = 2x2")
{
  DynMatrix<float> A(3, 2, 0.0f);
  A(0,0)=1; A(1,0)=2; A(2,0)=3;
  A(0,1)=4; A(1,1)=5; A(2,1)=6;
  DynMatrix<float> B(2, 3, 0.0f);
  B(0,0)=7;  B(1,0)=8;
  B(0,1)=9;  B(1,1)=10;
  B(0,2)=11; B(1,2)=12;
  auto R = A * B;
  ICL_TEST_EQ(R.cols(), 2u);
  ICL_TEST_EQ(R.rows(), 2u);
  ICL_TEST_NEAR(R(0,0), 58.0f,  1e-4f);
  ICL_TEST_NEAR(R(1,0), 64.0f,  1e-4f);
  ICL_TEST_NEAR(R(0,1), 139.0f, 1e-4f);
  ICL_TEST_NEAR(R(1,1), 154.0f, 1e-4f);
}

ICL_REGISTER_TEST("math.dyn.mult_dim_mismatch_throws", "incompatible dimensions throw")
{
  DynMatrix<float> A(3, 2, 1.0f), B(2, 2, 1.0f);  // A.cols()=3 != B.rows()=2
  ICL_TEST_THROW(A * B, IncompatibleMatrixDimensionException);
}

ICL_REGISTER_TEST("math.dyn.elementwise_mult", "elementwise multiplication")
{
  DynMatrix<float> a(2, 2, 0.0f), b(2, 2, 0.0f);
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
  DynMatrix<float> m(3, 3, 0.0f);
  for(unsigned i = 0; i < 9; ++i) m[i] = static_cast<float>(i+1);
  auto t = m.transp();
  for(unsigned r = 0; r < 3; ++r)
    for(unsigned c = 0; c < 3; ++c)
      ICL_TEST_EQ(t(c,r), m(r,c));
}

ICL_REGISTER_TEST("math.dyn.transp_rect", "2x3 transpose = 3x2")
{
  DynMatrix<float> m(3, 2, 0.0f);
  m(0,0)=1; m(1,0)=2; m(2,0)=3;
  m(0,1)=4; m(1,1)=5; m(2,1)=6;
  auto t = m.transp();
  ICL_TEST_EQ(t.cols(), 2u);
  ICL_TEST_EQ(t.rows(), 3u);
  ICL_TEST_EQ(t(0,0), 1.0f); ICL_TEST_EQ(t(1,0), 4.0f);
  ICL_TEST_EQ(t(0,1), 2.0f); ICL_TEST_EQ(t(1,1), 5.0f);
  ICL_TEST_EQ(t(0,2), 3.0f); ICL_TEST_EQ(t(1,2), 6.0f);
}

ICL_REGISTER_TEST("math.dyn.transp_double", "double transpose = original")
{
  DynMatrix<float> m(3, 2, 0.0f);
  for(unsigned i = 0; i < 6; ++i) m[i] = static_cast<float>(i+1);
  auto tt = m.transp().transp();
  for(unsigned i = 0; i < 6; ++i) ICL_TEST_EQ(tt[i], m[i]);
}

// =====================================================================
// DynMatrix — Determinant
// =====================================================================

ICL_REGISTER_TEST("math.dyn.det_2x2", "2x2 determinant")
{
  DynMatrix<float> m(2, 2, 0.0f);
  m(0,0)=1; m(1,0)=2;
  m(0,1)=3; m(1,1)=4;
  ICL_TEST_NEAR(m.det(), -2.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.dyn.det_3x3", "3x3 determinant")
{
  DynMatrix<float> m(3, 3, 0.0f);
  m(0,0)=6;  m(1,0)=1;  m(2,0)=1;
  m(0,1)=4;  m(1,1)=-2; m(2,1)=5;
  m(0,2)=2;  m(1,2)=8;  m(2,2)=7;
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
  DynMatrix<float> m(2, 2, 0.0f);
  m(0,0)=4; m(1,0)=7;
  m(0,1)=2; m(1,1)=6;
  auto R = m * m.inv();
  auto I = DynMatrix<float>::id(2);
  for(unsigned i = 0; i < 4; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-4f);
}

ICL_REGISTER_TEST("math.dyn.inv_3x3", "3x3 inverse round-trip")
{
  DynMatrix<float> m(3, 3, 0.0f);
  m(0,0)=1; m(1,0)=2; m(2,0)=3;
  m(0,1)=0; m(1,1)=1; m(2,1)=4;
  m(0,2)=5; m(1,2)=6; m(2,2)=0;
  auto R = m * m.inv();
  auto I = DynMatrix<float>::id(3);
  for(unsigned i = 0; i < 9; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-4f);
}

ICL_REGISTER_TEST("math.dyn.inv_4x4", "4x4 inverse round-trip")
{
  DynMatrix<float> m(4, 4, 0.0f);
  m(0,0)=1; m(1,0)=0; m(2,0)=2; m(3,0)=-1;
  m(0,1)=3; m(1,1)=0; m(2,1)=0; m(3,1)=5;
  m(0,2)=2; m(1,2)=1; m(2,2)=4; m(3,2)=-3;
  m(0,3)=1; m(1,3)=0; m(2,3)=5; m(3,3)=0;
  auto R = m * m.inv();
  auto I = DynMatrix<float>::id(4);
  for(unsigned i = 0; i < 16; ++i) ICL_TEST_NEAR(R[i], I[i], 1e-3f);
}

// =====================================================================
// DynMatrix — Trace / Diag / Reshape
// =====================================================================

ICL_REGISTER_TEST("math.dyn.trace", "trace of 3x3")
{
  DynMatrix<float> m(3, 3, 0.0f);
  m(0,0)=2; m(1,1)=5; m(2,2)=8;
  ICL_TEST_NEAR(m.trace(), 15.0f, 1e-6f);
}

ICL_REGISTER_TEST("math.dyn.diag", "diagonal extraction")
{
  DynMatrix<float> m(3, 3, 0.0f);
  m(0,0)=1; m(1,1)=5; m(2,2)=9;
  auto d = m.diag();
  ICL_TEST_EQ(d.rows(), 3u);
  ICL_TEST_EQ(d.cols(), 1u);
  ICL_TEST_EQ(d[0], 1.0f);
  ICL_TEST_EQ(d[1], 5.0f);
  ICL_TEST_EQ(d[2], 9.0f);
}

ICL_REGISTER_TEST("math.dyn.reshape", "reshape preserves data")
{
  DynMatrix<float> m(6, 1, 0.0f);
  for(unsigned i = 0; i < 6; ++i) m[i] = static_cast<float>(i);
  m.reshape(3, 2);
  ICL_TEST_EQ(m.cols(), 3u);
  ICL_TEST_EQ(m.rows(), 2u);
  for(unsigned i = 0; i < 6; ++i) ICL_TEST_EQ(m[i], static_cast<float>(i));
}

ICL_REGISTER_TEST("math.dyn.reshape_mismatch_throws", "reshape with wrong dim throws")
{
  DynMatrix<float> m(2, 3, 0.0f);
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

  DynMatrix<float> dA(4, 4, fA.data());
  DynMatrix<float> dB(4, 4, fB.data());
  auto dR = dA * dB;

  for(int i = 0; i < 16; ++i){
    ICL_TEST_NEAR(fR[i], dR[i], 1e-2f);
  }
}

ICL_REGISTER_TEST("math.cross.det_fixed_vs_dyn", "determinant: Fixed and Dyn agree")
{
  FixedMatrix<float,3,3> fm(0.0f);
  fm(0,0)=6;  fm(1,0)=1;  fm(2,0)=1;
  fm(0,1)=4;  fm(1,1)=-2; fm(2,1)=5;
  fm(0,2)=2;  fm(1,2)=8;  fm(2,2)=7;

  DynMatrix<float> dm(3, 3, fm.data());
  ICL_TEST_NEAR(fm.det(), dm.det(), 1e-3f);
}

ICL_REGISTER_TEST("math.cross.transp_fixed_vs_dyn", "transpose: Fixed and Dyn agree")
{
  FixedMatrix<float,3,3> fm(0.0f);
  for(int i = 0; i < 9; ++i) fm[i] = static_cast<float>(i+1);
  auto ft = fm.transp();

  DynMatrix<float> dm(3, 3, fm.data());
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
      ICL_TEST_NEAR(std::abs(Q(j,i)), std::abs(expected), 1e-5f);
      ICL_TEST_NEAR(std::abs(R(j,i)), std::abs(expected), 1e-5f);
    }
}

ICL_REGISTER_TEST("math.dyn.qr_reconstruct", "A = Q*R round-trip")
{
  DynMatrix<float> A(3, 3);
  A(0,0)=12; A(1,0)=-51; A(2,0)=4;
  A(0,1)=6;  A(1,1)=167; A(2,1)=-68;
  A(0,2)=-4; A(1,2)=24;  A(2,2)=-41;

  DynMatrix<float> Q, R;
  A.decompose_QR(Q, R);

  // Verify A = Q*R
  auto QR = Q * R;
  for(unsigned int i = 0; i < A.dim(); i++)
    ICL_TEST_NEAR(QR[i], A[i], 1e-3f);
}

ICL_REGISTER_TEST("math.dyn.qr_orthogonal", "Q^T * Q = I")
{
  DynMatrix<float> A(3, 3);
  A(0,0)=12; A(1,0)=-51; A(2,0)=4;
  A(0,1)=6;  A(1,1)=167; A(2,1)=-68;
  A(0,2)=-4; A(1,2)=24;  A(2,2)=-41;

  DynMatrix<float> Q, R;
  A.decompose_QR(Q, R);

  auto QtQ = Q.transp() * Q;
  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++)
      ICL_TEST_NEAR(QtQ(j,i), (i==j) ? 1.0f : 0.0f, 1e-4f);
}

ICL_REGISTER_TEST("math.dyn.qr_upper_triangular", "R is upper triangular")
{
  DynMatrix<float> A(3, 3);
  A(0,0)=12; A(1,0)=-51; A(2,0)=4;
  A(0,1)=6;  A(1,1)=167; A(2,1)=-68;
  A(0,2)=-4; A(1,2)=24;  A(2,2)=-41;

  DynMatrix<float> Q, R;
  A.decompose_QR(Q, R);

  for(int i = 0; i < 3; i++)
    for(int j = 0; j < i; j++)
      ICL_TEST_NEAR(R(j,i), 0.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.dyn.qr_double", "QR decomposition with double precision")
{
  DynMatrix<double> A(3, 3);
  A(0,0)=12; A(1,0)=-51; A(2,0)=4;
  A(0,1)=6;  A(1,1)=167; A(2,1)=-68;
  A(0,2)=-4; A(1,2)=24;  A(2,2)=-41;

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
  DynMatrix<float> A(3, 3);
  A(0,0)=2;  A(1,0)=1; A(2,0)=1;
  A(0,1)=4;  A(1,1)=3; A(2,1)=3;
  A(0,2)=8;  A(1,2)=7; A(2,2)=9;

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
        if(std::abs(LU(j,i) - A(j,k)) > 1e-3f) { match = false; break; }
      }
      if(match) { found = true; break; }
    }
    ICL_TEST_TRUE(found);
  }
}

ICL_REGISTER_TEST("math.dyn.lu_lower_triangular", "L is lower triangular with unit diagonal")
{
  DynMatrix<float> A(3, 3);
  A(0,0)=2;  A(1,0)=1; A(2,0)=1;
  A(0,1)=4;  A(1,1)=3; A(2,1)=3;
  A(0,2)=8;  A(1,2)=7; A(2,2)=9;

  DynMatrix<float> L, U;
  A.decompose_LU(L, U);

  for(int i = 0; i < 3; i++) {
    for(int j = i + 1; j < 3; j++)
      ICL_TEST_NEAR(L(j,i), 0.0f, 1e-5f);
  }
}

ICL_REGISTER_TEST("math.dyn.lu_upper_triangular", "U is upper triangular")
{
  DynMatrix<float> A(3, 3);
  A(0,0)=2;  A(1,0)=1; A(2,0)=1;
  A(0,1)=4;  A(1,1)=3; A(2,1)=3;
  A(0,2)=8;  A(1,2)=7; A(2,2)=9;

  DynMatrix<float> L, U;
  A.decompose_LU(L, U);

  for(int i = 0; i < 3; i++)
    for(int j = 0; j < i; j++)
      ICL_TEST_NEAR(U(j,i), 0.0f, 1e-5f);
}

// ============================================================
// RQ decomposition test
// ============================================================

ICL_REGISTER_TEST("math.dyn.rq_reconstruct", "R*Q round-trip")
{
  DynMatrix<float> A(3, 3);
  A(0,0)=12; A(1,0)=-51; A(2,0)=4;
  A(0,1)=6;  A(1,1)=167; A(2,1)=-68;
  A(0,2)=-4; A(1,2)=24;  A(2,2)=-41;

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
  DynMatrix<float> A(3, 3);
  A(0,0)=2; A(1,0)=1; A(2,0)=1;
  A(0,1)=4; A(1,1)=3; A(2,1)=3;
  A(0,2)=8; A(1,2)=7; A(2,2)=9;

  DynMatrix<float> b(1, 3);
  b[0] = 1; b[1] = 1; b[2] = 1;

  auto x = A.solve(b);
  auto Ax = A * x;
  for(int i = 0; i < 3; i++)
    ICL_TEST_NEAR(Ax[i], b[i], 1e-3f);
}

ICL_REGISTER_TEST("math.dyn.solve_svd", "solve via svd method")
{
  DynMatrix<float> A(3, 3);
  A(0,0)=2; A(1,0)=1; A(2,0)=1;
  A(0,1)=4; A(1,1)=3; A(2,1)=3;
  A(0,2)=8; A(1,2)=7; A(2,2)=9;

  DynMatrix<float> b(1, 3);
  b[0] = 1; b[1] = 1; b[2] = 1;

  auto x = A.solve(b);
  auto Ax = A * x;
  for(int i = 0; i < 3; i++)
    ICL_TEST_NEAR(Ax[i], b[i], 1e-3f);
}

ICL_REGISTER_TEST("math.dyn.solve_qr", "solve via qr method")
{
  DynMatrix<float> A(3, 3);
  A(0,0)=2; A(1,0)=1; A(2,0)=1;
  A(0,1)=4; A(1,1)=3; A(2,1)=3;
  A(0,2)=8; A(1,2)=7; A(2,2)=9;

  DynMatrix<float> b(1, 3);
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
  DynMatrix<float> A(3, 3);
  A(0,0)=1; A(1,0)=2; A(2,0)=3;
  A(0,1)=0; A(1,1)=1; A(2,1)=4;
  A(0,2)=5; A(1,2)=6; A(2,2)=0;

  auto P = A.pinv();
  auto APA = A * P * A;
  for(unsigned int i = 0; i < A.dim(); i++)
    ICL_TEST_NEAR(APA[i], A[i], 1e-3f);
}

ICL_REGISTER_TEST("math.dyn.pinv_qr", "pseudo-inverse via QR")
{
  DynMatrix<float> A(3, 3);
  A(0,0)=1; A(1,0)=2; A(2,0)=3;
  A(0,1)=0; A(1,1)=1; A(2,1)=4;
  A(0,2)=5; A(1,2)=6; A(2,2)=0;

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
  DynMatrix<float> A(5, 5, 0.0f);
  A(0,0)=2; A(1,1)=3; A(2,2)=4; A(3,3)=5; A(4,4)=6;
  ICL_TEST_NEAR(A.det(), 720.0f, 1e-2f);
}
