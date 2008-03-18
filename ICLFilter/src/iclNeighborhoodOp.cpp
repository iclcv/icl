#include <iclNeighborhoodOp.h>
#include <iclUnaryOpWork.h>
#include <iclMacros.h>
#include <iclImageSplitter.h>

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
    // NEW Code
    Rect imageRect(Point::null,poSrc->getSize());
    Rect imageROI = poSrc->getROI();
    
    Rect newROI = imageROI & (imageRect+m_oAnchor-m_oMaskSize+Size(1,1));
    oROIoffset = newROI.ul();
    oROIsize = newROI.size();
#ifdef WITH_IPP_OPTIMIZATION // workaround for IPP bug (anchor not correctly handled)
    if (m_oMaskSize.width % 2 == 0) oROIsize.width--;
    if (m_oMaskSize.height % 2 == 0) oROIsize.height--;
#endif
    return !!oROIsize.getDim();
    // TODO
    
    /* OLD Code
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
    */
  }

  void NeighborhoodOp::applyMT(const ImgBase *poSrc, ImgBase **ppoDst, unsigned int nThreads){
    ICLASSERT_RETURN( nThreads > 0 );
    ICLASSERT_RETURN( poSrc );
    if(nThreads == 1){
      apply(poSrc,ppoDst);
      return;
    }
    if(!prepare (ppoDst, poSrc)) return;
  
    bool ctr = getClipToROI();
    bool co = getCheckOnly();
    
    setClipToROI(false);
    setCheckOnly(true);
    
    const ImgBase *srcROIAdapted = poSrc->shallowCopy(Rect(getROIOffset(),(*ppoDst)->getROISize()));
    const std::vector<ImgBase*> srcs = ImageSplitter::split(srcROIAdapted,nThreads);
    std::vector<ImgBase*> dsts = ImageSplitter::split(*ppoDst,nThreads);
    delete srcROIAdapted;
    
    MultiThreader::WorkSet works(nThreads);
    
    for(unsigned int i=0;i<nThreads;i++){
      works[i] = new UnaryOpWork(this,srcs[i],const_cast<ImgBase*>(dsts[i]));
    }
    
    if(!m_poMT){
      m_poMT = new MultiThreader(nThreads);
    }else{
      if(m_poMT->getNumThreads() != (int)nThreads){
        delete m_poMT;
        m_poMT = new MultiThreader(nThreads);
      }
    }
    
    (*m_poMT)( works );
    
    for(unsigned int i=0;i<nThreads;i++){
      delete works[i];
    }

    setClipToROI(ctr);
    setCheckOnly(co);

    ImageSplitter::release(srcs);
    ImageSplitter::release(dsts);

  }
}
