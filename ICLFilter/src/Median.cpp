#include <Median.h>
#include <Img.h>
#include <ICLCC.h>
#include <ImgIterator.h>
#include <vector>
#include <algorithm>

namespace icl {
  
  // {{{ Constructor / Destructor

  Median::Median (const Size& maskSize) {
     // instantiate fallback methods
#define ICL_INSTANTIATE_DEPTH(T) \
     aMethods[depth ## T]  = &Median::cMedian<icl ## T>; aColorMethods[depth ## T] = 0;
#ifdef WITH_IPP_OPTIMIZATION 
ICL_INSTANTIATE_DEPTH(32s);
ICL_INSTANTIATE_DEPTH(32f);
ICL_INSTANTIATE_DEPTH(64f);
#else
ICL_INSTANTIATE_ALL_DEPTHS;
#endif
#undef ICL_INSTANTIATE_DEPTH

     if (maskSize.width <= 0 || maskSize.height <= 0) {
        ERROR_LOG("illegal width/height: " << maskSize.width << "/" << maskSize.height);
        setMask (Size(3,3));
     } else setMask (maskSize);
  }
   
  // }}}
  
  // {{{ IPP implementation

#ifdef WITH_IPP_OPTIMIZATION 
  template<typename Type, IppStatus (*ippiFunc) (const Type*, int, Type*, int, IppiSize, IppiSize, IppiPoint)>
  void Median::ippMedian(const ImgBase *poSrc, ImgBase *poDst) {
#if __GNUC__ == 3
     const Img<Type> *src = dynamic_cast<const Img<Type>*>(poSrc);
     Img<Type>       *dst = dynamic_cast<Img<Type>*>(poDst);
#else
     const Img<Type> *src = poSrc->asImg<Type>();
     Img<Type>       *dst = poDst->asImg<Type>();
#endif
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset), src->getLineStep(),
                 dst->getROIData (c), dst->getLineStep(), 
                 dst->getROISize(), oMaskSize, oAnchor);
     }
  }

  template<typename Type, IppStatus (*ippiFunc) (const Type*, int, Type*, int, IppiSize, IppiMaskSize)>
  void Median::ippMedianFixed(const ImgBase *poSrc, ImgBase *poDst) {
#if __GNUC__ == 3
     const Img<Type> *src = dynamic_cast<const Img<Type>*>(poSrc);
     Img<Type>       *dst = dynamic_cast<Img<Type>*>(poDst);
#else
     const Img<Type> *src = poSrc->asImg<Type>();
     Img<Type>       *dst = poDst->asImg<Type>();
#endif
     IppiMaskSize mask = oMaskSize.width == 3 ? ippMskSize3x3 : ippMskSize5x5;
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset), src->getLineStep(),
                 dst->getROIData (c), dst->getLineStep(), 
                 dst->getROISize(), mask);
     }
  }
  
  template<>
  void Median::ippMedianColor<icl8u> (const ImgBase *poSrc, ImgBase *poDst) {
    ICLASSERT_RETURN (oMaskSize.width==oMaskSize.height);
    ICLASSERT_RETURN (oMaskSize.width==3 || oMaskSize.width==5);
    IppiMaskSize mask = oMaskSize.width == 3 ? ippMskSize3x3 : ippMskSize5x5;
		int dim=poSrc->getWidth() * poSrc->getHeight();
	  m_oBuffer8u[0].resize(dim*poSrc->getChannels());
	  m_oBuffer8u[1].resize(dim*poSrc->getChannels());
    planarToInterleaved(poSrc->asImg<icl8u>(), *(m_oBuffer8u[0]));
      ippiFilterMedianColor_8u_C3R(*m_oBuffer8u[0], 
                                     poSrc->getLineStep()*poSrc->getChannels(),
                                     *m_oBuffer8u[1], 
                                     poDst->getLineStep()*poSrc->getChannels(), 
                                     poDst->getROISize(), mask);
    interleavedToPlanar(*(m_oBuffer8u[1]),poDst->getSize(),poSrc->getChannels(),poDst->asImg<icl8u>());
  }
  template <>
  void Median::ippMedianColor<icl32f> (const ImgBase *poSrc, ImgBase *poDst) {
    ICLASSERT_RETURN (oMaskSize.width==oMaskSize.height);
    ICLASSERT_RETURN (oMaskSize.width==3 || oMaskSize.width==5);
    IppiMaskSize mask = oMaskSize.width == 3 ? ippMskSize3x3 : ippMskSize5x5;
		int dim=poSrc->getWidth() * poSrc->getHeight();
	  m_oBuffer32f[0].resize(dim*poSrc->getChannels());
	  m_oBuffer32f[1].resize(dim*poSrc->getChannels());
    planarToInterleaved(poSrc->asImg<icl32f>(), *(m_oBuffer32f[0]));
      ippiFilterMedianColor_32f_C3R(*m_oBuffer32f[0], 
                                     poSrc->getLineStep()*poSrc->getChannels(),
                                     *m_oBuffer32f[1], 
                                     poDst->getLineStep()*poSrc->getChannels(), 
                                     poDst->getROISize(), mask);
    interleavedToPlanar(*(m_oBuffer32f[1]),poDst->getSize(),poSrc->getChannels(),poDst->asImg<icl32f>());
  }
  
#endif

  // }}}

  // {{{ Fallback Implementation

  template<typename T>
  void Median::cMedian (const ImgBase *poSrc, ImgBase *poDst) {
     Img<T> *poS = (Img<T>*) poSrc;
     Img<T> *poD = (Img<T>*) poDst;

     std::vector<T> oList(oMaskSize.width * oMaskSize.height);
     typename std::vector<T>::iterator itList = oList.begin();
     typename std::vector<T>::iterator itMedian = oList.begin()+((oMaskSize.width * oMaskSize.height)/2);
     
     for (int c=0;c<poSrc->getChannels();c++)
     {
        for (ImgIterator<T> s (poS->getData(c), poS->getSize().width, 
                               Rect (this->oROIoffset, poD->getROISize())),
                            d=poD->getROIIterator(c); s.inRegion(); ++s, ++d)
        {
           for(ImgIterator<T> sR(s,oMaskSize,oAnchor); sR.inRegion(); ++sR, ++itList)
           {
              *itList = *sR;
           }
           std::sort(oList.begin(),oList.end());
           *d = *itMedian;
           itList = oList.begin();
        }
     }
  }

  // }}}


  // {{{ setMask: set filter size and choose function pointers

  void Median::setMask (Size maskSize) {
     // make maskSize odd:
     maskSize.width  = (maskSize.width/2)*2 + 1;
     maskSize.height = (maskSize.height/2)*2 + 1;

     FilterMask::setMask (maskSize);
#ifdef WITH_IPP_OPTIMIZATION 
     // for 3x3 and 5x5 mask their exists special routines
     if ((maskSize.width == 3 && maskSize.height == 3) ||
         (maskSize.width == 5 && maskSize.height == 5)) {
        aMethods[depth8u]  = &Median::ippMedianFixed<icl8u,  ippiFilterMedianCross_8u_C1R>;
        aMethods[depth16s] = &Median::ippMedianFixed<icl16s, ippiFilterMedianCross_16s_C1R>;
        aColorMethods[depth8u] = &Median::ippMedianColor<icl8u>;
//        aColorMethods[depth16s] = &Median::ippMedianColor<icl16s>; TODO_depth
        aColorMethods[depth32f] = &Median::ippMedianColor<icl32f>;
     } else { // otherwise apply general routine
        aMethods[depth8u]  = &Median::ippMedian<icl8u,  ippiFilterMedian_8u_C1R>;
        aMethods[depth16s] = &Median::ippMedian<icl16s, ippiFilterMedian_16s_C1R>;
        aColorMethods[depth8u] = 0;
        aColorMethods[depth16s] = 0;
        aColorMethods[depth32f] = 0;
     }
#endif
  }

  // }}}

  void Median::apply(const ImgBase *poSrc, ImgBase **ppoDst)
  {
    FUNCTION_LOG("");

    if (!prepare (ppoDst, poSrc)) return;
    void (Median::*pMethod)(const ImgBase*, ImgBase*) 
       = this->aMethods[poSrc->getDepth()];
    if (!pMethod) ICL_INVALID_FORMAT;
    (this->*pMethod)(poSrc, *ppoDst);
  }

  void Median::apply(const ImgBase *poSrc, ImgBase *poDst)
  {
     ICLASSERT_RETURN(poSrc->getDepth() == poDst->getDepth());
     ImgBase **ppoDst = &poDst;

     apply (poSrc, ppoDst);
     ICLASSERT(*ppoDst == poDst);
  }

#ifdef WITH_IPP_OPTIMIZATION 
  void Median::applyColor(const ImgBase *poSrc, ImgBase **ppoDst)
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN(poSrc->getChannels() == 3);

    if (!prepare (ppoDst, poSrc)) return;
    void (Median::*pMethod)(const ImgBase*, ImgBase*) 
       = this->aColorMethods[poSrc->getDepth()];
    if (!pMethod) ICL_INVALID_FORMAT;
    (this->*pMethod)(poSrc, *ppoDst);
  }

  void Median::applyColor(const ImgBase *poSrc, ImgBase *poDst)
  {
     ICLASSERT_RETURN(poSrc->getDepth() == poDst->getDepth());
     ImgBase **ppoDst = &poDst;

     applyColor (poSrc, ppoDst);
     ICLASSERT(*ppoDst == poDst);
  }
#else
#warning "applyColor is not implemented without IPP optimization";
  void Median::applyColor(const ImgBase *poSrc, ImgBase **ppoDst)
  {
     ERROR_LOG ("applyColor is not implemented without IPP optimization");
  }
#endif
  
}
