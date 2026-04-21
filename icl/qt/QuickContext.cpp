// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/QuickContext.h>
#include <icl/core/CoreFunctions.h>
#include <icl/filter/UnaryOp.h>
#include <icl/filter/BinaryOp.h>
#include <icl/io/GenericGrabber.h>
#include <icl/utils/Macros.h>

#include <vector>
#include <map>
#include <algorithm>

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

  struct QuickContext::Data {
    std::vector<Image> buffers;
    size_t memoryCap;
    size_t currentUsage = 0;
    std::vector<ColorState> colorStack;
    std::map<std::string, std::shared_ptr<io::GenericGrabber>, std::less<>> grabbers;

    explicit Data(size_t cap) : memoryCap(cap) {}
  };

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

    // 1. Exact match among independent buffers
    for(auto &buf : buffers) {
      if(buf.isIndependent() && buf.getDepth() == d
         && buf.getSize() == params.getSize()
         && buf.getChannels() == params.getChannels()) {
        buf.setFormat(params.getFormat());
        return buf;
      }
    }

    // 2. Any independent buffer → resize
    for(auto &buf : buffers) {
      if(buf.isIndependent()) {
        m_data->currentUsage -= buf.memoryUsage();
        buf.ensureCompatible(d, params.getSize(), params.getChannels(), params.getFormat());
        m_data->currentUsage += buf.memoryUsage();
        return buf;
      }
    }

    // 3. Try eviction if over cap
    if(m_data->currentUsage + needed > m_data->memoryCap) {
      // Evict independent buffers (back to front) until enough room
      for(int i = static_cast<int>(buffers.size()) - 1; i >= 0; --i) {
        if(buffers[i].isIndependent()) {
          m_data->currentUsage -= buffers[i].memoryUsage();
          buffers.erase(buffers.begin() + i);
          if(m_data->currentUsage + needed <= m_data->memoryCap) break;
        }
      }
    }

    // 4. Still over cap → unpooled allocation + warning
    if(m_data->currentUsage + needed > m_data->memoryCap) {
      WARNING_LOG("QuickContext: pool memory cap exceeded ("
                  << (m_data->currentUsage >> 20) << "/"
                  << (m_data->memoryCap >> 20)
                  << " MB). Allocating unpooled image. "
                  "Consider increasing the cap or releasing held images.");
      return Image(params.getSize(), d, params.getChannels(), params.getFormat());
    }

    // 5. Room in pool → allocate and track
    buffers.emplace_back(params.getSize(), d, params.getChannels(), params.getFormat());
    m_data->currentUsage += needed;
    return buffers.back();
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
      if(buffers[i].isIndependent()) {
        m_data->currentUsage -= buffers[i].memoryUsage();
        buffers.erase(buffers.begin() + i);
      }
    }
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
