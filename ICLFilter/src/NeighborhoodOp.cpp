/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/src/NeighborhoodOp.cpp                       **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLFilter/NeighborhoodOp.h>
#include <ICLFilter/UnaryOpWork.h>
#include <ICLUtils/Macros.h>
#include <ICLFilter/ImageSplitter.h>

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
    oROIsize = newROI.getSize();


#ifdef HAVE_IPP // workaround for IPP bug (anchor not correctly handled)
    if (m_oMaskSize.width % 2 == 0) oROIsize.width--;
    if (m_oMaskSize.height % 2 == 0) oROIsize.height--;
#endif
    //    ERROR_LOG("params: x:" << oROIoffset.x << ",y:" << oROIoffset.y << ",w:" << oROIsize.width << ",h:" << oROIsize.height);
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
        #ifdef HAVE_IPP // workaround for IPP bug (anchor not correctly handled)
        if (m_oMaskSize.width % 2 == 0) oROIsize.width--;
        #endif
        }
        if (oROIsize.height > (a=oSize.height - (oROIoffset.y + m_oMaskSize.height - m_oAnchor.y - 1))) {
        oROIsize.height = a;
        #ifdef HAVE_IPP // workaround for IPP bug (anchor not correctly handled)
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

    /*
        DEBUG_LOG("src image:" << *poSrc);
        for(int i=0;i<srcs.size();++i){
        DEBUG_LOG("part " << i << " roi:" << srcs[i]->getROI());
        }
    */
    


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
