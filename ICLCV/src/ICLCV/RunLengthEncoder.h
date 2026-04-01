// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>
#include <ICLCV/WorkingLineSegment.h>
#include <vector>
#include <ICLCore/Image.h>

namespace icl {
  namespace core{ class Image; }
  namespace cv{

    /// Simple class for creation of a run-length encoding of an image
    /**
        \section GENERAL General Information

        Run-length encoding means that each image line is transformed into
        a set of so called LineSegments. Each of these Segments represents
        a sequence of adjacent image pixels that have the same value. By this means,
        uniform image Regions can be compressed significantly. However, if there
        are only short LineSegments, run-length-encoding might provide a
        larger encoding.


        \section WLS WorkingLineSegments

        The RunLengthEncoder class uses WorkingLineSegments internally, where
        each of these subsumes the following information
        * xstart (x-coordinate of the left-most point of the line segment)
        * xend (x-coordinate of the pixel right to the right-most pixel of the line segment)
        * y (the row index)
        * value (the underlying value)
        * anyData (a pointer to addinional information that might be used by other algorithms
          that use the RunLengthEncoder class

        \section USAGE Usage
        The RunLengthEncoder's main function is RunLengthEncoder::encode(const core::core::Img<T>&, const utils::Point&).
        After encode was called, the run-length-encoded representation of each image line can be accessed
        using RunLengthEncoder::begin(int) and RunLengthEncoder::end(int).

        \section DEPTHS Image Depths
        As the used WorkingLineSegment class provides only an int-value parameter,
        the RunLengthEncoder is <b>not</b> able to process icl32f and icl64f images

        \section ROI ROI Support
        The RunLengthEncoder provides ROI support. The internal encoding function is implemented
        twice (with and without ROI handling) because handling of an images ROI entails a few
        extra computation steps (e.g. shifting of line segments by roi offset).
        If an input image is used, that has a non-full ROI, the internal WorkingLineSegment
        buffer is optimized for that ROI size. Furthermore, the buffer containing the
        line end pointers will also be resized to the input images ROI-height. Therefore
        the given row-indices for the RunLengthEncoder::begin(int) and RunLengthEncoder::end(int)
        functions is always relative to the input images roi-offset.

        <pre>
        image (size: 10x7, roi: (1,2)4x3)
        ..........
        ..........
        .xxxxx.... <- begin(0) and end(0) refer to this line
        .xxxxx....
        .xxxxx.... <- begin(2) and end(2) refer to this line
        ..........
        ..........
        </pre>


        xstart, xend and y
    */
    class ICLCV_API RunLengthEncoder{
      /// internal typedef
      using WLS = WorkingLineSegment;

      /// internal data buffer
      std::vector<WLS> m_data;

      /// internal buffer for line ends
      std::vector<WLS*> m_ends;

      /// current image ROI, the RunLengthEncoder is optimized for (adatped automatically)
      utils::Rect m_imageROI;

      /// internal run-length-encoding template
      template<class T>
      void encode_internal(const core::Img<T> &image);

      /// internal preparation function (automatically called)
      void prepare(const utils::Rect &roi);

      /// resets all formerly used line segments (automatically called)
      void resetLineSegments();

      public:

      /// main encoding function
      void encode(const core::ImgBase *image);

      /// Image-based overload
      inline void encode(const core::Image &image) {
        encode(image.ptr());
      }

      /// Returns a begin()-pointer for the first encoded image line
      /** row-indices are always relative to the image ROI's offset (see \ref ROI) */
      inline WorkingLineSegment *begin(int row){
        return &m_data[row*m_imageROI.width];
      }

      /// Returns a begin()-pointer for the first encoded image line (const)
      /** row-indices are always relative to the image ROI's offset (see \ref ROI) */
      inline const WorkingLineSegment *begin(int row) const{
        return &m_data[row*m_imageROI.width];
      }

      /// Returns a end()-pointer for the first encoded image line
      /** row-indices are always relative to the image ROI's offset (see \ref ROI) */
      inline WorkingLineSegment *end(int row){
        return m_ends[row];
      }

      /// Returns a end()-pointer for the first encoded image line (const)
      /** row-indices are always relative to the image ROI's offset (see \ref ROI) */
      inline const WorkingLineSegment *end(int row) const {
        return m_ends[row];
      }
    };

  } // namespace cv
}
