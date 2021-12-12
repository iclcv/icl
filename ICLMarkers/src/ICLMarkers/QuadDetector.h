/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/QuadDetector.h               **
** Module : ICLMarkers                                             **
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
#include <ICLUtils/Configurable.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/ImgBase.h>
#include <ICLCV/RegionDetector.h>
#include <vector>

#include <ICLMarkers/TiltedQuad.h>

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
    class ICLMarkers_API QuadDetector : public utils::Configurable, public utils::Uncopyable{

      /// Internal Data class
      class Data;

      /// Internal data pointer (hidden)
      Data *data;

      public:


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
      const core::Img8u &getLastBinaryImage() const;


      /// returns the internal region detector instance
      icl::cv::RegionDetector* getRegionDetector();


      /// internal typedef for vector of points
      typedef std::vector<utils::Point32f> PVec;

      /// internal typedef for vector of vector of points
      typedef std::vector<PVec> PVecVec;

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
