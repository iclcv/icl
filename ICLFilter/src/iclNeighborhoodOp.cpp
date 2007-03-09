#include <iclNeighborhoodOp.h>

namespace icl {

  bool NeighborhoodOp::prepare (ImgBase **ppoDst, const ImgBase *poSrc) {
    return prepare(ppoDst,poSrc,poSrc->getDepth());
  }
  bool NeighborhoodOp::prepare (ImgBase **ppoDst, const ImgBase *poSrc, depth eDepth) {
    Size oROIsize;   //< to-be-used ROI size
    if (!computeROI (poSrc, m_oROIOffset, oROIsize)) return false;
    
    return UnaryOp::prepare (ppoDst, eDepth, 
                    getClipToROI() ? oROIsize : poSrc->getSize(),
                    poSrc->getFormat(), poSrc->getChannels (), 
                    Rect (getClipToROI() ? Point::null : m_oROIOffset, oROIsize),
                    poSrc->getTime());
  }
 
  bool NeighborhoodOp::computeROI(const ImgBase *poSrc, Point& oROIoffset, Size& oROIsize) {
    const Size& oSize = poSrc->getSize ();
    poSrc->getROI (oROIoffset, oROIsize);
    int a(0);
    
    if (oROIoffset.x < m_oAnchor.x) oROIoffset.x = m_oAnchor.x;
    if (oROIoffset.y < m_oAnchor.y) oROIoffset.y = m_oAnchor.y;
    if (oROIsize.width > (a=oSize.width - (oROIoffset.x + m_oMaskSize.width - m_oAnchor.x - 1))) {
      oROIsize.width = a;
#ifdef WITH_IPP_OPTIMIZATION // workaround for IPP bug (anchor not correctly handled)
      if (m_oMaskSize.width % 2 == 0) oROIsize.width--;
#endif
    }
    if (oROIsize.height > (a=oSize.height - (oROIoffset.y + m_oMaskSize.height - m_oAnchor.y - 1))) {
      oROIsize.height = a;
#ifdef WITH_IPP_OPTIMIZATION // workaround for IPP bug (anchor not correctly handled)
      if (m_oMaskSize.height % 2 == 0) oROIsize.height--;
#endif
    }
    if (oROIsize.width < 1 || oROIsize.height < 1) return false;
    return true;
  }
}
