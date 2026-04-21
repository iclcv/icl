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
