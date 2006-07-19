#include "ICLMedian.h"
#include "ICLIterator.h"
#include "ICL.h"
#include <vector>
#include <algorithm>

namespace icl{
  
  // {{{ Constructor / Destructor

  ICLMedian::ICLMedian(int iWidth, int iHeight):
    m_iWidth(iWidth),m_iHeight(iHeight){
    if(iWidth <= 0 || iHeight<=0){
      ERROR_LOG("Mask width/height <=0 ??");
    }
  }
  
  ICLMedian::~ICLMedian(){}

  // }}}
  
  // {{{ Macro IPP_MEDIAN(S,D,C,MSIZE,ANCHOR,DEPTH)

#define IPP_MEDIAN(S,D,MSIZE,ANCHOR,DEPTH)                  \
  for(int c=0;c<S->getChannels();c++){                      \
     ippiMedian_ ## DEPTH ## _C1R(S->ippData ## DEPTH(C),   \
                                  S->ippStep(),             \
                                  D->ippData ## DEPTH(C),   \
                                  D->ippStep(),             \
                                  D->ippROISize(),          \
                                  MSIZE,ANCHOR);            \
  }

  // }}}
  
  // {{{ Macro C_MEDIAN(S,D,DEPTH,TYPE)
#define C_MEDIAN(S,D,DEPTH,TYPE)                                                                                        \
  ICL ## DEPTH *poS = S->asIcl ## DEPTH();                                                                              \
  ICL ## DEPTH *poD = D->asIcl ## DEPTH();                                                                              \
                                                                                                                        \
  std::vector<TYPE> oList(m_iWidth * m_iHeight);                                                                        \
  std::vector<TYPE>::iterator itList = oList.begin();                                                                   \
  std::vector<TYPE>::iterator itMedian = oList.begin()+((m_iWidth * m_iHeight)/2);                                      \
                                                                                                                        \
  for(int c=0;c<poSrc->getChannels();c++)                                                                               \
  {                                                                                                                     \
      for(ICLIterator<TYPE> s=poS->begin(c), d=poD->begin(c); s.inRegion() ; s++, d++ )                                 \
      {                                                                                                                 \
         for(ICLIterator<TYPE> sR(s,m_iWidth,m_iHeight); sR.inRegion(); sR++, itList++)                                 \
           {                                                                                                            \
              *itList = *sR;                                                                                            \
           }                                                                                                            \
         std::sort(oList.begin(),oList.end());                                                                          \
         *d = *itMedian;                                                                                                \
         itList = oList.begin();                                                                                        \
      }                                                                                                                 \
  }
  

  // }}}

  void ICLMedian::apply(ICLBase *poSrc, ICLBase *poDst)
  {
    // {{{ prepare

    DEBUG_LOG4("ICLMedian:apply(ICLBase *,ICLBase*)");    
    if(poSrc->getDepth() != poDst->getDepth()){
      ERROR_LOG("depths must be equal");
    }
    
    morphROI(poSrc,-m_iWidth/2,-m_iHeight/2);    
    int iSrcRoiW, iSrcRoiH;
    poSrc->getROISize(iSrcRoiW,iSrcRoiH);
    poDst->renew(iSrcRoiW,iSrcRoiH,poSrc->getChannels());  

    // }}}

    // {{{ median

#ifdef WITH_IPP_OPTIMIZATION 
    IppiPoint oAnchor = {m_iWidth/2,m_iHeight/2};      
    IppiSize oMaskSize = {m_iWidth,m_iHeight};
#endif
    if(poSrc->getDepth() == depth8u)
      {
#ifdef WITH_IPP_OPTIMIZATION 
        IPP_MEDIAN(poSrc,poDst,c,oMaskSize,oAnchor,32f);      
#else
        C_MEDIAN(poSrc,poDst,8u,iclbyte);
#endif
      }
    else
      {
#ifdef WITH_IPP_OPTIMIZATION 
        IPP_MEDIAN(poSrc,poDst,c,oMaskSize,oAnchor,8u);
#else
        C_MEDIAN(poSrc,poDst,32f,iclfloat);
#endif
      }

  // }}}
  
    // {{{ finish

    morphROI(poSrc,m_iWidth/2,m_iHeight/2);    

    // }}}
    
  }
  
  
}
