/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Visitors.h                         **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLCore/Img.h>
#include <type_traits>

/// @file Visitors.h
/// Optimized line-based ROI visitors for Img<T>.
///
/// These replace per-pixel ImgIterator loops with tight line-pointer
/// loops: the ROI line walk is in the outer loop, the inner loop gets
/// a raw T* and width — no per-pixel branch, auto-vectorization friendly.
///
/// When the ROI spans the full image width, the line loop is elided
/// and the callback receives the entire channel as a single contiguous
/// block (width * height), maximizing SIMD throughput.
///
/// For multi-channel visitors (visitROILinesN, visitROILinesNWith),
/// see VisitorsN.h.
///
/// Naming convention:
///   visitROILines            — one image, one channel
///   visitROILinesWith        — src + dst, one channel each
///   ...PerChannel            — loop over all channels, one image
///   ...PerChannelWith        — loop over all channels, src + dst

namespace icl {
  namespace core {

    // =================================================================
    //  visitROILines — single channel, single image
    // =================================================================

    /// Calls f(const T* data, int width) for each ROI line.
    /// If ROI spans full image width, calls f once with width * height.
    template<class T, class F>
    void visitROILines(const Img<T> &img, int ch, F &&f) {
      const T *p = img.getROIData(ch);
      const int w = img.getROIWidth();
      const int h = img.getROIHeight();
      if(w == img.getWidth()){
        f(p, w * h);
      }else{
        const int stride = img.getWidth();
        for(int y = 0; y < h; ++y, p += stride) f(p, w);
      }
    }

    /// Mutable variant: f(T* data, int width).
    template<class T, class F>
    void visitROILines(Img<T> &img, int ch, F &&f) {
      T *p = img.getROIData(ch);
      const int w = img.getROIWidth();
      const int h = img.getROIHeight();
      if(w == img.getWidth()){
        f(p, w * h);
      }else{
        const int stride = img.getWidth();
        for(int y = 0; y < h; ++y, p += stride) f(p, w);
      }
    }

    // =================================================================
    //  visitROILinesWith — src channel + dst channel
    // =================================================================

    /// Calls f(const S* srcData, D* dstData, int width) for each ROI line.
    /// If both ROIs span full width, calls f once with width * height.
    template<class S, class D, class F>
    void visitROILinesWith(const Img<S> &src, int srcCh,
                           Img<D> &dst, int dstCh, F &&f) {
      const S *s = src.getROIData(srcCh);
      D *d = dst.getROIData(dstCh);
      const int w = src.getROIWidth();
      const int h = src.getROIHeight();
      if(w == src.getWidth() && w == dst.getWidth()){
        f(s, d, w * h);
      }else{
        const int srcStride = src.getWidth();
        const int dstStride = dst.getWidth();
        for(int y = 0; y < h; ++y, s += srcStride, d += dstStride) f(s, d, w);
      }
    }

    // =================================================================
    //  visitROILinesPerChannelWith — all channels, src + dst
    // =================================================================

    /// Calls f(const S* srcData, D* dstData, int channel, int width)
    /// for each channel, for each ROI line (or once per channel if contiguous).
    template<class S, class D, class F>
    void visitROILinesPerChannelWith(const Img<S> &src, Img<D> &dst, F &&f) {
      const int nc = src.getChannels();
      const int w = src.getROIWidth();
      const int h = src.getROIHeight();
      if(w == src.getWidth() && w == dst.getWidth()){
        for(int ch = 0; ch < nc; ++ch) {
          f(src.getROIData(ch), dst.getROIData(ch), ch, w * h);
        }
      }else{
        const int srcStride = src.getWidth();
        const int dstStride = dst.getWidth();
        for(int ch = 0; ch < nc; ++ch) {
          const S *s = src.getROIData(ch);
          D *d = dst.getROIData(ch);
          for(int y = 0; y < h; ++y, s += srcStride, d += dstStride) {
            f(s, d, ch, w);
          }
        }
      }
    }

    // =================================================================
    //  visitROILinesPerChannel — all channels, single image
    // =================================================================

    /// Const variant: f(const T* data, int channel, int width).
    template<class T, class F>
    void visitROILinesPerChannel(const Img<T> &img, F &&f) {
      const int nc = img.getChannels();
      const int w = img.getROIWidth();
      const int h = img.getROIHeight();
      if(w == img.getWidth()){
        for(int ch = 0; ch < nc; ++ch) f(img.getROIData(ch), ch, w * h);
      }else{
        const int stride = img.getWidth();
        for(int ch = 0; ch < nc; ++ch) {
          const T *p = img.getROIData(ch);
          for(int y = 0; y < h; ++y, p += stride) f(p, ch, w);
        }
      }
    }

    /// Mutable variant: f(T* data, int channel, int width).
    template<class T, class F>
    void visitROILinesPerChannel(Img<T> &img, F &&f) {
      const int nc = img.getChannels();
      const int w = img.getROIWidth();
      const int h = img.getROIHeight();
      if(w == img.getWidth()){
        for(int ch = 0; ch < nc; ++ch) f(img.getROIData(ch), ch, w * h);
      }else{
        const int stride = img.getWidth();
        for(int ch = 0; ch < nc; ++ch) {
          T *p = img.getROIData(ch);
          for(int y = 0; y < h; ++y, p += stride) f(p, ch, w);
        }
      }
    }

    // ================================================================
    // Per-pixel visitors — convenience over line-based visitors.
    // Not for inner loops of performance-critical filters (use
    // visitROILines* for those). Ideal for test code, initialization,
    // and non-hot paths.
    //
    // The lambda signature is auto-detected:
    //   f(T &val)                  — value only
    //   f(int x, int y, T &val)   — coordinates + value
    //   f(int x, int y, int c, T &val) — full (coordinates, channel, value)
    //
    // Iterates over the ROI (or full image if no ROI set).
    // ================================================================

    template<class T, class F>
    void visitPixels(Img<T> &img, F &&f) {
      const int rx = img.getROI().x, ry = img.getROI().y;
      const int rw = img.getROIWidth(), rh = img.getROIHeight();
      if constexpr (std::is_invocable_v<F, T&>) {
        for (int c = 0; c < img.getChannels(); c++)
          for (int y = ry; y < ry + rh; y++)
            for (int x = rx; x < rx + rw; x++)
              f(img(x, y, c));
      } else if constexpr (std::is_invocable_v<F, int, int, T&>) {
        for (int c = 0; c < img.getChannels(); c++)
          for (int y = ry; y < ry + rh; y++)
            for (int x = rx; x < rx + rw; x++)
              f(x, y, img(x, y, c));
      } else {
        for (int c = 0; c < img.getChannels(); c++)
          for (int y = ry; y < ry + rh; y++)
            for (int x = rx; x < rx + rw; x++)
              f(x, y, c, img(x, y, c));
      }
    }

    template<class T, class F>
    void visitPixels(const Img<T> &img, F &&f) {
      const int rx = img.getROI().x, ry = img.getROI().y;
      const int rw = img.getROIWidth(), rh = img.getROIHeight();
      if constexpr (std::is_invocable_v<F, const T&>) {
        for (int c = 0; c < img.getChannels(); c++)
          for (int y = ry; y < ry + rh; y++)
            for (int x = rx; x < rx + rw; x++)
              f(img(x, y, c));
      } else if constexpr (std::is_invocable_v<F, int, int, const T&>) {
        for (int c = 0; c < img.getChannels(); c++)
          for (int y = ry; y < ry + rh; y++)
            for (int x = rx; x < rx + rw; x++)
              f(x, y, img(x, y, c));
      } else {
        for (int c = 0; c < img.getChannels(); c++)
          for (int y = ry; y < ry + rh; y++)
            for (int x = rx; x < rx + rw; x++)
              f(x, y, c, img(x, y, c));
      }
    }

  } // namespace core
} // namespace icl
