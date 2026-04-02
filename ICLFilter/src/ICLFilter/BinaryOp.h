// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Image.h>
#include <ICLFilter/OpROIHandler.h>

namespace icl::filter {
  /// Abstract base class for binary image operations \ingroup BINARY
  /** A list of all binary operators can be found here: \n
      \ref BINARY
  **/
  class ICLFilter_API BinaryOp{
    public:

    /// default constructor
    BinaryOp();

    /// copy constructor
    BinaryOp(const BinaryOp &other);

    /// assignment operator
    BinaryOp &operator=(const BinaryOp &other);

    /// virtual destructor
    virtual ~BinaryOp();


    /// Primary apply method — all subclasses must implement this
    virtual void apply(const core::Image &src1, const core::Image &src2, core::Image &dst) = 0;

    /// Legacy ImgBase** apply — final bridge to Image-based apply
    virtual void apply(const core::ImgBase *operand1, const core::ImgBase *operand2,
                       core::ImgBase **result) final;

    /// Single-arg apply returning Image (uses internal buffer)
    core::Image apply(const core::ImgBase *operand1, const core::ImgBase *operand2);

    /// Single-arg Image apply
    inline core::Image apply(const core::Image &src1, const core::Image &src2){
      core::Image dst;
      apply(src1, src2, dst);
      return dst;
    }

    /// function operator (legacy)
    inline void operator()(const core::ImgBase *src1, const core::ImgBase *src2, core::ImgBase **dst){
      apply(src1, src2, dst);
    }

    /// function operator returning Image
    inline core::Image operator()(const core::Image &src1, const core::Image &src2){
      return apply(src1, src2);
    }

    /// sets if the image should be clip to ROI or not
    /**
      @param bClipToROI true=yes, false=no
    */
    void setClipToROI (bool bClipToROI) { m_oROIHandler.setClipToROI(bClipToROI); }

    /// sets if the destination image should be adapted to the source, or if it is only checked if it can be adapted.
    /**
      @param bCheckOnly true = destination image is only checked, false = destination image will be checked and adapted.
    */
    void setCheckOnly (bool bCheckOnly) { m_oROIHandler.setCheckOnly(bCheckOnly); }

    /// returns the ClipToROI status
    /**
      @return true=ClipToROI is enable, false=ClipToROI is disabled
    */
    bool getClipToROI() const { return m_oROIHandler.getClipToROI(); }

    /// returns the CheckOnly status
    /**
      @return true=CheckOnly is enable, false=CheckOnly is disabled
    */
    bool getCheckOnly() const { return m_oROIHandler.getCheckOnly(); }

    protected:

    // ---- Image-based prepare ----

    /// Prepare dst to match src1's params (same depth)
    bool prepare(core::Image &dst, const core::Image &src1);

    /// Prepare dst to match src1's params but with explicit depth
    bool prepare(core::Image &dst, const core::Image &src1, core::depth d);

    /// Prepare dst with fully explicit params
    bool prepare(core::Image &dst, core::depth d, const utils::Size &s,
                 core::format fmt, int ch, const utils::Rect &roi,
                 utils::Time t = utils::Time::null);

    /// Check that two source images are compatible (same channels, ROI size, optionally depth)
    static inline bool check(const core::Image &a, const core::Image &b, bool checkDepths = true) {
      if(!checkDepths) {
        return a.getChannels() == b.getChannels() && a.getROISize() == b.getROISize();
      } else {
        return a.getChannels() == b.getChannels() && a.getROISize() == b.getROISize()
            && a.getDepth() == b.getDepth();
      }
    }

    // ---- Legacy ImgBase** prepare (for internal use) ----

    bool prepare (core::ImgBase **ppoDst, core::depth eDepth, const utils::Size &imgSize,
                  core::format eFormat, int nChannels, const utils::Rect& roi,
                  utils::Time timestamp=utils::Time::null){
      return m_oROIHandler.prepare(ppoDst, eDepth,imgSize,eFormat, nChannels, roi, timestamp);
    }

    virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc) {
      return m_oROIHandler.prepare(ppoDst, poSrc);
    }

    virtual bool prepare (core::ImgBase **ppoDst,
                          const core::ImgBase *poSrc,
                          core::depth eDepth) {
      return m_oROIHandler.prepare(ppoDst, poSrc, eDepth);
    }

    static inline bool check(const core::ImgBase *operand1,
                             const core::ImgBase *operand2 ,
                             bool checkDepths = true) {
      if(!checkDepths) {
        return operand1->getChannels() == operand2->getChannels() &&
          operand1->getROISize() == operand2->getROISize() ;
      } else {
        return operand1->getChannels() == operand2->getChannels() &&
          operand1->getROISize() == operand2->getROISize() &&
          operand1->getDepth() == operand2->getDepth() ;
      }
    }

  private:
    OpROIHandler m_oROIHandler;

    /// internal image buffer which is used for the apply function without destination image argument
    core::ImgBase *m_buf;
  };
  } // namespace icl::filter