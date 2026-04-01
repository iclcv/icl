// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/VisualizationDescription.h>
#include <ICLUtils/Configurable.h>
#include <ICLCore/Image.h>

namespace icl{
  namespace core{ class Image; }

  namespace cv{

    /// Utility class wrapping OpenCV's cvFindChessboardCorners
    /** The CheckerboardDetector wrappes OpenCV's cvFindChessboardCorners and provides
        a flag to optionally optimize the detected corners using
        cvFindCornerSubPix */
    class ICLCV_API CheckerboardDetector : public utils::Configurable{
      struct Data;   //!< internal data data
      Data *m_data;  //!< internal data pointer

      ///intializes configurable properties internally
      void init_properties();

      public:
      CheckerboardDetector(const CheckerboardDetector&) = delete;
      CheckerboardDetector& operator=(const CheckerboardDetector&) = delete;


      /// Default constructor (creates a null instance)
      CheckerboardDetector();

      /// Constructor with given checkerboard size
      /** Please note: the checkerboard size given relates to the inner checkerboard
          corners that are expected. So if the checkerboard has 5 by 5 fields, i.e.
          the first row is like BWBWB (Black/White), then you have to pass a size
          of 4x4 */
      CheckerboardDetector(const utils::Size &size);

      /// Destructor
      ~CheckerboardDetector();

      /// for deferred initialization
      /** Please note: the checkerboard size given relates to the inner checkerboard
          corners that are expected. So if the checkerboard has 5 by 5 fields, i.e.
          the first row is like BWBWB (Black/White), then you have to pass a size
          of 4x4 */
      void init(const utils::Size &size);

      /// Internally used and returned result structure
      struct Checkerboard{
        bool found;        //!< was it found (i.e. all of the corners)
        utils::Size size;  //!< used size (see init)
        std::vector<utils::Point32f> corners; //!< found corners
        ICLCV_API utils::VisualizationDescription visualize() const;
      };

      /// returns whether this instance has been initilialized yet
      bool isNull() const;

      /// detects the defined checkerboard in the given image
      /** The image can have any format, but internally is is always converted
          to gray (if it is not of formatGray). If optSubPix was set in
          either the constructor or in init, the returned corners are
          automatically optimized using cvFindCornerSubPix */
      const Checkerboard &detect(const core::Img8u &image);

      /// convenience method that automatically scales the source images range to 0,255 if it is not already of type Img8u
      const Checkerboard &detect(const core::ImgBase *image);

      /// Image-based overload
      inline const Checkerboard &detect(const core::Image &image) {
        return detect(image.ptr());
      }
    };

  } // namespace cv
} // namespace icl
