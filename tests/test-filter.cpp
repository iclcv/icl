#include <ICLUtils/Test.h>
#include <ICLCore/Image.h>
#include <ICLCore/Img.h>
#include <ICLFilter/ThresholdOp.h>
#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ICLFilter/UnaryCompareOp.h>
#include <ICLFilter/UnaryLogicalOp.h>
#include <ICLFilter/WeightChannelsOp.h>
#include <ICLFilter/WeightedSumOp.h>
#include <ICLFilter/MirrorOp.h>
#include <ICLFilter/DitheringOp.h>
#include <ICLFilter/LUTOp.h>
#include <ICLFilter/IntegralImgOp.h>
#include <ICLFilter/ColorDistanceOp.h>
#include <ICLFilter/ColorSegmentationOp.h>
#include <ICLFilter/GaborOp.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;
using namespace icl::filter;

// ---- Helper: create a small 8u image with known pixel values ----

static Image make8u(int w, int h, int ch, const std::vector<icl8u> &data) {
  Image img(Size(w, h), depth8u, ch, formatMatrix);
  Img8u &ref = img.as8u();
  for(int c = 0; c < ch; ++c) {
    for(int y = 0; y < h; ++y) {
      for(int x = 0; x < w; ++x) {
        int idx = c * w * h + y * w + x;
        ref(x, y, c) = (idx < (int)data.size()) ? data[idx] : 0;
      }
    }
  }
  return img;
}

static Image make32f(int w, int h, int ch, const std::vector<float> &data) {
  Image img(Size(w, h), depth32f, ch, formatMatrix);
  Img32f &ref = img.as32f();
  for(int c = 0; c < ch; ++c) {
    for(int y = 0; y < h; ++y) {
      for(int x = 0; x < w; ++x) {
        int idx = c * w * h + y * w + x;
        ref(x, y, c) = (idx < (int)data.size()) ? data[idx] : 0.f;
      }
    }
  }
  return img;
}

// ====================================================================
// ThresholdOp
// ====================================================================

ICL_REGISTER_TEST("Filter.ThresholdOp.size", "output preserves size and depth") {
  Image src = make8u(4, 3, 1, {10, 50, 100, 150, 200, 250, 30, 60, 90, 120, 180, 210});
  ThresholdOp op(ThresholdOp::gt, 128, 128);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getWidth(), 4);
  ICL_TEST_EQ(dst.getHeight(), 3);
  ICL_TEST_EQ(dst.getChannels(), 1);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
}

ICL_REGISTER_TEST("Filter.ThresholdOp.gt_pixels", "gt clamps values above threshold") {
  // gt(128): values > 128 become 128, values <= 128 unchanged
  Image src = make8u(4, 1, 1, {10, 128, 200, 255});
  ThresholdOp op(ThresholdOp::gt, 128, 128);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 10);   // unchanged
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 128);  // unchanged
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 128);  // clamped
  ICL_TEST_EQ(static_cast<int>(d(3, 0, 0)), 128);  // clamped
}

ICL_REGISTER_TEST("Filter.ThresholdOp.lt_pixels", "lt clamps values below threshold") {
  // lt(128): values < 128 become 128, values >= 128 unchanged
  Image src = make8u(4, 1, 1, {10, 127, 128, 200});
  ThresholdOp op(ThresholdOp::lt, 128, 128);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 128);  // clamped
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 128);  // clamped
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 128);  // unchanged
  ICL_TEST_EQ(static_cast<int>(d(3, 0, 0)), 200);  // unchanged
}

ICL_REGISTER_TEST("Filter.ThresholdOp.32f", "threshold works on 32f images") {
  // gt(5.0): values > 5 become 5, values <= 5 unchanged
  Image src = make32f(3, 1, 1, {1.0f, 5.5f, 10.0f});
  ThresholdOp op(ThresholdOp::gt, 5.0f, 5.0f);
  Image dst = op.apply(src);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 1.0f, 1e-5f);   // unchanged
  ICL_TEST_NEAR(d(1, 0, 0), 5.0f, 1e-5f);   // clamped
  ICL_TEST_NEAR(d(2, 0, 0), 5.0f, 1e-5f);   // clamped
}

// ====================================================================
// UnaryArithmeticalOp
// ====================================================================

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.add_pixels", "addOp adds constant") {
  Image src = make32f(3, 1, 1, {10.0f, 20.0f, 30.0f});
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 5.0);
  Image dst = op.apply(src);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 15.0f, 1e-5f);
  ICL_TEST_NEAR(d(1, 0, 0), 25.0f, 1e-5f);
  ICL_TEST_NEAR(d(2, 0, 0), 35.0f, 1e-5f);
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.mul_pixels", "mulOp multiplies by constant") {
  Image src = make32f(3, 1, 1, {10.0f, 20.0f, 30.0f});
  UnaryArithmeticalOp op(UnaryArithmeticalOp::mulOp, 2.0);
  Image dst = op.apply(src);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 20.0f, 1e-5f);
  ICL_TEST_NEAR(d(1, 0, 0), 40.0f, 1e-5f);
  ICL_TEST_NEAR(d(2, 0, 0), 60.0f, 1e-5f);
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.sqr_pixels", "sqrOp squares each pixel") {
  Image src = make32f(3, 1, 1, {3.0f, 4.0f, 5.0f});
  UnaryArithmeticalOp op(UnaryArithmeticalOp::sqrOp);
  Image dst = op.apply(src);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 9.0f, 1e-4f);
  ICL_TEST_NEAR(d(1, 0, 0), 16.0f, 1e-4f);
  ICL_TEST_NEAR(d(2, 0, 0), 25.0f, 1e-4f);
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.size", "output matches input") {
  Image src = make32f(5, 4, 2, {});
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 1.0);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getWidth(), 5);
  ICL_TEST_EQ(dst.getHeight(), 4);
  ICL_TEST_EQ(dst.getChannels(), 2);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth32f));
}

// ====================================================================
// UnaryCompareOp
// ====================================================================

ICL_REGISTER_TEST("Filter.UnaryCompareOp.gt_pixels", "gt comparison produces 0/255") {
  Image src = make32f(4, 1, 1, {5.0f, 15.0f, 20.0f, 25.0f});
  UnaryCompareOp op(UnaryCompareOp::gt, 15.0);
  Image dst = op.apply(src);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(d(3, 0, 0)), 255);
}

ICL_REGISTER_TEST("Filter.UnaryCompareOp.output_depth", "output is always depth8u") {
  Image src = make32f(3, 3, 1, {});
  UnaryCompareOp op(UnaryCompareOp::lt, 0.0);
  Image dst = op.apply(src);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_EQ(dst.getWidth(), 3);
  ICL_TEST_EQ(dst.getHeight(), 3);
}

// ====================================================================
// UnaryLogicalOp
// ====================================================================

ICL_REGISTER_TEST("Filter.UnaryLogicalOp.and_pixels", "andOp masks bits") {
  Image src = make8u(3, 1, 1, {0xFF, 0x0F, 0xF0});
  UnaryLogicalOp op(UnaryLogicalOp::andOp, 0x0F);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0x0F);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0x0F);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 0x00);
}

ICL_REGISTER_TEST("Filter.UnaryLogicalOp.or_pixels", "orOp sets bits") {
  Image src = make8u(3, 1, 1, {0x00, 0x0F, 0xF0});
  UnaryLogicalOp op(UnaryLogicalOp::orOp, 0x0F);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0x0F);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0x0F);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 0xFF);
}

ICL_REGISTER_TEST("Filter.UnaryLogicalOp.not_pixels", "notOp inverts bits") {
  Image src = make8u(2, 1, 1, {0x00, 0xFF});
  UnaryLogicalOp op(UnaryLogicalOp::notOp);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0xFF);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0x00);
}

// ====================================================================
// MirrorOp
// ====================================================================

ICL_REGISTER_TEST("Filter.MirrorOp.horz_pixels", "axisHorz flips rows (around horizontal axis)") {
  // 3x2 image: [1,2,3; 4,5,6] → [4,5,6; 1,2,3] (rows swapped)
  Image src = make8u(3, 2, 1, {1, 2, 3, 4, 5, 6});
  MirrorOp op(axisHorz);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 4);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 5);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 6);
  ICL_TEST_EQ(static_cast<int>(d(0, 1, 0)), 1);
  ICL_TEST_EQ(static_cast<int>(d(1, 1, 0)), 2);
  ICL_TEST_EQ(static_cast<int>(d(2, 1, 0)), 3);
}

ICL_REGISTER_TEST("Filter.MirrorOp.vert_pixels", "axisVert flips columns (around vertical axis)") {
  // 3x2 image: [1,2,3; 4,5,6] → [3,2,1; 6,5,4] (columns reversed)
  Image src = make8u(3, 2, 1, {1, 2, 3, 4, 5, 6});
  MirrorOp op(axisVert);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 3);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 2);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 1);
  ICL_TEST_EQ(static_cast<int>(d(0, 1, 0)), 6);
  ICL_TEST_EQ(static_cast<int>(d(1, 1, 0)), 5);
  ICL_TEST_EQ(static_cast<int>(d(2, 1, 0)), 4);
}

ICL_REGISTER_TEST("Filter.MirrorOp.size", "mirror preserves size") {
  Image src = make8u(5, 7, 2, {});
  MirrorOp op(axisBoth);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getWidth(), 5);
  ICL_TEST_EQ(dst.getHeight(), 7);
  ICL_TEST_EQ(dst.getChannels(), 2);
}

// ====================================================================
// WeightChannelsOp
// ====================================================================

ICL_REGISTER_TEST("Filter.WeightChannelsOp.pixels", "per-channel multiply") {
  // 2-channel image, weights [2.0, 0.5]
  Image src = make32f(2, 1, 2, {10.0f, 20.0f, 40.0f, 80.0f});
  WeightChannelsOp op({2.0, 0.5});
  Image dst = op.apply(src);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 20.0f, 1e-5f);  // ch0: 10*2
  ICL_TEST_NEAR(d(1, 0, 0), 40.0f, 1e-5f);  // ch0: 20*2
  ICL_TEST_NEAR(d(0, 0, 1), 20.0f, 1e-5f);  // ch1: 40*0.5
  ICL_TEST_NEAR(d(1, 0, 1), 40.0f, 1e-5f);  // ch1: 80*0.5
}

// ====================================================================
// WeightedSumOp
// ====================================================================

ICL_REGISTER_TEST("Filter.WeightedSumOp.pixels", "weighted sum across channels") {
  // 2-channel image, weights [0.5, 0.5]
  // ch0: [10, 20], ch1: [30, 40]
  // result: [0.5*10+0.5*30, 0.5*20+0.5*40] = [20, 30]
  Image src = make32f(2, 1, 2, {10.0f, 20.0f, 30.0f, 40.0f});
  WeightedSumOp op({0.5, 0.5});
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 1);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 20.0f, 1e-4f);
  ICL_TEST_NEAR(d(1, 0, 0), 30.0f, 1e-4f);
}

ICL_REGISTER_TEST("Filter.WeightedSumOp.size", "output is single channel") {
  Image src = make32f(4, 3, 3, {});
  WeightedSumOp op({1.0, 1.0, 1.0});
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 1);
  ICL_TEST_EQ(dst.getWidth(), 4);
  ICL_TEST_EQ(dst.getHeight(), 3);
}

// ====================================================================
// DitheringOp
// ====================================================================

ICL_REGISTER_TEST("Filter.DitheringOp.output_depth", "output is always 8u") {
  Image src = make8u(8, 8, 1, {});
  DitheringOp op;
  Image dst = op.apply(src);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_EQ(dst.getWidth(), 8);
  ICL_TEST_EQ(dst.getHeight(), 8);
  ICL_TEST_EQ(dst.getChannels(), 1);
}

ICL_REGISTER_TEST("Filter.DitheringOp.binary_values", "with 2 levels, output is 0 or 255") {
  // Fill with a mid-gray value
  Image src = make8u(4, 4, 1, {128, 128, 128, 128, 128, 128, 128, 128,
                                128, 128, 128, 128, 128, 128, 128, 128});
  DitheringOp op(DitheringOp::FloydSteinberg, 2);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  for(int y = 0; y < 4; ++y) {
    for(int x = 0; x < 4; ++x) {
      int v = d(x, y, 0);
      ICL_TEST_TRUE(v == 0 || v == 255);
    }
  }
}

// ====================================================================
// LUTOp
// ====================================================================

ICL_REGISTER_TEST("Filter.LUTOp.identity", "identity LUT preserves pixels") {
  std::vector<icl8u> lut(256);
  for(int i = 0; i < 256; ++i) lut[i] = i;
  LUTOp op(lut);
  Image src = make8u(3, 1, 1, {0, 128, 255});
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 128);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 255);
}

ICL_REGISTER_TEST("Filter.LUTOp.invert", "invert LUT maps x to 255-x") {
  std::vector<icl8u> lut(256);
  for(int i = 0; i < 256; ++i) lut[i] = 255 - i;
  LUTOp op(lut);
  Image src = make8u(3, 1, 1, {0, 100, 255});
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 155);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 0);
}

// ====================================================================
// IntegralImgOp
// ====================================================================

ICL_REGISTER_TEST("Filter.IntegralImgOp.pixels", "integral image of known data") {
  // 3x2: [1,2,3; 4,5,6]
  // Integral: [1,  3,  6;
  //            5, 12, 21]
  Image src = make8u(3, 2, 1, {1, 2, 3, 4, 5, 6});
  IntegralImgOp op(depth32s);
  Image dst = op.apply(src);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth32s));
  const Img32s &d = dst.as<icl32s>();
  ICL_TEST_EQ(d(0, 0, 0), 1);
  ICL_TEST_EQ(d(1, 0, 0), 3);
  ICL_TEST_EQ(d(2, 0, 0), 6);
  ICL_TEST_EQ(d(0, 1, 0), 5);
  ICL_TEST_EQ(d(1, 1, 0), 12);
  ICL_TEST_EQ(d(2, 1, 0), 21);
}

ICL_REGISTER_TEST("Filter.IntegralImgOp.size", "output size matches input") {
  Image src = make8u(10, 8, 2, {});
  IntegralImgOp op(depth32s);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getWidth(), 10);
  ICL_TEST_EQ(dst.getHeight(), 8);
  ICL_TEST_EQ(dst.getChannels(), 2);
}

// ====================================================================
// ColorDistanceOp
// ====================================================================

ICL_REGISTER_TEST("Filter.ColorDistanceOp.zero_distance", "identical color gives distance 0") {
  // 3-channel image, ref = (100, 100, 100), pixel = (100, 100, 100)
  std::vector<float> data = {100.f, 100.f, 100.f, 100.f, 100.f, 100.f};
  Image src = make32f(1, 1, 3, {100.f, 100.f, 100.f});
  std::vector<double> ref = {100.0, 100.0, 100.0};
  ColorDistanceOp op(ref, -1);  // no threshold → compute actual distance
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 1);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 0.0f, 1e-5f);
}

ICL_REGISTER_TEST("Filter.ColorDistanceOp.known_distance", "computes euclidean distance") {
  // pixel (100,100,100), ref (103,104,100) → dist = sqrt(9+16+0) = 5
  Image src = make32f(1, 1, 3, {100.f, 100.f, 100.f});
  std::vector<double> ref = {103.0, 104.0, 100.0};
  ColorDistanceOp op(ref, -1);
  Image dst = op.apply(src);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 5.0f, 1e-4f);
}

ICL_REGISTER_TEST("Filter.ColorDistanceOp.threshold", "threshold mode produces binary output") {
  // ref = (100,100,100), pixel1 = (100,100,100) dist=0, pixel2 = (200,200,200) dist=~173
  Image src = make8u(2, 1, 3, {100, 200, 100, 200, 100, 200});
  std::vector<double> ref = {100.0, 100.0, 100.0};
  ColorDistanceOp op(ref, 10.0);  // threshold = 10
  Image dst = op.apply(src);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 255);  // within threshold
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0);    // outside threshold
}

// ====================================================================
// ColorSegmentationOp
// ====================================================================

ICL_REGISTER_TEST("Filter.ColorSegmentationOp.basic", "LUT lookup classifies pixels") {
  // Use RGB format with shift=0 (full 256 resolution)
  ColorSegmentationOp op(0, 0, 0, formatRGB);
  op.clearLUT(0);
  // Set class 1 for color (100, 100, 100) with radius 0
  op.lutEntry(100, 100, 100, 0, 0, 0, 1);

  // Create a 3-channel 8u RGB image
  Image src(Size(2, 1), depth8u, 3, formatRGB);
  Img8u &s = src.as8u();
  // pixel 0: (100, 100, 100) → class 1
  s(0, 0, 0) = 100; s(0, 0, 1) = 100; s(0, 0, 2) = 100;
  // pixel 1: (50, 50, 50) → class 0
  s(1, 0, 0) = 50; s(1, 0, 1) = 50; s(1, 0, 2) = 50;

  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 1);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 1);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0);
}

ICL_REGISTER_TEST("Filter.ColorSegmentationOp.size", "output size matches input") {
  ColorSegmentationOp op(2, 2, 2, formatRGB);
  Image src(Size(10, 8), depth8u, 3, formatRGB);
  src.clear();
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getWidth(), 10);
  ICL_TEST_EQ(dst.getHeight(), 8);
}

// ====================================================================
// GaborOp
// ====================================================================

ICL_REGISTER_TEST("Filter.GaborOp.output_channels", "output has one channel per kernel") {
  // 2 lambdas x 2 thetas x 1 psi x 1 sigma x 1 gamma = 4 kernels
  GaborOp op(Size(5, 5),
             {5.0f, 10.0f},  // lambdas
             {0.0f, 1.57f},  // thetas
             {0.0f},         // psis
             {2.0f},         // sigmas
             {1.0f});        // gammas
  Image src = make32f(16, 16, 1, {});
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 4);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth32f));
}

ICL_REGISTER_TEST("Filter.GaborOp.size", "output size is valid") {
  GaborOp op(Size(3, 3), {5.0f}, {0.0f}, {0.0f}, {2.0f}, {1.0f});
  Image src = make32f(20, 15, 1, {});
  Image dst = op.apply(src);
  ICL_TEST_TRUE(dst.getWidth() > 0);
  ICL_TEST_TRUE(dst.getHeight() > 0);
}

// ====================================================================
// ROI tests — verify filters handle ROI correctly
// ====================================================================

ICL_REGISTER_TEST("Filter.ROI.arithmetical_clip", "addOp with clipToROI produces smaller output") {
  Image src(Size(6, 6), depth32f, 1);
  Img32f &s = src.as32f();
  for(int y = 0; y < 6; ++y)
    for(int x = 0; x < 6; ++x)
      s(x, y, 0) = 10.0f;

  src.setROI(Rect(1, 1, 4, 4));

  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 5.0);
  op.setClipToROI(true);
  Image dst;
  op.apply(src, dst);

  // ClipToROI → output should be 4x4
  ICL_TEST_EQ(dst.getWidth(), 4);
  ICL_TEST_EQ(dst.getHeight(), 4);

  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 15.0f, 1e-5f);
  ICL_TEST_NEAR(d(3, 3, 0), 15.0f, 1e-5f);
}

ICL_REGISTER_TEST("Filter.ROI.threshold_clip", "thresholdOp with clipToROI") {
  Image src(Size(6, 6), depth8u, 1);
  Img8u &s = src.as8u();
  for(int y = 0; y < 6; ++y)
    for(int x = 0; x < 6; ++x)
      s(x, y, 0) = 200;

  src.setROI(Rect(1, 1, 4, 4));
  ThresholdOp op(ThresholdOp::gt, 128, 128);
  op.setClipToROI(true);
  Image dst;
  op.apply(src, dst);
  ICL_TEST_EQ(dst.getWidth(), 4);
  ICL_TEST_EQ(dst.getHeight(), 4);
  const Img8u &d = dst.as8u();
  // gt(128) clamps 200 → 128
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 128);
}

ICL_REGISTER_TEST("Filter.ROI.compare_no_clip", "compareOp with ROI, no clip preserves full size") {
  Image src(Size(8, 8), depth32f, 1);
  src.clear();
  src.as32f()(4, 4, 0) = 100.0f;
  src.setROI(Rect(2, 2, 4, 4));

  UnaryCompareOp op(UnaryCompareOp::gt, 50.0);
  op.setClipToROI(false);
  Image dst;
  op.apply(src, dst);

  // No clip → output is 8x8
  ICL_TEST_EQ(dst.getWidth(), 8);
  ICL_TEST_EQ(dst.getHeight(), 8);
}

// ====================================================================
// Multi-channel tests
// ====================================================================

ICL_REGISTER_TEST("Filter.MultiChannel.threshold_3ch", "gt threshold clamps on 3-channel image") {
  // 3-channel 2x1 image, gt(128) clamps above 128
  Image src(Size(2, 1), depth8u, 3);
  Img8u &s = src.as8u();
  s(0, 0, 0) = 200; s(1, 0, 0) = 50;   // ch0
  s(0, 0, 1) = 50;  s(1, 0, 1) = 200;  // ch1
  s(0, 0, 2) = 100; s(1, 0, 2) = 100;  // ch2

  ThresholdOp op(ThresholdOp::gt, 128, 128);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 3);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 128);  // ch0: 200 > 128 → clamped
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 50);   // ch0: 50 <= 128 → unchanged
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 1)), 50);   // ch1: 50 <= 128 → unchanged
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 1)), 128);  // ch1: 200 > 128 → clamped
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 2)), 100);  // ch2: 100 <= 128 → unchanged
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 2)), 100);  // ch2: 100 <= 128 → unchanged
}

ICL_REGISTER_TEST("Filter.MultiChannel.mirror_2ch", "axisVert mirror on 2-channel image") {
  // axisVert reverses columns: [1,2,3] → [3,2,1]
  Image src(Size(3, 1), depth8u, 2);
  Img8u &s = src.as8u();
  s(0, 0, 0) = 1; s(1, 0, 0) = 2; s(2, 0, 0) = 3;   // ch0
  s(0, 0, 1) = 10; s(1, 0, 1) = 20; s(2, 0, 1) = 30; // ch1

  MirrorOp op(axisVert);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 3);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 1);
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 1)), 30);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 1)), 10);
}

// ====================================================================
// Buffer reuse tests — calling apply twice reuses dst
// ====================================================================

ICL_REGISTER_TEST("Filter.BufferReuse.apply_twice", "second apply reuses dst buffer") {
  Image src1 = make32f(4, 4, 1, {1.f, 2.f, 3.f, 4.f});
  Image src2 = make32f(4, 4, 1, {10.f, 20.f, 30.f, 40.f});
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 1.0);

  Image dst;
  op.apply(src1, dst);
  ICL_TEST_FALSE(dst.isNull());
  ICL_TEST_NEAR(dst.as32f()(0, 0, 0), 2.0f, 1e-5f);

  op.apply(src2, dst);
  ICL_TEST_NEAR(dst.as32f()(0, 0, 0), 11.0f, 1e-5f);
}

// ====================================================================
// Depth dispatch tests — verify filters work across all supported depths
// ====================================================================

ICL_REGISTER_TEST("Filter.Depth.arithmetical_8u", "addOp works on 8u (clamped)") {
  Image src = make8u(2, 1, 1, {100, 250});
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 10.0);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 110);
  // 250+10=260 → clamped to 255
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 255);
}

ICL_REGISTER_TEST("Filter.Depth.compare_8u", "compareOp works on 8u input") {
  Image src = make8u(3, 1, 1, {50, 128, 200});
  UnaryCompareOp op(UnaryCompareOp::gt, 128);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 255);
}
