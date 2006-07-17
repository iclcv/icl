#ifndef ICL_FILTER_H
#define ICL_FILTER_H

#include <ICLBase.h>

namespace icl {
  /// virtaul class interface for filter classes
  /** The ICLFilter class builds a virtual interface
  for the ICLFilter folder.
  The only commonality for contained filter classes
  is the <b>apply</b>-method, wich performs the filter
  operation on the given source image and stores the 
  result into given destination image.
  
  <h3>Currently implemented Filters</h3>
  - ICLConvolution 
  - ICLMedian (in work!)
  - ICLMorphologicalOps (in work!)
  */
  class ICLFilter{
    public:
    virtual ~ICLFilter(){};
    
    /// Applies the individula filter operation on the source image
    /** @param poSrc source image
    @param poDst destination image
    */
    virtual void apply(ICLBase *poSrc, ICLBase *poDst)=0;
    
    protected:
    /// erodes or dilates an images roi
    /** The morphRoi function can be used by filters, that operate
    on an images ROI. E.g. the convolution filter needs to 
    ensure, that:
    <pre>
    source_width-filter_width/2 = destination_width
    source_height-filter_height/2 = destination_height
    </pre>
    Normally, the size of the destination image must be adapted
    by using morphRoi.
    @param poImage "owner" image of the ROI
    @param iHorz amount of pixels, that shoud be added left and righht
    to the ROI-rect (if negative, the pixels are removed)
    @param iVert amount of pixels, that shoud be added on the top and
    on the bottom to the ROI-rect 
    (if negative, the pixels are removed)
    */
    void morphROI(ICLBase *poImage, int iHorz, int iVert);
    
  };
}
#endif
