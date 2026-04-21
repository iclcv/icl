#include <icl/utils/Test.h>
#include <icl/qt/QuickContext.h>
#include <icl/core/Img.h>

using namespace icl;
using namespace icl::qt;
using namespace icl::core;
using namespace icl::utils;

// ---- Buffer Pool ----

ICL_REGISTER_TEST("Quick2.Context.getBuffer.basic", "buffer has correct params") {
  QuickContext ctx;
  Image buf = ctx.getBuffer(depth32f, Size(640, 480), 3);
  ICL_TEST_EQ(buf.getWidth(), 640);
  ICL_TEST_EQ(buf.getHeight(), 480);
  ICL_TEST_EQ(buf.getChannels(), 3);
  ICL_TEST_EQ(static_cast<int>(buf.getDepth()), static_cast<int>(depth32f));
}

ICL_REGISTER_TEST("Quick2.Context.getBuffer.reuse", "independent buffer is recycled") {
  QuickContext ctx;
  Image buf1 = ctx.getBuffer(depth8u, Size(100, 100), 1);
  const void *ptr1 = buf1.ptr();
  buf1 = Image(); // release reference
  Image buf2 = ctx.getBuffer(depth8u, Size(100, 100), 1);
  ICL_TEST_EQ(buf2.ptr(), ptr1); // should get the same ImgBase back
}

ICL_REGISTER_TEST("Quick2.Context.getBuffer.differentDepth", "different depth gets different buffer") {
  QuickContext ctx;
  Image buf8u = ctx.getBuffer(depth8u, Size(100, 100), 1);
  Image buf32f = ctx.getBuffer(depth32f, Size(100, 100), 1);
  ICL_TEST_NE(static_cast<int>(buf8u.getDepth()), static_cast<int>(buf32f.getDepth()));
}

ICL_REGISTER_TEST("Quick2.Context.memoryUsage", "tracks pool size") {
  QuickContext ctx;
  ICL_TEST_EQ(ctx.memoryUsage(), size_t(0));
  Image buf = ctx.getBuffer(depth8u, Size(100, 100), 1);
  ICL_TEST_EQ(ctx.memoryUsage(), size_t(100 * 100 * 1));
}

ICL_REGISTER_TEST("Quick2.Context.memoryCap", "cap is respected") {
  QuickContext ctx(1024); // tiny cap: 1KB
  // Fill pool beyond cap — should get unpooled images (with warning)
  Image a = ctx.getBuffer(depth8u, Size(100, 100), 1); // 10KB — over cap but first alloc
  // Hold a so it can't be evicted
  Image b = ctx.getBuffer(depth8u, Size(100, 100), 1); // should be unpooled (a is pinned)
  ICL_TEST_TRUE(!b.isNull());
}

ICL_REGISTER_TEST("Quick2.Context.clearBuffers", "clears independent buffers") {
  QuickContext ctx;
  Image buf = ctx.getBuffer(depth32f, Size(100, 100), 3);
  size_t usage = ctx.memoryUsage();
  ICL_TEST_TRUE(usage > 0);
  buf = Image(); // release
  ctx.clearBuffers();
  ICL_TEST_EQ(ctx.memoryUsage(), size_t(0));
}

// ---- QuickScope ----

ICL_REGISTER_TEST("Quick2.Context.scope", "QuickScope activates context") {
  QuickContext ctx;
  ctx.drawColor[0] = 42;
  {
    QuickScope scope(ctx);
    ICL_TEST_NEAR(activeContext().drawColor[0], 42.f, 0.01f);
  }
}

ICL_REGISTER_TEST("Quick2.Context.scope.nested", "scopes nest correctly") {
  QuickContext ctx1, ctx2;
  ctx1.drawColor[0] = 10;
  ctx2.drawColor[0] = 20;
  {
    QuickScope s1(ctx1);
    ICL_TEST_NEAR(activeContext().drawColor[0], 10.f, 0.01f);
    {
      QuickScope s2(ctx2);
      ICL_TEST_NEAR(activeContext().drawColor[0], 20.f, 0.01f);
    }
    ICL_TEST_NEAR(activeContext().drawColor[0], 10.f, 0.01f);
  }
}

// ---- Color State Stack ----

ICL_REGISTER_TEST("Quick2.Context.colorStack", "push/pop preserves state") {
  QuickContext ctx;
  ctx.drawColor[0] = 100;
  ctx.fillColor[0] = 200;
  ctx.pushColorState();
  ctx.drawColor[0] = 50;
  ctx.fillColor[0] = 60;
  ICL_TEST_NEAR(ctx.drawColor[0], 50.f, 0.01f);
  ctx.popColorState();
  ICL_TEST_NEAR(ctx.drawColor[0], 100.f, 0.01f);
  ICL_TEST_NEAR(ctx.fillColor[0], 200.f, 0.01f);
}
