// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Types.h>

namespace icl::core {
  /// Singleton that owns BackendSelectors for Img utility operations
  /// (mirror, min, max, lut, etc.). Uses ImgBase* as dispatch context
  /// so Img<T> methods can dispatch directly via `this`.
  ///
  /// Backends self-register from _Cpp.cpp / _Ipp.cpp / _Mkl.cpp files.
  class ICLCore_API ImgOps : public ImgBaseBackendDispatching {
  public:
    /// Operation keys — values must match addSelector() insertion order.
    enum class Op : int {
      mirror, clearChannelROI, lut, getMax, getMin, getMinMax, normalize, flippedCopy,
      channelMean, replicateBorder, planarToInterleaved, interleavedToPlanar,
      scaledCopy
    };

    // ---- Dispatch signatures (ImgBase& + operation args) ----
    using MirrorSig = void(ImgBase&, axis, bool roiOnly);
    using ClearChannelROISig = void(ImgBase&, int ch, icl64f val,
                                    const utils::Point& offs, const utils::Size& size);
    using LutSig = void(ImgBase& src, const void* lut, ImgBase& dst, int bits);
    using GetMaxSig = icl64f(ImgBase&, int ch, utils::Point* coords);
    using GetMinSig = icl64f(ImgBase&, int ch, utils::Point* coords);
    using GetMinMaxSig = void(ImgBase&, int ch,
                              icl64f* minVal, icl64f* maxVal,
                              utils::Point* minCoords, utils::Point* maxCoords);
    using NormalizeSig = void(ImgBase&, int ch,
                              icl64f srcMin, icl64f srcMax,
                              icl64f dstMin, icl64f dstMax);
    using FlippedCopySig = void(axis, ImgBase& src, int srcC,
                                const utils::Point& srcOffs, const utils::Size& srcSize,
                                ImgBase& dst, int dstC,
                                const utils::Point& dstOffs, const utils::Size& dstSize);
    using ChannelMeanSig = icl64f(ImgBase&, int channel, bool roiOnly);
    using ReplicateBorderSig = void(ImgBase&);
    using PlanarToInterleavedSig = void(ImgBase& src, void* dst, int dstLineStep);
    using InterleavedToPlanarSig = void(const void* src, ImgBase& dst, int srcLineStep);
    using ScaledCopySig = void(const ImgBase& src, int srcC,
                               const utils::Point& srcOffs, const utils::Size& srcSize,
                               ImgBase& dst, int dstC,
                               const utils::Point& dstOffs, const utils::Size& dstSize,
                               scalemode mode);

    /// Access the singleton instance (lazy-init, thread-safe)
    static ImgOps& instance();

  private:
    ImgOps();
  };

  /// ADL-visible toString for ImgOps::Op → registry name (defined in ImgOps.cpp)
  ICLCore_API const char* toString(ImgOps::Op op);

  } // namespace icl::core