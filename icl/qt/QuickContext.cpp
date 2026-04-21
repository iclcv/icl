// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/QuickContext.h>
#include <icl/core/CoreFunctions.h>
#include <icl/filter/UnaryOp.h>
#include <icl/filter/BinaryOp.h>
#include <icl/io/GenericGrabber.h>
#include <icl/utils/Macros.h>
#include <icl/utils/Exception.h>

#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace icl::core;
using namespace icl::utils;

namespace icl::qt {

  // ---- thread-local context routing ----

  static thread_local QuickContext* tl_active_context = nullptr;

  static QuickContext& defaultContext() {
    static QuickContext ctx;
    return ctx;
  }

  QuickContext& activeContext() {
    return tl_active_context ? *tl_active_context : defaultContext();
  }

  // ---- QuickScope ----

  QuickScope::QuickScope(QuickContext &ctx)
    : m_prev(tl_active_context)
  {
    tl_active_context = &ctx;
  }

  QuickScope::~QuickScope() {
    tl_active_context = m_prev;
  }

  // ---- QuickContext internals ----

  struct ColorState {
    float color[4];
    float fill[4];
  };

  // Each pooled buffer carries the number of bytes it contributed to
  // currentUsage when last tracked. Using this on decrement (instead of a
  // live `img.memoryUsage()` query) keeps the accounting correct even when
  // callers externally resize a handed-out Image. A proper fix is sketched
  // in project_memorypool.md — until then this makes the observed size_t
  // underflow go away without restructuring Quick2's data model.
  struct PooledBuffer {
    Image img;
    size_t trackedBytes;
  };

  struct QuickContext::Data {
    std::vector<PooledBuffer> buffers;
    size_t memoryCap;
    size_t currentUsage = 0;
    bool tracing = false;
    // The active-context pointer is thread_local but a QuickContext
    // instance can still be shared across threads via QuickScope(ctx).
    // All other flags here are plain bools (single-writer assumed);
    // keeping this one consistent — callers should set it before
    // spinning up worker threads.
    bool throwOnCapExceeded = false;
    std::vector<ColorState> colorStack;
    std::map<std::string, std::shared_ptr<io::GenericGrabber>, std::less<>> grabbers;

    explicit Data(size_t cap) : memoryCap(cap) {}
  };

  // Central usage-tracking helper. All mutations of currentUsage go through
  // this so we can (a) detect size_t underflow (tracking bug — delta larger
  // than current usage would wrap to 2^64) and (b) optionally trace every
  // pool event for diagnosis. The event string is short and lowercase
  // (alloc/resize/evict/unpooled/clear) to make grep-friendly logs.
  static void track_event(size_t &currentUsage, size_t memoryCap, bool tracing,
                          const char *event, std::ptrdiff_t delta,
                          const Image *buf = nullptr) {
    constexpr double MB = 1.0 / (1024.0 * 1024.0);
    const size_t before = currentUsage;

    if(delta < 0 && size_t(-delta) > before) {
      ERROR_LOG("QuickContext[" << event << "] tracking underflow: "
                "currentUsage=" << (before * MB) << " MB, attempted delta="
                << (delta * MB) << " MB — clamping to 0 (this indicates a "
                "pool accounting bug)");
      currentUsage = 0;
    } else {
      currentUsage = size_t(static_cast<std::ptrdiff_t>(before) + delta);
    }

    if(tracing) {
      // Direct cerr — we want output whenever tracing is explicitly
      // enabled, independent of ICL's compile-time LOG_LEVEL.
      std::cerr << "QuickContext[" << event << "] "
                << (delta >= 0 ? "+" : "") << (delta * MB) << " MB → usage "
                << (currentUsage * MB) << " / " << (memoryCap * MB) << " MB";
      if(buf) {
        std::cerr << "  [" << buf->getSize().width << "x" << buf->getSize().height
                  << " " << buf->getDepth() << " " << buf->getChannels() << "ch]";
      }
      std::cerr << std::endl;
    }
  }

  // ---- QuickContext ----

  QuickContext::QuickContext(size_t memoryCap)
    : m_data(new Data(memoryCap))
  {}

  QuickContext::~QuickContext() {
    delete m_data;
  }

  Image QuickContext::getBuffer(depth d, const ImgParams &params) {
    size_t needed = size_t(params.getDim()) * params.getChannels() * getSizeOf(d);
    auto &buffers = m_data->buffers;

    auto track = [&](const char *event, std::ptrdiff_t delta, const Image *buf){
      track_event(m_data->currentUsage, m_data->memoryCap, m_data->tracing,
                  event, delta, buf);
    };

    // 1. Exact match among independent buffers
    for(auto &pb : buffers) {
      if(pb.img.isExclusivelyOwned() && pb.img.getDepth() == d
         && pb.img.getSize() == params.getSize()
         && pb.img.getChannels() == params.getChannels()) {
        pb.img.setFormat(params.getFormat());
        if(m_data->tracing) track("reuse", 0, &pb.img);
        return pb.img;
      }
    }

    // 2. Any independent buffer → resize. We use pb.trackedBytes (what we
    // last credited to currentUsage for this buffer) rather than a live
    // memoryUsage() query — that query may have drifted if the caller
    // externally resized the Image.
    for(auto &pb : buffers) {
      if(pb.img.isExclusivelyOwned()) {
        pb.img.ensureCompatible(d, params.getSize(), params.getChannels(), params.getFormat());
        const size_t now = pb.img.memoryUsage();
        track("resize",
              static_cast<std::ptrdiff_t>(now) - static_cast<std::ptrdiff_t>(pb.trackedBytes),
              &pb.img);
        pb.trackedBytes = now;
        return pb.img;
      }
    }

    // 3. Try eviction if over cap
    if(m_data->currentUsage + needed > m_data->memoryCap) {
      // Evict independent buffers (back to front) until enough room
      for(int i = static_cast<int>(buffers.size()) - 1; i >= 0; --i) {
        if(buffers[i].img.isExclusivelyOwned()) {
          track("evict",
                -static_cast<std::ptrdiff_t>(buffers[i].trackedBytes),
                &buffers[i].img);
          buffers.erase(buffers.begin() + i);
          if(m_data->currentUsage + needed <= m_data->memoryCap) break;
        }
      }
    }

    // 4. Still over cap → either throw (strict mode, used by tests) or
    // fall back to an unpooled allocation with a warning.
    if(m_data->currentUsage + needed > m_data->memoryCap) {
      constexpr double MB = 1.0 / (1024.0 * 1024.0);
      std::ostringstream msg;
      msg << "QuickContext: pool memory cap exceeded — "
          << params.getSize().width << "x" << params.getSize().height
          << " " << d << " " << params.getChannels() << "ch image ("
          << (needed * MB) << " MB) while pool uses "
          << (m_data->currentUsage * MB) << " of "
          << (m_data->memoryCap * MB) << " MB";
      if(m_data->throwOnCapExceeded) {
        throw utils::ICLException(msg.str());
      }
      WARNING_LOG(msg.str() << " — allocating unpooled image. "
                  "Consider increasing the cap or releasing held images.");
      Image img(params.getSize(), d, params.getChannels(), params.getFormat());
      if(m_data->tracing) track("unpooled", 0, &img);
      return img;
    }

    // 5. Room in pool → allocate and track
    Image img(params.getSize(), d, params.getChannels(), params.getFormat());
    buffers.push_back({img, needed});
    track("alloc", static_cast<std::ptrdiff_t>(needed), &img);
    return img;
  }

  Image QuickContext::getBuffer(depth d, const Size &s, int channels) {
    return getBuffer(d, ImgParams(s, channels));
  }

  size_t QuickContext::memoryUsage() const {
    return m_data->currentUsage;
  }

  size_t QuickContext::memoryCap() const {
    return m_data->memoryCap;
  }

  void QuickContext::setMemoryCap(size_t cap) {
    m_data->memoryCap = cap;
  }

  void QuickContext::clearBuffers() {
    auto &buffers = m_data->buffers;
    for(int i = static_cast<int>(buffers.size()) - 1; i >= 0; --i) {
      if(buffers[i].img.isExclusivelyOwned()) {
        track_event(m_data->currentUsage, m_data->memoryCap, m_data->tracing,
                    "clear",
                    -static_cast<std::ptrdiff_t>(buffers[i].trackedBytes),
                    &buffers[i].img);
        buffers.erase(buffers.begin() + i);
      }
    }
  }

  void QuickContext::setTracing(bool enabled) {
    m_data->tracing = enabled;
  }

  bool QuickContext::isTracing() const {
    return m_data->tracing;
  }

  void QuickContext::setThrowOnCapExceeded(bool enabled) {
    m_data->throwOnCapExceeded = enabled;
  }

  bool QuickContext::throwsOnCapExceeded() const {
    return m_data->throwOnCapExceeded;
  }

  // ---- Color state stack ----

  void QuickContext::pushColorState() {
    ColorState s;
    std::copy(drawColor, drawColor + 4, s.color);
    std::copy(fillColor, fillColor + 4, s.fill);
    m_data->colorStack.push_back(s);
  }

  void QuickContext::popColorState() {
    if(m_data->colorStack.empty()) {
      WARNING_LOG("QuickContext::popColorState: stack is empty");
      return;
    }
    const ColorState &s = m_data->colorStack.back();
    std::copy(s.color, s.color + 4, drawColor);
    std::copy(s.fill, s.fill + 4, fillColor);
    m_data->colorStack.pop_back();
  }

  // ---- Grabber management ----

  static std::string grabberKey(const std::string &dev, const std::string &devSpec) {
    if(devSpec.substr(0, dev.length()) != (dev + "=")) {
      return dev + dev + "=" + devSpec;
    }
    return dev + devSpec;
  }

  std::shared_ptr<io::GenericGrabber>
  QuickContext::getGrabber(const std::string &dev, const std::string &devSpec) {
    std::string id = grabberKey(dev, devSpec);
    auto &grabbers = m_data->grabbers;
    if(auto it = grabbers.find(id); it != grabbers.end()) {
      return it->second;
    }
    auto g = std::make_shared<io::GenericGrabber>();
    g->init(dev, devSpec);
    grabbers[id] = g;
    return g;
  }

  void QuickContext::releaseGrabber(const std::string &dev, const std::string &devSpec) {
    m_data->grabbers.erase(grabberKey(dev, devSpec));
  }

  // ---- Op application ----

  Image QuickContext::applyOp(filter::UnaryOp &op, const Image &src) {
    auto [d, params] = op.getDestinationParams(src);
    Image dst = getBuffer(d, params);
    op.apply(src, dst);
    return dst;
  }

  Image QuickContext::applyOp(filter::BinaryOp &op, const Image &src1, const Image &src2) {
    auto [d, params] = op.getDestinationParams(src1, src2);
    Image dst = getBuffer(d, params);
    op.apply(src1, src2, dst);
    return dst;
  }

} // namespace icl::qt
