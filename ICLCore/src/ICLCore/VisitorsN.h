/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/VisitorsN.h                        **
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

#include <ICLCore/Visitors.h>
#include <array>
#include <utility>

/// @file VisitorsN.h
/// Multi-channel ROI visitors with flat callback arguments.
///
/// These extend the basic visitors in Visitors.h to access N source
/// channels and/or M destination channels simultaneously per ROI line.
/// The callback receives individual pointers for each channel, unpacked
/// via index_sequence — no manual array indexing needed.
///
/// Example (ColorDistanceOp: 3 src channels → 1 dst channel):
///   visitROILinesNWith<3,1>(src, dst,
///     [&](const S *r, const S *g, const S *b, D *d, int w) {
///       for(int i = 0; i < w; ++i) { ... }
///     });
///
/// When the ROI spans the full image width, the line loop is elided
/// and the callback receives the entire channel as a single contiguous
/// block (width * height), maximizing SIMD throughput.

namespace icl {
  namespace core {

    namespace detail {
      template<class T, int N, class F, size_t... Is>
      void call_n(F &&f, const std::array<const T*, N> &ptrs, int w,
                  std::index_sequence<Is...>) {
        f(ptrs[Is]..., w);
      }

      template<class S, class D, int N, int M, class F, size_t... Is, size_t... Js>
      void call_nm(F &&f, const std::array<const S*, N> &sp,
                   const std::array<D*, M> &dp, int w,
                   std::index_sequence<Is...>, std::index_sequence<Js...>) {
        f(sp[Is]..., dp[Js]..., w);
      }
    } // namespace detail

    // =================================================================
    //  visitROILinesN — N channels from one image, flat callback args
    //
    //  For N=3: f(const T* ch0, const T* ch1, const T* ch2, int width)
    // =================================================================

    /// Calls f(const T* ch0, const T* ch1, ..., int width) per ROI line.
    /// Uses channels 0 .. N-1. Contiguous fast-path when ROI spans full width.
    template<int N, class T, class F>
    void visitROILinesN(const Img<T> &img, F &&f) {
      const int w = img.getROIWidth();
      const int h = img.getROIHeight();
      std::array<const T*, N> ptrs;
      for(int i = 0; i < N; ++i) ptrs[i] = img.getROIData(i);
      if(w == img.getWidth()){
        detail::call_n<T, N>(f, ptrs, w * h, std::make_index_sequence<N>{});
      }else{
        const int stride = img.getWidth();
        for(int y = 0; y < h; ++y) {
          detail::call_n<T, N>(f, ptrs, w, std::make_index_sequence<N>{});
          for(auto &p : ptrs) p += stride;
        }
      }
    }

    // =================================================================
    //  visitROILinesNWith — N src channels + M dst channels, flat args
    //
    //  For N=3, M=1:
    //    f(const S* r, const S* g, const S* b, D* d, int width)
    // =================================================================

    /// Calls f(const S* sCh0, ..., D* dCh0, ..., int width) per ROI line.
    /// Uses src channels 0..N-1, dst channels 0..M-1.
    /// Contiguous fast-path when both ROIs span full width.
    template<int N, int M, class S, class D, class F>
    void visitROILinesNWith(const Img<S> &src, Img<D> &dst, F &&f) {
      const int w = src.getROIWidth();
      const int h = src.getROIHeight();
      std::array<const S*, N> sp;
      std::array<D*, M> dp;
      for(int i = 0; i < N; ++i) sp[i] = src.getROIData(i);
      for(int i = 0; i < M; ++i) dp[i] = dst.getROIData(i);
      if(w == src.getWidth() && w == dst.getWidth()){
        detail::call_nm<S, D, N, M>(f, sp, dp, w * h,
          std::make_index_sequence<N>{}, std::make_index_sequence<M>{});
      }else{
        const int srcStride = src.getWidth();
        const int dstStride = dst.getWidth();
        for(int y = 0; y < h; ++y) {
          detail::call_nm<S, D, N, M>(f, sp, dp, w,
            std::make_index_sequence<N>{}, std::make_index_sequence<M>{});
          for(auto &p : sp) p += srcStride;
          for(auto &p : dp) p += dstStride;
        }
      }
    }

  } // namespace core
} // namespace icl
