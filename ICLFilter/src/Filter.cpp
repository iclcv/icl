#include "Filter.h"

namespace icl{

  bool Filter::adaptROI(ImgI *poSrc, ImgI *poDst)
  {
    FUNCTION_LOG("");

    const Size& oSize = poSrc->getSize ();
    Size  oROIsize;   //< to-be-used ROI size
    Point oROIoffset; //< to-be-used ROI offset

    poSrc->getROI (oROIoffset, oROIsize);
    int a;
    if (oROIoffset.x < oAnchor.x) oROIoffset.x = oAnchor.x;
    if (oROIoffset.y < oAnchor.y) oROIoffset.y = oAnchor.y;
    if (oROIsize.width > (a=oSize.width - (oROIoffset.x + oMaskSize.width - oAnchor.x - 1)))
       oROIsize.width = a;
    if (oROIsize.height > (a=oSize.height - (oROIoffset.y + oMaskSize.height - oAnchor.y - 1)))
       oROIsize.height = a;

    if (oROIsize.width < 1 || oROIsize.height < 1) return false;
    
    poDst->setROI (oROIoffset, oROIsize);
    return true;
  }
}
