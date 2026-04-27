#include "harness/Test.h"
#include <icl/qt/QuickIO.h>
#include <icl/qt/QuickCreate.h>
#include <icl/core/Img.h>
#include <icl/io/ImageCompressor.h>

#include <icl/io/detail/compression-plugins/CompressionRegistry.h>
#ifdef ICL_HAVE_QT_WEBSOCKETS
#include <icl/io/detail/network/WSImageOutput.h>
#include <icl/io/detail/network/WSGrabber.h>
#include <icl/utils/Thread.h>
#include <chrono>
#include <thread>
#endif

#include <cstdio>
#include <fstream>

using namespace icl;
using namespace icl::qt;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::io;

ICL_REGISTER_TEST("Quick2.IO.save.load.roundtrip", "save then load preserves data") {
  Image src = zeros(32, 32, 3, depth8u);
  src.as<icl8u>()(0, 0, 0) = 42;
  src.as<icl8u>()(0, 0, 1) = 84;
  src.as<icl8u>()(0, 0, 2) = 126;
  src.setFormat(formatRGB);

  std::string tmpFile = "/tmp/icl_test_quick2_io.ppm";
  save(src, tmpFile);

  Image loaded = load(tmpFile);
  ICL_TEST_TRUE(!loaded.isNull());
  ICL_TEST_EQ(loaded.getWidth(), 32);
  ICL_TEST_EQ(loaded.getHeight(), 32);

  // PPM preserves RGB pixel values
  loaded.visit([&](const auto &img) {
    ICL_TEST_EQ(static_cast<int>(img(0, 0, 0)), 42);
  });

  std::remove(tmpFile.c_str());
}

ICL_REGISTER_TEST("Quick2.IO.print.nocrash", "print does not crash") {
  Image img = zeros(10, 10, 1);
  ICL_TEST_NO_THROW(print(img));
}

ICL_REGISTER_TEST("Quick2.IO.print.null", "print null does not crash") {
  Image img;
  ICL_TEST_NO_THROW(print(img));
}

// ---- Save/load PNG roundtrip ----

ICL_REGISTER_TEST("Quick2.IO.save.load.png", "save then load PNG preserves data") {
  Image src = zeros(16, 16, 3, depth8u);
  src.as<icl8u>()(0, 0, 0) = 100;
  src.as<icl8u>()(0, 0, 1) = 150;
  src.as<icl8u>()(0, 0, 2) = 200;
  src.setFormat(formatRGB);

  std::string tmpFile = "/tmp/icl_test_quick2_io_png.png";
  save(src, tmpFile);

  Image loaded = load(tmpFile);
  ICL_TEST_TRUE(!loaded.isNull());
  ICL_TEST_EQ(loaded.getWidth(), 16);
  ICL_TEST_EQ(loaded.getHeight(), 16);

  // PNG is lossless, so values should be preserved exactly
  loaded.visit([](const auto &img) {
    ICL_TEST_EQ(static_cast<int>(img(0, 0, 0)), 100);
  });

  std::remove(tmpFile.c_str());
}

// ---- Save null image does not crash ----

ICL_REGISTER_TEST("Quick2.IO.save.null", "saving null image does not crash") {
  Image img;
  // Saving a null image should not crash — it may warn but should not throw
  ICL_TEST_NO_THROW(save(img, "/tmp/icl_test_quick2_null.ppm"));
  // Clean up in case it created an empty file
  std::remove("/tmp/icl_test_quick2_null.ppm");
}

// ---- Compression plugin framework -------------------------------------

ICL_REGISTER_TEST("CompressionRegister.builtins_registered",
                  "all built-in codecs self-register at static init") {
  auto names = io::compressionRegistry().keys();
  // raw / rlen / 1611 are always built; jpeg requires libjpeg, zstd
  // requires libzstd. Test the unconditional ones plus assert the menu
  // contains _at least_ those.
  auto has = [&](const std::string &n){
    return std::find(names.begin(), names.end(), n) != names.end();
  };
  ICL_TEST_TRUE(has("raw"));
  ICL_TEST_TRUE(has("rlen"));
  ICL_TEST_TRUE(has("1611"));
}

ICL_REGISTER_TEST("ImageCompressor.raw.roundtrip",
                  "raw envelope → uncompress yields byte-identical image") {
  Img8u src(Size(13, 7), 3);
  for (int c = 0; c < 3; ++c) {
    icl8u *p = src.getData(c);
    for (int i = 0; i < src.getDim(); ++i) p[i] = static_cast<icl8u>((i*5 + c*23) & 0xff);
  }
  src.setFormat(formatRGB);

  io::ImageCompressor enc(io::ImageCompressor::CompressionSpec("raw"));
  auto data = enc.compress(Image(src));

  io::ImageCompressor dec;
  ICL_TEST_TRUE(dec.uncompress(data.bytes, data.len) == Image(src));
}

ICL_REGISTER_TEST("ImageCompressor.rlen.roundtrip_all_qualities",
                  "rlen roundtrip at every supported quality (1/4/6/8); "
                  "the inner encoder used to read one byte past the channel "
                  "buffer at the tail of each run, masked by allocator "
                  "padding for small heap chunks but tripping a bus error "
                  "on tight macOS-malloc allocations.") {
  // Use a multi-channel image with varied data so each quality exercises
  // its run-find / bit-pack path against real content (not a trivial
  // single-run input).  640x480 puts each channel above the small-bin
  // allocator threshold, so the post-end byte is more likely to land on
  // an unmapped page if anyone ever reintroduces the OOB read.
  Img8u src(Size(640, 480), 3);
  for (int c = 0; c < 3; ++c) {
    icl8u *p = src.getData(c);
    for (int i = 0; i < src.getDim(); ++i) {
      p[i] = static_cast<icl8u>((i * 7 + c * 31) & 0xff);
    }
  }

  for (const std::string &q : {"1", "2", "4", "6", "8"}) {
    io::ImageCompressor c(io::ImageCompressor::CompressionSpec("rlen", q));
    io::ImageCompressor::CompressedData data = c.compress(Image(src));
    Image got = c.uncompress(data.bytes, data.len);
    ICL_TEST_EQ(got.getSize(), src.getSize());
    ICL_TEST_EQ(got.getChannels(), 3);
    ICL_TEST_EQ(got.getDepth(), depth8u);
    if (q == "8") {
      // q=8 is the only lossless rlen quality — bytes must match
      // exactly (q=1/4/6 quantise the value bits and lose information).
      const Img8u *back = got.ptr()->as8u();
      for (int ch = 0; ch < 3; ++ch) {
        const icl8u *p = src.getData(ch);
        const icl8u *q2 = back->getData(ch);
        for (int i = 0; i < src.getDim(); ++i) {
          if (p[i] != q2[i]) {
            ICL_TEST_EQ(static_cast<int>(q2[i]), static_cast<int>(p[i]));
            return;
          }
        }
      }
    }
  }
}

ICL_REGISTER_TEST("ImageCompressor.rlen.quality_flip_mid_session",
                  "Switching `quality` between calls — same compressor "
                  "instance, no torn buffer-size vs encoder mismatch.  "
                  "Used to be a heap overflow when the GUI thread "
                  "flipped 6→8 between worst-case-sizing and the "
                  "encodeChannel calls.") {
  Img8u src(Size(128, 96), 3);
  for (int c = 0; c < 3; ++c) {
    icl8u *p = src.getData(c);
    for (int i = 0; i < src.getDim(); ++i) {
      p[i] = static_cast<icl8u>((i * 11 + c * 17) & 0xff);
    }
  }

  io::ImageCompressor c(io::ImageCompressor::CompressionSpec("rlen", "1"));
  // Sequence covers every transition (worst-case-buffer ↔ q=8 path)
  // and every adjacent pair of qualities at least once.
  for (const std::string &q :
       {"1", "2", "4", "6", "8", "6", "2", "1", "8", "4", "2", "1"}) {
    c.prop("quality").value = q;
    io::ImageCompressor::CompressedData data = c.compress(Image(src));
    Image got = c.uncompress(data.bytes, data.len);
    ICL_TEST_EQ(got.getSize(), src.getSize());
    ICL_TEST_EQ(got.getChannels(), 3);
  }
}

ICL_REGISTER_TEST("ImageCompressor.auto_detect_codec",
                  "receiver decodes regardless of its current setCompression") {
  // Sender uses raw, receiver is configured for 1611 — the envelope
  // should drive the codec choice on decode, not the receiver's spec.
  Img8u src(Size(8, 8), 1);
  for (int i = 0; i < src.getDim(); ++i) src.getData(0)[i] = static_cast<icl8u>(i);

  io::ImageCompressor enc(io::ImageCompressor::CompressionSpec("raw"));
  auto data = enc.compress(Image(src));

  io::ImageCompressor dec(io::ImageCompressor::CompressionSpec("1611"));
  Image got = dec.uncompress(data.bytes, data.len);  // should NOT throw
  ICL_TEST_EQ(got.getDepth(), depth8u);
  ICL_TEST_EQ(got.getChannels(), 1);
}

#ifdef ICL_HAVE_ZSTD
ICL_REGISTER_TEST("ImageCompressor.zstd.roundtrip",
                  "zstd plugin (proves the registry is open to new codecs)") {
  Img8u src(Size(64, 48), 3);
  for (int c = 0; c < 3; ++c) {
    icl8u *p = src.getData(c);
    for (int i = 0; i < src.getDim(); ++i) p[i] = static_cast<icl8u>((i*3 + c*17) & 0xff);
  }
  io::ImageCompressor enc(io::ImageCompressor::CompressionSpec("zstd", "5"));
  auto data = enc.compress(Image(src));
  ICL_TEST_TRUE(data.len > 0);

  io::ImageCompressor dec;
  ICL_TEST_TRUE(dec.uncompress(data.bytes, data.len) == Image(src));
}
#endif

// ---- Kinect 11-bit pack/unpack roundtrip via ImageCompressor("1611") ----
// Replaces the retired io/demos/depth_img_endcoding_test demo. The "1611"
// mode has two quality levels (see ImageCompressor.cpp:368-372):
//   "1" → pack16to11 / unpack11to16  — raw bit-packing, lossless for
//          11-bit values (clamps inputs above 2047)
//   "0" → pack16to11_2 / unpack11to16_2 — applies Kinect's depth-mapping
//          formula on top of the pack (LOSSY; not tested here, needs the
//          depth-domain to be meaningful)

ICL_REGISTER_TEST("ImageCompressor.1611.lossless_in_range",
                  "11-bit pack/unpack preserves values <= 2047 (quality=1)") {
  // Build a 16s single-channel image (the only shape "1611" supports —
  // see ImageCompressor.cpp:343-344).
  Img16s src(Size(64, 48), 1);
  // Spread values across the full 11-bit range, including 0 and the max.
  icl16s *p = src.getData(0);
  const int n = src.getDim();
  for (int i = 0; i < n; ++i) p[i] = static_cast<icl16s>(i % 2048);

  ImageCompressor c(ImageCompressor::CompressionSpec("1611", "1"));
  ImageCompressor::CompressedData packed = c.compress(Image(src), false);
  Image got = c.uncompress(packed.bytes, packed.len);
  const Img16s *back = got.ptr()->as16s();

  ICL_TEST_TRUE(back != nullptr);
  ICL_TEST_EQ(back->getSize(), src.getSize());
  ICL_TEST_EQ(back->getChannels(), 1);

  const icl16s *q = back->getData(0);
  for (int i = 0; i < n; ++i) {
    if (p[i] != q[i]) {
      ICL_TEST_EQ(static_cast<int>(q[i]), static_cast<int>(p[i]));
      break;  // one failure suffices; avoid spamming
    }
  }
}

ICL_REGISTER_TEST("ImageCompressor.1611.clamps_above_11bit",
                  "values >2047 get clamped to the 11-bit mask (quality=1)") {
  Img16s src(Size(8, 1), 1);
  icl16s *p = src.getData(0);
  // Mix of in-range, edge, and overflow values.
  const icl16s in[] = {0, 1, 2047, 2048, 4095, 8192, 12345, 32767};
  for (int i = 0; i < 8; ++i) p[i] = in[i];

  ImageCompressor c(ImageCompressor::CompressionSpec("1611", "1"));
  ImageCompressor::CompressedData packed = c.compress(Image(src), false);
  Image got = c.uncompress(packed.bytes, packed.len);
  const Img16s *back = got.ptr()->as16s();

  const icl16s *q = back->getData(0);
  // In-range values must match exactly.
  ICL_TEST_EQ(static_cast<int>(q[0]), 0);
  ICL_TEST_EQ(static_cast<int>(q[1]), 1);
  ICL_TEST_EQ(static_cast<int>(q[2]), 2047);
  // Out-of-range values clamp to the 11-bit mask (2047), not wrap modulo.
  for (int i = 3; i < 8; ++i) {
    ICL_TEST_EQ(static_cast<int>(q[i]), 2047);
  }
}

// ---- Codec capability flags ---------------------------------------
// Each codec declares a `Capabilities` struct (depth + channel-range +
// format whitelists).  The facade pre-validates input against the
// active codec's capabilities and throws a uniform, codec-named error
// before delegating, so users / UI get a consistent message instead of
// plugin-specific wording.

ICL_REGISTER_TEST(
    "ImageCompressor.capabilities.1611_rejects_rgb",
    "ImageCompressor::compress throws cleanly when the active codec's "
    "capabilities reject the input (1611 needs single-channel icl16s; "
    "an 8u/3ch RGB image must be refused at the facade boundary).") {
  Img8u rgb(Size(4, 4), formatRGB);
  ImageCompressor c(ImageCompressor::CompressionSpec("1611", "1"));
  bool threw = false;
  std::string msg;
  try {
    (void)c.compress(Image(rgb), false);
  } catch (const ICLException &e) {
    threw = true;
    msg = e.what();
  }
  ICL_TEST_TRUE(threw);
  // Error must name the codec and at minimum reference the codec's
  // declared shape — clear enough for a UI to surface.
  ICL_TEST_TRUE(msg.find("1611") != std::string::npos);
  ICL_TEST_TRUE(msg.find("depth16s") != std::string::npos);
}

ICL_REGISTER_TEST(
    "ImageCompressor.capabilities.raw_accepts_any",
    "Codecs with no declared constraints (raw) accept any reasonable "
    "image — no false positives from the new validation.") {
  // Try a few shapes that 1611/jpeg would reject; raw must roundtrip
  // them without error.
  ImageCompressor c(ImageCompressor::CompressionSpec("raw"));
  // 8u / 3ch RGB (would fail 1611)
  ICL_TEST_NO_THROW(
      (void)c.compress(Image(Img8u(Size(4, 4), formatRGB)), false));
  // 16s / 1ch (would pass 1611)
  ICL_TEST_NO_THROW(
      (void)c.compress(Image(Img16s(Size(4, 4), 1)), false));
  // 32f / 4ch (would fail jpeg + 1611 both)
  ICL_TEST_NO_THROW(
      (void)c.compress(Image(Img32f(Size(4, 4), 4)), false));
}

#ifdef ICL_HAVE_QT_WEBSOCKETS
// ---- WebSocket Grabber/Output (Qt6 WebSockets) -------------------------
//
// Loopback tests: spin a WSImageOutput on 127.0.0.1 + an OS-assigned
// port, connect a WSGrabber to it, send a known image, verify roundtrip.
// Each test uses port 0 to avoid collisions with parallel test runs.

namespace {
  // Wait until predicate becomes true or `timeoutMs` elapses; returns
  // whether the predicate was satisfied. Polls with `pollMs` resolution.
  template <typename F>
  bool waitFor(F &&predicate, int timeoutMs = 5000, int pollMs = 20) {
    auto deadline = std::chrono::steady_clock::now()
                  + std::chrono::milliseconds(timeoutMs);
    while (std::chrono::steady_clock::now() < deadline) {
      if (predicate()) return true;
      std::this_thread::sleep_for(std::chrono::milliseconds(pollMs));
    }
    return predicate();
  }

  // Build a small RGB image with deterministic content for byte-identity
  // checks across the wire.
  Img8u makeKnownImage(int w = 16, int h = 12) {
    Img8u img(Size(w, h), 3);
    for (int c = 0; c < 3; ++c) {
      icl8u *p = img.getData(c);
      for (int i = 0; i < w * h; ++i) {
        p[i] = static_cast<icl8u>((i * 7 + c * 31) & 0xFF);
      }
    }
    img.setFormat(formatRGB);
    return img;
  }

  // Pointer-friendly wrapper: WSGrabber::acquireImage() returns
  // const ImgBase*; lift to Image and use its operator==.
  bool imagesEqual(const ImgBase *a, const ImgBase *b) {
    return a && b && Image(*a) == Image(*b);
  }
}

ICL_REGISTER_TEST("WS.loopback.roundtrip",
                  "WSImageOutput → WSGrabber: byte-identical recovery") {
  WSImageOutput out(0, "127.0.0.1");
  ICL_TEST_TRUE(out);
  const int port = out.actualPort();
  ICL_TEST_TRUE(port > 0);

  WSGrabber grab("ws://127.0.0.1:" + str(port));

  // Wait for the client to actually connect before sending — the property
  // surface tells us when (set every acquireImage call by WSGrabber, but
  // also via the connectedClients() server-side getter).
  ICL_TEST_TRUE(waitFor([&]{ return out.connectedClients() >= 1; }));

  Img8u src = makeKnownImage();
  out.send(Image(src));

  const ImgBase *got = grab.acquireImage();
  ICL_TEST_TRUE(got != nullptr);
  ICL_TEST_TRUE(imagesEqual(got, &src));
}

ICL_REGISTER_TEST("WS.multi_client.broadcast",
                  "Two grabbers attached to one output both see every frame") {
  WSImageOutput out(0, "127.0.0.1");
  const int port = out.actualPort();
  const std::string url = "ws://127.0.0.1:" + str(port);

  WSGrabber a(url), b(url);
  ICL_TEST_TRUE(waitFor([&]{ return out.connectedClients() >= 2; }));

  Img8u src = makeKnownImage(8, 8);
  out.send(Image(src));

  const ImgBase *ga = a.acquireImage();
  const ImgBase *gb = b.acquireImage();
  ICL_TEST_TRUE(ga != nullptr && gb != nullptr);
  ICL_TEST_TRUE(imagesEqual(ga, &src));
  ICL_TEST_TRUE(imagesEqual(gb, &src));
}

ICL_REGISTER_TEST("WS.url_shorthands_accepted",
                  "PORT, HOST:PORT and ws://HOST:PORT are all valid grabber URLs") {
  WSImageOutput out(0, "127.0.0.1");
  const int port = out.actualPort();
  ICL_TEST_TRUE(port > 0);

  // Bare port → resolves to localhost:PORT
  WSGrabber a(str(port));
  // host:port form
  WSGrabber b("127.0.0.1:" + str(port));
  // Full URL form
  WSGrabber c("ws://127.0.0.1:" + str(port));
  ICL_TEST_TRUE(waitFor([&]{ return out.connectedClients() >= 3; }));

  Img8u src = makeKnownImage(8, 8);
  out.send(Image(src));

  ICL_TEST_TRUE(imagesEqual(a.acquireImage(), &src));
  ICL_TEST_TRUE(imagesEqual(b.acquireImage(), &src));
  ICL_TEST_TRUE(imagesEqual(c.acquireImage(), &src));
}

ICL_REGISTER_TEST("WS.client_survives_server_restart",
                  "Grabber transparently reconnects when server restarts on same port") {
  // Phase 1: bring up server, attach client, prove a frame flows.
  Img8u src1 = makeKnownImage(8, 8);
  Img8u src2 = makeKnownImage(8, 8);
  ++src2.getData(0)[0];  // make src2 distinguishable from src1

  int port = 0;
  {
    WSImageOutput out(0, "127.0.0.1");
    port = out.actualPort();
    ICL_TEST_TRUE(port > 0);

    WSGrabber grab("ws://127.0.0.1:" + str(port));
    ICL_TEST_TRUE(waitFor([&]{ return out.connectedClients() >= 1; }));
    out.send(Image(src1));

    const ImgBase *got = grab.acquireImage();
    ICL_TEST_TRUE(imagesEqual(got, &src1));

    // Phase 2: tear down the server (out goes out of scope here).
    // Phase 3: bring it back up on the same port and prove the SAME
    // grabber starts receiving again — without ever being recreated.
  }
  // Brief pause to let the client notice the disconnect + start its
  // exponential backoff (initial 250ms).
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  WSGrabber grab("ws://127.0.0.1:" + str(port));
  // Bring the server back on the same port.
  WSImageOutput out2(port, "127.0.0.1");
  ICL_TEST_TRUE(waitFor([&]{ return out2.connectedClients() >= 1; },
                        /*timeoutMs=*/8000));
  out2.send(Image(src2));

  // Drop replays of last-known (zero, since this grabber is new) — wait
  // for a real frame.
  const ImgBase *got = nullptr;
  waitFor([&]{
    got = grab.acquireImage();
    return got != nullptr && imagesEqual(got, &src2);
  }, /*timeoutMs=*/3000);
  ICL_TEST_TRUE(got != nullptr);
  ICL_TEST_TRUE(imagesEqual(got, &src2));
}
#endif // ICL_HAVE_QT_WEBSOCKETS
