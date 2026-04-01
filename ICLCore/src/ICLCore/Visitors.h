// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLCore/Img.h>

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

    // =================================================================
    //  visitROILinesPerChannel2With — two sources + one dest
    // =================================================================

    /// Calls f(const S* src1, const S* src2, D* dst, int channel, int width)
    /// for each channel, for each ROI line (or once per channel if contiguous).
    template<class S, class D, class F>
    void visitROILinesPerChannel2With(const Img<S> &src1, const Img<S> &src2,
                                      Img<D> &dst, F &&f) {
      const int nc = src1.getChannels();
      const int w = src1.getROIWidth();
      const int h = src1.getROIHeight();
      if(w == src1.getWidth() && w == src2.getWidth() && w == dst.getWidth()){
        for(int ch = 0; ch < nc; ++ch) {
          f(src1.getROIData(ch), src2.getROIData(ch), dst.getROIData(ch), ch, w * h);
        }
      }else{
        const int s1Stride = src1.getWidth();
        const int s2Stride = src2.getWidth();
        const int dStride = dst.getWidth();
        for(int ch = 0; ch < nc; ++ch) {
          const S *s1 = src1.getROIData(ch);
          const S *s2 = src2.getROIData(ch);
          D *d = dst.getROIData(ch);
          for(int y = 0; y < h; ++y, s1 += s1Stride, s2 += s2Stride, d += dStride) {
            f(s1, s2, d, ch, w);
          }
        }
      }
    }

  } // namespace core
} // namespace icl
