#include <icl/utils/Test.h>
#include <icl/qt/QuickMath.h>
#include <icl/qt/QuickCreate.h>
#include <icl/core/Img.h>

using namespace icl;
using namespace icl::qt;
using namespace icl::core;
using namespace icl::utils;

static Image constant(float val, int w = 10, int h = 10) {
  Image img = zeros(w, h, 1, depth32f);
  img.clear(-1, val);
  return img;
}

// ---- Image + Image ----

ICL_REGISTER_TEST("Quick2.Math.add.images", "10 + 20 = 30") {
  Image a = constant(10), b = constant(20);
  Image c = a + b;
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 30.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.sub.images", "20 - 10 = 10") {
  Image c = constant(20) - constant(10);
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 10.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.mul.images", "3 * 5 = 15") {
  Image c = constant(3) * constant(5);
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 15.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.div.images", "20 / 4 = 5") {
  Image c = constant(20) / constant(4);
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 5.f, 0.01f);
}

// ---- Image + scalar ----

ICL_REGISTER_TEST("Quick2.Math.add.scalar", "img + 5") {
  Image c = constant(10) + 5.f;
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 15.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.sub.scalar", "img - 3") {
  Image c = constant(10) - 3.f;
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 7.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.mul.scalar", "img * 2") {
  Image c = constant(10) * 2.f;
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 20.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.div.scalar", "img / 5") {
  Image c = constant(10) / 5.f;
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 2.f, 0.01f);
}

// ---- scalar + Image ----

ICL_REGISTER_TEST("Quick2.Math.scalar.add", "5 + img") {
  Image c = 5.f + constant(10);
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 15.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.scalar.sub", "100 - img") {
  Image c = 100.f - constant(30);
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 70.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.scalar.div", "100 / img") {
  Image c = 100.f / constant(25);
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 4.f, 0.01f);
}

// ---- unary minus ----

ICL_REGISTER_TEST("Quick2.Math.negate", "-img") {
  Image c = -constant(42);
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), -42.f, 0.01f);
}

// ---- math functions ----

ICL_REGISTER_TEST("Quick2.Math.sqr", "sqr(3) = 9") {
  Image c = sqr(constant(3));
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 9.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.sqrt", "sqrt(25) = 5") {
  Image c = sqrt(constant(25));
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 5.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.abs", "abs(-7) = 7") {
  Image c = abs(constant(-7));
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 7.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.exp", "exp(0) = 1") {
  Image c = exp(constant(0));
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 1.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.ln", "ln(1) = 0") {
  Image c = ln(constant(1));
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 0.f, 0.01f);
}

// ---- logical ----

ICL_REGISTER_TEST("Quick2.Math.logicalOr", "100 || 0 = 255") {
  Image c = constant(100) || constant(0);
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 255.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.logicalAnd", "100 && 0 = 0") {
  Image c = constant(100) && constant(0);
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 0.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.logicalAnd.true", "100 && 200 = 255") {
  Image c = constant(100) && constant(200);
  ICL_TEST_NEAR(c.as<icl32f>()(0, 0, 0), 255.f, 0.01f);
}

// ---- depth preservation ----

ICL_REGISTER_TEST("Quick2.Math.depthPreserved", "8u + 8u stays 8u") {
  Image a = zeros(10, 10, 1, depth8u);
  a.clear(-1, 10);
  Image b = zeros(10, 10, 1, depth8u);
  b.clear(-1, 20);
  Image c = a + b;
  ICL_TEST_EQ(static_cast<int>(c.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_EQ(c.as<icl8u>()(0, 0, 0), icl8u(30));
}
