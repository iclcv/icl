#include "gtest/gtest.h"
#include "ICLMath/DynMatrix.h"

using icl::math::DynMatrix;

double m3x3r[9] { 0.3987968 ,  0.11602292,  0.88068036,
                  0.36042386,  0.21228021,  0.52000919,
                  0.71980622,  0.79006622,  0.29349811 };

 double m3x3c[9] { 1,  2,  3,
                   4,  5,  6,
                   7,  8,  9 };

TEST(DynMatrixTest, EmptyConstructor) {
  DynMatrix<double> d;
  EXPECT_EQ(d.rows(), 0);
  EXPECT_EQ(d.cols(), 0);
}

TEST(DynMatrixTest, CreateIdentity) {
  DynMatrix<double> d = DynMatrix<double>::id(4);
  EXPECT_EQ(d.rows(), 4);
  EXPECT_EQ(d.cols(), 4);
}

TEST(DynMatrixTest, AddIdentity) {
  DynMatrix<double> i = DynMatrix<double>::id(4);
  EXPECT_EQ(i[0], 1);
  EXPECT_EQ(i[1], 0);
  EXPECT_EQ(i[5], 1);
  EXPECT_EQ((i+i)[1], 0);
  EXPECT_EQ((i+i)[5], 2);
}

TEST(DynMatrixTest, Create3x3) {
  DynMatrix<double> d = DynMatrix<double>(3, 3, m3x3r);
  EXPECT_EQ(d.rows(), 3);
  EXPECT_EQ(d.cols(), 3);
  DynMatrix<double> i = DynMatrix<double>::id(3);
  ASSERT_TRUE(d * i == d);
  ASSERT_FALSE(d * d == d);
  ASSERT_FALSE(d + i == d);
}
