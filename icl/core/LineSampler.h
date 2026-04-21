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
  };

  } // namespace icl::core