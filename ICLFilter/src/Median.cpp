#include "Median.h"
#include "ImgIterator.h"
#include "Img.h"
#include <vector>
#include <algorithm>

namespace icl{
  
  // {{{ Constructor / Destructor

  Median::Median(int iWidth, int iHeight):
     Filter ((iWidth/2)*2+1, (iHeight/2)*2+1) {
     if(iWidth <= 0 || iHeight<=0){
        ERROR_LOG("illegal width/height: " << iWidth << "/" << iHeight);
        setMask (3, 3); // set some sensible default
     }
  }
  
  // }}}
  
  // {{{ Macro IPP_MEDIAN(S,D,C,MSIZE,ANCHOR,DEPTH)

#define IPP_MEDIAN(S,D,MSIZE,ANCHOR,DEPTH)                     \
  for(int c=0;c<S->getChannels();c++){                         \
     ippiFilterMedian_ ## DEPTH ## _C1R(S->roiData ## DEPTH(c,&oROIoffset),\
                                  S->ippStep(),                \
                                  D->roiData ## DEPTH(c),      \
                                  D->ippStep(),                \
                                  D->getROISize(),             \
                                  MSIZE,ANCHOR);               \
  }

  // }}}
  
  // {{{ Macro C_MEDIAN(S,D,DEPTH,TYPE)
#define C_MEDIAN(S,D,DEPTH,TYPE)                                                                                        \
  Img ## DEPTH *poS = S->asIcl ## DEPTH();                                                                              \
  Img ## DEPTH *poD = D->asIcl ## DEPTH();                                                                              \
                                                                                                                        \
  std::vector<TYPE> oList(oMaskSize.width * oMaskSize.height);                                                                        \
  std::vector<TYPE>::iterator itList = oList.begin();                                                                   \
  std::vector<TYPE>::iterator itMedian = oList.begin()+((oMaskSize.width * oMaskSize.height)/2);                                      \
                                                                                                                        \
  for(int c=0;c<poSrc->getChannels();c++)                                                                               \
  {                                                                                                                     \
      for(ImgIterator<TYPE> s=poS->begin(c), d=poD->begin(c); s.inRegion() ; s++, d++ )                                 \
      {                                                                                                                 \
         for(ImgIterator<TYPE> sR(s,oMaskSize.width,oMaskSize.height); sR.inRegion(); sR++, itList++)                                 \
           {                                                                                                            \
              *itList = *sR;                                                                                            \
           }                                                                                                            \
         std::sort(oList.begin(),oList.end());                                                                          \
         *d = *itMedian;                                                                                                \
         itList = oList.begin();                                                                                        \
      }                                                                                                                 \
  }
  
  // }}}

  ImgI* Median::apply(ImgI *poSrc, ImgI *poDst)
  {
    FUNCTION_LOG("");

    poDst = prepare (poSrc, poDst);
    if (!adaptROI (poSrc, poDst)) return poDst;
       
    // {{{ median

    if(poSrc->getDepth() == depth8u)
      {
#ifdef WITH_IPP_OPTIMIZATION 
        IPP_MEDIAN(poSrc,poDst,oMaskSize,oAnchor,8u);      
#else
        C_MEDIAN(poSrc,poDst,8u,iclbyte);
#endif
      }
    else
      {
        // no ipp-optimization available, as the 
        // fast median algorithm uses the "histogramm"
        // based algorithm ...
        C_MEDIAN(poSrc,poDst,32f,iclfloat); 
      }

    // }}}
  
    return poDst;
  }  
}
