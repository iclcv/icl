#include <FilterMask.h>

namespace icl {

   bool FilterMask::prepare (ImgBase **ppoDst, const ImgBase *poSrc) {
      Size oROIsize;   //< to-be-used ROI size
      if (!computeROI (poSrc, oROIoffset, oROIsize)) return false;

      return Filter::prepare (ppoDst, poSrc->getDepth(), 
                              bClipToROI ? oROIsize : poSrc->getSize(),
                              poSrc->getFormat(), poSrc->getChannels (), 
                              Rect (bClipToROI ? Point::null : oROIoffset, oROIsize),
                              poSrc->getTime());
   }

   bool FilterMask::computeROI(const ImgBase *poSrc, Point& oROIoffset, Size& oROIsize)
   {
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
