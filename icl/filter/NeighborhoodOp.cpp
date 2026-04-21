// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/NeighborhoodOp.h>
#include <icl/core/Image.h>
#include <icl/utils/Macros.h>
#include <icl/filter/ImageSplitter.h>
#include <future>
#include <vector>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  std::pair<depth, ImgParams>
  NeighborhoodOp::getDestinationParams(const Image &src) const {
    Rect imageRect(Point::null, src.getSize());
    Rect shrunkROI = src.getROI() & (imageRect + m_oAnchor - m_oMaskSize + Size(1,1));

    m_oROIOffset = shrunkROI.ul();  // cached for apply implementations (mutable)

    Size dstSize = getClipToROI() ? shrunkROI.getSize() : src.getSize();
    Rect dstROI(getClipToROI() ? Point::null : shrunkROI.ul(), shrunkROI.getSize());
    return { src.getDepth(), ImgParams(dstSize, src.getChannels(), src.getFormat(), dstROI) };
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
    Rect imageRect(Point::null,poSrc->getSize());
    Rect imageROI = poSrc->getROI();

    Rect newROI = imageROI & (imageRect+m_oAnchor-m_oMaskSize+Size(1,1));
    oROIoffset = newROI.ul();
    oROIsize = newROI.getSize();

    return !!oROIsize.getDim();
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
  } // namespace icl::filter