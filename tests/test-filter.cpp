#include <ICLUtils/Test.h>
#include <ICLCore/Image.h>
#include <ICLCore/Img.h>
#include <ICLFilter/ThresholdOp.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLFilter/BinaryCompareOp.h>
#include <ICLFilter/BinaryLogicalOp.h>
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
#include <ICLFilter/ChamferOp.h>
#include <ICLFilter/AffineOp.h>
#include <ICLFilter/CannyOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLFilter/LocalThresholdOp.h>
#include <ICLFilter/WarpOp.h>
#include <ICLFilter/BilateralFilterOp.h>
#include <ICLFilter/FFTOp.h>
#include <ICLFilter/IFFTOp.h>
#include <ICLFilter/MotionSensitiveTemporalSmoothing.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;
using namespace icl::filter;

// Helper for zero-filled images of a specific size
static Image make_empty(int w, int h, depth d, int ch = 1) {
  Image img(Size(w, h), d, ch, formatMatrix);
  img.clear();
  return img;
}

// ====================================================================
// ThresholdOp
// ====================================================================

ICL_REGISTER_TEST("Filter.ThresholdOp.size", "output preserves size and depth") {
  Image src = Img8u{{10, 50, 100, 150},
                    {200, 250, 30, 60},
                    {90, 120, 180, 210}};
  ThresholdOp op(ThresholdOp::gt, 128, 128);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getWidth(), 4);
  ICL_TEST_EQ(dst.getHeight(), 3);
  ICL_TEST_EQ(dst.getChannels(), 1);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
}

ICL_REGISTER_TEST("Filter.ThresholdOp.gt_pixels", "gt clamps values above threshold") {
  // gt(128): values > 128 become 128, values <= 128 unchanged
  ThresholdOp op(ThresholdOp::gt, 128, 128);
  ICL_TEST_TRUE((op.apply(Img8u{{10, 128, 200, 255}}) == Img8u{{10, 128, 128, 128}}));
}

ICL_REGISTER_TEST("Filter.ThresholdOp.lt_pixels", "lt clamps values below threshold") {
  // lt(128): values < 128 become 128, values >= 128 unchanged
  ThresholdOp op(ThresholdOp::lt, 128, 128);
  ICL_TEST_TRUE((op.apply(Img8u{{10, 127, 128, 200}}) == Img8u{{128, 128, 128, 200}}));
}

ICL_REGISTER_TEST("Filter.ThresholdOp.32f", "threshold works on 32f images") {
  // gt(5.0): values > 5 become 5, values <= 5 unchanged
  Image src = Img32f{{1.0f, 5.5f, 10.0f}};
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
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 5.0);
  ICL_TEST_TRUE((op.apply(Img32f{{10.f, 20.f, 30.f}}) == Img32f{{15.f, 25.f, 35.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.mul_pixels", "mulOp multiplies by constant") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::mulOp, 2.0);
  ICL_TEST_TRUE((op.apply(Img32f{{10.f, 20.f, 30.f}}) == Img32f{{20.f, 40.f, 60.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.sqr_pixels", "sqrOp squares each pixel") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::sqrOp);
  ICL_TEST_TRUE((op.apply(Img32f{{3.f, 4.f, 5.f}}) == Img32f{{9.f, 16.f, 25.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.size", "output matches input") {
  Image src = make_empty(5, 4, depth32f, 2);
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
  Image src = Img32f{{5.0f, 15.0f, 20.0f, 25.0f}};
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
  Image src = make_empty(3, 3, depth32f);
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
  UnaryLogicalOp op(UnaryLogicalOp::andOp, 0x0F);
  ICL_TEST_TRUE((op.apply(Img8u{{0xFF, 0x0F, 0xF0}}) == Img8u{{0x0F, 0x0F, 0x00}}));
}

ICL_REGISTER_TEST("Filter.UnaryLogicalOp.or_pixels", "orOp sets bits") {
  UnaryLogicalOp op(UnaryLogicalOp::orOp, 0x0F);
  ICL_TEST_TRUE((op.apply(Img8u{{0x00, 0x0F, 0xF0}}) == Img8u{{0x0F, 0x0F, 0xFF}}));
}

ICL_REGISTER_TEST("Filter.UnaryLogicalOp.not_pixels", "notOp inverts bits") {
  UnaryLogicalOp op(UnaryLogicalOp::notOp);
  ICL_TEST_TRUE((op.apply(Img8u{{0x00, 0xFF}}) == Img8u{{0xFF, 0x00}}));
}

// ====================================================================
// MirrorOp
// ====================================================================

ICL_REGISTER_TEST("Filter.MirrorOp.horz_pixels", "axisHorz flips rows (around horizontal axis)") {
  MirrorOp op(axisHorz);
  ICL_TEST_TRUE((op.apply(Img8u{{1,2,3}, {4,5,6}}) == Img8u{{4,5,6}, {1,2,3}}));
}

ICL_REGISTER_TEST("Filter.MirrorOp.vert_pixels", "axisVert flips columns (around vertical axis)") {
  MirrorOp op(axisVert);
  ICL_TEST_TRUE((op.apply(Img8u{{1,2,3}, {4,5,6}}) == Img8u{{3,2,1}, {6,5,4}}));
}

ICL_REGISTER_TEST("Filter.MirrorOp.size", "mirror preserves size") {
  Image src = make_empty(5, 7, depth8u, 2);
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
  Image src = Img32f{{{10.0f, 20.0f}},     // ch0
                     {{40.0f, 80.0f}}};    // ch1
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
  Image src = Img32f{{{10.0f, 20.0f}},     // ch0
                     {{30.0f, 40.0f}}};    // ch1
  WeightedSumOp op({0.5, 0.5});
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 1);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 20.0f, 1e-4f);
  ICL_TEST_NEAR(d(1, 0, 0), 30.0f, 1e-4f);
}

ICL_REGISTER_TEST("Filter.WeightedSumOp.size", "output is single channel") {
  Image src = make_empty(4, 3, depth32f, 3);
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
  Image src = make_empty(8, 8, depth8u);
  DitheringOp op;
  Image dst = op.apply(src);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_EQ(dst.getWidth(), 8);
  ICL_TEST_EQ(dst.getHeight(), 8);
  ICL_TEST_EQ(dst.getChannels(), 1);
}

ICL_REGISTER_TEST("Filter.DitheringOp.binary_values", "with 2 levels, output is 0 or 255") {
  Image src = Img8u{{128, 128, 128, 128},
                    {128, 128, 128, 128},
                    {128, 128, 128, 128},
                    {128, 128, 128, 128}};
  DitheringOp op(DitheringOp::FloydSteinberg, 2);
  Image dst = op.apply(src);
  bool ok = true;
  dst.as8u().visitPixels([&](const icl8u &v) { if(v != 0 && v != 255) ok = false; });
  ICL_TEST_TRUE(ok);
}

// ====================================================================
// LUTOp
// ====================================================================

ICL_REGISTER_TEST("Filter.LUTOp.identity", "identity LUT preserves pixels") {
  std::vector<icl8u> lut(256);
  for(int i = 0; i < 256; ++i) lut[i] = i;
  LUTOp op(lut);
  Image src = Img8u{{0, 128, 255}};
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
  Image src = Img8u{{0, 100, 255}};
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
  Image src = Img8u{{1, 2, 3},
                    {4, 5, 6}};
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
  Image src = make_empty(10, 8, depth8u, 2);
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
  // 3-channel 2x1 image, ref = (100, 100, 100)
  Image src = Img32f{{{100.f, 100.f}}, {{100.f, 100.f}}, {{100.f, 100.f}}};
  std::vector<double> ref = {100.0, 100.0, 100.0};
  ColorDistanceOp op(ref, -1);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 1);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 0.0f, 1e-5f);
}

ICL_REGISTER_TEST("Filter.ColorDistanceOp.known_distance", "computes euclidean distance") {
  // pixel (100,100,100), ref (103,104,100) -> dist = sqrt(9+16+0) = 5
  Image src = Img32f{{{100.f, 100.f}}, {{100.f, 100.f}}, {{100.f, 100.f}}};
  std::vector<double> ref = {103.0, 104.0, 100.0};
  ColorDistanceOp op(ref, -1);
  Image dst = op.apply(src);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 5.0f, 1e-4f);
}

ICL_REGISTER_TEST("Filter.ColorDistanceOp.threshold", "threshold mode produces binary output") {
  // ref=(100,100,100), pixel1=(100,..) dist=0, pixel2=(200,..) dist=~173
  Image src = Img8u{{{100, 200}}, {{100, 200}}, {{100, 200}}};
  std::vector<double> ref = {100.0, 100.0, 100.0};
  ColorDistanceOp op(ref, 10.0);
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
  ColorSegmentationOp op(0, 0, 0, formatRGB);
  op.clearLUT(0);
  op.lutEntry(100, 100, 100, 0, 0, 0, 1);

  // 3-channel 8u RGB image via initializer list
  Image src(Size(2, 1), depth8u, 3, formatRGB);
  Img8u &s = src.as8u();
  s(0, 0, 0) = 100; s(0, 0, 1) = 100; s(0, 0, 2) = 100;
  s(1, 0, 0) = 50;  s(1, 0, 1) = 50;  s(1, 0, 2) = 50;

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
  GaborOp op(Size(5, 5),
             {5.0f, 10.0f}, {0.0f, 1.57f}, {0.0f}, {2.0f}, {1.0f});
  Image src = make_empty(16, 16, depth32f);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 4);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth32f));
}

ICL_REGISTER_TEST("Filter.GaborOp.size", "output size is valid") {
  GaborOp op(Size(3, 3), {5.0f}, {0.0f}, {0.0f}, {2.0f}, {1.0f});
  Image src = make_empty(20, 15, depth32f);
  Image dst = op.apply(src);
  ICL_TEST_TRUE(dst.getWidth() > 0);
  ICL_TEST_TRUE(dst.getHeight() > 0);
}

// ====================================================================
// Cross-backend validation helper
// ====================================================================

// Runs applyFn with all-C++ as reference, then iterates every backend
// combination and verifies pixel-exact identical output.
template<class Op, class ApplyFn>
static void crossValidateBackends(Op &op, const Image &ctx, ApplyFn &&applyFn) {
  op.forceAll(Backend::Cpp);
  Image ref = applyFn();
  op.unforceAll();

  auto combos = op.allBackendCombinations(ctx);
  int nCombos = 0;
  bool allMatch = true;
  op.forEachCombination(combos, [&](const std::vector<Backend>&) {
    Image dst = applyFn();
    dst.visit([&](const auto &d) {
      using T = typename std::remove_reference_t<decltype(d)>::type;
      if(!(d == ref.as<T>())) allMatch = false;
    });
    nCombos++;
  });
  ICL_TEST_TRUE(allMatch);
  ICL_TEST_TRUE(nCombos >= 1);
}

// ====================================================================
// Generic ROI test helpers
// ====================================================================

// Test that clipToROI and non-clip modes produce consistent results,
// and that pixels outside the ROI are not modified in non-clip mode.
// Works for any UnaryOp (including NeighborhoodOp subclasses).
static void testROIHandling(UnaryOp &op, const Image &src, const Rect &roi) {
  Image srcROI = src.deepCopy();
  srcROI.setROI(roi);

  // --- clipToROI=true: produces a smaller (or equal) image ---
  op.setClipToROI(true);
  Image clipped = op.apply(srcROI);
  // output must not be larger than the ROI
  ICL_TEST_TRUE(clipped.getWidth() <= roi.width);
  ICL_TEST_TRUE(clipped.getHeight() <= roi.height);
  ICL_TEST_TRUE(clipped.getWidth() > 0);
  ICL_TEST_TRUE(clipped.getHeight() > 0);

  // --- clipToROI=false: full-size image, ROI marks processed region ---
  op.setClipToROI(false);
  // Pre-allocate dst at output depth/channels (from clipped) but full src size,
  // so ensureCompatible reuses the buffer and our sentinel survives.
  const double SENTINEL = 222;
  Image full(src.getSize(), clipped.getDepth(), clipped.getChannels(), clipped.getFormat());
  full.clear(-1, SENTINEL);
  op.apply(srcROI, full);

  // full image size = src size
  ICL_TEST_EQ(full.getSize(), src.getSize());

  // ROI region of full should match clipped exactly
  ICL_TEST_EQ(full.getROISize(), clipped.getSize());

  bool contentMatch = true;
  full.visit([&](const auto &f) {
    using T = typename std::remove_reference_t<decltype(f)>::type;
    const auto &c = clipped.as<T>();
    Rect dr = f.getROI();
    for (int ch = 0; ch < f.getChannels(); ch++) {
      const T *fp = f.getROIData(ch);
      const T *cp = c.getData(ch);
      for (int y = 0; y < dr.height; y++) {
        for (int x = 0; x < dr.width; x++) {
          if (fp[x] != cp[x]) contentMatch = false;
        }
        fp += f.getWidth();
        cp += c.getWidth();
      }
    }
  });
  ICL_TEST_TRUE(contentMatch);

  // sentinel must survive outside the ROI
  bool sentinelOK = true;
  full.visit([&](const auto &f) {
    using T = typename std::remove_reference_t<decltype(f)>::type;
    T sv = static_cast<T>(SENTINEL);
    Rect dr = f.getROI();
    for (int ch = 0; ch < f.getChannels(); ch++) {
      for (int y = 0; y < f.getHeight(); y++) {
        for (int x = 0; x < f.getWidth(); x++) {
          bool inROI = x >= dr.x && x < dr.x + dr.width
                    && y >= dr.y && y < dr.y + dr.height;
          if (!inROI && f(x, y, ch) != sv) sentinelOK = false;
        }
      }
    }
  });
  ICL_TEST_TRUE(sentinelOK);

  // restore default
  op.setClipToROI(true);
}

// Create a gradient test image — uses Img<T>::from, then wraps as Image
template<class T>
static Image makeGradient(int w, int h, int ch = 1) {
  return Img<T>::from(w, h, ch, [w](int x, int y, int c) -> T {
    return static_cast<T>((x + y * w + c * 7) % 200 + 1);
  });
}

// ====================================================================
// ROI tests — non-NeighborhoodOp filters
// ====================================================================

ICL_REGISTER_TEST("Filter.ROI.ThresholdOp", "ROI handling for ThresholdOp") {
  ThresholdOp op(ThresholdOp::gt, 100, 100);
  Image src = makeGradient<icl8u>(10, 10);
  testROIHandling(op, src, Rect(2, 2, 6, 6));  // interior
  testROIHandling(op, src, Rect(0, 0, 5, 5));  // corner
}

ICL_REGISTER_TEST("Filter.ROI.UnaryArithmeticalOp", "ROI handling for UnaryArithmeticalOp") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 5.0);
  Image src = makeGradient<icl32f>(10, 10);
  testROIHandling(op, src, Rect(2, 2, 6, 6));
  testROIHandling(op, src, Rect(0, 0, 5, 5));
}

ICL_REGISTER_TEST("Filter.ROI.UnaryCompareOp", "ROI handling for UnaryCompareOp") {
  UnaryCompareOp op(UnaryCompareOp::gt, 50.0);
  Image src = makeGradient<icl32f>(10, 10);
  testROIHandling(op, src, Rect(2, 2, 6, 6));
}

ICL_REGISTER_TEST("Filter.ROI.UnaryLogicalOp", "ROI handling for UnaryLogicalOp") {
  UnaryLogicalOp op(UnaryLogicalOp::andOp, 0xF0);
  Image src = makeGradient<icl8u>(10, 10);
  testROIHandling(op, src, Rect(1, 1, 7, 7));
}

ICL_REGISTER_TEST("Filter.ROI.MirrorOp", "MirrorOp ROI clip produces correct mirrored sub-region") {
  // MirrorOp is an affine op: non-clip mode writes the full image (not just ROI),
  // so testROIHandling's sentinel check doesn't apply. Test clip mode manually.
  MirrorOp op(axisHorz);
  auto src = Img8u::from(10, 10, 1, [](int x, int y, int) -> icl8u { return x + y * 10; });
  Image srcImg(src);
  srcImg.setROI(Rect(2, 2, 6, 6));
  op.setClipToROI(true);
  Image dst;
  op.apply(srcImg, dst);
  // ROI (2,2,6,6) → dst is 6x6, horizontally flipped from the ROI region
  ICL_TEST_EQ(dst.getWidth(), 6);
  ICL_TEST_EQ(dst.getHeight(), 6);
  // dst row 0 should be src ROI row 5 (= src row 7)
  ICL_TEST_EQ(dst.as8u()(0, 0, 0), src(2, 7, 0));
  ICL_TEST_EQ(dst.as8u()(5, 0, 0), src(7, 7, 0));
  // dst row 5 should be src ROI row 0 (= src row 2)
  ICL_TEST_EQ(dst.as8u()(0, 5, 0), src(2, 2, 0));
}

// ====================================================================
// ROI tests — NeighborhoodOp filters
// ====================================================================

ICL_REGISTER_TEST("Filter.ROI.MedianOp", "ROI handling for MedianOp") {
  MedianOp op(Size(3, 3));
  Image src = makeGradient<icl8u>(12, 12);
  testROIHandling(op, src, Rect(2, 2, 8, 8));  // interior (no extra shrink)
  testROIHandling(op, src, Rect(0, 0, 8, 8));  // edge (shrinks further)
}

ICL_REGISTER_TEST("Filter.ROI.MedianOp_5x5", "ROI handling for MedianOp 5x5") {
  MedianOp op(Size(5, 5));
  Image src = makeGradient<icl32f>(14, 14);
  testROIHandling(op, src, Rect(3, 3, 8, 8));
}

// ====================================================================
// Multi-channel tests
// ====================================================================

ICL_REGISTER_TEST("Filter.MultiChannel.threshold_3ch", "gt threshold clamps on 3-channel image") {
  Image src = Img8u{{{200, 50}},     // ch0
                    {{50, 200}},     // ch1
                    {{100, 100}}};   // ch2
  ThresholdOp op(ThresholdOp::gt, 128, 128);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 3);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 128);  // ch0: 200 clamped
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 50);   // ch0: 50 unchanged
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 1)), 50);   // ch1: 50 unchanged
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 1)), 128);  // ch1: 200 clamped
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 2)), 100);  // ch2: unchanged
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 2)), 100);  // ch2: unchanged
}

ICL_REGISTER_TEST("Filter.MultiChannel.mirror_2ch", "axisVert mirror on 2-channel image") {
  Image src = Img8u{{{1, 2, 3}},       // ch0
                    {{10, 20, 30}}};   // ch1
  MirrorOp op(axisVert);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 3);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 1);
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 1)), 30);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 1)), 10);
}

// ====================================================================
// Buffer reuse tests
// ====================================================================

ICL_REGISTER_TEST("Filter.BufferReuse.apply_twice", "second apply reuses dst buffer") {
  Image src1 = Img32f{{1.f, 2.f, 3.f, 4.f}};
  Image src2 = Img32f{{10.f, 20.f, 30.f, 40.f}};
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 1.0);

  Image dst;
  op.apply(src1, dst);
  ICL_TEST_FALSE(dst.isNull());
  ICL_TEST_NEAR(dst.as32f()(0, 0, 0), 2.0f, 1e-5f);

  op.apply(src2, dst);
  ICL_TEST_NEAR(dst.as32f()(0, 0, 0), 11.0f, 1e-5f);
}

// ====================================================================
// Depth dispatch tests
// ====================================================================

ICL_REGISTER_TEST("Filter.Depth.arithmetical_8u", "addOp works on 8u (clamped)") {
  Image src = Img8u{{100, 250}};
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 10.0);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 110);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 255);  // 260 clamped
}

ICL_REGISTER_TEST("Filter.Depth.compare_8u", "compareOp works on 8u input") {
  Image src = Img8u{{50, 128, 200}};
  UnaryCompareOp op(UnaryCompareOp::gt, 128);
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 255);
}

// ====================================================================
// ChamferOp
// ====================================================================

ICL_REGISTER_TEST("Filter.ChamferOp.output_depth", "output is always depth32s") {
  Image src = Img8u{{0, 255, 0},
                    {0, 0, 0}};
  ChamferOp op;
  Image dst = op.apply(src);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth32s));
  ICL_TEST_EQ(dst.getWidth(), 3);
  ICL_TEST_EQ(dst.getHeight(), 2);
}

ICL_REGISTER_TEST("Filter.ChamferOp.zero_at_edge", "distance is 0 at white pixels") {
  // White pixel at (1,1), rest black
  Image src = Img8u{{0, 0, 0},
                    {0, 255, 0},
                    {0, 0, 0}};
  ChamferOp op(3, 4);  // d1=3 (horiz/vert), d2=4 (diagonal)
  Image dst = op.apply(src);
  const Img32s &d = dst.as<icl32s>();
  ICL_TEST_EQ(d(1, 1, 0), 0);  // distance at white pixel is 0
}

ICL_REGISTER_TEST("Filter.ChamferOp.neighbors", "adjacent pixels get d1 distance") {
  Image src = Img8u{{0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0},
                    {0, 0, 255, 0, 0},
                    {0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0}};
  ChamferOp op(3, 4);
  Image dst = op.apply(src);
  const Img32s &d = dst.as<icl32s>();
  ICL_TEST_EQ(d(2, 2, 0), 0);  // center white pixel
  ICL_TEST_EQ(d(2, 1, 0), 3);  // one step up (d1)
  ICL_TEST_EQ(d(3, 2, 0), 3);  // one step right (d1)
  ICL_TEST_EQ(d(3, 3, 0), 4);  // one step diagonal (d2)
}

// ====================================================================
// AffineOp
// ====================================================================

ICL_REGISTER_TEST("Filter.AffineOp.identity", "identity transform preserves image") {
  Image src = Img8u{{10, 20, 30},
                    {40, 50, 60}};
  AffineOp op;
  op.setAdaptResultImage(false);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getWidth(), 3);
  ICL_TEST_EQ(dst.getHeight(), 2);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_TRUE((dst == src));
}

ICL_REGISTER_TEST("Filter.AffineOp.translate", "translation shifts pixels") {
  Image src = Img32f{{1.f, 2.f, 3.f},
                     {4.f, 5.f, 6.f},
                     {7.f, 8.f, 9.f}};
  AffineOp op;
  op.translate(1, 0);  // shift right by 1
  op.setAdaptResultImage(false);
  Image dst = op.apply(src);
  const Img32f &d = dst.as32f();
  // pixel at (1,0) in dst should come from (0,0) in src = 1.0
  ICL_TEST_NEAR(d(1, 0, 0), 1.0f, 1e-5f);
  // pixel at (2,1) in dst should come from (1,1) in src = 5.0
  ICL_TEST_NEAR(d(2, 1, 0), 5.0f, 1e-5f);
  // pixel at (0,0) in dst has no source → 0
  ICL_TEST_NEAR(d(0, 0, 0), 0.0f, 1e-5f);
}

ICL_REGISTER_TEST("Filter.AffineOp.size_preserved", "non-adapt mode preserves size") {
  Image src = make_empty(10, 8, depth32f);
  AffineOp op;
  op.rotate(45);
  op.setAdaptResultImage(false);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getWidth(), 10);
  ICL_TEST_EQ(dst.getHeight(), 8);
}

ICL_REGISTER_TEST("Filter.AffineOp.scale", "scaling doubles dimensions in adapt mode") {
  Image src = Img8u{{1, 2},
                    {3, 4}};
  AffineOp op;
  op.scale(2.0, 2.0);
  op.setAdaptResultImage(true);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getWidth(), 4);
  ICL_TEST_EQ(dst.getHeight(), 4);
}

ICL_REGISTER_TEST("Filter.AffineOp.multichannel", "affine works on multi-channel image") {
  Image src = Img8u{{{10, 20}, {30, 40}},
                    {{50, 60}, {70, 80}}};
  AffineOp op;
  op.setAdaptResultImage(false);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 2);
  ICL_TEST_TRUE((dst == src));
}

ICL_REGISTER_TEST("Filter.AffineOp.cross_validate", "all backend combos produce identical output") {
  auto src = Img8u::from(30, 30, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  Image srcImg(src);
  AffineOp op(interpolateNN);
  op.setAdaptResultImage(false);
  crossValidateBackends(op, srcImg, [&]{ return op.apply(srcImg); });
}

ICL_REGISTER_TEST("Filter.AffineOp.cross_validate_per_depth", "all combos match across depths") {
  depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  for(auto d : depths) {
    Image src(Size(30, 30), d, 1, formatMatrix);
    src.visit([](auto &img) {
      img.visitPixels([](int x, int y, int, auto &val) {
        val = static_cast<std::remove_reference_t<decltype(val)>>((x * 7 + y * 13) % 200);
      });
    });
    AffineOp op(interpolateNN);
    op.setAdaptResultImage(false);
    crossValidateBackends(op, src, [&]{ return op.apply(src); });
  }
}

// ====================================================================
// ChamferOp — additional pixel tests
// ====================================================================

ICL_REGISTER_TEST("Filter.ChamferOp.two_sources", "two white pixels create correct distances") {
  Image src = Img8u{{0, 0, 0, 0, 0, 0, 0},
                    {0, 255, 0, 0, 0, 255, 0},
                    {0, 0, 0, 0, 0, 0, 0}};
  ChamferOp op(3, 4);
  Image dst = op.apply(src);
  const Img32s &d = dst.as<icl32s>();
  ICL_TEST_EQ(d(1, 1, 0), 0);  // first source
  ICL_TEST_EQ(d(5, 1, 0), 0);  // second source
  ICL_TEST_EQ(d(3, 1, 0), 6);  // midpoint: min(2*d1 from left, 2*d1 from right)
}

ICL_REGISTER_TEST("Filter.ChamferOp.all_white", "all-white image gives all zeros") {
  Image src = Img8u{{255, 255, 255},
                    {255, 255, 255}};
  ChamferOp op(3, 4);
  Image dst = op.apply(src);
  bool ok = true;
  dst.as<icl32s>().visitPixels([&](const icl32s &v) { if(v != 0) ok = false; });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.ChamferOp.from_32f", "chamfer accepts 32f input") {
  Image src = Img32f{{0.f, 0.f, 0.f, 0.f, 0.f},
                     {0.f, 0.f, 0.f, 0.f, 0.f},
                     {0.f, 0.f, 1.f, 0.f, 0.f},
                     {0.f, 0.f, 0.f, 0.f, 0.f},
                     {0.f, 0.f, 0.f, 0.f, 0.f}};
  ChamferOp op(3, 4);
  Image dst = op.apply(src);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth32s));
  const Img32s &d = dst.as<icl32s>();
  ICL_TEST_EQ(d(2, 2, 0), 0);  // nonzero pixel → distance 0
  ICL_TEST_EQ(d(2, 1, 0), 3);  // neighbor → d1
}

// ====================================================================
// ThresholdOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.ThresholdOp.ltgt", "ltgt clamps both ends") {
  // ltgt(50,200): values < 50 → 50, values > 200 → 200, else unchanged
  ThresholdOp op(ThresholdOp::ltgt, 50, 200);
  Image src = Img8u{{10, 50, 100, 200, 250}};
  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 50);   // clamped up
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 50);   // at boundary
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 100);  // unchanged
  ICL_TEST_EQ(static_cast<int>(d(3, 0, 0)), 200);  // at boundary
  ICL_TEST_EQ(static_cast<int>(d(4, 0, 0)), 200);  // clamped down
}

ICL_REGISTER_TEST("Filter.ThresholdOp.multichannel_32f", "threshold on 3ch 32f image") {
  ThresholdOp op(ThresholdOp::gt, 5.0f, 5.0f);
  Image src = Img32f{{{1.f, 10.f}},
                     {{3.f, 20.f}},
                     {{7.f, 2.f}}};
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 3);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 1.f, 1e-5f);   // ch0: 1 unchanged
  ICL_TEST_NEAR(d(1, 0, 0), 5.f, 1e-5f);   // ch0: 10 clamped
  ICL_TEST_NEAR(d(0, 0, 2), 5.f, 1e-5f);   // ch2: 7 clamped
  ICL_TEST_NEAR(d(1, 0, 2), 2.f, 1e-5f);   // ch2: 2 unchanged
}

// ====================================================================
// UnaryArithmeticalOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.sub_pixels", "subOp subtracts constant") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::subOp, 3.0);
  ICL_TEST_TRUE((op.apply(Img32f{{10.f, 20.f, 30.f}}) == Img32f{{7.f, 17.f, 27.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.div_pixels", "divOp divides by constant") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::divOp, 2.0);
  ICL_TEST_TRUE((op.apply(Img32f{{10.f, 20.f, 30.f}}) == Img32f{{5.f, 10.f, 15.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.abs_pixels", "absOp takes absolute value") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::absOp);
  ICL_TEST_TRUE((op.apply(Img32f{{-5.f, 0.f, 3.f}}) == Img32f{{5.f, 0.f, 3.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.sqrt_pixels", "sqrtOp takes square root") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::sqrtOp);
  ICL_TEST_TRUE((op.apply(Img32f{{4.f, 9.f, 16.f}}) == Img32f{{2.f, 3.f, 4.f}}));
}

// ====================================================================
// UnaryCompareOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.UnaryCompareOp.lt_pixels", "lt comparison") {
  UnaryCompareOp op(UnaryCompareOp::lt, 10.0);
  Image dst = op.apply(Img32f{{5.f, 10.f, 15.f}});
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 255);  // 5 < 10
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0);    // 10 not < 10
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 0);    // 15 not < 10
}

ICL_REGISTER_TEST("Filter.UnaryCompareOp.eq_pixels", "eq comparison") {
  UnaryCompareOp op(UnaryCompareOp::eq, 42.0);
  Image dst = op.apply(Img8u{{41, 42, 43}});
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 0);
}

// ====================================================================
// UnaryLogicalOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.UnaryLogicalOp.xor_pixels", "xorOp flips bits") {
  UnaryLogicalOp op(UnaryLogicalOp::xorOp, 0xFF);
  ICL_TEST_TRUE((op.apply(Img8u{{0x00, 0xFF, 0xAA}}) == Img8u{{0xFF, 0x00, 0x55}}));
}

ICL_REGISTER_TEST("Filter.UnaryLogicalOp.cross_validate", "all backend combos produce identical output") {
  auto src = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  Image srcImg(src);
  UnaryLogicalOp op(UnaryLogicalOp::andOp, 0x0F);
  crossValidateBackends(op, srcImg, [&]{ return op.apply(srcImg); });
}

ICL_REGISTER_TEST("Filter.UnaryLogicalOp.cross_validate_per_depth", "all combos match across integer depths") {
  depth depths[] = { depth8u, depth16s, depth32s };
  for(auto d : depths) {
    Image src(Size(20, 15), d, 1, formatMatrix);
    src.visit([](auto &img) {
      img.visitPixels([](int x, int y, int, auto &val) {
        val = static_cast<std::remove_reference_t<decltype(val)>>((x * 7 + y * 13) % 200);
      });
    });
    UnaryLogicalOp op(UnaryLogicalOp::andOp, 0x0F);
    crossValidateBackends(op, src, [&]{ return op.apply(src); });
  }
}

ICL_REGISTER_TEST("Filter.UnaryLogicalOp.roi", "ROI handling") {
  Image src = makeGradient<icl8u>(12, 12);
  UnaryLogicalOp op(UnaryLogicalOp::andOp, 0x0F);
  testROIHandling(op, src, Rect(2, 2, 8, 8));
}

// ====================================================================
// MirrorOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.MirrorOp.both_pixels", "axisBoth flips rows and columns") {
  MirrorOp op(axisBoth);
  // [1,2; 3,4] → [4,3; 2,1]
  ICL_TEST_TRUE((op.apply(Img8u{{1,2},{3,4}}) == Img8u{{4,3},{2,1}}));
}

ICL_REGISTER_TEST("Filter.MirrorOp.single_row", "mirror single-row image") {
  MirrorOp op(axisVert);
  ICL_TEST_TRUE((op.apply(Img8u{{1,2,3,4,5}}) == Img8u{{5,4,3,2,1}}));
}

// ====================================================================
// WeightChannelsOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.WeightChannelsOp.zero_weight", "zero weight zeroes channel") {
  Image src = Img32f{{{10.f, 20.f}},
                     {{30.f, 40.f}}};
  WeightChannelsOp op({1.0, 0.0});
  Image dst = op.apply(src);
  const Img32f &d = dst.as32f();
  ICL_TEST_NEAR(d(0, 0, 0), 10.f, 1e-5f);  // ch0 unchanged
  ICL_TEST_NEAR(d(0, 0, 1), 0.f, 1e-5f);   // ch1 zeroed
}

// ====================================================================
// IntegralImgOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.IntegralImgOp.uniform", "integral of uniform image") {
  // 3x3 all ones → integral [1,2,3; 2,4,6; 3,6,9]
  Image src = Img8u{{1, 1, 1},
                    {1, 1, 1},
                    {1, 1, 1}};
  IntegralImgOp op(depth32s);
  Image dst = op.apply(src);
  const Img32s &d = dst.as<icl32s>();
  ICL_TEST_EQ(d(0, 0, 0), 1);
  ICL_TEST_EQ(d(2, 0, 0), 3);
  ICL_TEST_EQ(d(0, 2, 0), 3);
  ICL_TEST_EQ(d(2, 2, 0), 9);
  ICL_TEST_EQ(d(1, 1, 0), 4);
}

// ====================================================================
// LUTOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.LUTOp.threshold_lut", "LUT as binary threshold") {
  std::vector<icl8u> lut(256, 0);
  for(int i = 128; i < 256; ++i) lut[i] = 255;
  LUTOp op(lut);
  ICL_TEST_TRUE((op.apply(Img8u{{50, 127, 128, 200}}) == Img8u{{0, 0, 255, 255}}));
}

ICL_REGISTER_TEST("Filter.LUTOp.multichannel", "LUT works on multi-channel image") {
  std::vector<icl8u> lut(256);
  for(int i = 0; i < 256; ++i) lut[i] = 255 - i;
  LUTOp op(lut);
  Image src = Img8u{{{10, 20}}, {{30, 40}}};
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 2);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 245);  // 255-10
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 1)), 225);  // 255-30
}

ICL_REGISTER_TEST("Filter.LUTOp.cross_validate", "all backend combos produce identical output") {
  auto src = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  Image srcImg(src);
  LUTOp op(32);
  crossValidateBackends(op, srcImg, [&]{ return op.apply(srcImg); });
}

ICL_REGISTER_TEST("Filter.LUTOp.cross_validate_levels", "all combos match for various quantization levels") {
  auto src = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  Image srcImg(src);
  for(int levels : {4, 16, 64, 128}) {
    LUTOp op(levels);
    crossValidateBackends(op, srcImg, [&]{ return op.apply(srcImg); });
  }
}

// ====================================================================
// ColorDistanceOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.ColorDistanceOp.output_size", "output is single-channel, matches input size") {
  Image src = Img32f{{{1.f, 2.f, 3.f}, {4.f, 5.f, 6.f}},
                     {{1.f, 2.f, 3.f}, {4.f, 5.f, 6.f}},
                     {{1.f, 2.f, 3.f}, {4.f, 5.f, 6.f}}};
  std::vector<double> ref = {0.0, 0.0, 0.0};
  ColorDistanceOp op(ref, -1);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 1);
  ICL_TEST_EQ(dst.getWidth(), 3);
  ICL_TEST_EQ(dst.getHeight(), 2);
}

// ====================================================================
// DitheringOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.DitheringOp.all_white", "all-white input stays white") {
  Image src = Img8u{{255, 255, 255, 255},
                    {255, 255, 255, 255}};
  DitheringOp op(DitheringOp::FloydSteinberg, 2);
  Image dst = op.apply(src);
  bool ok = true;
  dst.as8u().visitPixels([&](const icl8u &v) { if(v != 255) ok = false; });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.DitheringOp.all_black", "all-black input stays black") {
  Image src = Img8u{{0, 0, 0, 0},
                    {0, 0, 0, 0}};
  DitheringOp op(DitheringOp::FloydSteinberg, 2);
  Image dst = op.apply(src);
  bool ok = true;
  dst.as8u().visitPixels([&](const icl8u &v) { if(v != 0) ok = false; });
  ICL_TEST_TRUE(ok);
}

// ====================================================================
// ColorSegmentationOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.ColorSegmentationOp.multi_class", "multiple classes are distinguished") {
  ColorSegmentationOp op(0, 0, 0, formatRGB);
  op.clearLUT(0);
  op.lutEntry(200, 0, 0, 10, 10, 10, 1);    // red → class 1
  op.lutEntry(0, 200, 0, 10, 10, 10, 2);    // green → class 2

  Image src(Size(3, 1), depth8u, 3, formatRGB);
  Img8u &s = src.as8u();
  s(0,0,0)=200; s(0,0,1)=0;   s(0,0,2)=0;    // red
  s(1,0,0)=0;   s(1,0,1)=200; s(1,0,2)=0;    // green
  s(2,0,0)=0;   s(2,0,1)=0;   s(2,0,2)=200;  // blue (no class)

  Image dst = op.apply(src);
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 1);  // red → class 1
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 2);  // green → class 2
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 0);  // blue → unclassified
}

// ====================================================================
// GaborOp — additional tests
// ====================================================================

ICL_REGISTER_TEST("Filter.GaborOp.single_kernel", "single kernel gives 1 output channel") {
  GaborOp op(Size(3, 3), {5.0f}, {0.0f}, {0.0f}, {2.0f}, {1.0f});
  Image src = make_empty(10, 10, depth32f);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getChannels(), 1);
}

// ====================================================================
// CannyOp
// ====================================================================

ICL_REGISTER_TEST("Filter.CannyOp.output_depth", "output is always depth8u") {
  Image src = make_empty(20, 20, depth8u);
  CannyOp op(10, 100);
  Image dst = op.apply(src);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_TRUE(dst.getWidth() > 0);
  ICL_TEST_TRUE(dst.getHeight() > 0);
}

ICL_REGISTER_TEST("Filter.CannyOp.uniform_no_edges", "uniform image produces no edges") {
  auto src = Img8u::from(20, 20, 1, [](int,int,int) -> icl8u { return 128; });
  CannyOp op(10, 100);
  Image dst = op.apply(Image(src));
  bool anyEdge = false;
  dst.as8u().visitPixels([&](const icl8u &v) { if(v == 255) anyEdge = true; });
  ICL_TEST_FALSE(anyEdge);
}

ICL_REGISTER_TEST("Filter.CannyOp.strong_edge", "strong vertical edge is detected") {
  auto src = Img8u::from(20, 20, 1, [](int x,int,int) -> icl8u { return (x < 10) ? 0 : 255; });
  CannyOp op(10, 50);
  Image dst = op.apply(Image(src));
  bool hasEdge = false;
  dst.as8u().visitPixels([&](int x, int y, const icl8u &v) {
    if(y >= 2 && y < dst.getHeight()-2 && x >= 8 && x <= 12 && v == 255) hasEdge = true;
  });
  ICL_TEST_TRUE(hasEdge);
}

ICL_REGISTER_TEST("Filter.CannyOp.no_clip_roi", "non-clipToROI preserves full image size") {
  auto src = Img8u::from(20, 20, 1, [](int x,int,int) -> icl8u { return (x < 10) ? 0 : 255; });
  CannyOp op(10, 50);
  op.setClipToROI(false);
  Image dst;
  op.apply(Image(src), dst);
  ICL_TEST_EQ(dst.getWidth(), 20);
  ICL_TEST_EQ(dst.getHeight(), 20);
  bool ok = true;
  dst.as8u().visitPixels([&](const icl8u &v) { if(v != 0 && v != 255) ok = false; });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.CannyOp.binary_output", "output only contains 0 and 255") {
  auto src = Img8u::from(20, 20, 1, [](int x, int y, int) -> icl8u { return (icl8u)((x+y)*6); });
  CannyOp op(5, 30);
  Image dst = op.apply(Image(src));
  bool ok = true;
  dst.as8u().visitPixels([&](const icl8u &v) { if(v != 0 && v != 255) ok = false; });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.GaborOp.impulse_response", "gabor on impulse produces non-zero output") {
  GaborOp op(Size(3, 3), {3.0f}, {0.0f}, {0.0f}, {1.0f}, {1.0f});
  Img32f src(Size(7, 7), 1);
  src.clear();
  src(3, 3, 0) = 255.f;
  Image dst = op.apply(Image(src));
  bool hasNonZero = false;
  dst.as32f().visitPixels([&](const icl32f &v) { if(std::abs(v) > 1e-6f) hasNonZero = true; });
  ICL_TEST_TRUE(hasNonZero);
}

// ============================================================
// LocalThresholdOp tests
// ============================================================

ICL_REGISTER_TEST("Filter.LocalThreshold.regionMean_binary", "regionMean produces binary output") {
  // Left half dark, right half bright
  auto src = Img8u::from(40, 40, 1, [](int x, int, int) -> icl8u { return (x < 20) ? 30 : 200; });
  LocalThresholdOp op(5, 0);
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  // Output should be binary (0 or 255)
  bool ok = true;
  dst.as8u().visitPixels([&](const icl8u &v) { if(v != 0 && v != 255) ok = false; });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.LocalThreshold.tiledNN", "tiledNN algorithm works") {
  auto src = Img8u::from(40, 40, 1, [](int x, int, int) -> icl8u { return (x < 20) ? 30 : 200; });
  LocalThresholdOp op(LocalThresholdOp::tiledNN, 5, 0);
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_TRUE(dst.getWidth() > 0);
}

ICL_REGISTER_TEST("Filter.LocalThreshold.tiledLIN", "tiledLIN algorithm works") {
  auto src = Img8u::from(40, 40, 1, [](int x, int, int) -> icl8u { return (x < 20) ? 30 : 200; });
  LocalThresholdOp op(LocalThresholdOp::tiledLIN, 5, 0);
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_TRUE(dst.getWidth() > 0);
}

ICL_REGISTER_TEST("Filter.LocalThreshold.global", "global algorithm delegates to simple threshold") {
  auto src = Img8u::from(20, 20, 1, [](int x, int y, int) -> icl8u { return x + y * 20; });
  LocalThresholdOp op(LocalThresholdOp::global, 5, 100);
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  // Pixels > 100 should be 255
  ICL_TEST_EQ(dst.as8u()(10, 10, 0), (icl8u)255);  // 10+200=210 > 100
  // Pixels <= 100 should be 0
  ICL_TEST_EQ(dst.as8u()(0, 0, 0), (icl8u)0);  // 0 <= 100
}

// ============================================================
// ConvolutionOp tests
// ============================================================

ICL_REGISTER_TEST("Filter.ConvolutionOp.identity_3x3", "identity kernel preserves image") {
  // Identity 3x3 kernel: center=1, all others=0, factor=1
  int k[] = {0,0,0, 0,1,0, 0,0,0};
  ConvolutionOp op(ConvolutionKernel(k, Size(3,3), 1, false));
  auto src = Img32f::from(7, 7, 1, [](int x, int y, int) -> icl32f { return x + y * 7.f; });
  Image dst = op.apply(Image(src));
  // output is 5x5 (shrunk by 1), values should match src interior
  ICL_TEST_EQ(dst.getWidth(), 5);
  ICL_TEST_EQ(dst.getHeight(), 5);
  ICL_TEST_EQ(dst.as32f()(0, 0, 0), src(1, 1, 0));
  ICL_TEST_EQ(dst.as32f()(4, 4, 0), src(5, 5, 0));
}

ICL_REGISTER_TEST("Filter.ConvolutionOp.sobelX_detects_vertical", "sobelX responds to vertical edge") {
  auto src = Img8u::from(20, 20, 1, [](int x,int,int) -> icl8u { return (x < 10) ? 0 : 255; });
  ConvolutionKernel kernel{ConvolutionKernel::sobelX3x3};
  ConvolutionOp op{kernel};
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth16s));
  ICL_TEST_TRUE(std::abs(dst.as16s()(9, 9, 0)) > 100);
}

ICL_REGISTER_TEST("Filter.ConvolutionOp.gauss_smooths", "gauss3x3 reduces impulse") {
  Img32f src(Size(7, 7), 1);
  src.clear();
  src(3, 3, 0) = 100.f;
  ConvolutionKernel kernel{ConvolutionKernel::gauss3x3};
  ConvolutionOp op{kernel};
  Image dst = op.apply(Image(src));
  ICL_TEST_TRUE(dst.as32f()(2, 2, 0) < 100.f);
  ICL_TEST_TRUE(dst.as32f()(2, 2, 0) > 0.f);
}

ICL_REGISTER_TEST("Filter.ConvolutionOp.force_unsigned", "forceUnsignedOutput keeps 8u depth") {
  auto src = Img8u::from(7, 7, 1, [](int x, int y, int) -> icl8u { return (x+y) * 5; });
  ConvolutionKernel kernel{ConvolutionKernel::sobelX3x3};
  ConvolutionOp op{kernel, true};
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
}

ICL_REGISTER_TEST("Filter.ConvolutionOp.custom_float_kernel", "custom float kernel works") {
  // Averaging 3x3 kernel
  float k[] = {1,1,1, 1,1,1, 1,1,1};
  ConvolutionOp op(ConvolutionKernel(k, Size(3,3), false));
  Img32f src(Size(5, 5), 1);
  src.clear(-1, 9.f);
  Image dst = op.apply(Image(src));
  // Uniform image: average of nine 9s = 81 (no factor normalization for float kernel)
  ICL_TEST_NEAR(dst.as32f()(0, 0, 0), 81.f, 0.01f);
}

ICL_REGISTER_TEST("Filter.ROI.ConvolutionOp", "ROI handling for ConvolutionOp") {
  ConvolutionKernel kernel{ConvolutionKernel::gauss3x3};
  ConvolutionOp op{kernel};
  Image src = makeGradient<icl32f>(12, 12);
  testROIHandling(op, src, Rect(2, 2, 8, 8));
}

ICL_REGISTER_TEST("Filter.ConvolutionOp.cross_validate", "all backend combos produce identical output") {
  auto src = Img8u::from(30, 20, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  Image srcImg(src);
  ConvolutionKernel kernel{ConvolutionKernel::gauss3x3};
    ConvolutionOp op{kernel};
  crossValidateBackends(op, srcImg, [&]{ return op.apply(srcImg); });
}

ICL_REGISTER_TEST("Filter.ConvolutionOp.cross_validate_per_depth", "all combos match across depths") {
  depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  for(auto d : depths) {
    Image src(Size(30, 20), d, 1, formatMatrix);
    src.visit([](auto &img) {
      img.visitPixels([](int x, int y, int, auto &val) {
        val = static_cast<std::remove_reference_t<decltype(val)>>((x * 7 + y * 13) % 200);
      });
    });
    ConvolutionKernel kernel{ConvolutionKernel::gauss3x3};
    ConvolutionOp op{kernel};
    crossValidateBackends(op, src, [&]{ return op.apply(src); });
  }
}

ICL_REGISTER_TEST("Filter.ConvolutionOp.even_4x4_identity", "4x4 identity kernel output size and values") {
  // 4x4 kernel with identity at anchor (2,2): only element at (2,2)=1
  int k[] = {0,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,0};
  ConvolutionOp op(ConvolutionKernel(k, Size(4,4), 1, false));
  auto src = Img32f::from(10, 10, 1, [](int x, int y, int) -> icl32f { return x + y * 10.f; });
  Image dst = op.apply(Image(src));
  // 4x4 mask, anchor (2,2): left/top border=2, right/bottom border=1 → 7x7 output
  ICL_TEST_EQ(dst.getWidth(), 7);
  ICL_TEST_EQ(dst.getHeight(), 7);
  // dst(0,0) ← src(2,2), dst(6,6) ← src(8,8)
  ICL_TEST_EQ(dst.as32f()(0, 0, 0), src(2, 2, 0));
  ICL_TEST_EQ(dst.as32f()(6, 6, 0), src(8, 8, 0));
  ICL_TEST_EQ(dst.as32f()(3, 3, 0), src(5, 5, 0));
}

ICL_REGISTER_TEST("Filter.ConvolutionOp.even_2x2_sum", "2x2 sum kernel output size") {
  int k[] = {1,1, 1,1};
  ConvolutionOp op(ConvolutionKernel(k, Size(2,2), 1, false));
  auto src = Img32f::from(8, 8, 1, [](int x, int y, int) -> icl32f { return 1.f; });
  Image dst = op.apply(Image(src));
  // 2x2 mask, anchor (1,1): left/top border=1, right/bottom border=0 → 7x7 output
  ICL_TEST_EQ(dst.getWidth(), 7);
  ICL_TEST_EQ(dst.getHeight(), 7);
  // sum of four 1.0s = 4.0
  ICL_TEST_NEAR(dst.as32f()(0, 0, 0), 4.f, 0.01f);
  ICL_TEST_NEAR(dst.as32f()(6, 6, 0), 4.f, 0.01f);
}

ICL_REGISTER_TEST("Filter.ConvolutionOp.even_4x4_cross_validate", "even kernel backends match") {
  auto src = Img32f::from(20, 15, 1, [](int x, int y, int) -> icl32f {
    return static_cast<icl32f>((x * 7 + y * 13) % 200);
  });
  Image srcImg(src);
  float k[] = {1,2,1,0, 2,4,2,0, 1,2,1,0, 0,0,0,0};
  ConvolutionOp op(ConvolutionKernel(k, Size(4,4), false));
  crossValidateBackends(op, srcImg, [&]{ return op.apply(srcImg); });
}

// ============================================================
// MorphologicalOp tests
// ============================================================

ICL_REGISTER_TEST("Filter.MorphOp.dilate_expands_white", "dilate grows bright regions") {
  Img8u src(Size(7, 7), 1);
  src.clear();
  src(3, 3, 0) = 255;  // single white pixel
  MorphologicalOp op(MorphologicalOp::dilate, Size(3, 3));
  Image dst = op.apply(Image(src));
  // center and its 4-neighbors should be 255
  ICL_TEST_EQ(dst.as8u()(2, 2, 0), (icl8u)255);  // center of dst maps to src(3,3)
  ICL_TEST_EQ(dst.as8u()(1, 2, 0), (icl8u)255);  // left neighbor
  ICL_TEST_EQ(dst.as8u()(3, 2, 0), (icl8u)255);  // right neighbor
}

ICL_REGISTER_TEST("Filter.MorphOp.erode_shrinks_white", "erode shrinks bright regions") {
  Img8u src(Size(7, 7), 1);
  src.clear(-1, 255);
  src(0, 0, 0) = 0;  // single black pixel at corner
  MorphologicalOp op(MorphologicalOp::erode, Size(3, 3));
  Image dst = op.apply(Image(src));
  // erosion with 3x3 mask: pixels near the black corner become 0
  ICL_TEST_EQ(dst.as8u()(0, 0, 0), (icl8u)0);
}

ICL_REGISTER_TEST("Filter.MorphOp.dilate_uniform", "dilate on uniform image is identity") {
  Img8u src(Size(7, 7), 1);
  src.clear(-1, 100);
  MorphologicalOp op(MorphologicalOp::dilate, Size(3, 3));
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(dst.getWidth(), 5);
  ICL_TEST_EQ(dst.getHeight(), 5);
  // all output pixels should be 100 (max of uniform neighborhood)
  ICL_TEST_EQ(dst.as8u()(0, 0, 0), (icl8u)100);
  ICL_TEST_EQ(dst.as8u()(2, 2, 0), (icl8u)100);
  ICL_TEST_EQ(dst.as8u()(4, 4, 0), (icl8u)100);
}

ICL_REGISTER_TEST("Filter.MorphOp.erode3x3", "erode3x3 shortcut works") {
  Img8u src(Size(7, 7), 1);
  src.clear(-1, 255);
  src(3, 3, 0) = 0;
  MorphologicalOp op(MorphologicalOp::erode3x3);
  Image dst = op.apply(Image(src));
  // erode3x3 should spread the 0 to neighbors
  ICL_TEST_EQ(dst.as8u()(2, 2, 0), (icl8u)0);
}

ICL_REGISTER_TEST("Filter.MorphOp.32f", "morphological ops work on 32f") {
  auto src = Img32f::from(7, 7, 1, [](int x, int y, int) -> icl32f { return (x == 3 && y == 3) ? 100.f : 0.f; });
  MorphologicalOp op(MorphologicalOp::dilate, Size(3, 3));
  Image dst = op.apply(Image(src));
  // dilated: center region should be 100
  ICL_TEST_EQ(dst.as32f()(2, 2, 0), 100.f);
}

ICL_REGISTER_TEST("Filter.ROI.MorphOp", "ROI handling for MorphologicalOp") {
  MorphologicalOp op(MorphologicalOp::erode, Size(3, 3));
  Image src = makeGradient<icl8u>(12, 12);
  testROIHandling(op, src, Rect(2, 2, 8, 8));
}

ICL_REGISTER_TEST("Filter.MorphOp.cross_validate", "all backend combos produce identical output") {
  auto src = Img8u::from(30, 20, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  Image srcImg(src);
  MorphologicalOp op(MorphologicalOp::dilate, Size(3, 3));
  crossValidateBackends(op, srcImg, [&]{ return op.apply(srcImg); });
}

ICL_REGISTER_TEST("Filter.MorphOp.cross_validate_per_depth", "all combos match across supported depths") {
  depth depths[] = { depth8u, depth32f };  // MorphologicalOp only supports 8u and 32f
  for(auto d : depths) {
    Image src(Size(30, 20), d, 1, formatMatrix);
    src.visit([](auto &img) {
      img.visitPixels([](int x, int y, int, auto &val) {
        val = static_cast<std::remove_reference_t<decltype(val)>>((x * 7 + y * 13) % 200);
      });
    });
    MorphologicalOp op(MorphologicalOp::erode, Size(3, 3));
    crossValidateBackends(op, src, [&]{ return op.apply(src); });
  }
}

// ============================================================
// MedianOp tests
// ============================================================

ICL_REGISTER_TEST("Filter.MedianOp.3x3_8u", "3x3 median picks middle value") {
  // 5x5 src → 3x3 dst (border shrink = 1 on each side)
  Img8u src(Size(5, 5), 1);
  src.clear();
  // Fill a 3x3 region centered at (2,2): values 1..9
  icl8u vals[] = {3, 7, 1, 5, 9, 2, 8, 4, 6};
  int idx = 0;
  for (int y = 1; y <= 3; y++)
    for (int x = 1; x <= 3; x++)
      src(x, y, 0) = vals[idx++];

  MedianOp op(Size(3, 3));
  Image dst = op.apply(Image(src));
  // dst(1,1) maps to src center (2,2) whose 3x3 neighborhood = {3,7,1,5,9,2,8,4,6}
  // sorted: {1,2,3,4,5,6,7,8,9} → median = 5
  ICL_TEST_EQ(dst.as8u()(1, 1, 0), (icl8u)5);
}

ICL_REGISTER_TEST("Filter.MedianOp.3x3_32f", "3x3 median on float image") {
  Img32f src(Size(5, 5), 1);
  src.clear();
  icl32f vals[] = {3, 7, 1, 5, 9, 2, 8, 4, 6};
  int idx = 0;
  for (int y = 1; y <= 3; y++)
    for (int x = 1; x <= 3; x++)
      src(x, y, 0) = vals[idx++];

  MedianOp op(Size(3, 3));
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(dst.as32f()(1, 1, 0), 5.0f);
}

ICL_REGISTER_TEST("Filter.MedianOp.3x3_uniform", "3x3 median preserves uniform region") {
  Img8u src(Size(7, 7), 1);
  src.clear(-1, 42);
  MedianOp op(Size(3, 3));
  Image dst = op.apply(Image(src));
  bool ok = true;
  dst.as8u().visitPixels([&](const icl8u &v) { if(v != 42) ok = false; });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.MedianOp.3x3_removes_impulse", "3x3 median removes salt-and-pepper noise") {
  Img8u src(Size(5, 5), 1);
  src.clear(-1, 100);
  src(2, 2, 0) = 255;  // impulse

  MedianOp op(Size(3, 3));
  Image dst = op.apply(Image(src));
  // Center pixel should be 100 (median of eight 100s and one 255)
  ICL_TEST_EQ(dst.as8u()(0, 0, 0), (icl8u)100);
}

ICL_REGISTER_TEST("Filter.MedianOp.5x5_8u", "5x5 median on 8u image") {
  // 9x9 src → 5x5 dst (border shrink = 2 on each side)
  Img8u src(Size(9, 9), 1);
  src.clear();
  // Fill the 5x5 neighborhood of center (4,4): values 1..25
  icl8u val = 1;
  for (int y = 2; y <= 6; y++)
    for (int x = 2; x <= 6; x++)
      src(x, y, 0) = val++;

  MedianOp op(Size(5, 5));
  Image dst = op.apply(Image(src));
  // dst(2,2) maps to src center (4,4) whose 5x5 neighborhood = 1..25, median = 13
  ICL_TEST_EQ(dst.as8u()(2, 2, 0), (icl8u)13);
}

ICL_REGISTER_TEST("Filter.MedianOp.5x5_32f", "5x5 median on 32f image") {
  Img32f src(Size(9, 9), 1);
  src.clear();
  icl32f val = 1;
  for (int y = 2; y <= 6; y++)
    for (int x = 2; x <= 6; x++)
      src(x, y, 0) = val++;

  MedianOp op(Size(5, 5));
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(dst.as32f()(2, 2, 0), 13.0f);
}

ICL_REGISTER_TEST("Filter.MedianOp.7x7_arbitrary_mask", "7x7 median (arbitrary mask path)") {
  Img8u src(Size(11, 11), 1);
  src.clear(-1, 50);
  src(5, 5, 0) = 200;  // single outlier

  MedianOp op(Size(7, 7));
  Image dst = op.apply(Image(src));
  // 7x7 = 49 pixels, 48 are 50, 1 is 200. Median = 50.
  // Check a pixel whose mask includes the outlier
  const Img8u &d = dst.as8u();
  // dst size = 11-6 = 5, center of dst (2,2) maps to src (5,5)
  ICL_TEST_EQ(d(0, 0, 0), (icl8u)50);
}

ICL_REGISTER_TEST("Filter.MedianOp.multichannel", "3x3 median on multi-channel image") {
  Img8u src(Size(5, 5), 2);
  src.clear();
  // Channel 0: center 3x3 = 1..9
  // Channel 1: center 3x3 = 11..19
  icl8u vals0[] = {3, 7, 1, 5, 9, 2, 8, 4, 6};
  icl8u vals1[] = {13, 17, 11, 15, 19, 12, 18, 14, 16};
  int idx = 0;
  for (int y = 1; y <= 3; y++)
    for (int x = 1; x <= 3; x++) {
      src(x, y, 0) = vals0[idx];
      src(x, y, 1) = vals1[idx];
      idx++;
    }

  MedianOp op(Size(3, 3));
  Image dst = op.apply(Image(src));
  // dst(1,1) maps to src center (2,2) — full 3x3 neighborhood of set values
  ICL_TEST_EQ(dst.as8u()(1, 1, 0), (icl8u)5);
  ICL_TEST_EQ(dst.as8u()(1, 1, 1), (icl8u)15);
}

ICL_REGISTER_TEST("Filter.MedianOp.output_size", "median output has correct shrunk size") {
  Img32f src(Size(10, 8), 1);
  src.clear();
  MedianOp op3(Size(3, 3));
  Image dst3 = op3.apply(Image(src));
  ICL_TEST_EQ(dst3.getWidth(), 8);
  ICL_TEST_EQ(dst3.getHeight(), 6);

  MedianOp op5(Size(5, 5));
  Image dst5 = op5.apply(Image(src));
  ICL_TEST_EQ(dst5.getWidth(), 6);
  ICL_TEST_EQ(dst5.getHeight(), 4);
}

ICL_REGISTER_TEST("Filter.MedianOp.even_mask_rounds_up", "even mask size is rounded to odd") {
  // MedianOp(Size(4,4)) should adapt to Size(5,5)
  MedianOp op(Size(4, 4));
  ICL_TEST_EQ(op.getMaskSize(), Size(5, 5));
}

ICL_REGISTER_TEST("Filter.MedianOp.3x3_16s", "3x3 median on 16s image") {
  Img16s src(Size(5, 5), 1);
  src.clear();
  icl16s vals[] = {3, 7, 1, 5, 9, 2, 8, 4, 6};
  int idx = 0;
  for (int y = 1; y <= 3; y++)
    for (int x = 1; x <= 3; x++)
      src(x, y, 0) = vals[idx++];

  MedianOp op(Size(3, 3));
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(dst.as16s()(1, 1, 0), (icl16s)5);
}

ICL_REGISTER_TEST("Filter.MedianOp.3x3_32s", "3x3 median on 32s image") {
  Img32s src(Size(5, 5), 1);
  src.clear();
  icl32s vals[] = {3, 7, 1, 5, 9, 2, 8, 4, 6};
  int idx = 0;
  for (int y = 1; y <= 3; y++)
    for (int x = 1; x <= 3; x++)
      src(x, y, 0) = vals[idx++];

  MedianOp op(Size(3, 3));
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(dst.as32s()(1, 1, 0), (icl32s)5);
}

ICL_REGISTER_TEST("Filter.MedianOp.3x3_64f", "3x3 median on 64f image") {
  Img64f src(Size(5, 5), 1);
  src.clear();
  icl64f vals[] = {3, 7, 1, 5, 9, 2, 8, 4, 6};
  int idx = 0;
  for (int y = 1; y <= 3; y++)
    for (int x = 1; x <= 3; x++)
      src(x, y, 0) = vals[idx++];

  MedianOp op(Size(3, 3));
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(dst.as64f()(1, 1, 0), 5.0);
}

ICL_REGISTER_TEST("Filter.MedianOp.larger_image", "3x3 median on a larger image") {
  auto src = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u { return (x + y * 20) % 256; });

  MedianOp op(Size(3, 3));
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(dst.getWidth(), 18);
  ICL_TEST_EQ(dst.getHeight(), 13);

  // Spot-check center pixel: src neighborhood at (10,7):
  // rows 6-8, cols 9-11
  // (9+6*20)%256=129, (10+6*20)%256=130, (11+6*20)%256=131
  // (9+7*20)%256=149, (10+7*20)%256=150, (11+7*20)%256=151
  // (9+8*20)%256=169, (10+8*20)%256=170, (11+8*20)%256=171
  // sorted: 129,130,131,149,150,151,169,170,171 → median=150
  ICL_TEST_EQ(dst.as8u()(9, 6, 0), (icl8u)150);
}

ICL_REGISTER_TEST("Filter.MedianOp.huang_nontrivial", "7x7 Huang median with non-trivial values") {
  auto src = Img8u::from(11, 11, 1, [](int x, int y, int) -> icl8u { return (x * 7 + y * 13) % 256; });

  MedianOp op(Size(7, 7));
  Image dst = op.apply(Image(src));
  // dst is 5x5 (11 - 6)
  ICL_TEST_EQ(dst.getWidth(), 5);
  ICL_TEST_EQ(dst.getHeight(), 5);

  // Verify center pixel dst(2,2) → src center (5,5), 7x7 neighborhood (2,2)-(8,8)
  // Collect expected values manually
  std::vector<icl8u> vals;
  for (int y = 2; y <= 8; y++)
    for (int x = 2; x <= 8; x++)
      vals.push_back((x * 7 + y * 13) % 256);
  std::sort(vals.begin(), vals.end());
  ICL_TEST_EQ(dst.as8u()(2, 2, 0), vals[49/2]);
}

ICL_REGISTER_TEST("Filter.MedianOp.cross_validate", "all backend combos produce identical output") {
  auto src = Img8u::from(30, 20, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  Image srcImg(src);
  MedianOp op(Size(3, 3));
  crossValidateBackends(op, srcImg, [&]{ return op.apply(srcImg); });
}

ICL_REGISTER_TEST("Filter.MedianOp.cross_validate_per_depth", "all combos match across depths") {
  depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  for(auto d : depths) {
    Image src(Size(30, 20), d, 1, formatMatrix);
    src.visit([](auto &img) {
      img.visitPixels([](int x, int y, int, auto &val) {
        val = static_cast<std::remove_reference_t<decltype(val)>>((x * 7 + y * 13) % 200);
      });
    });
    MedianOp op(Size(3, 3));
    crossValidateBackends(op, src, [&]{ return op.apply(src); });
  }
}

// ============================================================
// WarpOp tests
// ============================================================

// Helper: create an identity warp map (dst pixel maps to same src pixel)
static Img32f makeIdentityWarpMap(int w, int h) {
  return Img32f::from(w, h, 2, [](int x, int y, int c) -> icl32f {
    return c == 0 ? static_cast<icl32f>(x) : static_cast<icl32f>(y);
  });
}

// Helper: create a shift warp map (dst pixel reads from src shifted by dx,dy)
static Img32f makeShiftWarpMap(int w, int h, float dx, float dy) {
  return Img32f::from(w, h, 2, [dx, dy](int x, int y, int c) -> icl32f {
    return c == 0 ? x + dx : y + dy;
  });
}

ICL_REGISTER_TEST("Filter.WarpOp.identity_nn", "identity warp with NN preserves image") {
  auto src = Img8u::from(10, 8, 1, [](int x, int y, int) -> icl8u {
    return x + y * 10;
  });
  Img32f wm = makeIdentityWarpMap(10, 8);
  WarpOp op(wm, interpolateNN);
  op.forceAll(Backend::Cpp);
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(dst.getSize(), src.getSize());
  ICL_TEST_TRUE((dst.as8u() == src));
}

ICL_REGISTER_TEST("Filter.WarpOp.identity_lin", "identity warp with LIN preserves image") {
  auto src = Img32f::from(10, 8, 1, [](int x, int y, int) -> icl32f {
    return x * 1.5f + y * 3.0f;
  });
  Img32f wm = makeIdentityWarpMap(10, 8);
  WarpOp op(wm, interpolateLIN);
  Image dst = op.apply(Image(src));
  ICL_TEST_TRUE((dst.as32f() == src));
}

ICL_REGISTER_TEST("Filter.WarpOp.shift_nn", "integer shift warp with NN") {
  auto src = Img8u::from(10, 10, 1, [](int x, int y, int) -> icl8u {
    return x + y * 10;
  });
  // Shift by (2, 3): dst(x,y) reads from src(x+2, y+3)
  Img32f wm = makeShiftWarpMap(10, 10, 2, 3);
  WarpOp op(wm, interpolateNN);
  op.forceAll(Backend::Cpp);
  Image dst = op.apply(Image(src));
  // Interior pixels should match shifted source
  // dst(0,0) = src(2,3) = 2+3*10 = 32
  ICL_TEST_EQ(dst.as8u()(0, 0, 0), (icl8u)32);
  // dst(3,4) = src(5,7) = 5+7*10 = 75
  ICL_TEST_EQ(dst.as8u()(3, 4, 0), (icl8u)75);
  // Out-of-bounds: dst(8,8) = src(10,11) → out of bounds → 0
  ICL_TEST_EQ(dst.as8u()(8, 8, 0), (icl8u)0);
}

ICL_REGISTER_TEST("Filter.WarpOp.multichannel", "warp works on multi-channel images") {
  auto src = Img8u::from(8, 8, 3, [](int x, int y, int c) -> icl8u {
    return x + y * 8 + c * 64;
  });
  Img32f wm = makeIdentityWarpMap(8, 8);
  WarpOp op(wm, interpolateNN);
  op.forceAll(Backend::Cpp);
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(dst.getChannels(), 3);
  ICL_TEST_TRUE((dst.as8u() == src));
}

ICL_REGISTER_TEST("Filter.WarpOp.all_depths_identity", "identity warp works for all depths") {
  depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  Img32f wm = makeIdentityWarpMap(8, 6);

  for(auto d : depths) {
    Image src(Size(8, 6), d, 1, formatMatrix);
    src.visit([](auto &img) {
      img.visitPixels([](int x, int y, int, auto &val) {
        val = static_cast<std::remove_reference_t<decltype(val)>>(x + y * 8);
      });
    });
    WarpOp op(wm, interpolateNN);
    op.forceAll(Backend::Cpp);
    Image dst = op.apply(src);
    ICL_TEST_EQ(dst.getSize(), src.getSize());
    ICL_TEST_EQ(dst.getDepth(), src.getDepth());
    // Spot check: pixel (3,2) = 3+2*8 = 19
    bool ok = false;
    dst.visit([&](const auto &img) {
      using T = typename std::remove_reference_t<decltype(img)>::type;
      ok = (img(3, 2, 0) == static_cast<T>(19));
    });
    ICL_TEST_TRUE(ok);
  }
}

ICL_REGISTER_TEST("Filter.WarpOp.lin_subpixel", "bilinear interpolation produces correct sub-pixel values") {
  // 4x4 image with value = x (so we can check horizontal interpolation)
  auto src = Img32f::from(4, 4, 1, [](int x, int, int) -> icl32f {
    return static_cast<icl32f>(x);
  });
  // Warp map: shift by 0.5 in x → dst(x,y) reads from src(x+0.5, y)
  Img32f wm = makeShiftWarpMap(4, 4, 0.5f, 0.0f);
  WarpOp op(wm, interpolateLIN);
  Image dst = op.apply(Image(src));
  // dst(0,0) = src(0.5, 0) = lerp(src(0,0), src(1,0), 0.5) = (0+1)/2 = 0.5
  ICL_TEST_EQ(dst.as32f()(0, 0, 0), 0.5f);
  // dst(1,0) = src(1.5, 0) = lerp(1, 2, 0.5) = 1.5
  ICL_TEST_EQ(dst.as32f()(1, 0, 0), 1.5f);
}

ICL_REGISTER_TEST("Filter.WarpOp.oob_returns_zero", "out-of-bounds warp coords produce zero") {
  Img8u src(Size(4, 4), 1);
  src.clear(-1, 100);  // fill with 100
  // Warp map: all coords point to (-1, -1) → out of bounds
  auto wm = Img32f::from(4, 4, 2, [](int, int, int) -> icl32f { return -1.0f; });
  WarpOp op(wm, interpolateNN);
  Image dst = op.apply(Image(src));
  bool allZero = true;
  dst.as8u().visitPixels([&](const icl8u &v) { if(v != 0) allZero = false; });
  ICL_TEST_TRUE(allZero);

  // Also test LIN
  WarpOp opLin(wm, interpolateLIN);
  Image dstLin = opLin.apply(Image(src));
  bool allZeroLin = true;
  dstLin.as8u().visitPixels([&](const icl8u &v) { if(v != 0) allZeroLin = false; });
  ICL_TEST_TRUE(allZeroLin);
}

ICL_REGISTER_TEST("Filter.WarpOp.warp_map_scaling", "warp map auto-scales to match image size") {
  // 4x4 identity warp map, 8x8 source — scaling interpolates coordinate values,
  // so scaled identity != identity at new resolution. Just verify it works and
  // produces correct size output.
  Img32f wm = makeIdentityWarpMap(4, 4);
  auto src = Img8u::from(8, 8, 1, [](int x, int y, int) -> icl8u {
    return x + y * 8;
  });
  WarpOp op(wm, interpolateNN, true);
  Image dst = op.apply(Image(src));
  ICL_TEST_EQ(dst.getSize(), src.getSize());
  ICL_TEST_EQ(dst.getDepth(), depth8u);
  // Corner (0,0) maps to (0,0) regardless of scaling
  ICL_TEST_EQ(dst.as8u()(0, 0, 0), src(0, 0, 0));
}

ICL_REGISTER_TEST("Filter.WarpOp.roi_clip", "clipToROI produces correctly sized output") {
  auto src = Img8u::from(10, 10, 1, [](int x, int y, int) -> icl8u {
    return x + y * 10;
  });
  Img32f wm = makeIdentityWarpMap(10, 10);
  WarpOp op(wm, interpolateNN);
  op.setClipToROI(true);

  Image srcImg(src);
  srcImg.setROI(Rect(2, 3, 5, 4));
  Image dst = op.apply(srcImg);

  // Clipped output should be ROI-sized
  ICL_TEST_EQ(dst.getWidth(), 5);
  ICL_TEST_EQ(dst.getHeight(), 4);
  // dst(0,0) should be src(2,3) = 2+3*10 = 32 (identity warp, offset by ROI)
  ICL_TEST_EQ(dst.as8u()(0, 0, 0), (icl8u)32);
  // dst(4,3) should be src(6,6) = 6+6*10 = 66
  ICL_TEST_EQ(dst.as8u()(4, 3, 0), (icl8u)66);
}

ICL_REGISTER_TEST("Filter.WarpOp.roi_nonclip_sentinel", "non-clip ROI preserves sentinel outside ROI") {
  auto src = Img8u::from(10, 10, 1, [](int x, int y, int) -> icl8u {
    return x + y * 10;
  });
  Img32f wm = makeIdentityWarpMap(10, 10);
  WarpOp op(wm, interpolateNN);
  op.setClipToROI(false);

  Image srcImg(src);
  srcImg.setROI(Rect(2, 2, 6, 6));

  // Pre-fill dst with sentinel
  Image dst(Size(10, 10), depth8u, 1);
  dst.clear(-1, 222);
  op.apply(srcImg, dst);

  // dst should be full size
  ICL_TEST_EQ(dst.getSize(), Size(10, 10));

  // ROI pixels should match source (identity warp)
  const Img8u &d = dst.as8u();
  bool roiOK = true;
  for(int y = 2; y < 8; ++y)
    for(int x = 2; x < 8; ++x)
      if(d(x, y, 0) != src(x, y, 0)) roiOK = false;
  ICL_TEST_TRUE(roiOK);

  // Sentinel should survive outside ROI
  bool sentinelOK = true;
  for(int y = 0; y < 10; ++y)
    for(int x = 0; x < 10; ++x) {
      bool inROI = x >= 2 && x < 8 && y >= 2 && y < 8;
      if(!inROI && d(x, y, 0) != 222) sentinelOK = false;
    }
  ICL_TEST_TRUE(sentinelOK);
}

ICL_REGISTER_TEST("Filter.WarpOp.roi_clip_nonclip_consistent", "clip and non-clip ROI produce same pixel values") {
  auto src = Img8u::from(12, 12, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  // Use a shift warp to make it non-trivial
  Img32f wm = makeShiftWarpMap(12, 12, 1, 2);
  Rect roi(3, 3, 6, 6);

  Image srcImg(src);
  srcImg.setROI(roi);

  WarpOp opClip(wm, interpolateNN);
  opClip.setClipToROI(true);
  Image clipped = opClip.apply(srcImg);

  WarpOp opFull(wm, interpolateNN);
  opFull.setClipToROI(false);
  Image full(Size(12, 12), depth8u, 1);
  full.clear(-1, 222);
  opFull.apply(srcImg, full);

  // clipped content should match full's ROI content
  ICL_TEST_EQ(clipped.getSize(), Size(6, 6));
  const Img8u &c = clipped.as8u();
  const Img8u &f = full.as8u();
  bool match = true;
  for(int y = 0; y < 6; ++y)
    for(int x = 0; x < 6; ++x)
      if(c(x, y, 0) != f(x + 3, y + 3, 0)) match = false;
  ICL_TEST_TRUE(match);
}

ICL_REGISTER_TEST("Filter.WarpOp.cross_validate", "all backend combos produce identical output") {
  auto src = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  Img32f wm = makeIdentityWarpMap(20, 15);
  Image srcImg(src);
  WarpOp op(wm, interpolateNN);
  crossValidateBackends(op, srcImg, [&]{ return op.apply(srcImg); });
}

ICL_REGISTER_TEST("Filter.WarpOp.cross_validate_per_depth", "all combos match across depths") {
  depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  for(auto d : depths) {
    Image src(Size(20, 15), d, 1, formatMatrix);
    src.visit([](auto &img) {
      img.visitPixels([](int x, int y, int, auto &val) {
        val = static_cast<std::remove_reference_t<decltype(val)>>((x * 7 + y * 13) % 200);
      });
    });
    Img32f wm = makeIdentityWarpMap(20, 15);
    WarpOp op(wm, interpolateNN);
    crossValidateBackends(op, src, [&]{ return op.apply(src); });
  }
}

// ============================================================
// ThresholdOp — FilterDispatch framework proof-of-concept
// ============================================================

ICL_REGISTER_TEST("Filter.ThresholdOp.basic_ltval", "ltVal clamps below threshold") {
  Image src = Img8u{{10, 50, 100, 150},
                    {200, 250, 30, 60}};
  ThresholdOp op(ThresholdOp::ltVal, 100, 100, 0, 255);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.getWidth(), 4);
  ICL_TEST_EQ(dst.getHeight(), 2);
  // pixels < 100 should become 0
  ICL_TEST_EQ(dst.as8u()(0, 0, 0), (icl8u)0);   // 10 < 100 → 0
  ICL_TEST_EQ(dst.as8u()(1, 0, 0), (icl8u)0);   // 50 < 100 → 0
  ICL_TEST_EQ(dst.as8u()(2, 0, 0), (icl8u)100);  // 100 not < 100 → unchanged
  ICL_TEST_EQ(dst.as8u()(3, 0, 0), (icl8u)150);  // 150 → unchanged
}

ICL_REGISTER_TEST("Filter.ThresholdOp.basic_gtval", "gtVal clamps above threshold") {
  Image src = Img8u{{10, 50, 100, 150, 200}};
  ThresholdOp op(ThresholdOp::gtVal, 100, 100, 0, 255);
  Image dst = op.apply(src);
  ICL_TEST_EQ(dst.as8u()(0, 0, 0), (icl8u)10);   // unchanged
  ICL_TEST_EQ(dst.as8u()(3, 0, 0), (icl8u)255);  // 150 > 100 → 255
  ICL_TEST_EQ(dst.as8u()(4, 0, 0), (icl8u)255);  // 200 > 100 → 255
}

ICL_REGISTER_TEST("Filter.ThresholdOp.introspection", "dispatch introspection works") {
  ThresholdOp op(ThresholdOp::ltVal);
  auto* ltVal = op.selector(ThresholdOp::Op::ltVal);
  ICL_TEST_TRUE(ltVal != nullptr);

  auto backends = ltVal->registeredBackends();
  // Should have at least C++
  bool hasCpp = false;
  for(auto b : backends) if(b == Backend::Cpp) hasCpp = true;
  ICL_TEST_TRUE(hasCpp);

  // bestBackendFor should return something
  Image src = Img8u{{1, 2, 3}};
  Backend best = ltVal->bestBackendFor(src);
  (void)best; // just verify it doesn't crash

  // switches() should return 3
  ICL_TEST_EQ((int)op.selectors().size(), 3);
}

ICL_REGISTER_TEST("Filter.ThresholdOp.force_backend", "forced backend produces same result as cascade") {
  auto src = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  ThresholdOp op(ThresholdOp::ltVal, 100, 100, 42, 200);
  Image srcImg(src);

  // Default (cascade) result
  Image cascadeDst = op.apply(srcImg);

  // Force C++ fallback
  op.forceAll(Backend::Cpp);
  Image cppDst = op.apply(srcImg);
  op.unforceAll();

  // Both should match
  ICL_TEST_TRUE((cascadeDst.as8u() == cppDst.as8u()));
}

ICL_REGISTER_TEST("Filter.ThresholdOp.cross_validate_all_combos", "all backend combos produce identical output") {
  // Test ltgtVal — exercises all 3 switches
  auto src = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  ThresholdOp op(ThresholdOp::ltgtVal, 50, 200, 10, 240);
  Image srcImg(src);

  // Reference: all-C++ fallback
  op.forceAll(Backend::Cpp);
  Image ref = op.apply(srcImg);
  op.unforceAll();

  // Test every cartesian product of backend combinations
  auto combos = op.allBackendCombinations(srcImg);
  int nCombos = 0;
  bool allMatch = true;
  op.forEachCombination(combos, [&](const std::vector<Backend>&) {
    Image dst = op.apply(srcImg);
    if(!(dst.as8u() == ref.as8u())) allMatch = false;
    nCombos++;
  });

  ICL_TEST_TRUE(allMatch);
  ICL_TEST_TRUE(nCombos >= 1);  // at least the all-C++ combo
}

ICL_REGISTER_TEST("Filter.ThresholdOp.cross_validate_per_depth", "all combos match for each depth") {
  // Verify across multiple depths (8u uses SIMD, 32f uses SIMD, 16s/32s/64f use C++ only)
  depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  for(auto d : depths) {
    Image src(Size(20, 15), d, 1, formatMatrix);
    src.visit([](auto &img) {
      img.visitPixels([](int x, int y, int, auto &val) {
        val = static_cast<std::remove_reference_t<decltype(val)>>((x * 7 + y * 13) % 200);
      });
    });

    ThresholdOp op(ThresholdOp::ltVal, 100, 100, 42, 42);

    op.forceAll(Backend::Cpp);
    Image ref = op.apply(src);
    op.unforceAll();

    auto combos = op.allBackendCombinations(src);
    bool allMatch = true;
    op.forEachCombination(combos, [&](const std::vector<Backend>&) {
      Image dst = op.apply(src);
      dst.visit([&](const auto &dImg) {
        using T = typename std::remove_reference_t<decltype(dImg)>::type;
        if(!(dImg == ref.as<T>())) allMatch = false;
      });
    });
    ICL_TEST_TRUE(allMatch);
  }
}

// ====================================================================
// UnaryCompareOp — BackendDispatch framework
// ====================================================================

ICL_REGISTER_TEST("Filter.UnaryCompareOp.basic_gt", "gt produces 0/255") {
  Image src = Img32f{{5.0f, 15.0f, 20.0f, 25.0f}};
  UnaryCompareOp op(UnaryCompareOp::gt, 15.0);
  Image dst = op.apply(src);
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(d(3, 0, 0)), 255);
}

ICL_REGISTER_TEST("Filter.UnaryCompareOp.basic_lt", "lt comparison") {
  UnaryCompareOp op(UnaryCompareOp::lt, 10.0);
  Image dst = op.apply(Img32f{{5.f, 10.f, 15.f}});
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 0);
}

ICL_REGISTER_TEST("Filter.UnaryCompareOp.basic_eq", "eq comparison") {
  UnaryCompareOp op(UnaryCompareOp::eq, 42.0);
  Image dst = op.apply(Img8u{{41, 42, 43}});
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 0);
}

ICL_REGISTER_TEST("Filter.UnaryCompareOp.basic_lteq", "lteq comparison") {
  UnaryCompareOp op(UnaryCompareOp::lteq, 10.0);
  Image dst = op.apply(Img32f{{5.f, 10.f, 15.f}});
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 0);
}

ICL_REGISTER_TEST("Filter.UnaryCompareOp.basic_gteq", "gteq comparison") {
  UnaryCompareOp op(UnaryCompareOp::gteq, 10.0);
  Image dst = op.apply(Img32f{{5.f, 10.f, 15.f}});
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 255);
}

ICL_REGISTER_TEST("Filter.UnaryCompareOp.basic_eqt", "eqt with tolerance") {
  UnaryCompareOp op(UnaryCompareOp::eqt, 10.0, 2.0);
  Image dst = op.apply(Img32f{{7.f, 8.f, 10.f, 12.f, 13.f}});
  const Img8u &d = dst.as8u();
  ICL_TEST_EQ(static_cast<int>(d(0, 0, 0)), 0);    // |7-10|=3 > 2
  ICL_TEST_EQ(static_cast<int>(d(1, 0, 0)), 255);   // |8-10|=2 <= 2
  ICL_TEST_EQ(static_cast<int>(d(2, 0, 0)), 255);   // |10-10|=0 <= 2
  ICL_TEST_EQ(static_cast<int>(d(3, 0, 0)), 255);   // |12-10|=2 <= 2
  ICL_TEST_EQ(static_cast<int>(d(4, 0, 0)), 0);    // |13-10|=3 > 2
}

ICL_REGISTER_TEST("Filter.UnaryCompareOp.introspection", "dispatch introspection") {
  UnaryCompareOp op(UnaryCompareOp::gt);
  auto* cmp = op.selector(UnaryCompareOp::Op::compare);
  ICL_TEST_TRUE(cmp != nullptr);
  auto* eqt = op.selector(UnaryCompareOp::Op::compareEqTol);
  ICL_TEST_TRUE(eqt != nullptr);
  ICL_TEST_EQ((int)op.selectors().size(), 2);

  auto backends = cmp->registeredBackends();
  bool hasCpp = false;
  for(auto b : backends) if(b == Backend::Cpp) hasCpp = true;
  ICL_TEST_TRUE(hasCpp);
}

ICL_REGISTER_TEST("Filter.UnaryCompareOp.cross_validate", "all backend combos match for gt") {
  auto src = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u {
    return (x * 7 + y * 13) % 256;
  });
  UnaryCompareOp op(UnaryCompareOp::gt, 100);
  Image srcImg(src);

  op.forceAll(Backend::Cpp);
  Image ref = op.apply(srcImg);
  op.unforceAll();

  auto combos = op.allBackendCombinations(srcImg);
  bool allMatch = true;
  int nCombos = 0;
  op.forEachCombination(combos, [&](const std::vector<Backend>&) {
    Image dst = op.apply(srcImg);
    if(!(dst.as8u() == ref.as8u())) allMatch = false;
    nCombos++;
  });
  ICL_TEST_TRUE(allMatch);
  ICL_TEST_TRUE(nCombos >= 1);
}

ICL_REGISTER_TEST("Filter.UnaryCompareOp.cross_validate_per_depth", "all combos match across depths") {
  depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  for(auto d : depths) {
    Image src(Size(20, 15), d, 1, formatMatrix);
    src.visit([](auto &img) {
      img.visitPixels([](int x, int y, int, auto &val) {
        val = static_cast<std::remove_reference_t<decltype(val)>>((x * 7 + y * 13) % 200);
      });
    });
    UnaryCompareOp op(UnaryCompareOp::lt, 100);
    op.forceAll(Backend::Cpp);
    Image ref = op.apply(src);
    op.unforceAll();

    auto combos = op.allBackendCombinations(src);
    bool allMatch = true;
    op.forEachCombination(combos, [&](const std::vector<Backend>&) {
      Image dst = op.apply(src);
      if(!(dst.as8u() == ref.as8u())) allMatch = false;
    });
    ICL_TEST_TRUE(allMatch);
  }
}

// ====================================================================
// UnaryArithmeticalOp — BackendDispatch framework
// ====================================================================

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.basic_add", "addOp adds constant") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 5.0);
  ICL_TEST_TRUE((op.apply(Img32f{{10.f, 20.f, 30.f}}) == Img32f{{15.f, 25.f, 35.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.basic_sub", "subOp subtracts constant") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::subOp, 3.0);
  ICL_TEST_TRUE((op.apply(Img32f{{10.f, 20.f, 30.f}}) == Img32f{{7.f, 17.f, 27.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.basic_mul", "mulOp multiplies") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::mulOp, 2.0);
  ICL_TEST_TRUE((op.apply(Img32f{{10.f, 20.f, 30.f}}) == Img32f{{20.f, 40.f, 60.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.basic_div", "divOp divides") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::divOp, 2.0);
  ICL_TEST_TRUE((op.apply(Img32f{{10.f, 20.f, 30.f}}) == Img32f{{5.f, 10.f, 15.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.basic_sqr", "sqrOp squares") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::sqrOp);
  ICL_TEST_TRUE((op.apply(Img32f{{3.f, 4.f, 5.f}}) == Img32f{{9.f, 16.f, 25.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.basic_sqrt", "sqrtOp takes sqrt") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::sqrtOp);
  ICL_TEST_TRUE((op.apply(Img32f{{4.f, 9.f, 16.f}}) == Img32f{{2.f, 3.f, 4.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.basic_abs", "absOp takes abs") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::absOp);
  ICL_TEST_TRUE((op.apply(Img32f{{-5.f, 0.f, 3.f}}) == Img32f{{5.f, 0.f, 3.f}}));
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.introspection", "dispatch introspection") {
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 1);
  ICL_TEST_EQ((int)op.selectors().size(), 2);
  auto* wv = op.selector(UnaryArithmeticalOp::Op::withVal);
  ICL_TEST_TRUE(wv != nullptr);
  auto* nv = op.selector(UnaryArithmeticalOp::Op::noVal);
  ICL_TEST_TRUE(nv != nullptr);
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.cross_validate", "all backend combos match for add") {
  auto src = Img32f::from(20, 15, 1, [](int x, int y, int) -> icl32f {
    return x * 3.7f + y * 11.3f;
  });
  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 42.5);
  Image srcImg(src);

  op.forceAll(Backend::Cpp);
  Image ref = op.apply(srcImg);
  op.unforceAll();

  auto combos = op.allBackendCombinations(srcImg);
  bool allMatch = true;
  int nCombos = 0;
  op.forEachCombination(combos, [&](const std::vector<Backend>&) {
    Image dst = op.apply(srcImg);
    if(!(dst.as32f() == ref.as32f())) allMatch = false;
    nCombos++;
  });
  ICL_TEST_TRUE(allMatch);
  ICL_TEST_TRUE(nCombos >= 1);
}

ICL_REGISTER_TEST("Filter.UnaryArithmeticalOp.cross_validate_per_depth", "all combos match across depths") {
  depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  for(auto d : depths) {
    Image src(Size(20, 15), d, 1, formatMatrix);
    src.visit([](auto &img) {
      img.visitPixels([](int x, int y, int, auto &val) {
        val = static_cast<std::remove_reference_t<decltype(val)>>(1 + (x * 7 + y * 13) % 100);
      });
    });
    UnaryArithmeticalOp op(UnaryArithmeticalOp::sqrOp);
    op.forceAll(Backend::Cpp);
    Image ref = op.apply(src);
    op.unforceAll();

    auto combos = op.allBackendCombinations(src);
    bool allMatch = true;
    op.forEachCombination(combos, [&](const std::vector<Backend>&) {
      Image dst = op.apply(src);
      dst.visit([&](const auto &dImg) {
        using T = typename std::remove_reference_t<decltype(dImg)>::type;
        if(!(dImg == ref.as<T>())) allMatch = false;
      });
    });
    ICL_TEST_TRUE(allMatch);
  }
}

// ====================================================================
// BinaryArithmeticalOp
// ====================================================================

ICL_REGISTER_TEST("Filter.BinaryArithmeticalOp.add", "pixel-wise addition") {
  BinaryArithmeticalOp op(BinaryArithmeticalOp::addOp);
  ICL_TEST_TRUE((op.apply(Image(Img32f{{1.f, 2.f, 3.f}}), Image(Img32f{{10.f, 20.f, 30.f}}))
                 == Img32f{{11.f, 22.f, 33.f}}));
}

ICL_REGISTER_TEST("Filter.BinaryArithmeticalOp.sub", "pixel-wise subtraction") {
  BinaryArithmeticalOp op(BinaryArithmeticalOp::subOp);
  ICL_TEST_TRUE((op.apply(Image(Img32f{{10.f, 20.f, 30.f}}), Image(Img32f{{1.f, 2.f, 3.f}}))
                 == Img32f{{9.f, 18.f, 27.f}}));
}

ICL_REGISTER_TEST("Filter.BinaryArithmeticalOp.mul", "pixel-wise multiplication") {
  BinaryArithmeticalOp op(BinaryArithmeticalOp::mulOp);
  ICL_TEST_TRUE((op.apply(Image(Img32f{{2.f, 3.f, 4.f}}), Image(Img32f{{5.f, 6.f, 7.f}}))
                 == Img32f{{10.f, 18.f, 28.f}}));
}

ICL_REGISTER_TEST("Filter.BinaryArithmeticalOp.div", "pixel-wise division") {
  BinaryArithmeticalOp op(BinaryArithmeticalOp::divOp);
  ICL_TEST_TRUE((op.apply(Image(Img32f{{10.f, 20.f, 30.f}}), Image(Img32f{{2.f, 4.f, 5.f}}))
                 == Img32f{{5.f, 5.f, 6.f}}));
}

ICL_REGISTER_TEST("Filter.BinaryArithmeticalOp.absSub", "pixel-wise abs(a-b)") {
  BinaryArithmeticalOp op(BinaryArithmeticalOp::absSubOp);
  ICL_TEST_TRUE((op.apply(Image(Img32f{{1.f, 5.f, 10.f}}), Image(Img32f{{3.f, 2.f, 10.f}}))
                 == Img32f{{2.f, 3.f, 0.f}}));
}

ICL_REGISTER_TEST("Filter.BinaryArithmeticalOp.8u", "works on 8u depth") {
  BinaryArithmeticalOp op(BinaryArithmeticalOp::addOp);
  ICL_TEST_TRUE((op.apply(Image(Img8u{{10, 20, 30}}), Image(Img8u{{1, 2, 3}}))
                 == Img8u{{11, 22, 33}}));
}

ICL_REGISTER_TEST("Filter.BinaryArithmeticalOp.multichannel", "works with multiple channels") {
  auto a = Img32f::from(5, 5, 2, [](int x, int y, int c) -> icl32f { return x + y + c * 10.f; });
  auto b = Img32f::from(5, 5, 2, [](int x, int y, int) -> icl32f { return 1.f; });
  BinaryArithmeticalOp op(BinaryArithmeticalOp::addOp);
  Image dst = op.apply(Image(a), Image(b));
  ICL_TEST_EQ(dst.getChannels(), 2);
  ICL_TEST_EQ(dst.as32f()(0, 0, 0), 1.f);
  ICL_TEST_EQ(dst.as32f()(0, 0, 1), 11.f);
}

ICL_REGISTER_TEST("Filter.BinaryArithmeticalOp.introspection", "dispatch introspection") {
  BinaryArithmeticalOp op(BinaryArithmeticalOp::addOp);
  ICL_TEST_EQ((int)op.selectors().size(), 1);
  ICL_TEST_TRUE(op.selector(BinaryArithmeticalOp::Op::apply) != nullptr);
}

ICL_REGISTER_TEST("Filter.BinaryArithmeticalOp.cross_validate", "all backend combos produce identical output") {
  auto s1 = Img32f::from(20, 15, 1, [](int x, int y, int) -> icl32f { return x * 1.5f + y; });
  auto s2 = Img32f::from(20, 15, 1, [](int x, int y, int) -> icl32f { return x + y * 2.0f; });
  Image src1(s1), src2(s2);
  BinaryArithmeticalOp op(BinaryArithmeticalOp::addOp);
  crossValidateBackends(op, src1, [&]{ return op.apply(src1, src2); });
}

ICL_REGISTER_TEST("Filter.BinaryArithmeticalOp.cross_validate_per_depth", "all combos match across depths") {
  depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  for(auto d : depths) {
    Image src1(Size(20, 15), d, 1, formatMatrix);
    Image src2(Size(20, 15), d, 1, formatMatrix);
    src1.visit([](auto &img) { img.visitPixels([](int x, int y, int, auto &v) {
      v = static_cast<std::remove_reference_t<decltype(v)>>((x * 3 + y * 5) % 100); }); });
    src2.visit([](auto &img) { img.visitPixels([](int x, int y, int, auto &v) {
      v = static_cast<std::remove_reference_t<decltype(v)>>((x * 7 + y * 2) % 100); }); });
    BinaryArithmeticalOp op(BinaryArithmeticalOp::addOp);
    crossValidateBackends(op, src1, [&]{ return op.apply(src1, src2); });
  }
}

// ====================================================================
// BinaryCompareOp
// ====================================================================

ICL_REGISTER_TEST("Filter.BinaryCompareOp.gt", "pixel-wise > comparison") {
  BinaryCompareOp op(BinaryCompareOp::gt);
  Image dst = op.apply(Image(Img32f{{5.f, 10.f, 15.f}}), Image(Img32f{{10.f, 10.f, 10.f}}));
  ICL_TEST_EQ(static_cast<int>(dst.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(0, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(1, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(2, 0, 0)), 255);
}

ICL_REGISTER_TEST("Filter.BinaryCompareOp.lt", "pixel-wise < comparison") {
  BinaryCompareOp op(BinaryCompareOp::lt);
  Image dst = op.apply(Image(Img32f{{5.f, 10.f, 15.f}}), Image(Img32f{{10.f, 10.f, 10.f}}));
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(0, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(1, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(2, 0, 0)), 0);
}

ICL_REGISTER_TEST("Filter.BinaryCompareOp.eq", "pixel-wise == comparison") {
  BinaryCompareOp op(BinaryCompareOp::eq);
  Image dst = op.apply(Image(Img8u{{1, 2, 3}}), Image(Img8u{{1, 5, 3}}));
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(0, 0, 0)), 255);
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(1, 0, 0)), 0);
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(2, 0, 0)), 255);
}

ICL_REGISTER_TEST("Filter.BinaryCompareOp.eqt", "pixel-wise == with tolerance") {
  BinaryCompareOp op(BinaryCompareOp::eqt, 2.0);
  Image dst = op.apply(Image(Img32f{{10.f, 10.f, 10.f}}), Image(Img32f{{8.f, 11.f, 15.f}}));
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(0, 0, 0)), 255);  // |10-8|=2 <= 2
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(1, 0, 0)), 255);  // |10-11|=1 <= 2
  ICL_TEST_EQ(static_cast<int>(dst.as8u()(2, 0, 0)), 0);    // |10-15|=5 > 2
}

ICL_REGISTER_TEST("Filter.BinaryCompareOp.introspection", "dispatch introspection") {
  BinaryCompareOp op(BinaryCompareOp::gt);
  ICL_TEST_EQ((int)op.selectors().size(), 2);
  ICL_TEST_TRUE(op.selector(BinaryCompareOp::Op::compare) != nullptr);
  ICL_TEST_TRUE(op.selector(BinaryCompareOp::Op::compareEqTol) != nullptr);
}

ICL_REGISTER_TEST("Filter.BinaryCompareOp.cross_validate", "all backend combos produce identical output") {
  auto s1 = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u { return (x * 7 + y * 13) % 256; });
  auto s2 = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u { return (x * 3 + y * 19) % 256; });
  Image src1(s1), src2(s2);
  BinaryCompareOp op(BinaryCompareOp::gt);
  crossValidateBackends(op, src1, [&]{ return op.apply(src1, src2); });
}

ICL_REGISTER_TEST("Filter.BinaryCompareOp.cross_validate_per_depth", "all combos match across depths") {
  depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  for(auto d : depths) {
    Image src1(Size(20, 15), d, 1, formatMatrix);
    Image src2(Size(20, 15), d, 1, formatMatrix);
    src1.visit([](auto &img) { img.visitPixels([](int x, int y, int, auto &v) {
      v = static_cast<std::remove_reference_t<decltype(v)>>((x * 7 + y * 13) % 200); }); });
    src2.visit([](auto &img) { img.visitPixels([](int x, int y, int, auto &v) {
      v = static_cast<std::remove_reference_t<decltype(v)>>((x * 3 + y * 19) % 200); }); });
    BinaryCompareOp op(BinaryCompareOp::gt);
    crossValidateBackends(op, src1, [&]{ return op.apply(src1, src2); });
  }
}

// ====================================================================
// BinaryLogicalOp
// ====================================================================

ICL_REGISTER_TEST("Filter.BinaryLogicalOp.and", "pixel-wise AND") {
  BinaryLogicalOp op(BinaryLogicalOp::andOp);
  ICL_TEST_TRUE((op.apply(Image(Img8u{{0xFF, 0xF0, 0x0F}}), Image(Img8u{{0xFF, 0x0F, 0x0F}}))
                 == Img8u{{0xFF, 0x00, 0x0F}}));
}

ICL_REGISTER_TEST("Filter.BinaryLogicalOp.or", "pixel-wise OR") {
  BinaryLogicalOp op(BinaryLogicalOp::orOp);
  ICL_TEST_TRUE((op.apply(Image(Img8u{{0xF0, 0x0F, 0x00}}), Image(Img8u{{0x0F, 0x0F, 0xFF}}))
                 == Img8u{{0xFF, 0x0F, 0xFF}}));
}

ICL_REGISTER_TEST("Filter.BinaryLogicalOp.xor", "pixel-wise XOR") {
  BinaryLogicalOp op(BinaryLogicalOp::xorOp);
  ICL_TEST_TRUE((op.apply(Image(Img8u{{0xFF, 0xF0, 0x00}}), Image(Img8u{{0xFF, 0x0F, 0x00}}))
                 == Img8u{{0x00, 0xFF, 0x00}}));
}

ICL_REGISTER_TEST("Filter.BinaryLogicalOp.32s", "works on 32s depth") {
  BinaryLogicalOp op(BinaryLogicalOp::andOp);
  ICL_TEST_TRUE((op.apply(Image(Img32s{{0xFF00, 0x00FF}}), Image(Img32s{{0xFFFF, 0xFFFF}}))
                 == Img32s{{0xFF00, 0x00FF}}));
}

ICL_REGISTER_TEST("Filter.BinaryLogicalOp.introspection", "dispatch introspection") {
  BinaryLogicalOp op(BinaryLogicalOp::andOp);
  ICL_TEST_EQ((int)op.selectors().size(), 1);
  ICL_TEST_TRUE(op.selector(BinaryLogicalOp::Op::apply) != nullptr);
}

ICL_REGISTER_TEST("Filter.BinaryLogicalOp.cross_validate", "all backend combos produce identical output") {
  auto s1 = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u { return (x * 7 + y * 13) % 256; });
  auto s2 = Img8u::from(20, 15, 1, [](int x, int y, int) -> icl8u { return (x * 3 + y * 19) % 256; });
  Image src1(s1), src2(s2);
  BinaryLogicalOp op(BinaryLogicalOp::andOp);
  crossValidateBackends(op, src1, [&]{ return op.apply(src1, src2); });
}

ICL_REGISTER_TEST("Filter.BinaryLogicalOp.cross_validate_per_depth", "all combos match across integer depths") {
  depth depths[] = { depth8u, depth16s, depth32s };
  for(auto d : depths) {
    Image src1(Size(20, 15), d, 1, formatMatrix);
    Image src2(Size(20, 15), d, 1, formatMatrix);
    src1.visit([](auto &img) { img.visitPixels([](int x, int y, int, auto &v) {
      v = static_cast<std::remove_reference_t<decltype(v)>>((x * 7 + y * 13) % 200); }); });
    src2.visit([](auto &img) { img.visitPixels([](int x, int y, int, auto &v) {
      v = static_cast<std::remove_reference_t<decltype(v)>>((x * 3 + y * 19) % 200); }); });
    BinaryLogicalOp op(BinaryLogicalOp::andOp);
    crossValidateBackends(op, src1, [&]{ return op.apply(src1, src2); });
  }
}

// ====================================================================
// BilateralFilterOp
// ====================================================================

ICL_REGISTER_TEST("Filter.BilateralFilterOp.uniform_8u", "uniform input passes through unchanged") {
  Img8u src(Size(20, 20), 1);
  src.clear(-1, 128);
  BilateralFilterOp op(3, 2.f, 30.f, false);
  Image dst = op.apply(Image(src));
  bool ok = true;
  dst.as8u().visitPixels([&](const icl8u &v) { if(v != 128) ok = false; });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.uniform_32f", "uniform float passes through") {
  Img32f src(Size(20, 20), 1);
  src.clear(-1, 42.5f);
  BilateralFilterOp op(3, 2.f, 30.f, false);
  Image dst = op.apply(Image(src));
  bool ok = true;
  dst.as32f().visitPixels([&](const icl32f &v) {
    if(std::abs(v - 42.5f) > 0.01f) ok = false;
  });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.radius_zero", "radius=0 is identity") {
  auto src = Img8u::from(10, 10, 1, [](int x, int y, int) -> icl8u {
    return x + y * 10;
  });
  BilateralFilterOp op(0, 2.f, 30.f, false);
  Image dst = op.apply(Image(src));
  ICL_TEST_TRUE(dst == Image(src));
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.smoothing", "reduces noise on smooth region") {
  // Create a mostly-constant image with a single noisy pixel
  Img8u src(Size(11, 11), 1);
  src.clear(-1, 100);
  src(5, 5, 0) = 200;  // single bright pixel in uniform background

  BilateralFilterOp op(2, 2.f, 50.f, false);
  Image dst = op.apply(Image(src));

  // The bright pixel should be attenuated towards the background
  icl8u center = dst.as8u()(5, 5, 0);
  ICL_TEST_TRUE(center < 200);
  ICL_TEST_TRUE(center > 100);

  // Surrounding pixels should still be close to 100
  icl8u neighbor = dst.as8u()(3, 3, 0);
  ICL_TEST_TRUE(neighbor >= 99 && neighbor <= 101);
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.edge_preservation", "preserves sharp edges") {
  // Step edge: left half = 0, right half = 255
  auto src = Img8u::from(20, 10, 1, [](int x, int, int) -> icl8u {
    return x < 10 ? 0 : 255;
  });

  // Strong edge preservation: small sigma_r means distant values get low weight
  BilateralFilterOp op(3, 3.f, 10.f, false);
  Image dst = op.apply(Image(src));

  // Pixels well within each region should stay near original
  icl8u left_val = dst.as8u()(2, 5, 0);
  icl8u right_val = dst.as8u()(17, 5, 0);
  ICL_TEST_TRUE(left_val < 10);
  ICL_TEST_TRUE(right_val > 245);
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.color_3ch", "3-channel with use_lab=false") {
  Img8u src(Size(10, 10), formatRGB);
  src.clear(-1, 100);
  BilateralFilterOp op(2, 2.f, 30.f, false);
  op.forceAll(Backend::Cpp);  // OpenCL has float precision differences on 8u uniform
  Image dst = op.apply(Image(src));

  // Uniform input → uniform output for all 3 channels
  bool ok = true;
  for(int c = 0; c < 3; ++c) {
    const icl8u* data = dst.as8u().getData(c);
    for(int i = 0; i < 100; ++i) {
      if(data[i] != 100) ok = false;
    }
  }
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.color_lab", "3-channel with use_lab=true") {
  Img8u src(Size(10, 10), formatRGB);
  src.clear(-1, 100);
  BilateralFilterOp op(2, 2.f, 30.f, true);
  op.forceAll(Backend::Cpp);  // OpenCL has float precision differences on 8u uniform
  Image dst = op.apply(Image(src));

  // Uniform input → uniform output (LAB distance is 0 for identical pixels)
  bool ok = true;
  for(int c = 0; c < 3; ++c) {
    const icl8u* data = dst.as8u().getData(c);
    for(int i = 0; i < 100; ++i) {
      if(data[i] != 100) ok = false;
    }
  }
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.all_depths", "C++ backend handles all 5 depths") {
  const depth depths[] = { depth8u, depth16s, depth32s, depth32f, depth64f };
  for(depth d : depths) {
    Image src(Size(10, 10), d, 1, formatMatrix);
    src.clear();
    BilateralFilterOp op(1, 1.f, 10.f, false);
    op.forceAll(Backend::Cpp);
    Image dst = op.apply(src);
    ICL_TEST_EQ(dst.getDepth(), d);
    ICL_TEST_EQ(dst.getSize(), Size(10, 10));
  }
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.symmetric", "symmetric input gives symmetric output") {
  // Symmetric horizontal gradient
  auto src = Img8u::from(11, 1, 1, [](int x, int, int) -> icl8u {
    return static_cast<icl8u>(std::abs(x - 5) * 50);  // V-shape: 250,200,...,0,...,200,250
  });
  BilateralFilterOp op(2, 2.f, 100.f, false);
  Image dst = op.apply(Image(src));

  // Output should be symmetric around center
  const Img8u& d = dst.as8u();
  bool ok = true;
  for(int x = 0; x < 5; ++x) {
    if(d(x, 0, 0) != d(10 - x, 0, 0)) ok = false;
  }
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.introspection", "dispatch introspection") {
  BilateralFilterOp op;
  ICL_TEST_EQ((int)op.selectors().size(), 1);
  ICL_TEST_TRUE(op.selector(BilateralFilterOp::Op::apply) != nullptr);

  // Cpp backend should always be registered
  auto backends = op.selector(BilateralFilterOp::Op::apply)->registeredBackends();
  bool hasCpp = false;
  for(auto b : backends) { if(b == Backend::Cpp) hasCpp = true; }
  ICL_TEST_TRUE(hasCpp);
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.cross_validate_mono8u", "C++ and OpenCL match for mono 8u") {
  auto src = Img8u::from(30, 20, 1, [](int x, int y, int) -> icl8u {
    return static_cast<icl8u>((x * 7 + y * 13) % 256);
  });
  BilateralFilterOp op(2, 2.f, 30.f, false);
  Image srcImg(src);

  op.forceAll(Backend::Cpp);
  Image ref = op.apply(srcImg);
  op.unforceAll();

  auto combos = op.allBackendCombinations(srcImg);
  bool allMatch = true;
  int nCombos = 0;
  op.forEachCombination(combos, [&](const std::vector<Backend>&) {
    Image dst = op.apply(srcImg);
    if(!(dst.as8u() == ref.as8u())) allMatch = false;
    nCombos++;
  });
  ICL_TEST_TRUE(allMatch);
  ICL_TEST_TRUE(nCombos >= 1);
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.cross_validate_mono32f", "C++ and OpenCL match for mono 32f") {
  auto src = Img32f::from(30, 20, 1, [](int x, int y, int) -> icl32f {
    return static_cast<icl32f>((x * 7 + y * 13) % 256);
  });
  BilateralFilterOp op(2, 2.f, 30.f, false);
  Image srcImg(src);

  op.forceAll(Backend::Cpp);
  Image ref = op.apply(srcImg);
  op.unforceAll();

  auto combos = op.allBackendCombinations(srcImg);
  bool allMatch = true;
  int nCombos = 0;
  op.forEachCombination(combos, [&](const std::vector<Backend>&) {
    Image dst = op.apply(srcImg);
    if(!(dst.as32f() == ref.as32f())) allMatch = false;
    nCombos++;
  });
  ICL_TEST_TRUE(allMatch);
  ICL_TEST_TRUE(nCombos >= 1);
}

ICL_REGISTER_TEST("Filter.BilateralFilterOp.cross_validate_color8u", "C++ and OpenCL match for 3ch 8u") {
  Img8u src(Size(30, 20), formatRGB);
  src.visitPixels([](int x, int y, int c, icl8u& val) {
    val = static_cast<icl8u>((x * 7 + y * 13 + c * 37) % 256);
  });
  BilateralFilterOp op(2, 2.f, 30.f, true);
  Image srcImg(src);

  op.forceAll(Backend::Cpp);
  Image ref = op.apply(srcImg);
  op.unforceAll();

  auto combos = op.allBackendCombinations(srcImg);
  bool allMatch = true;
  int nCombos = 0;
  op.forEachCombination(combos, [&](const std::vector<Backend>&) {
    Image dst = op.apply(srcImg);
    if(!(dst.as8u() == ref.as8u())) allMatch = false;
    nCombos++;
  });
  ICL_TEST_TRUE(allMatch);
  ICL_TEST_TRUE(nCombos >= 1);
}

// ====================================================================
// FFTOp / IFFTOp
// ====================================================================

ICL_REGISTER_TEST("Filter.FFTOp.roundtrip", "FFT then IFFT recovers original") {
  auto src = Img32f::from(8, 8, 1, [](int x, int y, int) -> icl32f {
    return x * 3.0f + y * 7.0f + 1.0f;
  });
  FFTOp fft(FFTOp::TWO_CHANNEL_COMPLEX, FFTOp::NO_SCALE, false, false);
  IFFTOp ifft(IFFTOp::REAL_ONLY, IFFTOp::NO_SCALE, Rect(0,0,0,0), true, false, false);

  Image freq = fft.apply(Image(src));
  ICL_TEST_EQ(freq.getChannels(), 2);

  Image recovered = ifft.apply(freq);
  ICL_TEST_EQ(recovered.getChannels(), 1);

  bool ok = true;
  const Img32f& rec = recovered.as32f();
  for(int y = 0; y < 8; ++y) {
    for(int x = 0; x < 8; ++x) {
      float diff = std::abs(rec(x, y, 0) - src(x, y, 0));
      if(diff > 0.01f) ok = false;
    }
  }
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.FFTOp.uniform_dc", "FFT of uniform image has only DC component") {
  Img32f src(Size(8, 8), 1);
  src.clear(-1, 42.0f);
  FFTOp fft(FFTOp::TWO_CHANNEL_COMPLEX, FFTOp::NO_SCALE, false, false);
  Image freq = fft.apply(Image(src));

  const Img32f& f = freq.as32f();
  float dc_real = f(0, 0, 0);
  ICL_TEST_TRUE(std::abs(dc_real - 42.0f * 64.0f) < 1.0f);
  float dc_imag = f(0, 0, 1);
  ICL_TEST_TRUE(std::abs(dc_imag) < 0.01f);

  bool ok = true;
  for(int y = 0; y < 8; ++y) {
    for(int x = 0; x < 8; ++x) {
      if(x == 0 && y == 0) continue;
      if(std::abs(f(x, y, 0)) > 0.01f) ok = false;
      if(std::abs(f(x, y, 1)) > 0.01f) ok = false;
    }
  }
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.FFTOp.result_modes_channels", "result modes produce correct channel counts") {
  Img32f src(Size(8, 8), 1);
  src.clear(-1, 1.0f);

  FFTOp fft1(FFTOp::TWO_CHANNEL_COMPLEX);
  ICL_TEST_EQ(fft1.apply(Image(src)).getChannels(), 2);

  FFTOp fft2(FFTOp::REAL_ONLY);
  ICL_TEST_EQ(fft2.apply(Image(src)).getChannels(), 1);

  FFTOp fft3(FFTOp::MAGNITUDE_ONLY);
  ICL_TEST_EQ(fft3.apply(Image(src)).getChannels(), 1);

  FFTOp fft4(FFTOp::TWO_CHANNEL_MAGNITUDE_PHASE);
  ICL_TEST_EQ(fft4.apply(Image(src)).getChannels(), 2);
}

ICL_REGISTER_TEST("Filter.FFTOp.pad_zero_power_of_2", "PAD_ZERO produces power-of-2 output") {
  Img32f src(Size(10, 7), 1);
  src.clear(-1, 1.0f);
  FFTOp fft(FFTOp::REAL_ONLY, FFTOp::PAD_ZERO, false);
  Image dst = fft.apply(Image(src));
  ICL_TEST_EQ(dst.getWidth(), 16);
  ICL_TEST_EQ(dst.getHeight(), 8);
}

ICL_REGISTER_TEST("Filter.FFTOp.scale_up_power_of_2", "SCALE_UP produces power-of-2 output") {
  Img32f src(Size(10, 7), 1);
  src.clear(-1, 1.0f);
  FFTOp fft(FFTOp::REAL_ONLY, FFTOp::SCALE_UP, false);
  Image dst = fft.apply(Image(src));
  ICL_TEST_EQ(dst.getWidth(), 16);
  ICL_TEST_EQ(dst.getHeight(), 8);
}

ICL_REGISTER_TEST("Filter.FFTOp.parseval", "energy preserved between spatial and frequency domain") {
  auto src = Img32f::from(8, 8, 1, [](int x, int y, int) -> icl32f {
    return static_cast<icl32f>(x + y * 3);
  });
  float spatialEnergy = 0;
  src.visitPixels([&](const icl32f& v) { spatialEnergy += v * v; });

  FFTOp fft(FFTOp::POWER_SPECTRUM, FFTOp::NO_SCALE, false, false);
  Image ps = fft.apply(Image(src));
  float freqEnergy = 0;
  ps.as32f().visitPixels([&](const icl32f& v) { freqEnergy += v; });
  freqEnergy /= 64.0f;

  float ratio = spatialEnergy / freqEnergy;
  ICL_TEST_TRUE(std::abs(ratio - 1.0f) < 0.01f);
}

ICL_REGISTER_TEST("Filter.FFTOp.multichannel", "FFT works on multi-channel images") {
  Img32f src(Size(8, 8), 3);
  src.clear(-1, 10.0f);
  FFTOp fft(FFTOp::REAL_ONLY, FFTOp::NO_SCALE, false);
  Image dst = fft.apply(Image(src));
  ICL_TEST_EQ(dst.getChannels(), 3);
}

ICL_REGISTER_TEST("Filter.IFFTOp.join", "join combines two channels into complex for roundtrip") {
  // FFT a known image to get a 2-channel (real+imag) frequency representation
  auto spatial = Img32f::from(8, 8, 1, [](int x, int y, int) -> icl32f {
    return x * 2.0f + y * 5.0f;
  });
  FFTOp fft(FFTOp::TWO_CHANNEL_COMPLEX, FFTOp::NO_SCALE, false, false);
  Image freq = fft.apply(Image(spatial));
  ICL_TEST_EQ(freq.getChannels(), 2);

  // IFFT with join=true: combines the 2 channels back into complex, then IFFT
  IFFTOp ifft(IFFTOp::REAL_ONLY, IFFTOp::NO_SCALE, Rect(0,0,0,0), true, false, false);
  Image recovered = ifft.apply(freq);
  ICL_TEST_EQ(recovered.getChannels(), 1);

  // Should recover original image
  bool ok = true;
  const Img32f& rec = recovered.as32f();
  for(int y = 0; y < 8; ++y) {
    for(int x = 0; x < 8; ++x) {
      float diff = std::abs(rec(x, y, 0) - spatial(x, y, 0));
      if(diff > 0.01f) ok = false;
    }
  }
  ICL_TEST_TRUE(ok);
}

// === MotionSensitiveTemporalSmoothing ===

ICL_REGISTER_TEST("Filter.MSTS.single_frame", "first frame returns itself") {
  MotionSensitiveTemporalSmoothing op(-1, 5);
  op.setFilterSize(3);
  op.setDifference(10);

  Img8u src(Size(4, 4), 1);
  src.clear(-1, 100);

  Image dst = op.apply(Image(src));
  ICL_TEST_TRUE(dst.as8u() == src);
}

ICL_REGISTER_TEST("Filter.MSTS.uniform_frames", "identical frames produce same output") {
  MotionSensitiveTemporalSmoothing op(-1, 5);
  op.setFilterSize(3);
  op.setDifference(10);

  Img8u src(Size(4, 4), 1);
  src.clear(-1, 42);

  for(int i = 0; i < 5; i++) {
    Image dst = op.apply(Image(src));
    ICL_TEST_TRUE(dst.as8u() == src);
  }
}

ICL_REGISTER_TEST("Filter.MSTS.averaging_32f", "averages stable float frames") {
  MotionSensitiveTemporalSmoothing op(-1, 5);
  op.setFilterSize(3);
  op.setDifference(20);

  Img32f src1(Size(2, 2), 1); src1.clear(-1, 10.f);
  Img32f src2(Size(2, 2), 1); src2.clear(-1, 12.f);
  Img32f src3(Size(2, 2), 1); src3.clear(-1, 14.f);

  (void)op.apply(Image(src1));
  (void)op.apply(Image(src2));
  Image dst = op.apply(Image(src3));

  float expected = (10.f + 12.f + 14.f) / 3.f;
  bool ok = true;
  dst.as32f().visitPixels([&](const icl32f& v) {
    if(std::abs(v - expected) > 0.01f) ok = false;
  });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.MSTS.averaging_8u", "averages stable 8u frames") {
  MotionSensitiveTemporalSmoothing op(-1, 5);
  op.setFilterSize(2);
  op.setDifference(20);

  Img8u src1(Size(4, 4), 1); src1.clear(-1, 100);
  Img8u src2(Size(4, 4), 1); src2.clear(-1, 104);

  (void)op.apply(Image(src1));
  Image dst = op.apply(Image(src2));

  bool ok = true;
  dst.as8u().visitPixels([&](const icl8u& v) {
    if(v != 102) ok = false;
  });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.MSTS.motion_detection", "motion bypasses averaging") {
  MotionSensitiveTemporalSmoothing op(-1, 5);
  op.setFilterSize(3);
  op.setDifference(5);

  Img32f src1(Size(2, 2), 1); src1.clear(-1, 10.f);
  Img32f src2(Size(2, 2), 1); src2.clear(-1, 10.f);
  Img32f src3(Size(2, 2), 1); src3.clear(-1, 100.f);

  (void)op.apply(Image(src1));
  (void)op.apply(Image(src2));
  Image dst = op.apply(Image(src3));

  // max-min = 90 > 5 -> motion -> output current frame
  ICL_TEST_TRUE(dst.as32f() == src3);
}

ICL_REGISTER_TEST("Filter.MSTS.null_value_exclusion", "null values excluded from averaging") {
  MotionSensitiveTemporalSmoothing op(0, 5);  // 0.0 is null
  op.setFilterSize(3);
  op.setDifference(20);

  Img32f src1(Size(2, 2), 1); src1.clear(-1, 0.f);    // all null
  Img32f src2(Size(2, 2), 1); src2.clear(-1, 10.f);
  Img32f src3(Size(2, 2), 1); src3.clear(-1, 14.f);

  (void)op.apply(Image(src1));
  (void)op.apply(Image(src2));
  Image dst = op.apply(Image(src3));

  // average of 10, 14 (null excluded) = 12
  float expected = (10.f + 14.f) / 2.f;
  bool ok = true;
  dst.as32f().visitPixels([&](const icl32f& v) {
    if(std::abs(v - expected) > 0.01f) ok = false;
  });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.MSTS.all_null", "all null values produce null output") {
  MotionSensitiveTemporalSmoothing op(0, 5);
  op.setFilterSize(3);
  op.setDifference(10);

  Img32f src(Size(2, 2), 1);
  src.clear(-1, 0.f);

  (void)op.apply(Image(src));
  (void)op.apply(Image(src));
  Image dst = op.apply(Image(src));

  ICL_TEST_TRUE(dst.as32f() == src);
}

ICL_REGISTER_TEST("Filter.MSTS.null_minus_one_no_effect_8u", "nullValue=-1 means no null handling for 8u") {
  MotionSensitiveTemporalSmoothing op(-1, 5);
  op.setFilterSize(2);
  op.setDifference(20);

  // Even pixel value 255 should be treated as valid with null=-1
  Img8u src1(Size(2, 2), 1); src1.clear(-1, 250);
  Img8u src2(Size(2, 2), 1); src2.clear(-1, 254);

  (void)op.apply(Image(src1));
  Image dst = op.apply(Image(src2));

  bool ok = true;
  dst.as8u().visitPixels([&](const icl8u& v) {
    if(v != 252) ok = false;  // avg(250, 254) = 252
  });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.MSTS.multichannel", "processes each channel independently") {
  MotionSensitiveTemporalSmoothing op(-1, 5);
  op.setFilterSize(2);
  op.setDifference(20);

  Img32f src1(Size(2, 2), 2);
  src1.clear(0, 10.f);
  src1.clear(1, 20.f);

  Img32f src2(Size(2, 2), 2);
  src2.clear(0, 14.f);
  src2.clear(1, 24.f);

  (void)op.apply(Image(src1));
  Image dst = op.apply(Image(src2));

  bool ok = true;
  dst.as32f().visitPixels([&](int, int, int c, const icl32f& v) {
    float expected = (c == 0) ? 12.f : 22.f;
    if(std::abs(v - expected) > 0.01f) ok = false;
  });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.MSTS.ring_buffer_wrap", "ring buffer wraps correctly") {
  MotionSensitiveTemporalSmoothing op(-1, 3);
  op.setFilterSize(3);
  op.setDifference(50);

  Img32f src1(Size(2, 2), 1); src1.clear(-1, 10.f);
  Img32f src2(Size(2, 2), 1); src2.clear(-1, 20.f);
  Img32f src3(Size(2, 2), 1); src3.clear(-1, 30.f);
  Img32f src4(Size(2, 2), 1); src4.clear(-1, 24.f);

  (void)op.apply(Image(src1));
  (void)op.apply(Image(src2));
  (void)op.apply(Image(src3));
  Image dst = op.apply(Image(src4));

  // Ring buffer after 4 frames with size 3: slots contain src4(24), src2(20), src3(30)
  // (src1 was overwritten by src4)
  // avg(24, 20, 30) = 24.666...
  float expected = (24.f + 20.f + 30.f) / 3.f;
  bool ok = true;
  dst.as32f().visitPixels([&](const icl32f& v) {
    if(std::abs(v - expected) > 0.01f) ok = false;
  });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.MSTS.filter_size_change_resets", "changing filter size resets history") {
  MotionSensitiveTemporalSmoothing op(-1, 10);
  op.setFilterSize(3);
  op.setDifference(50);

  Img32f src1(Size(2, 2), 1); src1.clear(-1, 10.f);
  Img32f src2(Size(2, 2), 1); src2.clear(-1, 20.f);
  Img32f src3(Size(2, 2), 1); src3.clear(-1, 100.f);

  (void)op.apply(Image(src1));
  (void)op.apply(Image(src2));

  // Change filter size — resets history
  op.setFilterSize(5);

  // First frame after reset: should return the input as-is
  Image dst = op.apply(Image(src3));
  ICL_TEST_TRUE(dst.as32f() == src3);
}

ICL_REGISTER_TEST("Filter.MSTS.motion_image", "motion image marks motion pixels") {
  MotionSensitiveTemporalSmoothing op(-1, 5);
  op.setFilterSize(3);
  op.setDifference(5);

  Img32f src1(Size(2, 2), 1); src1.clear(-1, 10.f);
  Img32f src2(Size(2, 2), 1); src2.clear(-1, 10.f);
  Img32f src3(Size(2, 2), 1); src3.clear(-1, 100.f);

  (void)op.apply(Image(src1));
  (void)op.apply(Image(src2));
  (void)op.apply(Image(src3));

  Img32f motion = op.getMotionImage();
  ICL_TEST_TRUE(motion.getWidth() == 2);
  ICL_TEST_TRUE(motion.getHeight() == 2);

  bool ok = true;
  motion.visitPixels([&](const icl32f& v) {
    if(v != 255.f) ok = false;
  });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.MSTS.motion_image_static", "static scene has zero motion") {
  MotionSensitiveTemporalSmoothing op(-1, 5);
  op.setFilterSize(3);
  op.setDifference(10);

  Img32f src(Size(4, 4), 1); src.clear(-1, 42.f);

  for(int i = 0; i < 3; i++) (void)op.apply(Image(src));

  Img32f motion = op.getMotionImage();
  bool ok = true;
  motion.visitPixels([&](const icl32f& v) {
    if(v != 0.f) ok = false;
  });
  ICL_TEST_TRUE(ok);
}

ICL_REGISTER_TEST("Filter.MSTS.depth_exception", "rejects unsupported depths") {
  MotionSensitiveTemporalSmoothing op(-1, 5);

  Img16s src(Size(4, 4), 1);
  src.clear(-1, 0);

  bool threw = false;
  try {
    (void)op.apply(Image(src));
  } catch(const ICLException&) {
    threw = true;
  }
  ICL_TEST_TRUE(threw);
}

ICL_REGISTER_TEST("Filter.MSTS.getters", "getters return correct values") {
  MotionSensitiveTemporalSmoothing op(2047, 15);
  ICL_TEST_TRUE(op.getNullValue() == 2047);
  ICL_TEST_TRUE(op.getMaxFilterSize() == 15);
  ICL_TEST_TRUE(op.getFilterSize() == 7);  // max/2
  ICL_TEST_TRUE(op.getDifference() == 10);

  op.setFilterSize(5);
  op.setDifference(3);
  ICL_TEST_TRUE(op.getFilterSize() == 5);
  ICL_TEST_TRUE(op.getDifference() == 3);
}
