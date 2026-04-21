#include <icl/utils/Test.h>
#include <icl/qt/QuickDraw.h>
#include <icl/qt/QuickCreate.h>
#include <icl/qt/QuickContext.h>
#include <icl/core/Img.h>

using namespace icl;
using namespace icl::qt;
using namespace icl::core;
using namespace icl::utils;

// ---- pix ----

ICL_REGISTER_TEST("Quick2.Draw.pix.basic", "draws a pixel with draw color") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(20, 20, 3, depth32f);
  color(255, 0, 0, 255);
  pix(img, 10, 10);
  ICL_TEST_NEAR(img.as<icl32f>()(10, 10, 0), 255.f, 0.01f);
  ICL_TEST_NEAR(img.as<icl32f>()(10, 10, 1), 0.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Draw.pix.outOfBounds", "out-of-bounds pixel is ignored") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(10, 10, 1, depth32f);
  color(255, 255, 255, 255);
  pix(img, -1, -1);
  pix(img, 100, 100);
  // No crash, image stays zero
  ICL_TEST_NEAR(img.as<icl32f>()(0, 0, 0), 0.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Draw.pix.8u", "pix works on 8u images") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(10, 10, 1, depth8u);
  color(200);
  pix(img, 5, 5);
  ICL_TEST_EQ(img.as<icl8u>()(5, 5, 0), icl8u(200));
}

// ---- line ----

ICL_REGISTER_TEST("Quick2.Draw.line.horizontal", "horizontal line fills row") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(20, 20, 1, depth32f);
  color(100);
  line(img, 0, 5, 19, 5);
  ICL_TEST_NEAR(img.as<icl32f>()(0, 5, 0), 100.f, 0.01f);
  ICL_TEST_NEAR(img.as<icl32f>()(19, 5, 0), 100.f, 0.01f);
  ICL_TEST_NEAR(img.as<icl32f>()(10, 0, 0), 0.f, 0.01f); // untouched
}

ICL_REGISTER_TEST("Quick2.Draw.line.vertical", "vertical line fills column") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(20, 20, 1, depth32f);
  color(100);
  line(img, 5, 0, 5, 19);
  ICL_TEST_NEAR(img.as<icl32f>()(5, 0, 0), 100.f, 0.01f);
  ICL_TEST_NEAR(img.as<icl32f>()(5, 19, 0), 100.f, 0.01f);
}

// ---- cross ----

ICL_REGISTER_TEST("Quick2.Draw.cross", "cross draws at center") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(20, 20, 1, depth32f);
  color(100);
  cross(img, 10, 10);
  ICL_TEST_NEAR(img.as<icl32f>()(10, 10, 0), 100.f, 0.01f);
}

// ---- rect ----

ICL_REGISTER_TEST("Quick2.Draw.rect.outline", "rect outline draws border") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(30, 30, 1, depth32f);
  color(200);
  fill(0, 0, 0, 0); // no fill
  rect(img, 5, 5, 20, 20);
  ICL_TEST_NEAR(img.as<icl32f>()(5, 5, 0), 200.f, 0.01f);     // top-left corner
  ICL_TEST_NEAR(img.as<icl32f>()(24, 24, 0), 200.f, 0.01f);   // bottom-right corner
  ICL_TEST_NEAR(img.as<icl32f>()(15, 15, 0), 0.f, 0.01f);     // center unfilled
}

ICL_REGISTER_TEST("Quick2.Draw.rect.filled", "rect with fill") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(30, 30, 1, depth32f);
  color(200);
  fill(100, 100, 100, 255);
  rect(img, 5, 5, 20, 20);
  ICL_TEST_NEAR(img.as<icl32f>()(15, 15, 0), 100.f, 0.01f); // center filled
}

// ---- circle ----

ICL_REGISTER_TEST("Quick2.Draw.circle", "filled circle touches center") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(50, 50, 1, depth32f);
  color(200);
  fill(100, 100, 100, 255);
  circle(img, 25, 25, 10);
  ICL_TEST_NEAR(img.as<icl32f>()(25, 25, 0), 100.f, 0.01f); // center filled
  ICL_TEST_NEAR(img.as<icl32f>()(0, 0, 0), 0.f, 0.01f);     // corner untouched
}

// ---- triangle ----

ICL_REGISTER_TEST("Quick2.Draw.triangle", "triangle draws edges") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(50, 50, 1, depth32f);
  color(100);
  fill(0, 0, 0, 0);
  triangle(img, 10, 10, 40, 10, 25, 40);
  // Top edge midpoint should be drawn
  ICL_TEST_NEAR(img.as<icl32f>()(25, 10, 0), 100.f, 0.01f);
}

// ---- alpha blending ----

ICL_REGISTER_TEST("Quick2.Draw.alpha", "50% alpha blends correctly") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image img = zeros(10, 10, 1, depth32f);
  img.clear(-1, 100);
  color(200, 200, 200, 128); // ~50% alpha
  pix(img, 5, 5);
  float v = img.as<icl32f>()(5, 5, 0);
  // Should be about (1 - 128/255)*100 + (128/255)*200 ≈ 150
  ICL_TEST_TRUE(v > 140.f && v < 160.f);
}

// ---- multi-depth ----

ICL_REGISTER_TEST("Quick2.Draw.multiDepth", "drawing works on all depths") {
  QuickContext ctx;
  QuickScope scope(ctx);
  color(128);
  for(int d = 0; d <= 4; ++d) {
    Image img = zeros(20, 20, 1, static_cast<depth>(d));
    ICL_TEST_NO_THROW(pix(img, 5, 5));
    ICL_TEST_NO_THROW(line(img, 0, 0, 19, 19));
    ICL_TEST_NO_THROW(circle(img, 10, 10, 5));
  }
}
