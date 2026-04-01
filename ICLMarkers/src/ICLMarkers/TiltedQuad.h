// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Point32f.h>
#include <ICLCV/ImageRegion.h>
#include <algorithm>

namespace icl{
  namespace markers{


    /// Utility class that represents a tilted quad in an image
    /** A tilted quad is represented by It's for corner points
        The class provides access to these points using the index
        operator
    */
    class TiltedQuad{

      /// list of points (usually sorted in clock-wise order)
      utils::Point32f ps[4];

      /// associated image region
      cv::ImageRegion region;

      public:

      /// creates a null-instance
      inline TiltedQuad():region(0){}

      /// creates a TiltedQuad instance with given 4 corners and given ImageRegion
      inline TiltedQuad(const utils::Point32f &a, const utils::Point32f &b,
                        const utils::Point32f &c, const utils::Point32f &d,
                        const cv::ImageRegion r): region(r){
        ps[0]=a; ps[1]=b; ps[2]=c; ps[3]=d;
      }

      /// creates a TiltedQuad instance with given 4D array of points and image region
      inline TiltedQuad(const utils::Point32f *ps, cv::ImageRegion r): region(r){
        std::copy(ps,ps+4,this->ps);
      }

      /// accesses the i-th corner point
      inline utils::Point32f &operator[](int i){ return ps[i]; }

      /// accesses the i-th corner point (const)
      inline const utils::Point32f &operator[](int i) const{ return ps[i]; }

      /// returns the associated image region (always const)
      cv::ImageRegion getRegion() const { return region; }

      /// sets the image region (this is usually not used explicitly)
      void setRegion(cv::ImageRegion region){ this->region = region; }

      /// returns whether the image region had been set before
      operator bool() const { return region; }

      /// returns the internal utils::Point-data pointer
      const utils::Point32f *data() const { return ps; }
    };
  } // namespace markers
}
