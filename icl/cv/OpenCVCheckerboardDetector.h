// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/cv/CheckerboardDetector.h>
#include <icl/core/Img.h>
#include <icl/utils/VisualizationDescription.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/Image.h>

namespace icl{
  namespace core{ class Image; }

  namespace cv{

    /// CheckerboardDetector backend wrapping OpenCV's findChessboardCorners.
    /** Wraps OpenCV's findChessboardCorners and optionally refines the detected
        corners with cornerSubPix. findChessboardCorners returns the inner corners
        already ordered row-major, so a successful detection maps directly onto a
        complete cv::CheckerboardGrid.

        Besides the polymorphic CheckerboardDetector::detect() seam, the original
        concrete API (init() + the cached Checkerboard struct + Configurable
        subpixel properties) is preserved for existing consumers. */
    class ICLCV_API OpenCVCheckerboardDetector : public CheckerboardDetector,
                                                 public utils::Configurable{
      struct Data;   //!< internal data data
      Data *m_data;  //!< internal data pointer

      ///intializes configurable properties internally
      void init_properties();

      public:
      OpenCVCheckerboardDetector(const OpenCVCheckerboardDetector&) = delete;
      OpenCVCheckerboardDetector& operator=(const OpenCVCheckerboardDetector&) = delete;


      /// Default constructor (creates a null instance)
      OpenCVCheckerboardDetector();

      /// Constructor with given checkerboard size
      /** Please note: the checkerboard size given relates to the inner checkerboard
          corners that are expected. So if the checkerboard has 5 by 5 fields, i.e.
          the first row is like BWBWB (Black/White), then you have to pass a size
          of 4x4 */
      OpenCVCheckerboardDetector(const utils::Size &size);

      /// Destructor
      ~OpenCVCheckerboardDetector();

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

      /// CheckerboardDetector seam: \a hints.boardCells (re)initialises the board
      /// size; a successful detection yields one complete CheckerboardGrid.
      /** No default \a hints (unlike the base) so the 1-arg legacy detect()
          overloads above stay unambiguous. */
      Result detect(const core::Img8u &image, const Hints &hints) override;
      std::string name() const override { return "opencv"; }
    };

  } // namespace cv
} // namespace icl
