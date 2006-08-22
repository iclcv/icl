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

      Here we distinguish two variants: Either the destination images size
      is reduced to the source images ROI, or its size is adapted to the
      full size of the source image. Which method is used is indicated by the
      flag bClipToROI.

      Most filter operations operate on a certain subregion of the
      source ROI (a mask), e.g. a 3x3 convolution mask works on 3x3 
      subregion of the image. Applying these masks to the border of the image
      (in case of full-image ROI) leads to problems, because the filter
      would access undefined pixel values outside the image, actually
      causing a segfault typically.

      Hence, the used ROI size of the source image is <em> shrinked </em> if
      neccessary, such that the filter mask always fits into the image.  For
      this purpose the method adaptROI is provided, which computes the
      to-be-used ROI. The ROI of the source image is not actually changed,
      which makes the filter operation thread safe, because simultaneous
      operations running on the source image within different threads do not
      interfere.
  
      The actual filter operation is performed by the virtual method void
      apply(ImgI* poSrc, ImgI** poDst). The ImgI** is neccessary, because
      poDst may be changed in depth, which actually allocates a new image
      instance.

  */
  class Filter{
    public:
    virtual ~Filter() {};
    /// Change handling of border around ROI of source image
    /** Either the destination images size can be reduced to the source
        images ROI (where the filter is applied only) or the source
        images size can be kept for the destination, where the border
        around the ROI is left unchanged (with possibly undefined values).
        While the first case is the default, the latter case requires some
        subsequent operation to fill the border with useful values, e.g.
        fixed (black) pixels or extending the out ROI rim into the border.
    */
    void setClipToROI (bool bClip) { this->bClipToROI = bClip; }

    /// Applies the individual filter operation on the source image
    /** @param poSrc source image
        @param  poDst destination image
    */
    virtual void apply(ImgI *poSrc, ImgI **ppoDst) = 0;

    protected:
    Filter() : oMaskSize(1,1), oAnchor (0,0), bClipToROI(true) {}
    Filter(const Size &size) : bClipToROI(true) {
       setMask (size);
    }

    void setMask (const Size &size) {
       oMaskSize = size;
       oAnchor   = Point (size.width/2, size.height/2);
    }

    /// prepare filter operation: ensure compatible image format and size
    bool prepare (ImgI *poSrc, ImgI **ppoDst);
    
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
    bool adaptROI(ImgI *poSrc, Point& oROIoffset, Size& oROIsize);

    protected:
    Size  oMaskSize;  //< size of filter mask
    Point oAnchor;    //< anchor of filter mask
    Point oROIoffset; //< to-be-used ROI offset for source image
    bool  bClipToROI; //< reduce destination image's size to source ROI
  };
}
#endif
