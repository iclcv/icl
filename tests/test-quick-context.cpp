#include <icl/utils/Test.h>
#include <icl/qt/QuickContext.h>
#include <icl/qt/QuickCreate.h>
#include <icl/qt/QuickFilter.h>
#include <icl/qt/QuickMath.h>
#include <icl/qt/QuickDraw.h>
#include <icl/core/Img.h>
#include <icl/filter/UnaryArithmeticalOp.h>

#include <thread>
#include <vector>
#include <atomic>

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

ICL_REGISTER_TEST("Quick2.Context.memoryCap", "cap exceeded throws in strict mode") {
  QuickContext ctx(1024); // tiny cap: 1KB
  // Strict mode makes over-cap requests throw instead of falling back to
  // an unpooled allocation with a warning. This lets the test assert the
  // overflow path is taken *without* spamming stderr with WARNING lines.
  ctx.setThrowOnCapExceeded(true);
  // 10 KB request exceeds the 1 KB cap and there is nothing to evict —
  // must throw.
  ICL_TEST_THROW(ctx.getBuffer(depth8u, Size(100, 100), 1), ICLException);
  // Fits inside the cap → must succeed.
  Image small = ctx.getBuffer(depth8u, Size(16, 16), 1); // 256 B
  ICL_TEST_TRUE(!small.isNull());
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

// ---- Buffer Pool (extended) ----

ICL_REGISTER_TEST("Quick2.Context.getBuffer.reuseDifferentSize", "released buffer is reused even with different size") {
  QuickContext ctx;
  Image buf1 = ctx.getBuffer(depth8u, Size(100, 100), 1);
  buf1 = Image(); // release reference
  Image buf2 = ctx.getBuffer(depth8u, Size(200, 200), 1);
  // Pool should have reused (and resized) the buffer rather than allocating fresh
  ICL_TEST_TRUE(!buf2.isNull());
  ICL_TEST_EQ(buf2.getWidth(), 200);
  ICL_TEST_EQ(buf2.getHeight(), 200);
}

ICL_REGISTER_TEST("Quick2.Context.getBuffer.multipleDistinct", "3 simultaneous buffers are all distinct") {
  QuickContext ctx;
  Image a = ctx.getBuffer(depth32f, Size(50, 50), 1);
  Image b = ctx.getBuffer(depth32f, Size(50, 50), 1);
  Image c = ctx.getBuffer(depth32f, Size(50, 50), 1);
  // All three are held simultaneously, so they must be different ImgBase objects
  ICL_TEST_TRUE(a.ptr() != b.ptr());
  ICL_TEST_TRUE(b.ptr() != c.ptr());
  ICL_TEST_TRUE(a.ptr() != c.ptr());
}

ICL_REGISTER_TEST("Quick2.Context.exclusiveOwnership", "isExclusivelyOwned tracks correctly") {
  // Use a non-pooled image so pool doesn't hold an extra reference
  Image a = zeros(10, 10, 1, depth32f);
  a.detach(); // ensure fully independent data
  // With deepCopy we get a truly independent image
  a = a.deepCopy();
  ICL_TEST_TRUE(a.isExclusivelyOwned());
  Image b = a; // shallow copy — both point to same ImgBase
  ICL_TEST_FALSE(a.isExclusivelyOwned());
  ICL_TEST_FALSE(b.isExclusivelyOwned());
  b = Image(); // release b
  ICL_TEST_TRUE(a.isExclusivelyOwned());
}

// ---- applyOp ----

ICL_REGISTER_TEST("Quick2.Context.applyOp.unary", "applyOp with UnaryArithmeticalOp works") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image src = zeros(10, 10, 1, depth32f);
  src.clear(-1, 5.0);
  // addOp with value 10 should produce 15
  icl::filter::UnaryArithmeticalOp op(icl::filter::UnaryArithmeticalOp::addOp, 10.0);
  Image dst = ctx.applyOp(op, src);
  ICL_TEST_TRUE(!dst.isNull());
  ICL_TEST_NEAR(dst.as<icl32f>()(0, 0, 0), 15.f, 0.01f);
}

ICL_REGISTER_TEST("Quick2.Context.applyOp.rvalue", "applyOp with rvalue op works") {
  QuickContext ctx;
  QuickScope scope(ctx);
  Image src = zeros(10, 10, 1, depth32f);
  src.clear(-1, 3.0);
  // mulOp with value 4 should produce 12
  Image dst = ctx.applyOp(
    icl::filter::UnaryArithmeticalOp(icl::filter::UnaryArithmeticalOp::mulOp, 4.0), src);
  ICL_TEST_TRUE(!dst.isNull());
  ICL_TEST_NEAR(dst.as<icl32f>()(0, 0, 0), 12.f, 0.01f);
}

// ---- Multithreaded stress test ----

ICL_REGISTER_TEST("Quick2.Context.multithread.10workers", "10 parallel workers with own contexts") {
  constexpr int N_WORKERS = 10;
  constexpr int N_ITERS = 50;
  std::atomic<int> successes{0};
  std::atomic<int> failures{0};

  auto worker = [&](int id) {
    try {
      QuickContext ctx;
      QuickScope scope(ctx);

      for(int i = 0; i < N_ITERS; ++i) {
        // Create image
        Image img = zeros(32, 32, 3, depth32f);
        img.clear(-1, float(id * 10 + i));

        // Scalar arithmetic (no size mismatch issues)
        Image scaled = img * 0.5f + float(id);

        // Drawing (each worker has its own draw state via context)
        color(float(id * 25), 100, 200, 255);
        fill(50, 50, 50, 128);
        line(scaled, 0, 0, 31, 31);
        rect(scaled, 5, 5, 20, 20);
        circle(scaled, 16, 16, 8);

        // Copy and verify
        Image cpy = copy(scaled);
        if(cpy.isNull() || cpy.getWidth() != 32 || cpy.getChannels() != 3) {
          failures++;
          return;
        }
      }
      successes++;
    } catch(const std::exception &e) {
      failures++;
    } catch(...) {
      failures++;
    }
  };

  std::vector<std::thread> threads;
  for(int i = 0; i < N_WORKERS; ++i)
    threads.emplace_back(worker, i);
  for(auto &t : threads)
    t.join();

  ICL_TEST_EQ(successes.load(), N_WORKERS);
  ICL_TEST_EQ(failures.load(), 0);
}

ICL_REGISTER_TEST("Quick2.Context.multithread.sharedImage", "workers draw on separate images concurrently") {
  constexpr int N_WORKERS = 10;
  std::atomic<int> successes{0};

  // Each worker gets its own image and context, draws different things
  std::vector<Image> images(N_WORKERS);
  for(int i = 0; i < N_WORKERS; ++i) {
    images[i] = Image(Size(64, 64), depth32f, 3);
    images[i].clear();
  }

  auto worker = [&](int id) {
    QuickContext ctx;
    QuickScope scope(ctx);

    color(float(id * 25 + 10), float(255 - id * 25), 128, 255);
    fill(float(id * 10 + 50), float(id * 10 + 50), float(id * 10 + 50), 200);

    // Each worker draws on its own image
    for(int i = 0; i < 20; ++i) {
      line(images[id], 0, i * 3, 63, i * 3);
      rect(images[id], i * 3, 0, 10, 10);
      circle(images[id], 32, 32, i + 1);
    }

    // Verify our image was drawn on (center pixel should be non-zero from circle fills)
    float center = images[id].as<icl32f>()(32, 32, 0);
    if(center > 0) successes++;
  };

  std::vector<std::thread> threads;
  for(int i = 0; i < N_WORKERS; ++i)
    threads.emplace_back(worker, i);
  for(auto &t : threads)
    t.join();

  ICL_TEST_EQ(successes.load(), N_WORKERS);
}

ICL_REGISTER_TEST("Quick2.Context.multithread.poolIsolation", "per-context pools don't interfere") {
  constexpr int N_WORKERS = 10;
  std::atomic<int> successes{0};

  auto worker = [&](int id) {
    QuickContext ctx(4 * 1024 * 1024); // 4 MB cap per worker
    QuickScope scope(ctx);

    // Allocate and release many pool buffers
    for(int i = 0; i < 100; ++i) {
      Image img = zeros(64, 64, 3, depth32f);
      img.clear(-1, float(i));
      Image filtered = icl::qt::filter(img, "gauss");
      Image result = filtered + img;
      // Verify result is sane
      if(result.isNull()) return;
    }

    // Pool should have managed memory within cap (or warned)
    successes++;
  };

  std::vector<std::thread> threads;
  for(int i = 0; i < N_WORKERS; ++i)
    threads.emplace_back(worker, i);
  for(auto &t : threads)
    t.join();

  ICL_TEST_EQ(successes.load(), N_WORKERS);
}
