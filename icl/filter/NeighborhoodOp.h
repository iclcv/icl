// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/filter/UnaryOp.h>
#include <icl/core/Image.h>

namespace icl::filter {
  /// unary operators that work on each pixels neighborhood \ingroup UNARY \ingroup NBH
  /** The NeighborhoodOp class builds a base class for unary operations employing
      a filter mask which is moved over the ROI of the source image(s),
      e.g. convolution filters use some convolution masks.
      To this end the class provides members to store the size and anchor
      of the filter mask.

      Special care has to be taken, when applying a filter mask to the border
      of an image, e.g. in case of full-image ROI. In this case the filter
      might access undefined pixel values outside the image, actually causing
      a segfault in most cases.

      Hence, the used ROI size of the source image is shrunk if necessary,
      such that the filter mask always fits into the image, when moved over
      the ROI. For this purpose the method computeROI is provided, which
      computes the to-be-used ROI. The ROI of the source image is not
      actually changed, which makes the filter operation thread safe, because
      simultaneous operations running on the source image within different
      threads do not interfere.
  */
  class ICLFilter_API NeighborhoodOp : public UnaryOp {
    public:

    ///Destructor
    virtual ~NeighborhoodOp(){}

    /// compute neccessary ROI offset and size
    /** This functions computes the to-be-used ROI for the source image,
        such that the filter mask of given size (oMaskSize) fits everywhere
        into the image if placed arbitrarily within the ROI.
        The original ROI of the source image is not changed, instead the
        adapted ROI is returned in parameters oROIsize and oROIoffset.
        @param poSrc  image whose ROI is adapted
        @param oROIoffset  new ROI offset
        @param oROIsize    new ROI size
        @return whether a valid ROI remains
        */
    bool computeROI(const core::ImgBase *poSrc, utils::Point& oROIoffset, utils::Size& oROIsize);

    /// *NEW* apply function for multithreaded filtering (reimplemented here for special roi handling!)
    virtual void applyMT(const core::ImgBase *operand1, core::ImgBase **dst, unsigned int nThreads);

    /// Returns destination params accounting for mask margin shrinkage
    std::pair<core::depth, core::ImgParams> getDestinationParams(const core::Image &src) const override;

    /// Import unaryOps apply function without destination image
    using UnaryOp::apply;

    protected:
    NeighborhoodOp() : m_oMaskSize(1,1), m_oAnchor (0,0) {}
    NeighborhoodOp(const utils::Size &size) {
      setMask (size);
    }

    void setMask(const utils::Size &size) {
        m_oMaskSize = adaptSize(size);
        m_oAnchor   = utils::Point (m_oMaskSize.width/2, m_oMaskSize.height/2);
    }
    void setMask(const utils::Size &size, const utils::Point &anchor){
      m_oMaskSize = adaptSize(size);
      m_oAnchor = anchor;
    }
    void setROIOffset(const utils::Point &offs){
      m_oROIOffset = offs;
    }
    public:
    const utils::Size &getMaskSize() const{
      return m_oMaskSize;
    }
    const utils::Point &getAnchor() const {
      return m_oAnchor;
    }
    const utils::Point &getROIOffset() const{
      return m_oROIOffset;
    }
    protected:

    // Bring in UnaryOp's Image-based prepare (otherwise hidden by legacy overrides below)
    using UnaryOp::prepare;

    /// Legacy prepare: ensure compatible image format and size
    bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc) override;

    /// Legacy prepare: as above, but with depth parameter
    bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc, core::depth eDepht) override;

    /// this function can be reimplemented e.g to enshure an odd mask width and height
     /** E.g. some implementations of Neighborhood-operation could demand odd or even
         mask size parameters. In this case, this function can be implemented in another
         way. (Example: MedianOp)
         @param size size to ajust
         @return the given size in this base implementation
         **/
    virtual utils::Size adaptSize(const utils::Size &size){ return size; }

    protected:
    utils::Size  m_oMaskSize;  ///< size of filter mask
    utils::Point m_oAnchor;    ///< anchor of filter mask
    mutable utils::Point m_oROIOffset; ///< to-be-used ROI offset for source image (mutable: set by getDestinationParams)
  };
  } // namespace icl::filter