#include "Filter.h"

namespace icl{

  bool Filter::prepare (ImgI *poSrc, ImgI **ppoDst) {
     FUNCTION_LOG("");

     Size  oROIsize;   //< to-be-used ROI size
     if (!adaptROI (poSrc, oROIoffset, oROIsize)) return false;

     if (bClipToROI) {
        ensureCompatible (ppoDst, poSrc->getDepth(), oROIsize, 
                          poSrc->getFormat(), poSrc->getChannels(),
                          Rect (Point::zero, oROIsize));
     } else {
        ensureCompatible (ppoDst, poSrc);
        (*ppoDst)->setROI (oROIoffset, oROIsize);
     }
     return true;
  }

  bool Filter::adaptROI(ImgI *poSrc, Point& oROIoffset, Size& oROIsize)
  {
    FUNCTION_LOG("");

    const Size& oSize = poSrc->getSize ();
    poSrc->getROI (oROIoffset, oROIsize);
    int a;

    if (oROIoffset.x < oAnchor.x) oROIoffset.x = oAnchor.x;
    if (oROIoffset.y < oAnchor.y) oROIoffset.y = oAnchor.y;
    if (oROIsize.width > (a=oSize.width - (oROIoffset.x + oMaskSize.width - oAnchor.x - 1))) {
       oROIsize.width = a;
#ifdef WITH_IPP_OPTIMIZATION // workaround for IPP bug (anchor not correctly handled)
       if (oMaskSize.width % 2 == 0) oROIsize.width--;
#endif
    }
    if (oROIsize.height > (a=oSize.height - (oROIoffset.y + oMaskSize.height - oAnchor.y - 1))) {
       oROIsize.height = a;
#ifdef WITH_IPP_OPTIMIZATION // workaround for IPP bug (anchor not correctly handled)
       if (oMaskSize.height % 2 == 0) oROIsize.height--;
#endif
    }
    if (oROIsize.width < 1 || oROIsize.height < 1) return false;
    return true;
  }
}
