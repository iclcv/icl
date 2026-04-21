// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/QuickCompose.h>
#include <icl/qt/QuickContext.h>
#include <icl/core/Img.h>
#include <icl/core/ImgBase.h>
#include <icl/core/CoreFunctions.h>

#include <algorithm>

using namespace icl::core;
using namespace icl::utils;

namespace icl::qt {

  // Helper: depth promotion — use the deeper of two depths
  static depth promoteDepth(const Image &a, const Image &b) {
    return std::max(a.getDepth(), b.getDepth());
  }

  // Helper: convert to target depth if needed (pool-backed)
  static Image ensureDepth(const Image &img, depth d) {
    if(img.getDepth() == d) return img;
    Image dst = activeContext().getBuffer(d,
        ImgParams(img.getSize(), img.getChannels(), img.getFormat()));
    img.ptr()->convert(dst.ptr());
    return dst;
  }

  // ---- horizontal concatenation ----

  // Helper: blit src into dst at given offset, channel-by-channel
  static void blitInto(const Image &src, Image &dst, const Point &dstOffset) {
    dst.visit([&](auto &dstImg) {
      using T = typename std::remove_reference_t<decltype(dstImg)>::type;
      const auto &srcImg = src.as<T>();
      int ch = std::min(srcImg.getChannels(), dstImg.getChannels());
      Size srcSize = srcImg.getSize();
      for(int c = 0; c < ch; ++c) {
        deepCopyChannelROI(&srcImg, c, Point::null, srcSize,
                           &dstImg, c, dstOffset, srcSize);
      }
    });
  }

  Image operator,(const Image &a, const Image &b) {
    if(a.isNull() || a.getSize() == Size::null) return b.isNull() ? Image() : b.deepCopy();
    if(b.isNull() || b.getSize() == Size::null) return a.deepCopy();

    depth d = promoteDepth(a, b);
    Image ca = ensureDepth(a, d), cb = ensureDepth(b, d);

    int ch = std::max(ca.getChannels(), cb.getChannels());
    Size sz(ca.getWidth() + cb.getWidth(), std::max(ca.getHeight(), cb.getHeight()));

    Image r = activeContext().getBuffer(d, ImgParams(sz, ch));
    r.clear();
    blitInto(ca, r, Point::null);
    blitInto(cb, r, Point(ca.getWidth(), 0));
    return r;
  }

  // ---- vertical concatenation ----

  Image operator%(const Image &a, const Image &b) {
    if(a.isNull() || a.getSize() == Size::null) return b.isNull() ? Image() : b.deepCopy();
    if(b.isNull() || b.getSize() == Size::null) return a.deepCopy();

    depth d = promoteDepth(a, b);
    Image ca = ensureDepth(a, d), cb = ensureDepth(b, d);

    int ch = std::max(ca.getChannels(), cb.getChannels());
    Size sz(std::max(ca.getWidth(), cb.getWidth()), ca.getHeight() + cb.getHeight());

    Image r = activeContext().getBuffer(d, ImgParams(sz, ch));
    r.clear();
    blitInto(ca, r, Point::null);
    blitInto(cb, r, Point(0, ca.getHeight()));
    return r;
  }

  // ---- channel concatenation ----

  Image operator|(const Image &a, const Image &b) {
    if(a.isNull() || a.getChannels() == 0) return b.isNull() ? Image() : b.deepCopy();
    if(b.isNull() || b.getChannels() == 0) return a.deepCopy();

    depth d = promoteDepth(a, b);
    Image ca = ensureDepth(a, d), cb = ensureDepth(b, d);

    Size sz(std::max(ca.getWidth(), cb.getWidth()), std::max(ca.getHeight(), cb.getHeight()));
    int totalCh = ca.getChannels() + cb.getChannels();

    Image r = activeContext().getBuffer(d, ImgParams(sz, totalCh));
    r.clear();

    // Copy channels from a, then from b
    r.visit([&](auto &dst) {
      using T = typename std::remove_reference_t<decltype(dst)>::type;
      const auto &sa = ca.as<T>();
      const auto &sb = cb.as<T>();
      for(int c = 0; c < sa.getChannels(); ++c) {
        deepCopyChannelROI(&sa, c, sa.getROIOffset(), sa.getROISize(),
                           &dst, c, Point::null, sa.getROISize());
      }
      for(int c = 0; c < sb.getChannels(); ++c) {
        deepCopyChannelROI(&sb, c, sb.getROIOffset(), sb.getROISize(),
                           &dst, c + sa.getChannels(), Point::null, sb.getROISize());
      }
    });
    return r;
  }

  // ---- ImgROI2 ----

  ImgROI2 &ImgROI2::operator=(const Image &i) {
    ICLASSERT_RETURN_VAL(image.getROISize() == i.getROISize(), *this);
    image.visit([&](auto &dst) {
      using T = typename std::remove_reference_t<decltype(dst)>::type;
      const auto &src = i.as<T>();
      int ch = std::min(dst.getChannels(), src.getChannels());
      for(int c = 0; c < ch; ++c) {
        deepCopyChannelROI(&src, c, src.getROIOffset(), src.getROISize(),
                           &dst, c, dst.getROIOffset(), dst.getROISize());
      }
    });
    return *this;
  }

  ImgROI2 &ImgROI2::operator=(float val) {
    image.clear(-1, val);
    return *this;
  }

  ImgROI2 &ImgROI2::operator=(const ImgROI2 &r) {
    return (*this = r.image);
  }

  ImgROI2::operator Image() {
    return image;
  }

  ImgROI2 roi(Image &r) {
    return ImgROI2{r.shallowCopy()};
  }

  ImgROI2 data(Image &r) {
    ImgROI2 result{r.shallowCopy()};
    result.image.setFullROI();
    return result;
  }

} // namespace icl::qt
