/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLAlgorithms/ViewBasedTemplateMatcher.h       **
** Module : ICLAlgorithms                                          **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_VIEW_BASED_TEMPLATE_MATCHER_H
#define ICL_VIEW_BASED_TEMPLATE_MATCHER_H

#include <ICLAlgorithms/UsefulFunctions.h>
#include <ICLUtils/UncopiedInstance.h>

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
    UncopiedInstance<Img8u> m_aoBuffers[3];           ///< interanlly used buffers
    std::vector<Rect> m_vecResults; ///< internal result buffer
  };
  
}

#endif
