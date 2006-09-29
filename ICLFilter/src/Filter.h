#ifndef ICL_FILTER_H
#define ICL_FILTER_H

#include <ImgI.h>

namespace icl {
  /// Abstract class interface for filter operations
  /** The Filter class builds a base class for all ICL filter operations. Each
      filter operation is performed on the ROI of the source image only. The
      destination image is <em> always </em> adapted in its parameters to the
      necessary values. We distinguish the following modes:

      - Adapt the destination image in its size and ROI to the source
        image. Hence the destination will have the same size and ROI as the
        source image. Nevertheless the filter operates on the ROI only, 
        leaving the previous content of the destination image unchanged
        in this border region. Hence this mode requires subsequent handling
        of the border.
      - Adapt the destination images size, such that is exactly comprises
        the ROI of the source image. (bClipToROI = true)
      - Only check the destination images parameters and issue a warning
        if not set correctly already. In this case the filter operation itself
        should not be performed. (bCheckOnly = false)

      To this end the Filter class provides variables bCheck and bClipToROI as
      well as several version of prepare () methods which check and adapt the
      destination image if neccessary.
  */
  class Filter {
  public:
     /// change adaption of destination image (see class description)
     void setClipToROI (bool bClipToROI) {this->bClipToROI = bClipToROI;}
     void setCheckOnly (bool bCheckOnly) {this->bCheckOnly = bCheckOnly;}

  protected:
     /// Filter is a base class for other classes and should be instantiated
     Filter() : bClipToROI (true), bCheckOnly (false) {}
     ~Filter() {};

     /// check+adapt destination images parameters against given values
     /// bCheckOnly mode ignores the given imgSize
     bool prepare (ImgI **ppoDst, depth eDepth, const Size &imgSize, 
                   format eFormat, int nChannels, const Rect& roi);
     /// adapt destination image to properties of given source image
     bool prepare (ImgI **ppoDst, const ImgI *poSrc);

     bool bClipToROI, bCheckOnly;
  };
}
#endif
