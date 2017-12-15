#include "gtest/gtest.h"
#include "ICLMath/DynMatrix.h"

using icl::math::DynMatrix;

#define DOUBLE_MARGIN 1e-15

double m3x3r0[9] {1, 2, 3,
                  1, 0, 1,
                  3, 4, 7 };

double m3x3r1[9] { 0.3987968 ,  0.11602292,  0.88068036,
                   0.36042386,  0.21228021,  0.52000919,
                   0.71980622,  0.79006622,  0.29349811 };
double m3x3r2[9] { 0.950307  ,  0.53584179,  0.05760179,
                   0.25855725,  0.74725683,  0.24040829,
                   0.92026487,  0.05839824,  0.41791317 };
// addition
double m3x3r3[9] { 1.3491038 ,  0.65186471,  0.93828215,
                   0.61898111,  0.95953704,  0.76041748,
                   1.64007109,  0.84846446,  0.71141128 };

// element wise
double m3x3r4[9] { 0.37897939061759999824,  0.06216992913382679564, 0.05072876515384440455,
                   0.09319020207598499583,  0.15862783679633429834, 0.12501452015218508795,
                   0.66241237747349146492,  0.04613847673145279882, 0.12265672553910869635 };
// dot product
double m3x3r5[9] { 1.21943715475672331827,  0.35182109359138197258, 0.41891220232861997586,
                   0.87594609408619783508,  0.38212562457726922949, 0.28911367080368260973,
                   1.15841023871503079334,  0.99322440549254276121, 0.35405732120120630979 };

double m4x4r1[16] { 0.36228671,  0.18756258,  0.47559801,  0.23159873,
                    0.16747942,  0.8166452 ,  0.26111905,  0.25612094,
                    0.55120608,  0.39206604,  0.16824783,  0.3067865 ,
                    0.49421738,  0.36987849,  0.63399134,  0.31633955 };
double m4x4r2[16] { 0.54490757,  0.32095175,  0.96347888,  0.30934882,
                    0.02317498,  0.20314625,  0.41469638,  0.73932676,
                    0.48979231,  0.34369873,  0.13328761,  0.88741035,
                    0.90179324,  0.13721667,  0.46019886,  0.93384174 };

TEST(DynMatrixTest, EmptyConstructor) {
  DynMatrix<double> d;
  EXPECT_EQ(0, d.rows());
  EXPECT_EQ(0, d.cols());
}

TEST(DynMatrixTest, CreateIdentity) {
  DynMatrix<double> d = DynMatrix<double>::id(4);
  EXPECT_EQ(4, d.rows());
  EXPECT_EQ(4, d.cols());
}

TEST(DynMatrixTest, AddIdentity) {
  DynMatrix<double> i = DynMatrix<double>::id(4);
  EXPECT_EQ(1, i[0]);
  EXPECT_EQ(0, i[1]);
  EXPECT_EQ(1, i[5]);
  EXPECT_EQ(0, (i+i)[1]);
  EXPECT_EQ(2, (i+i)[5]);
}

TEST(DynMatrixTest, Create3x3) {
  DynMatrix<double> d(3, 3, m3x3r1);
  const DynMatrix<double> i = DynMatrix<double>::id(3);
  const DynMatrix<double> dShallowCopy(3, 3, d.data(), false);
  const DynMatrix<double> dDeepCopy(3, 3, d.data());
  const DynMatrix<double> dValue(3, 3, 42);
  DynMatrix<double> dCopyTarget(1, 1, NULL, false);
  DynMatrix<double> dCopyTarget2;

  EXPECT_EQ(3, d.rows());
  EXPECT_EQ(3, d.cols());
  EXPECT_FALSE(d.isNull());
  EXPECT_TRUE(dCopyTarget.isNull());

  EXPECT_EQ(d.data(), dShallowCopy.data());
  EXPECT_NE(d.data(), dDeepCopy.data());
  EXPECT_EQ(dShallowCopy.at(1, 1), dShallowCopy.at(1, 1));
  for(unsigned int i=0; i<9; ++i) ASSERT_EQ(42, dValue[i]);

  ASSERT_TRUE(d * i == d);
  ASSERT_TRUE(d * 2 == d + d);
  ASSERT_FALSE(d * d == d);
  ASSERT_FALSE(d + i == d);

  dCopyTarget = dShallowCopy; // creates another shallow copy
  dCopyTarget2 = dDeepCopy; // creates a deep copy and deletes old data
  ASSERT_EQ(d.data(), dCopyTarget.data());
  ASSERT_EQ(d.at(1,1), dCopyTarget2.at(1,1));
  ASSERT_NE(d.data(), dCopyTarget2.data());
}

TEST(DynMatrixTest, BasicOp3x3) {
  DynMatrix<double> d1 = DynMatrix<double>(3, 3, m3x3r1);
  DynMatrix<double> d2 = DynMatrix<double>(3, 3, m3x3r2);
  DynMatrix<double> d3 = DynMatrix<double>(3, 3, m3x3r3);
  DynMatrix<double> d4 = DynMatrix<double>(3, 3, m3x3r4);
  DynMatrix<double> d5 = DynMatrix<double>(3, 3, m3x3r5);

  DynMatrix<double> sum = d1 + d2;
  DynMatrix<double> diff = d3 - d1;
  DynMatrix<double> mult = d1.elementwise_mult(d2);
  DynMatrix<double> dot = d1 * d2;

  for(unsigned int i=0; i<9; ++i) ASSERT_DOUBLE_EQ(d3[i], sum[i]);
  for(unsigned int i=0; i<9; ++i) ASSERT_NEAR(d2[i], diff[i], DOUBLE_MARGIN);
  for(unsigned int i=0; i<9; ++i) ASSERT_NEAR(d4[i], mult[i], DOUBLE_MARGIN);
  for(unsigned int i=0; i<9; ++i) ASSERT_NEAR(d5[i], dot[i],  DOUBLE_MARGIN);
}

TEST(DynMatrixTest, ErrorCheck) {
  const DynMatrix<double> d0 = DynMatrix<double>(3, 3, m3x3r0);
  const DynMatrix<double> d1 = DynMatrix<double>(3, 3, m3x3r1);
  const DynMatrix<double> d2 = DynMatrix<double>(4, 5, m4x4r1);

  EXPECT_THROW(DynMatrix<double>(0, 0), icl::math::InvalidMatrixDimensionException);
  EXPECT_THROW(d1 + d2, icl::math::IncompatibleMatrixDimensionException);
  EXPECT_THROW(d1 - d2, icl::math::IncompatibleMatrixDimensionException);
  EXPECT_THROW(d1 * d2, icl::math::IncompatibleMatrixDimensionException);
  EXPECT_THROW(d1.elementwise_mult(d2), icl::math::IncompatibleMatrixDimensionException);
  EXPECT_THROW(d1.elementwise_div(d2), icl::math::IncompatibleMatrixDimensionException);
  EXPECT_NO_THROW(d1.at(0, 0));
  EXPECT_THROW(d1.at(3, 3), icl::math::InvalidIndexException);
  EXPECT_THROW(d0.inv(), icl::math::SingularMatrixException);
  EXPECT_NO_THROW(d1.inv());
}
