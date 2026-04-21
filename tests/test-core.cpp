#include <icl/utils/Test.h>
#include <icl/core/Image.h>
#include <icl/core/Img.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

// ---- Image: construction and null ----

ICL_REGISTER_TEST("Image.null", "default-constructed image is null") {
  Image img;
  ICL_TEST_EQ(img.isNull(), true);
  ICL_TEST_EQ(static_cast<bool>(img), false);
}

ICL_REGISTER_TEST("Image.construct", "construct with size/depth/channels") {
  Image img(Size(320, 240), depth8u, 3, formatRGB);
  ICL_TEST_EQ(img.isNull(), false);
  ICL_TEST_EQ(img.getWidth(), 320);
  ICL_TEST_EQ(img.getHeight(), 240);
  ICL_TEST_EQ(img.getChannels(), 3);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_EQ(static_cast<int>(img.getFormat()), static_cast<int>(formatRGB));
  ICL_TEST_EQ(img.getDim(), 320 * 240);
}

ICL_REGISTER_TEST("Image.construct_defaults", "default channels and format") {
  Image img(Size(64, 64), depth32f);
  ICL_TEST_EQ(img.getChannels(), 1);
  ICL_TEST_EQ(static_cast<int>(img.getFormat()), static_cast<int>(formatMatrix));
}

// ---- Image: is<T> and as<T> ----

ICL_REGISTER_TEST("Image.is", "type check with is<T>") {
  Image img(Size(10, 10), depth8u);
  ICL_TEST_EQ(img.is<icl8u>(), true);
  ICL_TEST_EQ(img.is<icl32f>(), false);
  ICL_TEST_EQ(img.is<icl64f>(), false);
}

ICL_REGISTER_TEST("Image.as", "typed access with as<T>") {
  Image img(Size(4, 4), depth32f, 1);
  Img<icl32f> &typed = img.as<icl32f>();
  typed(0, 0, 0) = 42.0f;
  ICL_TEST_NEAR(img.as<icl32f>()(0, 0, 0), 42.0f, 1e-6f);
}

// ---- Image: shallow copy ----

ICL_REGISTER_TEST("Image.shallow_copy", "copy shares data") {
  Image a(Size(8, 8), depth32f, 1);
  a.as<icl32f>()(0, 0, 0) = 7.0f;

  Image b = a;  // shallow copy
  ICL_TEST_NEAR(b.as<icl32f>()(0, 0, 0), 7.0f, 1e-6f);

  // Modify through b, visible in a
  b.as<icl32f>()(0, 0, 0) = 99.0f;
  ICL_TEST_NEAR(a.as<icl32f>()(0, 0, 0), 99.0f, 1e-6f);
}

// ---- Image: deep copy ----

ICL_REGISTER_TEST("Image.deep_copy", "deepCopy creates independent data") {
  Image a(Size(8, 8), depth32f, 1);
  a.as<icl32f>()(0, 0, 0) = 7.0f;

  Image b = a.deepCopy();
  ICL_TEST_NEAR(b.as<icl32f>()(0, 0, 0), 7.0f, 1e-6f);

  b.as<icl32f>()(0, 0, 0) = 99.0f;
  ICL_TEST_NEAR(a.as<icl32f>()(0, 0, 0), 7.0f, 1e-6f);  // unchanged
}

// ---- Image: ensureCompatible ----

ICL_REGISTER_TEST("Image.ensureCompatible_null", "ensureCompatible on null image") {
  Image img;
  img.ensureCompatible(depth8u, Size(100, 100), 3);
  ICL_TEST_EQ(img.isNull(), false);
  ICL_TEST_EQ(img.getWidth(), 100);
  ICL_TEST_EQ(img.getChannels(), 3);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth8u));
}

ICL_REGISTER_TEST("Image.ensureCompatible_same_depth", "same depth adapts in-place") {
  Image img(Size(10, 10), depth8u, 1);
  ImgBase *origPtr = img.ptr();
  img.ensureCompatible(depth8u, Size(20, 20), 3);
  ICL_TEST_EQ(img.ptr(), origPtr);  // same allocation
  ICL_TEST_EQ(img.getWidth(), 20);
  ICL_TEST_EQ(img.getChannels(), 3);
}

ICL_REGISTER_TEST("Image.ensureCompatible_depth_change", "depth change reallocates") {
  Image img(Size(10, 10), depth8u, 1);
  ImgBase *origPtr = img.ptr();
  img.ensureCompatible(depth32f, Size(10, 10), 1);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth32f));
  ICL_TEST_EQ(img.ptr() != origPtr, true);  // different allocation
}

// ---- Image: convert ----

ICL_REGISTER_TEST("Image.convert", "depth conversion") {
  Image img(Size(4, 4), depth8u, 1);
  img.as<icl8u>()(0, 0, 0) = 200;

  Image f = img.convert(depth32f);
  ICL_TEST_EQ(f.is<icl32f>(), true);
  ICL_TEST_NEAR(f.as<icl32f>()(0, 0, 0), 200.0f, 1e-3f);
}

// ---- Image: visit ----

ICL_REGISTER_TEST("Image.visit_mut", "mutable visit") {
  Image img(Size(4, 4), depth32f, 1);
  img.visit([](auto &typed) {
    typed(0, 0, 0) = 123;
  });
  ICL_TEST_NEAR(img.as<icl32f>()(0, 0, 0), 123.0f, 1e-6f);
}

ICL_REGISTER_TEST("Image.visit_const", "const visit reads correctly") {
  Image img(Size(4, 4), depth8u, 1);
  img.as<icl8u>()(0, 0, 0) = 42;

  const Image &cimg = img;
  int val = cimg.visit([](const auto &typed) -> int {
    return static_cast<int>(typed(0, 0, 0));
  });
  ICL_TEST_EQ(val, 42);
}

ICL_REGISTER_TEST("Image.visit_all_depths", "visit works for all 5 depths") {
  depth depths[] = {depth8u, depth16s, depth32s, depth32f, depth64f};
  for(auto d : depths){
    Image img(Size(2, 2), d, 1);
    img.visit([](auto &typed) { typed(0, 0, 0) = 1; });
    int v = img.visit([](const auto &typed) -> int {
      return static_cast<int>(typed(0, 0, 0));
    });
    ICL_TEST_EQ(v, 1);
  }
}

// ---- Image: ROI ----

ICL_REGISTER_TEST("Image.roi", "ROI set/get/full") {
  Image img(Size(100, 100), depth8u, 1);
  ICL_TEST_EQ(img.hasFullROI(), true);

  img.setROI(Rect(10, 10, 50, 50));
  ICL_TEST_EQ(img.hasFullROI(), false);
  ICL_TEST_EQ(img.getROISize().width, 50);
  ICL_TEST_EQ(img.getROIOffset().x, 10);

  img.setFullROI();
  ICL_TEST_EQ(img.hasFullROI(), true);
}

// ---- Image: clear ----

ICL_REGISTER_TEST("Image.clear", "clear fills with zero") {
  Image img(Size(4, 4), depth32f, 1);
  img.as<icl32f>()(0, 0, 0) = 99.0f;
  img.clear();
  ICL_TEST_NEAR(img.as<icl32f>()(0, 0, 0), 0.0f, 1e-6f);
}

// ---- Image: selectChannel ----

ICL_REGISTER_TEST("Image.selectChannel", "extract single channel") {
  Image img(Size(4, 4), depth8u, 3, formatRGB);
  img.as<icl8u>()(0, 0, 1) = 42;  // green channel

  Image ch = img.selectChannel(1);
  ICL_TEST_EQ(ch.getChannels(), 1);
  ICL_TEST_EQ(static_cast<int>(ch.as<icl8u>()(0, 0, 0)), 42);
}

// ---- Image: swap ----

ICL_REGISTER_TEST("Image.swap", "swap exchanges contents") {
  Image a(Size(10, 10), depth8u, 1);
  Image b(Size(20, 20), depth32f, 3);
  a.swap(b);
  ICL_TEST_EQ(a.getWidth(), 20);
  ICL_TEST_EQ(a.is<icl32f>(), true);
  ICL_TEST_EQ(b.getWidth(), 10);
  ICL_TEST_EQ(b.is<icl8u>(), true);
}

// ---- Image: visitWith ----

ICL_REGISTER_TEST("Image.visitWith", "binary visit dispatches correctly") {
  Image a(Size(4, 4), depth32f, 1);
  Image b(Size(4, 4), depth32f, 1);
  a.as<icl32f>()(0, 0, 0) = 10.0f;
  b.as<icl32f>()(0, 0, 0) = 20.0f;

  a.visitWith(b, [](auto &ta, auto &tb) {
    ta(0, 0, 0) = ta(0, 0, 0) + tb(0, 0, 0);
  });
  ICL_TEST_NEAR(a.as<icl32f>()(0, 0, 0), 30.0f, 1e-6f);
}

// ---- Image: stream output ----

ICL_REGISTER_TEST("Image.print_null", "null image doesn't crash on print") {
  Image img;
  img.print("test-null");
  ICL_TEST_EQ(img.isNull(), true);
}

// ---- Img<T>: initializer list constructors ----

ICL_REGISTER_TEST("Img.init_list_1ch", "single-channel from 2D initializer list") {
  Img32f img = {{1, 2, 3},
                {4, 5, 6}};
  ICL_TEST_EQ(img.getWidth(), 3);
  ICL_TEST_EQ(img.getHeight(), 2);
  ICL_TEST_EQ(img.getChannels(), 1);
  ICL_TEST_NEAR(img(0, 0, 0), 1.f, 1e-6f);
  ICL_TEST_NEAR(img(2, 0, 0), 3.f, 1e-6f);
  ICL_TEST_NEAR(img(0, 1, 0), 4.f, 1e-6f);
  ICL_TEST_NEAR(img(2, 1, 0), 6.f, 1e-6f);
}

ICL_REGISTER_TEST("Img.init_list_8u", "8u single-channel from initializer list") {
  Img8u img = {{10, 20},
               {30, 40}};
  ICL_TEST_EQ(img.getWidth(), 2);
  ICL_TEST_EQ(img.getHeight(), 2);
  ICL_TEST_EQ(static_cast<int>(img(0, 0, 0)), 10);
  ICL_TEST_EQ(static_cast<int>(img(1, 1, 0)), 40);
}

ICL_REGISTER_TEST("Img.init_list_multi_ch", "multi-channel from 3D initializer list") {
  Img32f img = {{{1, 2}, {3, 4}},    // channel 0
                {{5, 6}, {7, 8}}};   // channel 1
  ICL_TEST_EQ(img.getWidth(), 2);
  ICL_TEST_EQ(img.getHeight(), 2);
  ICL_TEST_EQ(img.getChannels(), 2);
  ICL_TEST_NEAR(img(0, 0, 0), 1.f, 1e-6f);
  ICL_TEST_NEAR(img(1, 1, 0), 4.f, 1e-6f);
  ICL_TEST_NEAR(img(0, 0, 1), 5.f, 1e-6f);
  ICL_TEST_NEAR(img(1, 1, 1), 8.f, 1e-6f);
}

ICL_REGISTER_TEST("Img.init_list_to_image", "initializer list Img converts to Image") {
  Image img = Img32f{{10, 20, 30}};
  ICL_TEST_EQ(img.getWidth(), 3);
  ICL_TEST_EQ(img.getHeight(), 1);
  ICL_TEST_EQ(img.is<icl32f>(), true);
  ICL_TEST_NEAR(img.as32f()(2, 0, 0), 30.f, 1e-6f);
}

ICL_REGISTER_TEST("Img.init_list_bad_rows", "inconsistent row lengths throw") {
  ICL_TEST_THROW((Img32f{{1, 2}, {3}}), utils::ICLException);
}

ICL_REGISTER_TEST("Img.init_list_bad_channels", "inconsistent channel dims throw") {
  ICL_TEST_THROW((Img32f{{{1, 2}}, {{3}}}), utils::ICLException);
}

// ---- Img<T>: equality operators ----

ICL_REGISTER_TEST("Img.eq_identical", "identical images are equal") {
  Img32f a = {{1, 2}, {3, 4}};
  Img32f b = {{1, 2}, {3, 4}};
  ICL_TEST_TRUE(a == b);
  ICL_TEST_FALSE(a != b);
}

ICL_REGISTER_TEST("Img.eq_different_pixel", "differing pixel values are not equal") {
  Img8u a = {{1, 2}, {3, 4}};
  Img8u b = {{1, 2}, {3, 5}};
  ICL_TEST_FALSE(a == b);
  ICL_TEST_TRUE(a != b);
}

ICL_REGISTER_TEST("Img.eq_different_size", "different sizes are not equal") {
  Img8u a = {{1, 2, 3}};
  Img8u b = {{1, 2}};
  ICL_TEST_FALSE(a == b);
}

ICL_REGISTER_TEST("Img.eq_float_epsilon", "float comparison uses epsilon") {
  Img32f a = {{1.0f, 2.0f}};
  Img32f b = {{1.0f + 1e-8f, 2.0f - 1e-8f}};  // well within 4*FLT_EPSILON (~4.8e-7)
  ICL_TEST_TRUE(a == b);
}

ICL_REGISTER_TEST("Img.eq_float_beyond_eps", "float values beyond epsilon are not equal") {
  Img32f a = {{1.0f}};
  Img32f b = {{1.0f + 1e-5f}};  // beyond 4*FLT_EPSILON (~4.8e-7)
  ICL_TEST_FALSE(a == b);
}

ICL_REGISTER_TEST("Img.eq_multichannel", "multi-channel equality") {
  Img8u a = {{{10, 20}}, {{30, 40}}};
  Img8u b = {{{10, 20}}, {{30, 40}}};
  ICL_TEST_TRUE(a == b);
  Img8u c = {{{10, 20}}, {{30, 41}}};
  ICL_TEST_FALSE(a == c);
}

// ---- Image: equality operators ----

ICL_REGISTER_TEST("Image.eq_same", "Image equality via Img<T>") {
  Image a = Img8u{{1, 2}, {3, 4}};
  Image b = Img8u{{1, 2}, {3, 4}};
  ICL_TEST_TRUE(a == b);
}

ICL_REGISTER_TEST("Image.eq_different_depth", "different depth images are not equal") {
  Image a = Img8u{{1, 2}};
  Image b = Img32f{{1.f, 2.f}};
  ICL_TEST_FALSE(a == b);
}

ICL_REGISTER_TEST("Image.eq_null", "two null images are equal") {
  Image a, b;
  ICL_TEST_TRUE(a == b);
}

ICL_REGISTER_TEST("Img.from_single_channel", "Img::from creates image from generator") {
  auto img = Img8u::from(4, 3, 1, [](int x, int y, int) -> icl8u {
    return x + y * 4;
  });
  ICL_TEST_EQ(img.getWidth(), 4);
  ICL_TEST_EQ(img.getHeight(), 3);
  ICL_TEST_EQ(img.getChannels(), 1);
  ICL_TEST_EQ(img(0, 0, 0), (icl8u)0);
  ICL_TEST_EQ(img(3, 0, 0), (icl8u)3);
  ICL_TEST_EQ(img(0, 2, 0), (icl8u)8);
  ICL_TEST_EQ(img(3, 2, 0), (icl8u)11);
}

ICL_REGISTER_TEST("Img.from_multi_channel", "Img::from with multiple channels") {
  auto img = Img32f::from(3, 2, 2, [](int x, int y, int c) -> icl32f {
    return (c + 1) * 10.f + x + y * 3;
  });
  ICL_TEST_EQ(img.getChannels(), 2);
  ICL_TEST_EQ(img(0, 0, 0), 10.f);  // ch0: 10 + 0 + 0
  ICL_TEST_EQ(img(2, 1, 0), 15.f);  // ch0: 10 + 2 + 3
  ICL_TEST_EQ(img(0, 0, 1), 20.f);  // ch1: 20 + 0 + 0
  ICL_TEST_EQ(img(2, 1, 1), 25.f);  // ch1: 20 + 2 + 3
}

ICL_REGISTER_TEST("Img.visitPixels_value", "visitPixels with f(T&)") {
  auto img = Img8u::from(3, 2, 1, [](int x, int y, int) -> icl8u { return x + y * 3; });
  int sum = 0;
  img.visitPixels([&](const icl8u &v) { sum += v; });
  // 0+1+2+3+4+5 = 15
  ICL_TEST_EQ(sum, 15);
}

ICL_REGISTER_TEST("Img.visitPixels_xy", "visitPixels with f(x,y,T&)") {
  auto img = Img32f::from(4, 3, 1, [](int, int, int) -> icl32f { return 0; });
  img.visitPixels([](int x, int y, icl32f &v) { v = x * 10.f + y; });
  ICL_TEST_EQ(img(2, 1, 0), 21.f);
  ICL_TEST_EQ(img(3, 2, 0), 32.f);
}

ICL_REGISTER_TEST("Img.visitPixels_xyc", "visitPixels with f(x,y,c,T&)") {
  auto img = Img8u::from(2, 2, 2, [](int, int, int) -> icl8u { return 0; });
  img.visitPixels([](int x, int y, int c, icl8u &v) { v = c * 100 + y * 10 + x; });
  ICL_TEST_EQ(img(0, 0, 0), (icl8u)0);
  ICL_TEST_EQ(img(1, 1, 0), (icl8u)11);
  ICL_TEST_EQ(img(0, 0, 1), (icl8u)100);
  ICL_TEST_EQ(img(1, 1, 1), (icl8u)111);
}

ICL_REGISTER_TEST("Img.visitPixels_roi", "visitPixels respects ROI by default") {
  Img8u img(utils::Size(6, 6), 1);
  img.clear();
  img.setROI(utils::Rect(1, 1, 4, 4));
  img.visitPixels([](int x, int y, icl8u &v) { v = x + y * 10; });
  // inside ROI: set
  ICL_TEST_EQ(img(2, 2, 0), (icl8u)22);
  // outside ROI: still 0
  ICL_TEST_EQ(img(0, 0, 0), (icl8u)0);
  ICL_TEST_EQ(img(5, 5, 0), (icl8u)0);
}

// ---- Scaled copy (tests all interpolation modes and backend dispatch) ----

ICL_REGISTER_TEST("Img.scaledCopy_NN_8u", "NN scaling 8u preserves corner pixels") {
  Img8u src(Size(4, 4), 1);
  src.clear();
  src(0, 0, 0) = 100;
  src(3, 0, 0) = 200;
  src(0, 3, 0) = 150;
  src(3, 3, 0) = 250;
  Img8u dst(Size(8, 8), 1);
  src.scaledCopy(&dst, interpolateNN);
  // Top-left corner should be 100
  ICL_TEST_EQ(dst(0, 0, 0), (icl8u)100);
  // Bottom-right area should be 250
  ICL_TEST_EQ(dst(7, 7, 0), (icl8u)250);
}

ICL_REGISTER_TEST("Img.scaledCopy_LIN_8u", "bilinear scaling 8u produces smooth result") {
  Img8u src(Size(2, 2), 1);
  src(0, 0, 0) = 0;
  src(1, 0, 0) = 100;
  src(0, 1, 0) = 100;
  src(1, 1, 0) = 200;
  Img8u dst(Size(4, 4), 1);
  src.scaledCopy(&dst, interpolateLIN);
  // Center should be interpolated (not 0 or 200)
  icl8u center = dst(2, 2, 0);
  ICL_TEST_TRUE(center > 30 && center < 170);
}

ICL_REGISTER_TEST("Img.scaledCopy_LIN_32f", "bilinear scaling 32f correctness") {
  Img32f src(Size(4, 4), 1);
  for(int y = 0; y < 4; ++y)
    for(int x = 0; x < 4; ++x)
      src(x, y, 0) = float(x + y * 4);
  Img32f dst(Size(2, 2), 1);
  src.scaledCopy(&dst, interpolateLIN);
  // Downscaled values should be reasonable averages
  ICL_TEST_TRUE(dst(0, 0, 0) >= 0.0f && dst(0, 0, 0) <= 15.0f);
  ICL_TEST_TRUE(dst(1, 1, 0) >= 0.0f && dst(1, 1, 0) <= 15.0f);
}

ICL_REGISTER_TEST("Img.scaledCopy_RA_8u", "region average scaling 8u") {
  Img8u src(Size(8, 8), 1);
  // Fill with uniform value
  for(int y = 0; y < 8; ++y)
    for(int x = 0; x < 8; ++x)
      src(x, y, 0) = 128;
  Img8u dst(Size(4, 4), 1);
  src.scaledCopy(&dst, interpolateRA);
  // Uniform input → uniform output
  ICL_TEST_NEAR(dst(0, 0, 0), 128, 2);
  ICL_TEST_NEAR(dst(3, 3, 0), 128, 2);
}

ICL_REGISTER_TEST("Img.scaledCopy_identity", "scaling to same size preserves image") {
  Img32f src(Size(8, 6), 1);
  for(int y = 0; y < 6; ++y)
    for(int x = 0; x < 8; ++x)
      src(x, y, 0) = float(x * 10 + y);
  Img32f dst(Size(8, 6), 1);
  src.scaledCopy(&dst, interpolateLIN);
  for(int y = 0; y < 6; ++y)
    for(int x = 0; x < 8; ++x)
      ICL_TEST_NEAR(dst(x, y, 0), src(x, y, 0), 0.01f);
}

ICL_REGISTER_TEST("Img.visitPixels_full", "visitPixels roiOnly=false ignores ROI") {
  Img8u img(utils::Size(6, 6), 1);
  img.clear();
  img.setROI(utils::Rect(1, 1, 4, 4));
  img.visitPixels([](icl8u &v) { v = 42; }, false);
  // ALL pixels set, including outside ROI
  ICL_TEST_EQ(img(0, 0, 0), (icl8u)42);
  ICL_TEST_EQ(img(5, 5, 0), (icl8u)42);
  ICL_TEST_EQ(img(3, 3, 0), (icl8u)42);
}
