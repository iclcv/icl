#include <icl/utils/Test.h>
#include <icl/qt/QuickFilter.h>
#include <icl/qt/QuickCreate.h>
#include <icl/core/Img.h>

using namespace icl;
using namespace icl::qt;
using namespace icl::core;
using namespace icl::utils;

static Image testImage(int w = 64, int h = 48, int ch = 3, depth d = depth32f) {
  Image img = zeros(w, h, ch, d);
  img.visit([](auto &typed) {
    for(int c = 0; c < typed.getChannels(); ++c)
      for(int i = 0; i < typed.getDim(); ++i)
        typed.getData(c)[i] = static_cast<typename std::remove_reference_t<decltype(typed)>::type>(i % 256);
  });
  return img;
}

// ---- filter ----

ICL_REGISTER_TEST("Quick2.Filter.filter.gauss", "gauss filter produces output") {
  Image src = testImage();
  Image dst = filter(src, "gauss");
  ICL_TEST_TRUE(!dst.isNull());
  ICL_TEST_EQ(dst.getChannels(), 3);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth32f));
}

ICL_REGISTER_TEST("Quick2.Filter.filter.sobelx", "sobelx produces output") {
  Image dst = filter(testImage(), "sobelx");
  ICL_TEST_TRUE(!dst.isNull());
}

ICL_REGISTER_TEST("Quick2.Filter.filter.unknown", "unknown filter returns null") {
  Image dst = filter(testImage(), "doesnotexist");
  ICL_TEST_TRUE(dst.isNull());
}

ICL_REGISTER_TEST("Quick2.Filter.filter.depth8u", "filter preserves depth8u") {
  Image src = testImage(64, 48, 1, depth8u);
  Image dst = filter(src, "median");
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
}

// ---- blur ----

ICL_REGISTER_TEST("Quick2.Filter.blur.radius0", "radius 0 returns copy") {
  Image src = testImage(32, 32, 1);
  Image dst = blur(src, 0);
  ICL_TEST_EQ(dst.getSize(), src.getSize());
}

ICL_REGISTER_TEST("Quick2.Filter.blur.radius1", "radius 1 is gauss") {
  Image src = testImage(32, 32, 1);
  Image dst = blur(src, 1);
  ICL_TEST_TRUE(!dst.isNull());
}

ICL_REGISTER_TEST("Quick2.Filter.blur.radius3", "larger radius works") {
  Image dst = blur(testImage(64, 64, 1), 3);
  ICL_TEST_TRUE(!dst.isNull());
}

// ---- cc / rgb / gray ----

ICL_REGISTER_TEST("Quick2.Filter.cc.toGray", "RGB to gray reduces channels") {
  Image src = testImage(32, 32, 3);
  src.setFormat(formatRGB);
  Image dst = gray(src);
  ICL_TEST_EQ(dst.getChannels(), 1);
  ICL_TEST_EQ(static_cast<int>(dst.getFormat()), static_cast<int>(formatGray));
}

ICL_REGISTER_TEST("Quick2.Filter.cc.toRGB", "gray to RGB expands channels") {
  Image src = zeros(32, 32, 1);
  src.setFormat(formatGray);
  Image dst = rgb(src);
  ICL_TEST_EQ(dst.getChannels(), 3);
}

// ---- scale ----

ICL_REGISTER_TEST("Quick2.Filter.scale.factor", "scale by factor") {
  Image src = testImage(100, 100, 1);
  Image dst = scale(src, 0.5f);
  ICL_TEST_EQ(dst.getWidth(), 50);
  ICL_TEST_EQ(dst.getHeight(), 50);
}

ICL_REGISTER_TEST("Quick2.Filter.scale.absolute", "scale to absolute size") {
  Image dst = scale(testImage(100, 100, 1), 200, 200);
  ICL_TEST_EQ(dst.getWidth(), 200);
  ICL_TEST_EQ(dst.getHeight(), 200);
}

// ---- channel ----

ICL_REGISTER_TEST("Quick2.Filter.channel", "extract single channel") {
  Image src = testImage(32, 32, 3);
  Image ch = channel(src, 1);
  ICL_TEST_EQ(ch.getChannels(), 1);
  ICL_TEST_EQ(ch.getSize(), src.getSize());
}

// ---- thresh ----

ICL_REGISTER_TEST("Quick2.Filter.thresh", "threshold binarizes") {
  Image src = zeros(10, 10, 1, depth32f);
  auto &f = src.as<icl32f>();
  f(5, 5, 0) = 200;
  f(3, 3, 0) = 50;
  Image dst = thresh(src, 100);
  const auto &r = dst.as<icl32f>();
  ICL_TEST_NEAR(r(5, 5, 0), 255.f, 0.01f);
  ICL_TEST_NEAR(r(3, 3, 0), 0.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Filter.thresh.depth8u", "thresh works on 8u") {
  Image src = zeros(10, 10, 1, depth8u);
  src.as<icl8u>()(5, 5, 0) = 200;
  Image dst = thresh(src, 100);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_EQ(dst.as<icl8u>()(5, 5, 0), icl8u(255));
}

// ---- copy / copyroi / norm ----

ICL_REGISTER_TEST("Quick2.Filter.copy", "copy is data-independent") {
  Image src = zeros(32, 32, 1, depth32f);
  src.as<icl32f>()(5, 5, 0) = 42;
  Image dst = copy(src);
  ICL_TEST_EQ(dst.getSize(), src.getSize());
  ICL_TEST_NEAR(dst.as<icl32f>()(5, 5, 0), 42.f, 0.01f);
  // Modifying dst doesn't affect src
  dst.as<icl32f>()(5, 5, 0) = 99;
  ICL_TEST_NEAR(src.as<icl32f>()(5, 5, 0), 42.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Filter.norm", "normalize to 0-255") {
  Image src = zeros(10, 10, 1, depth32f);
  src.as<icl32f>()(0, 0, 0) = -100;
  src.as<icl32f>()(5, 5, 0) = 500;
  Image dst = norm(src);
  ICL_TEST_NEAR(dst.getMin(0), 0.0, 0.5);
  ICL_TEST_NEAR(dst.getMax(0), 255.0, 0.5);
}

// ---- rotate ----

ICL_REGISTER_TEST("Quick2.Filter.rotate", "rotation produces output") {
  Image dst = rotate(testImage(64, 64, 1), 45.0f);
  ICL_TEST_TRUE(!dst.isNull());
}

// ---- flip ----

ICL_REGISTER_TEST("Quick2.Filter.flipx", "horizontal flip") {
  Image src = zeros(10, 10, 1, depth32f);
  src.as<icl32f>()(0, 0, 0) = 100;
  Image dst = flipx(src);
  ICL_TEST_NEAR(dst.as<icl32f>()(9, 0, 0), 100.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Filter.flipy", "vertical flip") {
  Image src = zeros(10, 10, 1, depth32f);
  src.as<icl32f>()(0, 0, 0) = 100;
  Image dst = flipy(src);
  ICL_TEST_NEAR(dst.as<icl32f>()(0, 9, 0), 100.f, 0.01f);
}
