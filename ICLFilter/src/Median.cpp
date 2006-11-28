#include <Median.h>
#include <Img.h>
#include <ICLcc.h>
#include <ImgIterator.h>
#include <vector>
#include <algorithm>

namespace icl {
  
  // {{{ Constructor / Destructor

  Median::Median (const Size& maskSize) {
     if (maskSize.width <= 0 || maskSize.height<=0) {
        ERROR_LOG("illegal width/height: " << maskSize.width << "/" << maskSize.height);
        setMask (Size(3,3));
     } else setMask (maskSize);
  }
   
  // }}}
  
  // {{{ IPP implementation

#ifdef WITH_IPP_OPTIMIZATION 
  template<>
  void Median::ippMedian<icl8u> (const ImgBase *poSrc, ImgBase *poDst) {
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFilterMedian_8u_C1R(poSrc->asImg<icl8u>()->getROIData (c, this->oROIoffset), 
                                poSrc->getLineStep(),
                                poDst->asImg<icl8u>()->getROIData (c), 
                                poDst->getLineStep(), 
                                poDst->getROISize(), oMaskSize, oAnchor);
     }
  }

  template<>
  void Median::ippMedianFixed<icl8u> (const ImgBase *poSrc, ImgBase *poDst) {
     IppiMaskSize mask = oMaskSize.width == 3 ? ippMskSize3x3 : ippMskSize5x5;
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFilterMedianCross_8u_C1R(poSrc->asImg<icl8u>()->getROIData (c, this->oROIoffset), 
                                     poSrc->getLineStep(),
                                     poDst->asImg<icl8u>()->getROIData (c), 
                                     poDst->getLineStep(), 
                                     poDst->getROISize(), mask);
     }
  }
  
  //template <typename T, IppStatus (*ippiFunc) (T*, int, IppiSize)>
  //template<,IppStatus (*ippiFunc) (const icl8u*, int, icl8u*,int, IppiSize)>
  template<>
  void Median::ippMedianColor<icl8u> (const ImgBase *poSrc, ImgBase *poDst) {
		int dim=poSrc->getWidth() * poSrc->getHeight();
		int dim2=poDst->getWidth() * poDst->getHeight();
	  m_oBuffer8u[0].resize(dim*poSrc->getChannels());
	  m_oBuffer8u[1].resize(dim2*poSrc->getChannels());
     //planarToInterleaved(poSrc->asImg<icl8u>(), *(m_oBuffer8u[0]),this->oROIoffset);
    planarToInterleaved(poSrc->asImg<icl8u>(), *(m_oBuffer8u[0]));
    IppiMaskSize mask = oMaskSize.width == 3 ? ippMskSize3x3 : ippMskSize5x5;
    if (poSrc->getChannels()==3){
      ippiFilterMedianColor_8u_C3R(*m_oBuffer8u[0], 
                                     poSrc->getLineStep()*poSrc->getChannels(),
                                     *m_oBuffer8u[1], 
                                     poDst->getLineStep()*poSrc->getChannels(), 
                                     poDst->getROISize(), mask);
    }
    else { //ch=4
      ippiFilterMedianColor_8u_AC4R(*m_oBuffer8u[0], 
                                     poSrc->getLineStep()*poSrc->getChannels(),
                                     *m_oBuffer8u[1], 
                                     poDst->getLineStep()*poSrc->getChannels(), 
                                     poDst->getROISize(), mask);
    }
    interleavedToPlanar(*(m_oBuffer8u[1]),poDst->getSize(),poSrc->getChannels(),poDst->asImg<icl8u>());
  }
  template <>
  void Median::ippMedianColor<icl32f> (const ImgBase *poSrc, ImgBase *poDst) {
		int dim=poSrc->getWidth() * poSrc->getHeight();
		int dim2=poDst->getWidth() * poDst->getHeight();
	  m_oBuffer32f[0].resize(dim*poSrc->getChannels());
	  m_oBuffer32f[1].resize(dim2*poSrc->getChannels());
     //planarToInterleaved(poSrc->asImg<icl32f>(), *(m_oBuffer32f[0]),this->oROIoffset);
    planarToInterleaved(poSrc->asImg<icl32f>(), *(m_oBuffer32f[0]));
    IppiMaskSize mask = oMaskSize.width == 3 ? ippMskSize3x3 : ippMskSize5x5;
    if (poSrc->getChannels()==3){
      ippiFilterMedianColor_32f_C3R(*m_oBuffer32f[0], 
                                     poSrc->getLineStep()*poSrc->getChannels(),
                                     *m_oBuffer32f[1], 
                                     poDst->getLineStep()*poSrc->getChannels(), 
                                     poDst->getROISize(), mask);
    }
    else { //ch=4
      ippiFilterMedianColor_32f_AC4R(*m_oBuffer32f[0], 
                                     poSrc->getLineStep()*poSrc->getChannels(),
                                     *m_oBuffer32f[1], 
                                     poDst->getLineStep()*poSrc->getChannels(), 
                                     poDst->getROISize(), mask);
    }
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
     if (maskSize.width == 3 && maskSize.height == 3)
        aMethods[depth8u] = &Median::ippMedianFixed<icl8u>;
     else if (maskSize.width == 5 && maskSize.height == 5)
        aMethods[depth8u] = &Median::ippMedianFixed<icl8u>;
     // otherwise apply general routine
     else aMethods[depth8u] = &Median::ippMedian<icl8u>;
     // for floats there is no IPP routine yet
     aMethods[depth32f] = &Median::cMedian<icl32f>;
#else
     aMethods[depth8u]  = &Median::cMedian<icl8u>;
     aMethods[depth32f] = &Median::cMedian<icl32f>;
#endif
  }

  // }}}

  void Median::apply(const ImgBase *poSrc, ImgBase **ppoDst)
  {
    FUNCTION_LOG("");

    if (!prepare (ppoDst, poSrc)) return;
    (this->*(aMethods[poSrc->getDepth()]))(poSrc, *ppoDst);
  }


#ifdef WITH_IPP_OPTIMIZATION 
  void Median::applyColor(const ImgBase *poSrc, ImgBase **ppoDst)
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN(poSrc->getChannels() == 3);

    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u:
         ippMedianColor<icl8u>(poSrc, *ppoDst);
         break;
      case depth32f:
         ippMedianColor<icl32f>(poSrc, *ppoDst);
         break;

      default:
         ICL_INVALID_FORMAT;
         break;
    }
  }
#else
  void Median::applyColor(const ImgBase *poSrc, ImgBase **ppoDst)
  {
     #warning "applyColor is not implemented without IPP optimization";
  }
#endif
  
}
