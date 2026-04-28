// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Point.h>
#include <icl/utils/Rect.h>
#include <icl/utils/Exception.h>

#include <vector>


namespace icl::core {
  /// Utility class for line sampling
  /** The LineSampler class provides a generic framework for efficient line renderig into images.
      It uses the Bresenham line sampling algorithm, that manages to render arbitray lines between
      two images points without any floating point operations. The LineSampler provides an internal
      bounding box check for save line rendering into images

      \section FORMER Former Class Layout
      The LineSampler replaces the former LineSampler class and the SampledLine class

      \section OPT Optimizations

      The class uses several optimizations to speed up linesampling.
      - compile-time switching about several parameters (such as line steepness, and use of boundingbox)
      - bounding-box checks are ommitted if both start- and end-point are within the bounding box
      - optimization of horizontal an vertical lines

      \section PERF Performance Comparison
      The LineSampler works faster than the core::Line class, be cause it does not
      have to allocate memory for the result. A test, conducted on a 2.4 GHz Core-2-Duo
      with an optimized build (-04 -march=native) demonstrated the speed of the line sampler.
      Rendering all possible lines staring at any pixel and pointing to the center of a VGA image
      (640x480 lines), takes about 650 ms. This leads to an approximate time of 0.002ms per line
      or in other words, 500 lines per millisecond;
  */
  class ICLCore_API LineSampler{
    protected:
    std::vector<utils::Point> m_buf; //!< internal buffer
    std::vector<int> m_br;           //!< optionally given bounding rect

    public:

    /// result type providing a Point-pointer and number of sample points
    struct Result{
      const utils::Point *points; //!< sampled points data
      int n;               //!< number of sampled points

      /// convenience inde operator to iterate over sampled points
      const utils::Point &operator[](int idx) const { return points[idx]; }
    };

    /// create linesampler with given maximum number of  points
    /** As long a no bounding rect is given, bound-checks are suppressed */
    LineSampler(int maxLen=0);

    /// createe line sampler with given bounding rect
    LineSampler(const utils::Rect &br);

    /// sets a bounding rect for line sampling
    /** The result will only contain pixels that are within this rectangle */
    void setBoundingRect(const utils::Rect &bb);

    /// removes the bouding rect and sets the internal buffer size
    void removeBoundingBox(int bufferSize);

    /// samples a line between a and b
    Result sample(const utils::Point &a, const utils::Point &b);

    /// samples the line into the given destination vector
    void sample(const utils::Point &a, const utils::Point &b, std::vector<utils::Point> &dst);

    /// Calls f(x, y) for each pixel on the line (no allocation)
    /** The callback is invoked in order from a to b. Pixels outside the
        bounding rect (if set) are skipped. The callback is fully inlined
        by the compiler since the Bresenham engine is templated. */
    template<class F>
    void forEach(const utils::Point &a, const utils::Point &b, F &&f);

  private:
    /// Returns bounding rect data pointer (or nullptr if no bounds)
    const int* brData() const { return m_br.empty() ? nullptr : m_br.data(); }
  };


  // ================================================================
  // Bresenham engine (header-inline for template forEach)
  // ================================================================

  namespace detail {

    template<bool useBounds>
    inline bool boundsCheck(int x, int y, const int br[4]) {
      return x >= br[0] && y >= br[1] && x < br[2] && y < br[3];
    }
    template<> inline bool boundsCheck<false>(int, int, const int[4]) { return true; }

    /// Callback adapter for the Bresenham engine
    template<class F>
    struct CallbackFunc {
      F &f;
      bool inverted = false;
      void init(bool inv) { inverted = inv; }
      void push(const utils::Point &p) { f(p.x, p.y); }
      void inc(int) {}
    };

    template<class Func, int ystep, bool steep2, bool steep, bool usebr>
    inline void bresenham_l4(int x0, int y0, int x1, int y1, const int br[4], Func func) {
      const int deltax = x1 - x0;
      const int deltay = std::abs(y1 - y0);
      int error = 0;
      const int dp = steep2 ? -1 : 1;
      func.init(steep2);
      for(int x = x0, y = y0; x <= x1; x++) {
        if(boundsCheck<usebr>(steep ? y : x, steep ? x : y, br)) {
          func.push(utils::Point(steep ? y : x, steep ? x : y));
          func.inc(dp);
        }
        error += deltay;
        if(2 * error >= deltax) { y += ystep; error -= deltax; }
      }
    }

    template<class Func, int ystep, bool steep2, bool steep>
    inline void bresenham_l3(int x0, int y0, int x1, int y1, const int br[4], Func f) {
      if(br) bresenham_l4<Func, ystep, steep2, steep, true>(x0, y0, x1, y1, br, f);
      else   bresenham_l4<Func, ystep, steep2, steep, false>(x0, y0, x1, y1, br, f);
    }

    template<class Func, int ystep, bool steep2>
    inline void bresenham_l2(int x0, int y0, int x1, int y1, const int br[4], bool steep, Func f) {
      if(steep) bresenham_l3<Func, ystep, steep2, true>(x0, y0, x1, y1, br, f);
      else      bresenham_l3<Func, ystep, steep2, false>(x0, y0, x1, y1, br, f);
    }

    template<class Func, int ystep>
    inline void bresenham_l1(int x0, int y0, int x1, int y1, const int br[4], bool steep, bool steep2, Func f) {
      if(steep2) bresenham_l2<Func, ystep, true>(x0, y0, x1, y1, br, steep, f);
      else       bresenham_l2<Func, ystep, false>(x0, y0, x1, y1, br, steep, f);
    }

    template<class Func>
    inline void bresenham(int x0, int y0, int x1, int y1, const int br[4], Func f) {
      const int *useBR = br;
      if(br && boundsCheck<true>(x0, y0, br) && boundsCheck<true>(x1, y1, br)) useBR = nullptr;
      bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
      if(steep) { std::swap(x0, y0); std::swap(x1, y1); }
      bool steep2 = x0 > x1;
      if(steep2) { std::swap(x0, x1); std::swap(y0, y1); }
      if(y0 < y1) bresenham_l1<Func, 1>(x0, y0, x1, y1, useBR, steep, steep2, f);
      else         bresenham_l1<Func,-1>(x0, y0, x1, y1, useBR, steep, steep2, f);
    }

  } // namespace detail

  // ---- forEach implementation ----

  template<class F>
  void LineSampler::forEach(const utils::Point &a, const utils::Point &b, F &&f) {
    // Optimize horizontal / vertical lines
    const int *br = brData();
    if(a.x == b.x || a.y == b.y) {
      // Simple case: just iterate directly
      if(a.x == b.x) {
        int yStart = std::min(a.y, b.y), yEnd = std::max(a.y, b.y);
        for(int y = yStart; y <= yEnd; ++y) {
          if(!br || detail::boundsCheck<true>(a.x, y, br)) f(a.x, y);
        }
      } else {
        int xStart = std::min(a.x, b.x), xEnd = std::max(a.x, b.x);
        for(int x = xStart; x <= xEnd; ++x) {
          if(!br || detail::boundsCheck<true>(x, a.y, br)) f(x, a.y);
        }
      }
      return;
    }
    detail::CallbackFunc<std::remove_reference_t<F>> cb{f};
    detail::bresenham(a.x, a.y, b.x, b.y, br, cb);
  }

  } // namespace icl::core