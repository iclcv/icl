#include "ICLConvolution.h"

namespace icl{
  void icl_morph_roi_intern(ICLBase *poImage, int iHorz, int iVert)
  {
    DEBUG_LOG4("icl_morph_roi_intern(ICLBase*,int,int)");
    
    int a,b;
    poImage->getROIOffset(a,b);
    poImage->setROIOffset(a-iHorz,b-iVert);
    
    poImage->getROISize(a,b);
    poImage->setROISize(a+2*iHorz,b+2*iVert);
  }
  
  inline void icl_conv_channel_intern(ICLBase *poSrc,
                                      int iSrcChannel,
                                      ICLBase *poDst,
                                      int iDstChannel, 
                                      ICLBase *poMask, 
                                      int iMaskChannel)
  {
    DEBUG_LOG4("icl_conv_channel_intern(ICLBase*,int,ICLBase*,int,ICLBase*,int)");
    // mask has to be tranlated into integers
#ifdef WITH_IPP_OPTIMIZATION
    if(poSrc->getDepth() != poDst->getDepth()){
      ERROR_LOG("icl_conv_channel_intern source and destination depth must be equal!\n");
    }
    
    IppiPoint oAnchor = {
      poMask->getWidth()/2,
      poMask->getHeight()/2
    };
    if(poMask->getDepth() == depth8u)
      {
        if(poSrc->getDepth() == depth8u)
          {
            Ipp32s *piBuf = new Ipp32s[poMask->getWidth()*poMask->getHeight()];
            ippiConvert_8u32s_C1R(poMask->ippData8u(iMaskChannel),
                                  poMask->ippStep(),
                                  piBuf,
                                  poMask->getWidth()*sizeof(int),
                                  poMask->ippRoiSize());
        
            ippiFilter_8u_C1R(poSrc->ippData8u(iSrcChannel),
                              poSrc->ippStep(),
                              poDst->ippData8u(iDstChannel),
                              poDst->ippStep(),
                              poDst->ippRoiSize(),
                              piBuf,
                              poMask->ippRoiSize(),
                              oAnchor,
                              1);
            delete piBuf;
          }
        else 
          {
            Ipp32f *pfBuf = new Ipp32f[poMask->getWidth()*poMask->getHeight()];
            ippiConvert_8u32f_C1R(poMask->ippData8u(iMaskChannel),
                                  poMask->ippStep(),
                                  pfBuf,
                                  poMask->getWidth()*sizeof(int),
                                  poMask->ippRoiSize());
            
            ippiFilter_32f_C1R(poSrc->ippData32f(iSrcChannel),
                               poSrc->ippStep(),
                               poDst->ippData32f(iDstChannel),
                               poDst->ippStep(),
                               poDst->ippRoiSize(),
                               pfBuf,
                               poMask->ippRoiSize(),
                               oAnchor);
            delete pfBuf;
          }
      }
    else  // poMask->getDepth() is depth32f
      {
        if(poSrc->getDepth() == depth8u)
          {
            ippiFilter32f_8u_C1R(poSrc->ippData8u(iSrcChannel),
                                 poSrc->ippStep(),
                                 poDst->ippData8u(iDstChannel),
                                 poDst->ippStep(),
                                 poDst->ippRoiSize(),
                                 poMask->ippData32f(iMaskChannel),
                                 poMask->ippRoiSize(),
                                 oAnchor);
          }
        else
          {
            ippiFilter_32f_C1R(poSrc->ippData32f(iSrcChannel),
                               poSrc->ippStep(),
                               poDst->ippData32f(iDstChannel),
                               poDst->ippStep(),
                               poDst->ippRoiSize(),
                               poMask->ippData32f(iMaskChannel),
                               poMask->ippRoiSize(),
                               oAnchor);
          }
      }
  }
#else
  printf("icl_conv_channel_intern is just supported if WITH_IPP_OPTIMIZATION is defined \n");
#endif
        
  void iclConv(ICLBase *poSrc, ICLBase *poDst, ICLBase *poMask)
  {
    DEBUG_LOG4("iclConv(ICLBase*,ICLBase*,ICLBase*)");
#ifdef WITH_IPP_OPTIMIZATION
    int iMaskW = poMask->getWidth();
    int iMaskH = poMask->getHeight();
    icl_morph_roi_intern(poSrc,-iMaskW/2,-iMaskH/2);
    
    // ensure, that destination image has the correct channel-count
    poDst->setNumChannels(poSrc->getChannels());
    
    // ensure, that destination image has the correct size
    poDst->resize(poSrc->ippRoiSize().width,poSrc->ippRoiSize().height);
    
    for(int c=0,m=0;c<poSrc->getChannels();c++){
      icl_conv_channel_intern(poSrc,c,poDst,c,poMask,m);      
      (++m)%=poMask->getChannels();
    }
    
    icl_morph_roi_intern(poSrc,iMaskW/2,iMaskH/2);
#else
  printf("iclConv is just supported if WITH_IPP_OPTIMIZATION is defined \n");
#endif
  }
  
  void iclConv(ICLBase *poSrc, ICLBase *poDst, iclfloat *pfMask, int iMaskW, int iMaskH)
  {
    DEBUG_LOG4("iclConv(ICLBase*,ICLBase*,iclfloat*,int,int)");
#ifdef WITH_IPP_OPTIMIZATION
    //using shared memory for wrapping an ICLImage around the float *pfMask
    ICL32f oMask(iMaskW,iMaskH,formatMatrix,1,&pfMask);
    iclConv(poSrc,poDst,&oMask);
#else
  printf("iclConv is just supported if WITH_IPP_OPTIMIZATION is defined \n");
#endif
  }
  
  void iclConv(ICLBase *poSrc, ICLBase *poDst, iclbyte *pucMask, int iMaskW, int iMaskH)
  {
    DEBUG_LOG4("iclConv(ICLBase*,ICLBase*,iclbyte*,int,int)");
#ifdef WITH_IPP_OPTIMIZATION
    //using shared memory for wrapping an ICLImage around the float *pfMask
    ICL8u oMask(iMaskW,iMaskH,formatMatrix,1,&pucMask);
    iclConv(poSrc,poDst,&oMask);
#else
    printf("iclConv is just supported if WITH_IPP_OPTIMIZATION is defined \n");
#endif
  }
  
}
