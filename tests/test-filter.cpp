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
#include <ICLFilter/ChamferOp.h>
#include <ICLFilter/AffineOp.h>
#include <ICLFilter/CannyOp.h>
#include <ICLFilter/MedianOp.h>

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
