#include "ICLFilter.h"

namespace icl{

  bool ICLFilter::adaptROI(ICLBase *poSrc, ICLBase *poDst)
  {
    FUNCTION_LOG("");

    poSrc->getROI (oROIoffset, oROIsize);
    int a;
    if (oROIoffset.x < oAnchor.x) oROIoffset.x = oAnchor.x;
    if (oROIoffset.y < oAnchor.y) oROIoffset.y = oAnchor.y;
    if (oROIsize.width > (a=poSrc->getWidth() - (oROIoffset.x + oMaskSize.width - oAnchor.x - 1)))
       oROIsize.width = a;
    if (oROIsize.height > (a=poSrc->getHeight() - (oROIoffset.y + oMaskSize.height - oAnchor.y - 1)))
       oROIsize.height = a;

    if (oROIsize.width < 1 || oROIsize.height < 1) return false;
    
    poDst->setROI (oROIoffset, oROIsize);
    return true;
  }
}
