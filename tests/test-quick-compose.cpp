#include <icl/utils/Test.h>
#include <icl/qt/QuickCompose.h>
#include <icl/qt/QuickCreate.h>
#include <icl/core/Img.h>

using namespace icl;
using namespace icl::qt;
using namespace icl::core;
using namespace icl::utils;

// ---- horizontal concatenation ----

ICL_REGISTER_TEST("Quick2.Compose.hconcat", "width = a.w + b.w") {
  Image a = zeros(30, 20, 1);
  Image b = zeros(40, 20, 1);
  Image c = (a, b);
  ICL_TEST_EQ(c.getWidth(), 70);
  ICL_TEST_EQ(c.getHeight(), 20);
}

ICL_REGISTER_TEST("Quick2.Compose.hconcat.diffHeight", "height = max(a.h, b.h)") {
  Image a = zeros(30, 10, 1);
  Image b = zeros(40, 20, 1);
  Image c = (a, b);
  ICL_TEST_EQ(c.getHeight(), 20);
}

// ---- vertical concatenation ----

ICL_REGISTER_TEST("Quick2.Compose.vconcat", "height = a.h + b.h") {
  Image a = zeros(20, 30, 1);
  Image b = zeros(20, 40, 1);
  Image c = a % b;
  ICL_TEST_EQ(c.getWidth(), 20);
  ICL_TEST_EQ(c.getHeight(), 70);
}

// ---- channel concatenation ----

ICL_REGISTER_TEST("Quick2.Compose.chanconcat", "channels = a.ch + b.ch") {
  Image a = zeros(20, 20, 1);
  Image b = zeros(20, 20, 2);
  Image c = a | b;
  ICL_TEST_EQ(c.getChannels(), 3);
  ICL_TEST_EQ(c.getSize(), Size(20, 20));
}

// ---- ROI ----

ICL_REGISTER_TEST("Quick2.Compose.roi.assign", "roi copy writes to ROI region") {
  Image a = zeros(20, 20, 1, depth32f);
  Image b = ones(10, 10, 1, depth32f);
  a.setROI(Rect(5, 5, 10, 10));
  roi(a) = b;
  a.setFullROI();
  // Pixel inside ROI should be 1
  ICL_TEST_NEAR(a.as<icl32f>()(7, 7, 0), 1.f, 0.01f);
  // Pixel outside ROI should still be 0
  ICL_TEST_NEAR(a.as<icl32f>()(0, 0, 0), 0.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Compose.roi.fillVal", "roi fill with scalar") {
  Image a = zeros(20, 20, 1, depth32f);
  a.setROI(Rect(0, 0, 10, 10));
  roi(a) = 42.f;
  a.setFullROI();
  ICL_TEST_NEAR(a.as<icl32f>()(5, 5, 0), 42.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Compose.data.noSideEffect", "data() does not change original ROI") {
  Image a = zeros(20, 20, 1);
  a.setROI(Rect(5, 5, 10, 10));
  Rect origROI = a.getROI();
  auto d = data(a); // should not change a's ROI
  (void)d;
  ICL_TEST_EQ(a.getROI(), origROI);
}

// ---- Depth promotion ----

ICL_REGISTER_TEST("Quick2.Compose.hconcat.depthPromotion", "concat 8u with 32f promotes to 32f") {
  Image a = zeros(10, 10, 1, depth8u);
  Image b = zeros(10, 10, 1, depth32f);
  Image c = (a, b);
  ICL_TEST_EQ(static_cast<int>(c.getDepth()), static_cast<int>(depth32f));
}

// ---- Content verification ----

ICL_REGISTER_TEST("Quick2.Compose.hconcat.content", "hconcat preserves pixel data from both images") {
  Image a = zeros(10, 10, 1, depth32f);
  a.as<icl32f>()(5, 5, 0) = 42.f;
  Image b = zeros(10, 10, 1, depth32f);
  b.as<icl32f>()(3, 3, 0) = 77.f;
  Image c = (a, b);
  ICL_TEST_EQ(c.getWidth(), 20);
  // Left image pixel (5,5) should be at (5,5) in result
  ICL_TEST_NEAR(c.as<icl32f>()(5, 5, 0), 42.f, 0.01f);
  // Right image pixel (3,3) should be at (13,3) in result (offset by a's width=10)
  ICL_TEST_NEAR(c.as<icl32f>()(13, 3, 0), 77.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Compose.vconcat.content", "vconcat preserves pixel data from both images") {
  Image a = zeros(10, 10, 1, depth32f);
  a.as<icl32f>()(5, 5, 0) = 42.f;
  Image b = zeros(10, 10, 1, depth32f);
  b.as<icl32f>()(3, 3, 0) = 77.f;
  Image c = a % b;
  ICL_TEST_EQ(c.getHeight(), 20);
  // Top image pixel
  ICL_TEST_NEAR(c.as<icl32f>()(5, 5, 0), 42.f, 0.01f);
  // Bottom image pixel (3,3) should be at (3,13) in result (offset by a's height=10)
  ICL_TEST_NEAR(c.as<icl32f>()(3, 13, 0), 77.f, 0.01f);
}

// ---- Nested concat ----

ICL_REGISTER_TEST("Quick2.Compose.hconcat.triple", "(a, b, c) chains correctly") {
  Image a = zeros(10, 10, 1, depth32f);
  Image b = zeros(20, 10, 1, depth32f);
  Image c = zeros(15, 10, 1, depth32f);
  Image result = (a, b, c);
  ICL_TEST_EQ(result.getWidth(), 45); // 10 + 20 + 15
  ICL_TEST_EQ(result.getHeight(), 10);
}

ICL_REGISTER_TEST("Quick2.Compose.vconcat.triple", "a % b % c chains correctly") {
  Image a = zeros(10, 10, 1, depth32f);
  Image b = zeros(10, 20, 1, depth32f);
  Image c = zeros(10, 15, 1, depth32f);
  Image result = a % b % c;
  ICL_TEST_EQ(result.getWidth(), 10);
  ICL_TEST_EQ(result.getHeight(), 45); // 10 + 20 + 15
}

// ---- Null image concat ----

ICL_REGISTER_TEST("Quick2.Compose.hconcat.nullFirst", "null , a returns a") {
  Image null;
  Image a = zeros(10, 10, 1, depth32f);
  Image c = (null, a);
  ICL_TEST_TRUE(!c.isNull());
  ICL_TEST_EQ(c.getWidth(), 10);
  ICL_TEST_EQ(c.getHeight(), 10);
}

ICL_REGISTER_TEST("Quick2.Compose.hconcat.nullSecond", "a , null returns a") {
  Image a = zeros(10, 10, 1, depth32f);
  Image null;
  Image c = (a, null);
  ICL_TEST_TRUE(!c.isNull());
  ICL_TEST_EQ(c.getWidth(), 10);
}
