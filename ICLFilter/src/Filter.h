#ifndef ICL_FILTER_H
#define ICL_FILTER_H

#include <ImgI.h>

namespace icl {
  /// Abstract class interface for filter classes
  /** The Filter class builds a base class for all ICL filter
      operations. Each filter operation is performed on the ROI
      of the source image only. The destination image is <em>always</em>
      adapted in its depth, number of channels and size to the 
      appropriate values of the source image.
      Most filter operations operate on a certain subregion of the
      source ROI (a mask), e.g. a 3x3 convolution mask works on 3x3 
      subregion of the image. Applying these masks to the border of the image
      (in case of full-image ROI) leads to problems, because the filter
      would access undefined pixel values outside the image, actually
      causing a segfault typically.
      Hence, the used ROI size of the source image is <em> shrinked </em> 
      if neccessary, such that the filter mask always fits into the image.
      For this purpose the method adaptROI is provided, which computes
      the to-be-used ROI directly sets it for the destination image. The
      ROI of the source image is not actually changed, which makes the
      filter operation thread safe, because simultaneous operations running
      on the source image within different threads do not interfere.
  
      The actual filter operation is performed by the virtual method void
      apply(ImgI* poSrc, ImgI** poDst). The ImgI** is neccessary, because
      poDst may be changed in depth, which actually allocates a new image
      instance.

  */
  class Filter{
    public:
    virtual ~Filter() {};
    
    /// Applies the individual filter operation on the source image
    /** @param poSrc source image
        @param  poDst destination image
    */
    virtual void apply(ImgI *poSrc, ImgI **ppoDst) = 0;

    protected:
    Filter() : oMaskSize(1,1), oAnchor (0,0) {}
    Filter(const Size &size) {
       setMask (size);
    }

    void setMask (const Size &size) {
       oMaskSize = size;
       oAnchor   = Point (size.width/2, size.height/2);
    }

    /// prepare filter operation: ensure compatible image format and size
    bool prepare (ImgI *poSrc, ImgI **ppoDst) {
       ensureCompatible (ppoDst, poSrc);
       return adaptROI (poSrc, *ppoDst);
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
    bool adaptROI(ImgI *poSrc, ImgI *poDst);

    protected:
    Size  oMaskSize; //< size of filter mask
    Point oAnchor;   //< anchor of filter mask
  };
}
#endif
