/********************************************************************
**                Image Component Library (ICL)                    **
**  Tests for ICLMath                                              **
********************************************************************/

#include <ICLUtils/Test.h>
#include <ICLMath/FixedMatrix.h>

using namespace icl::utils;
using namespace icl::math;

// --- FixedMatrix 4x4 multiply ---

ICL_REGISTER_TEST("math.fixedmat4x4.mult_identity", "4x4 * identity = same")
{
  FixedMatrix<float,4,4> A;
  for(int i = 0; i < 16; ++i) A[i] = static_cast<float>(i + 1);

  FixedMatrix<float,4,4> I = FixedMatrix<float,4,4>::id();
  FixedMatrix<float,4,4> R = A * I;
  for(int i = 0; i < 16; ++i){
    ICL_TEST_NEAR(R[i], A[i], 1e-5f);
  }
}

ICL_REGISTER_TEST("math.fixedmat4x4.mult_values", "4x4 multiply correctness")
{
  // A = [[1,2,3,4],[5,6,7,8],[9,10,11,12],[13,14,15,16]]  (row-major)
  FixedMatrix<float,4,4> A;
  for(int i = 0; i < 16; ++i) A[i] = static_cast<float>(i + 1);

  // B = identity scaled by 2
  FixedMatrix<float,4,4> B = FixedMatrix<float,4,4>::id();
  for(int i = 0; i < 16; ++i) B[i] *= 2.0f;

  FixedMatrix<float,4,4> R = A * B;
  for(int i = 0; i < 16; ++i){
    ICL_TEST_NEAR(R[i], A[i] * 2.0f, 1e-4f);
  }
}

ICL_REGISTER_TEST("math.fixedmat4x4.mult_general", "4x4 general multiply")
{
  FixedMatrix<float,4,4> A, B;
  for(int i = 0; i < 16; ++i){
    A[i] = static_cast<float>(i + 1);
    B[i] = static_cast<float>(16 - i);
  }
  FixedMatrix<float,4,4> R = A * B;

  // Verify against naive computation
  for(int r = 0; r < 4; ++r){
    for(int c = 0; c < 4; ++c){
      float expected = 0;
      for(int k = 0; k < 4; ++k){
        expected += A(k, r) * B(c, k);
      }
      ICL_TEST_NEAR(R(c, r), expected, 1e-3f);
    }
  }
}

// --- FixedMatrix length ---

ICL_REGISTER_TEST("math.fixedmat.length_l2", "L2 norm")
{
  FixedMatrix<float,3,1> v;
  v[0] = 3; v[1] = 4; v[2] = 0;
  ICL_TEST_NEAR(v.length(), 5.0, 1e-10);
}

ICL_REGISTER_TEST("math.fixedmat.length_l1", "L1 norm")
{
  FixedMatrix<float,3,1> v;
  v[0] = -3; v[1] = 4; v[2] = -1;
  ICL_TEST_NEAR(v.length(1), 8.0, 1e-10);
}

// --- FixedMatrix det ---

ICL_REGISTER_TEST("math.fixedmat.det_2x2", "2x2 determinant")
{
  FixedMatrix<float,2,2> m;
  m(0,0) = 1; m(1,0) = 2;
  m(0,1) = 3; m(1,1) = 4;
  ICL_TEST_NEAR(m.det(), -2.0f, 1e-5f);
}

ICL_REGISTER_TEST("math.fixedmat.det_3x3", "3x3 determinant")
{
  FixedMatrix<float,3,3> m;
  m(0,0)=6;  m(1,0)=1;  m(2,0)=1;
  m(0,1)=4;  m(1,1)=-2; m(2,1)=5;
  m(0,2)=2;  m(1,2)=8;  m(2,2)=7;
  ICL_TEST_NEAR(m.det(), -306.0f, 1e-3f);
}

// --- FixedMatrix inv ---

ICL_REGISTER_TEST("math.fixedmat.inv_3x3", "3x3 inverse round-trip")
{
  FixedMatrix<float,3,3> m;
  m(0,0)=1; m(1,0)=2; m(2,0)=3;
  m(0,1)=0; m(1,1)=1; m(2,1)=4;
  m(0,2)=5; m(1,2)=6; m(2,2)=0;
  auto inv = m.inv();
  auto I = m * inv;
  auto eye = FixedMatrix<float,3,3>::id();
  for(int i = 0; i < 9; ++i){
    ICL_TEST_NEAR(I[i], eye[i], 1e-4f);
  }
}
