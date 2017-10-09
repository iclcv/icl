/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/LineSampler.h                      **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Rect.h>
#include <ICLUtils/Exception.h>

#include <vector>


namespace icl{
  namespace core{
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

  } // namespace core
}

