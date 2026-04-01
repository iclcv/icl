// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/NeighborhoodOp.h>
#include <ICLCore/Image.h>
#include <ICLUtils/Macros.h>
#include <ICLFilter/ImageSplitter.h>
#include <future>
#include <vector>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    bool NeighborhoodOp::prepare(Image &dst, const Image &src) {
      return prepare(dst, src, src.getDepth());
    }

    bool NeighborhoodOp::prepare(Image &dst, const Image &src, depth d) {
      Size oROIsize;
      if (!computeROI(src.ptr(), m_oROIOffset, oROIsize)) return false;
      return UnaryOp::prepare(dst, d,
                              getClipToROI() ? oROIsize : src.getSize(),
                              src.getFormat(), src.getChannels(),
                              Rect(getClipToROI() ? Point::null : m_oROIOffset, oROIsize),
                              src.getTime());
    }

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


      // Shrink ROI by 1 pixel for even-sized masks (workaround for anchor handling)
      if (m_oMaskSize.width % 2 == 0) oROIsize.width--;
      if (m_oMaskSize.height % 2 == 0) oROIsize.height--;
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
          #ifdef ICL_HAVE_IPP // workaround for IPP bug (anchor not correctly handled)
          if (m_oMaskSize.width % 2 == 0) oROIsize.width--;
          #endif
          }
          if (oROIsize.height > (a=oSize.height - (oROIoffset.y + m_oMaskSize.height - m_oAnchor.y - 1))) {
          oROIsize.height = a;
          #ifdef ICL_HAVE_IPP // workaround for IPP bug (anchor not correctly handled)
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



      std::vector<std::future<void>> futures;
      for(unsigned int i=0;i<nThreads;i++){
        ImgBase *s = srcs[i];
        ImgBase *d = const_cast<ImgBase*>(dsts[i]);
        futures.push_back(std::async(std::launch::async, [this,s,&d]{ apply(s, &d); }));
      }
      for(auto &f : futures) f.get();

      setClipToROI(ctr);
      setCheckOnly(co);

      ImageSplitter::release(srcs);
      ImageSplitter::release(dsts);

    }
  } // namespace filter
}
