// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Configurable.h>
#include <ICLUtils/Uncopyable.h>
// forward declaration (was #include <ICLCore/ImgBase.h>)
#include <ICLCV/RegionDetector.h>
#include <vector>

#include <ICLMarkers/TiltedQuad.h>
namespace icl { namespace core { class ImgBase; template<class T> class Img; } }

namespace icl{
  namespace markers{

    /// Tool-class for detecting tilted quads in images
    /** The quad detector combines usual steps that are used
        to find quad-like structures in input images.\n

        \section GEN General Information

        The QuadDetector combines a local threshold preprocessor, optional
        further preprocessing steps such as median or morphological
        operations with an icl::cv::RegionDetector based search for
        regions with 4 corners.

        \section CONF utils::Configurable interface
        The QuadDetector forwards the local-threshold and the
        RegionDetector options. It also adds some extra properties
        for the post-processing the local-threshold result image before
        it's passed to the region detector internally
    */
    class ICLMarkers_API QuadDetector : public utils::Configurable{

      /// Internal Data class
      class Data;

      /// Internal data pointer (hidden)
      Data *data;

      public:
      QuadDetector(const QuadDetector&) = delete;
      QuadDetector& operator=(const QuadDetector&) = delete;



      /// enum, that helps to specify what quads are searched in the threshold-result image
      enum QuadColor{
        BlackOnly,    //!< only quads that are black (default, value 0)
        WhiteOnly,    //!< only quads that are white (value 255)
        BlackAndWhite //!< white and black quads
      };

      /// Base constructor
      /** @param c the detected quads binary value
          @param dynamic if this is set to true, there will be a changable
                         property for the quad color, otherwise, the initial
                         value will remain fixed

          */
      QuadDetector(QuadColor c = BlackOnly, bool dynamic=false,
    		  float minQRating = 0.4);

      /// Destructor
      ~QuadDetector();

      /// apply-method, that extracts quad-like structures in the input image
      /** This method first applys a local threshold to the given input image,
          which results in a binary icl8u-image. This image is then optionally
          processed by a median and/or by some morphological operations */
      const std::vector<TiltedQuad> &detect(const core::ImgBase *image);

      /// returns the last binary image that was produced internally
      const core::Img8u &getLastBinaryDisplay() const;


      /// returns the internal region detector instance
      icl::cv::RegionDetector* getRegionDetector();


      /// internal typedef for vector of points
      using PVec = std::vector<utils::Point32f>;

      /// internal typedef for vector of vector of points
      using PVecVec = std::vector<PVec>;

      /// returns all corners (const)
      const PVecVec &getAllCorners() const;

      /// returns all corners
      PVecVec &getAllCorners();

      /// returns longest corners (const)
      const PVecVec &getLongestCorners() const;

      /// returns longest corners
      PVecVec &getLongestCorners();

      /// returns 2nd longest corners (const)
      const PVecVec &getSecLongestCorners() const;

      /// returns 2nd longest corners
      PVecVec &getSecLongestCorners();

      /// return inter? conrners (const)
      const PVecVec &getInterCorners() const;

      /// return inter? conrners
      PVecVec &getInterCorners();

      /// returns perpendicular corners (const)
      const PVecVec &getPerpCorners() const;

      /// returns perpendicular corners
      PVecVec &getPerpCorners();

      /// returns mirror corners (const)
      const PVecVec &getMirrorCorners() const;

      /// returns mirror corners (const)
      PVecVec &getMirrorCorners();

      /// computes corners for the given image region
      std::vector<utils::Point32f> computeCorners(const cv::ImageRegion &r) const;
    };


    /// ostream operator for QuadDetector::QuadColor instances
    ICLMarkers_API std::ostream &operator<<(std::ostream &s, const QuadDetector::QuadColor &c);

    /// istream operator for QuadDetector::QuadColor instances
    ICLMarkers_API std::istream &operator>>(std::istream &s, QuadDetector::QuadColor &c);



  } // namespace markers
}
