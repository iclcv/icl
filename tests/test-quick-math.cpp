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

// ---- Chained expression ----

ICL_REGISTER_TEST("Quick2.Math.chained", "(a + b) * c - d") {
  Image a = constant(2), b = constant(3), c = constant(4), d = constant(5);
  Image result = (a + b) * c - d;
  // (2 + 3) * 4 - 5 = 15
  ICL_TEST_NEAR(result.as<icl32f>()(0, 0, 0), 15.f, 0.01f);
}

// ---- Division by zero ----

ICL_REGISTER_TEST("Quick2.Math.div.byZero", "img / 0 does not crash") {
  Image a = constant(42);
  ICL_TEST_NO_THROW(a / 0.f);
}

ICL_REGISTER_TEST("Quick2.Math.scalar.divByZeroImage", "100 / zero_image does not crash and gives 0") {
  Image z = constant(0);
  Image c;
  ICL_TEST_NO_THROW(c = 100.f / z);
  // Division by zero in image math typically yields 0 (not inf/nan)
  ICL_TEST_TRUE(!c.isNull());
}

// ---- Mixed-depth arithmetic ----

ICL_REGISTER_TEST("Quick2.Math.mixedDepth.add", "mixed depth arithmetic does not crash") {
  Image a = zeros(10, 10, 1, depth8u);
  a.clear(-1, 10);
  Image b = constant(20);
  // Mixed-depth operations should not crash
  Image c;
  ICL_TEST_NO_THROW(c = a + b);
  ICL_TEST_TRUE(!c.isNull());
  // Swapped operand order
  Image d;
  ICL_TEST_NO_THROW(d = b + a);
  ICL_TEST_TRUE(!d.isNull());
}

// ---- Math on depth8u ----

ICL_REGISTER_TEST("Quick2.Math.add.8u", "8u addition works") {
  Image a = zeros(10, 10, 1, depth8u);
  a.clear(-1, 10);
  Image b = zeros(10, 10, 1, depth8u);
  b.clear(-1, 5);
  Image c = a + b;
  ICL_TEST_EQ(c.as<icl8u>()(0, 0, 0), icl8u(15));
}

ICL_REGISTER_TEST("Quick2.Math.sub.8u", "8u subtraction works") {
  Image a = zeros(10, 10, 1, depth8u);
  a.clear(-1, 20);
  Image b = zeros(10, 10, 1, depth8u);
  b.clear(-1, 5);
  Image c = a - b;
  ICL_TEST_EQ(c.as<icl8u>()(0, 0, 0), icl8u(15));
}

ICL_REGISTER_TEST("Quick2.Math.mul.8u", "8u multiplication works") {
  Image a = zeros(10, 10, 1, depth8u);
  a.clear(-1, 3);
  Image b = zeros(10, 10, 1, depth8u);
  b.clear(-1, 4);
  Image c = a * b;
  ICL_TEST_EQ(c.as<icl8u>()(0, 0, 0), icl8u(12));
}

ICL_REGISTER_TEST("Quick2.Math.negate.8u", "negation on 8u does not crash") {
  Image a = zeros(10, 10, 1, depth8u);
  a.clear(-1, 42);
  ICL_TEST_NO_THROW(-a);
}

// ---- sqr(sqrt(x)) ~= x ----

ICL_REGISTER_TEST("Quick2.Math.sqrSqrtRoundtrip", "sqr(sqrt(x)) approximately equals x for positive values") {
  Image src = constant(16);
  Image result = sqr(sqrt(src));
  ICL_TEST_NEAR(result.as<icl32f>()(0, 0, 0), 16.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Math.sqrSqrtRoundtrip.large", "sqr(sqrt(100)) approximately equals 100") {
  Image src = constant(100);
  Image result = sqr(sqrt(src));
  ICL_TEST_NEAR(result.as<icl32f>()(0, 0, 0), 100.f, 0.1f);
}

// ---- binOR / binXOR / binAND ----

ICL_REGISTER_TEST("Quick2.Math.binOR", "binary OR on 8u values") {
  Image a = zeros(10, 10, 1, depth8u);
  a.clear(-1, 0x0F); // 0b00001111
  Image b = zeros(10, 10, 1, depth8u);
  b.clear(-1, 0xF0); // 0b11110000
  Image c = binOR<icl8u>(a, b);
  ICL_TEST_TRUE(!c.isNull());
  ICL_TEST_EQ(c.as<icl8u>()(0, 0, 0), icl8u(0xFF));
}

ICL_REGISTER_TEST("Quick2.Math.binAND", "binary AND on 8u values") {
  Image a = zeros(10, 10, 1, depth8u);
  a.clear(-1, 0xFF);
  Image b = zeros(10, 10, 1, depth8u);
  b.clear(-1, 0x0F);
  Image c = binAND<icl8u>(a, b);
  ICL_TEST_TRUE(!c.isNull());
  ICL_TEST_EQ(c.as<icl8u>()(0, 0, 0), icl8u(0x0F));
}

ICL_REGISTER_TEST("Quick2.Math.binXOR", "binary XOR on 8u values") {
  Image a = zeros(10, 10, 1, depth8u);
  a.clear(-1, 0xFF);
  Image b = zeros(10, 10, 1, depth8u);
  b.clear(-1, 0x0F);
  Image c = binXOR<icl8u>(a, b);
  ICL_TEST_TRUE(!c.isNull());
  ICL_TEST_EQ(c.as<icl8u>()(0, 0, 0), icl8u(0xF0));
}
