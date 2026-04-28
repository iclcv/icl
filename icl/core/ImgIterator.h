// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting, Robert Haschke

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/la/MatrixSubRectIterator.h>

namespace icl::core {

  /// STL-style iterator over the ROI pixels of one image channel. \ingroup IMAGE
  /** Walks the ROI rectangle of a single channel, line by line, jumping
      back to the ROI's left edge at each line break.  Reachable as
      `Img<T>::roi_iterator` via `Img<T>::beginROI(channel)` /
      `endROI(channel)`; that is the canonical way to construct one.

      A second constructor produces a *sub-region* iterator anchored at
      another iterator's current position — used by neighborhood ops
      (convolution, median, erosion) to walk a fixed-size kernel mask
      around the outer iterator's pixel.

      For tight per-pixel loops over a whole channel, prefer the
      pointer-based primitives in `Visitors.h` (`visitChannels`,
      `visitROILines`, …) which avoid per-pixel ROI bookkeeping.
      `ImgIterator` exists for the cases where STL iterator semantics
      (compatibility with `<algorithm>` etc.) are wanted. */
  template <typename Type>
  class ImgIterator : public math::MatrixSubRectIterator<Type>{
    public:

    /// builds the past-the-end iterator that pairs with a beginROI
    static inline const ImgIterator<Type> create_end_roi_iterator(const Type *data,
                                                                  int width,
                                                                  const utils::Rect &roi){
      ImgIterator<Type> i(const_cast<Type*>(data),width,roi);
      i.m_dataCurr = i.m_dataEnd - roi.width + width;
      i.m_currLineEnd = i.m_dataCurr + roi.width;
      return i;
    }

    /// default constructor (singular iterator)
    inline ImgIterator() = default;

    /// channel-data + image-width + ROI rect constructor
    inline ImgIterator(Type *data,int imageWidth,const utils::Rect &roi):
    math::MatrixSubRectIterator<Type>(data,imageWidth,roi.x,roi.y,roi.width,roi.height){}

    /// sub-region iterator anchored at `origin`'s current position
    /** Used for neighborhood ops: walks a `s.width × s.height` mask
        around the pixel currently pointed to by `origin`, with the
        mask anchor at offset `a`. */
    inline ImgIterator(const ImgIterator<Type> &origin, const utils::Size &s, const utils::Point &a){
      ImgIterator<Type>::m_matrixWidth=origin.m_matrixWidth;
      ImgIterator<Type>::m_subRectWidth=s.width;
      ImgIterator<Type>::m_subRectHeight=s.height;
      ImgIterator<Type>::m_dataOrigin=origin.m_dataOrigin;
      ImgIterator<Type>::m_dataCurr=origin.m_dataCurr - a.x - a.y * origin.m_matrixWidth;
      ImgIterator<Type>::init();
    }

    /// allows assigning from a base-class iterator
    inline ImgIterator<Type> &operator=(const math::MatrixSubRectIterator<Type> &other){
      math::MatrixSubRectIterator<Type>::operator=(other);
      return *this;
    }

    /// const-target version of the base-class assignment
    inline const ImgIterator<Type> &operator=(const math::MatrixSubRectIterator<Type> &other) const{
      math::MatrixSubRectIterator<Type>::operator=(other);
      return *this;
    }

    /// width of the ROI this iterator walks
    int getROIWidth() const {
      return math::MatrixSubRectIterator<Type>::getSubRectWidth();
    }

    /// height of the ROI this iterator walks
    int getROIHeight() const {
      return math::MatrixSubRectIterator<Type>::getSubRectHeight();
    }
  };
  } // namespace icl::core
