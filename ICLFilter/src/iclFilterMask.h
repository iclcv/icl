#ifndef ICL_FILTERMASK_H
#define ICL_FILTERMASK_H

#include <iclFilter.h>

namespace icl {
  /// Abstract class interface for filter classes which use moving masks
  /** The Filter class builds a base class for filter operations employing
      a filter mask which is moved over the ROI of the source image(s), 
      e.g. convolution filters use some convolution masks.
      To this end the class provides members to store the size and anchor
      of the filter mask.

      Special care has to be taken, when applying a filter mask to the border
      of an image, e.g. in case of full-image ROI. In this case the filter
      might access undefined pixel values outside the image, actually causing
      a segfault in most cases.

      Hence, the used ROI size of the source image is <em> shrinked </em> if
      neccessary, such that the filter mask always fits into the image, when
      moved over the ROI. For this purpose the method adaptROI is provided,
      which computes the to-be-used ROI. The ROI of the source image is not
      actually changed, which makes the filter operation thread safe, because
      simultaneous operations running on the source image within different
      threads do not interfere.
  */
  class FilterMask : public Filter {
  protected:
     FilterMask() : oMaskSize(1,1), oAnchor (0,0) {}
     FilterMask(const Size &size) {
        setMask (size);
     }

     void setMask (const Size &size) {
        oMaskSize = size;
        oAnchor   = Point (size.width/2, size.height/2);
     }

     /// prepare filter operation: ensure compatible image format and size
     bool prepare (ImgBase **ppoDst, const ImgBase *poSrc);
    
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
     bool computeROI(const ImgBase *poSrc, Point& oROIoffset, Size& oROIsize);
     
  protected:
     Size  oMaskSize;  //< size of filter mask
     Point oAnchor;    //< anchor of filter mask
     Point oROIoffset; //< to-be-used ROI offset for source image
  };
}
#endif
