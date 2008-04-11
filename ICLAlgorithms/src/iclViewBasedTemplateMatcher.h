#ifndef ICL_VIEW_BASED_TEMPLATE_MATCHER_H
#define ICL_VIEW_BASED_TEMPLATE_MATCHER_H

#include "iclUsefulFunctions.h"

namespace icl{
  
  /// TemplateMatching class (wrapping UsefulFunctions::matchTemplate)
  class ViewBasedTemplateMatcher{
    public:
    
    /// internally use matching mode
    enum mode{
      sqrtDistance,     ///<  use square distance proximity measurement
      crossCorrelation, ///<  use normalized cross correlation proximity measurement
    };
    
    /// Create a new ViewBasedTemplateMatcher instance with given matching significance and mode
    /** @param significance significance level for matching
                            appropriate range depends on matching mode
        @param mode matching mode:
                    - sqrtDistance (better mode for good matching) appropriate range; [0.5,...]
                    - crossCorrelation appropriate range: [0.92, ...]
        @param clipBuffersToROI if set, internally all buffers are clipped to ROI. This might be usefull,
                                if given templates and source images do change size in successive calls
                                otherwise, it's more efficient, to use buffers of constant size and to
                                adapt the bufers ROI only.
   */
    ViewBasedTemplateMatcher(float significance=0.9, mode m=sqrtDistance, bool clipBuffersToROI = false);
    
    /// set significance level
    /** @params significance significance level (apropriate range depends on matching mode
        @see class constructor for more detail*/
    void setSignificance(float significance);
    
    /// set matching mode (see constructor description)
    void setMode(mode m);
    
    /// set buffer clipping mode (see constructor description)
    void setClipBuffersToROI(bool flag);
    
    /// apply matching with given image and template (optionally image and template masks can be given)
    const std::vector<Rect> &match(const Img8u &image, const Img8u &templ, const Img8u &imageMask=Img8u::null, const Img8u &templMask=Img8u::null);
    
    /// returns the interanly used binary buffer buffer
    const Img8u getBuffer() { return p2o(m_aoBuffers[2].selectChannel(0)); }
    private:
    float m_fSignificance;          ///< significance level
    mode m_eMode;                   ///< matching mode
    bool m_bClipBuffersToROI;       ///< buffer clipping mode
    RegionDetector m_oRD;           ///< internally recycled RegionDetector instance
    Img8u m_aoBuffers[3];           ///< interanlly used buffers
    std::vector<Rect> m_vecResults; ///< internal result buffer
  };
  
}

#endif
