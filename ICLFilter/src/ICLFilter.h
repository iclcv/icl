#ifndef ICL_FILTER_H
#define ICL_FILTER_H

#include <ICLBase.h>

namespace icl {
  /// Abstract class interface for filter classes
  /** The ICLFilter class builds a base class for all ICL filter
      operations. Each filter operation is performed on the ROI
      of the source image only. The destination image is <em>always</em>
      adapted in its depth, number of channels and size to the 
      appropriate values of the source image.
      Most filter operations operate on a certain subregion of the
      source ROI (a mask), e.g. a 3x3 convolution mask works on 3x3 
      subregion of the image. Applying these masks to the border of the image
      (in case of full-image ROI) leads to problems, because the filter
      would access undefined pixel values outside the image. Actually,
      IPP would crash in this case.
      Hence, the used ROI size of the source image is <em> shrinked </em> 
      if neccessary, such that the filter mask always fits into the image.
      For this purpose the method adaptROI is provided, which computes
      the to-be-used ROI and stores it in oROIsize and oROIoffset. The
      ROI of the source image is not actually changed, which make the
      filter operation thread safe.
  
      The actual filter operation is performed by the virtual method
      ICLBase* apply(ICLBase* poSrc, ICLBase* poDst). The return value
      is neccessary, because poDst may be changed in depth, which actually
      allocated a new image instance, which is returned.

      <h3>Currently implemented Filters</h3>
      - ICLConvolution 
      - ICLMedian (in work!)
      - ICLMorphologicalOps (in work!)
  */
  class ICLFilter{
    public:
    virtual ~ICLFilter() {};
    
    /// Applies the individual filter operation on the source image
    /** @param poSrc source image
    @param  poDst destination image
    @return the destination image (eventually converted in depth)
    */
    virtual ICLBase* apply(ICLBase *poSrc, ICLBase *poDst) = 0;

    protected:
    ICLFilter(int iWidth=1, int iHeight=1) {
       setMask (iWidth, iHeight);
    }

    void setMask (int iWidth, int iHeight) {
       oMaskSize.width = iWidth;
       oMaskSize.height = iHeight;
       oAnchor.x = iWidth/2;
       oAnchor.y = iHeight/2;       
    }

    /// prepare filter operation: ensure compatible image format and size
    ICLBase* prepare (ICLBase *poSrc, ICLBase *poDst) {
       iclEnsureCompatible (&poDst, poSrc);
       return poDst;
    }
    
    /// shrink source image ROI if neccessary
    /** This functions adapts the to-be-used ROI of the source image, 
        such that the filter mask of given size (oMaskSize) fits everywhere
        into the image if placed arbitrarily within the ROI.
        The original ROI of the source image is not changed. 
        The used ROI is stored in oROIsize and oROIoffset.
    @param poSrc image whose ROI is adapted
    @param poDst image whose ROI is actually changed
    @return whether a valid ROI remains
    */
    bool adaptROI(ICLBase *poSrc, ICLBase *poDst);

    protected:
    ICLsize  oMaskSize; //< size of filter mask
    ICLpoint oAnchor;   //< anchor of filter mask
    ICLsize  oROIsize;  //< used ROI size
    ICLpoint oROIoffset; //< used ROI offset
  };
}
#endif
