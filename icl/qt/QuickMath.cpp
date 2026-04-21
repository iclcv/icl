// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/QuickMath.h>
#include <icl/qt/QuickContext.h>
#include <icl/core/Img.h>
#include <icl/core/ImgBase.h>
#include <icl/filter/BinaryArithmeticalOp.h>
#include <icl/filter/UnaryArithmeticalOp.h>

using namespace icl::core;
using namespace icl::utils;
using namespace icl::filter;

namespace icl::qt {

  // ---- binary image ops ----

  Image operator+(const Image &a, const Image &b) {
    return activeContext().applyOp(BinaryArithmeticalOp(BinaryArithmeticalOp::addOp), a, b);
  }
  Image operator-(const Image &a, const Image &b) {
    return activeContext().applyOp(BinaryArithmeticalOp(BinaryArithmeticalOp::subOp), a, b);
  }
  Image operator*(const Image &a, const Image &b) {
    return activeContext().applyOp(BinaryArithmeticalOp(BinaryArithmeticalOp::mulOp), a, b);
  }
  Image operator/(const Image &a, const Image &b) {
    return activeContext().applyOp(BinaryArithmeticalOp(BinaryArithmeticalOp::divOp), a, b);
  }

  // ---- image + scalar ----

  Image operator+(const Image &image, float val) {
    return activeContext().applyOp(UnaryArithmeticalOp(UnaryArithmeticalOp::addOp, val), image);
  }
  Image operator-(const Image &image, float val) {
    return activeContext().applyOp(UnaryArithmeticalOp(UnaryArithmeticalOp::subOp, val), image);
  }
  Image operator*(const Image &image, float val) {
    return activeContext().applyOp(UnaryArithmeticalOp(UnaryArithmeticalOp::mulOp, val), image);
  }
  Image operator/(const Image &image, float val) {
    return activeContext().applyOp(UnaryArithmeticalOp(UnaryArithmeticalOp::divOp, val), image);
  }

  // ---- scalar + image (commutative delegate, non-commutative via visit) ----

  Image operator+(float val, const Image &image) { return image + val; }
  Image operator*(float val, const Image &image) { return image * val; }

  Image operator-(float val, const Image &image) {
    // val - image = -(image - val) = negate(image - val)
    // or equivalently: val + (-1 * image)
    return (image * -1.0f) + val;
  }

  Image operator/(float val, const Image &image) {
    // val / image — need per-pixel: result[i] = (image[i] == 0) ? 0 : val / image[i]
    auto &ctx = activeContext();
    Image result = ctx.getBuffer(image.getDepth(),
        ImgParams(image.getROISize(), image.getChannels(), image.getFormat()));
    result.visit([&](auto &dst) {
      using T = typename std::remove_reference_t<decltype(dst)>::type;
      const auto &src = image.as<T>();
      T v = static_cast<T>(val);
      for(int c = 0; c < src.getChannels(); ++c) {
        const T *sp = src.getROIData(c);
        T *dp = dst.getData(c);
        int dim = dst.getDim();
        for(int i = 0; i < dim; ++i) {
          dp[i] = (sp[i] == T(0)) ? T(0) : static_cast<T>(v / sp[i]);
        }
      }
    });
    return result;
  }

  // ---- unary negation ----

  Image operator-(const Image &image) { return image * -1.0f; }

  // ---- math functions ----

  Image exp(const Image &image) {
    return activeContext().applyOp(UnaryArithmeticalOp(UnaryArithmeticalOp::expOp), image);
  }
  Image ln(const Image &image) {
    return activeContext().applyOp(UnaryArithmeticalOp(UnaryArithmeticalOp::lnOp), image);
  }
  Image sqr(const Image &image) {
    return activeContext().applyOp(UnaryArithmeticalOp(UnaryArithmeticalOp::sqrOp), image);
  }
  Image sqrt(const Image &image) {
    return activeContext().applyOp(UnaryArithmeticalOp(UnaryArithmeticalOp::sqrtOp), image);
  }
  Image abs(const Image &image) {
    return activeContext().applyOp(UnaryArithmeticalOp(UnaryArithmeticalOp::absOp), image);
  }

  // ---- logical operators ----

  namespace {
    /// Promotes both images to the deeper depth and ensures matching ROI/channels
    std::pair<Image, Image> prepareLogicalOp(const Image &a, const Image &b) {
      // Use the deeper depth for the result
      depth d = std::max(a.getDepth(), b.getDepth());
      auto &ctx = activeContext();

      Size s(std::max(a.getROISize().width, b.getROISize().width),
             std::max(a.getROISize().height, b.getROISize().height));
      int ch = std::max(a.getChannels(), b.getChannels());

      Image na = ctx.getBuffer(d, ImgParams(s, ch));
      Image nb = ctx.getBuffer(d, ImgParams(s, ch));

      na.clear();
      nb.clear();

      // Copy a's ROI data into na
      ImgBase *naPtr = na.ptr();
      a.ptr()->deepCopyROI(&naPtr);

      ImgBase *nbPtr = nb.ptr();
      b.ptr()->deepCopyROI(&nbPtr);

      return {na, nb};
    }
  }

  Image operator||(const Image &a, const Image &b) {
    auto prepared = prepareLogicalOp(a, b);
    Image &na = prepared.first, &nb = prepared.second;
    Image result = activeContext().getBuffer(na.getDepth(),
        ImgParams(na.getSize(), na.getChannels()));
    result.visit([&](auto &dst) {
      using T = typename std::remove_reference_t<decltype(dst)>::type;
      const auto &sa = na.as<T>();
      const auto &sb = nb.as<T>();
      for(int c = 0; c < dst.getChannels(); ++c) {
        const T *ap = sa.getData(c);
        const T *bp = sb.getData(c);
        T *dp = dst.getData(c);
        int dim = dst.getDim();
        for(int i = 0; i < dim; ++i) {
          dp[i] = static_cast<T>(255) * (ap[i] > T(0) || bp[i] > T(0));
        }
      }
    });
    return result;
  }

  Image operator&&(const Image &a, const Image &b) {
    auto prepared = prepareLogicalOp(a, b);
    Image &na = prepared.first, &nb = prepared.second;
    Image result = activeContext().getBuffer(na.getDepth(),
        ImgParams(na.getSize(), na.getChannels()));
    result.visit([&](auto &dst) {
      using T = typename std::remove_reference_t<decltype(dst)>::type;
      const auto &sa = na.as<T>();
      const auto &sb = nb.as<T>();
      for(int c = 0; c < dst.getChannels(); ++c) {
        const T *ap = sa.getData(c);
        const T *bp = sb.getData(c);
        T *dp = dst.getData(c);
        int dim = dst.getDim();
        for(int i = 0; i < dim; ++i) {
          dp[i] = static_cast<T>(255) * (ap[i] > T(0) && bp[i] > T(0));
        }
      }
    });
    return result;
  }

  // ---- bitwise operators ----

  namespace {
    template<class CastT, class BinFunc>
    Image applyBitwise(const Image &a, const Image &b, BinFunc func) {
      auto prepared = prepareLogicalOp(a, b);
      Image &na = prepared.first, &nb = prepared.second;
      Image result = activeContext().getBuffer(na.getDepth(),
          ImgParams(na.getSize(), na.getChannels()));
      result.visit([&](auto &dst) {
        using T = typename std::remove_reference_t<decltype(dst)>::type;
        const auto &sa = na.as<T>();
        const auto &sb = nb.as<T>();
        for(int c = 0; c < dst.getChannels(); ++c) {
          const T *ap = sa.getData(c);
          const T *bp = sb.getData(c);
          T *dp = dst.getData(c);
          int dim = dst.getDim();
          for(int i = 0; i < dim; ++i) {
            CastT va = static_cast<CastT>(ap[i]);
            CastT vb = static_cast<CastT>(bp[i]);
            dp[i] = static_cast<T>(func(va, vb));
          }
        }
      });
      return result;
    }
  }

  template<class T>
  Image binOR(const Image &a, const Image &b) {
    return applyBitwise<T>(a, b, [](T x, T y) { return x | y; });
  }

  template<class T>
  Image binXOR(const Image &a, const Image &b) {
    return applyBitwise<T>(a, b, [](T x, T y) { return x ^ y; });
  }

  template<class T>
  Image binAND(const Image &a, const Image &b) {
    return applyBitwise<T>(a, b, [](T x, T y) { return x & y; });
  }

  // Explicit instantiations for integer cast types
  template ICLQt_API Image binOR<icl8u>(const Image&, const Image&);
  template ICLQt_API Image binOR<icl16s>(const Image&, const Image&);
  template ICLQt_API Image binOR<icl32s>(const Image&, const Image&);

  template ICLQt_API Image binXOR<icl8u>(const Image&, const Image&);
  template ICLQt_API Image binXOR<icl16s>(const Image&, const Image&);
  template ICLQt_API Image binXOR<icl32s>(const Image&, const Image&);

  template ICLQt_API Image binAND<icl8u>(const Image&, const Image&);
  template ICLQt_API Image binAND<icl16s>(const Image&, const Image&);
  template ICLQt_API Image binAND<icl32s>(const Image&, const Image&);

} // namespace icl::qt
