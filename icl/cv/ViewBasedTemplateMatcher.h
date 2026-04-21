// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/cv/CV.h>
#include <icl/utils/UncopiedInstance.h>

namespace icl::cv {
  /// TemplateMatching class (wrapping UsefulFunctions::matchTemplate)
  class ICLCV_API ViewBasedTemplateMatcher{
    public:

    /// internally use matching mode
    enum mode{
      sqrtDistance,     ///<  use square distance proximity measurement
      crossCorrelation, ///<  use normalized cross correlation proximity measurement
    };

    /// Create a new ViewBasedTemplateMatcher instance with given matching significance and mode
    /** @param significance significance level for matching
                            appropriate range depends on matching mode
        @param m matching mode:
                    - sqrtDistance (better mode for good matching) appropriate range; [0.5,...]
                    - crossCorrelation appropriate range: [0.92, ...]
        @param clipBuffersToROI if set, internally all buffers are clipped to ROI. This might be usefull,
                                if given templates and source images do change size in successive calls
                                otherwise, it's more efficient, to use buffers of constant size and to
                                adapt the bufers ROI only.
   */
    ViewBasedTemplateMatcher(float significance=0.9, mode m=sqrtDistance, bool clipBuffersToROI = false);

    /// set significance level
    /** @param significance significance level (apropriate range depends on matching mode
        @see class constructor for more detail*/
    void setSignificance(float significance);

    /// set matching mode (see constructor description)
    void setMode(mode m);

    /// set buffer clipping mode (see constructor description)
    void setClipBuffersToROI(bool flag);

    /// apply matching with given image and template (optionally image and template masks can be given)
    const std::vector<utils::Rect> &match(const core::Img8u &image, const core::Img8u &templ, const core::Img8u &imageMask=core::Img8u::null, const core::Img8u &templMask=core::Img8u::null);

    /// returns the interanly used binary buffer buffer
    const core::Img8u getBuffer() { return p2o(m_aoBuffers[2].selectChannel(0)); }

    private:
    float m_fSignificance;          ///< significance level
    mode m_eMode;                   ///< matching mode
    bool m_bClipBuffersToROI;       ///< buffer clipping mode
    utils::UncopiedInstance<RegionDetector> m_oRD;           ///< internally recycled RegionDetector instance
    utils::UncopiedInstance<core::Img8u> m_aoBuffers[3];           ///< interanlly used buffers
    std::vector<utils::Rect> m_vecResults; ///< internal result buffer
  };

  } // namespace icl::cv