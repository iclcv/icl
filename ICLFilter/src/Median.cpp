 #include "Median.h"
#include "Img.h"
#include "ImgIterator.h"
#include <vector>
#include <algorithm>

namespace icl{
  
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
  void Median::ippMedian<icl8u> (ImgI *poSrc, ImgI *poDst) {
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFilterMedian_8u_C1R(poSrc->asImg<icl8u>()->getROIData (c, this->oROIoffset), 
                                poSrc->getLineStep(),
                                poDst->asImg<icl8u>()->getROIData (c), 
                                poDst->getLineStep(), 
                                poDst->getROISize(), oMaskSize, oAnchor);
     }
  }

  template<>
  void Median::ippMedianFixed<icl8u> (ImgI *poSrc, ImgI *poDst) {
     IppiMaskSize mask = oMaskSize.width == 3 ? ippMskSize3x3 : ippMskSize5x5;
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFilterMedianCross_8u_C1R(poSrc->asImg<icl8u>()->getROIData (c, this->oROIoffset), 
                                     poSrc->getLineStep(),
                                     poDst->asImg<icl8u>()->getROIData (c), 
                                     poDst->getLineStep(), 
                                     poDst->getROISize(), mask);
     }
  }
#endif

  // }}}

  // {{{ Fallback Implementation
  template<typename T>
  void Median::cMedian (ImgI *poSrc, ImgI *poDst) {
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

     Filter::setMask (maskSize);
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

  void Median::apply(ImgI *poSrc, ImgI **ppoDst)
  {
    FUNCTION_LOG("");

    if (!prepare (poSrc, ppoDst)) return;
    (this->*(aMethods[poSrc->getDepth()]))(poSrc, *ppoDst);
  }
}
