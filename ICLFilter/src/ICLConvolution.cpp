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
  
  ICLConvolution::ICLConvolution(iclkernel eKernel):
    pfKernel(0),piKernel(0),iW(3),iH(3),bDeleteData(0),eKernel(eKernel){
    DEBUG_LOG4("ICLConvolution::ICLConvolution(iclkernel)");

#ifndef WITH_IPP_OPTIMIZATION
    switch(eKernel){
      case kernelSobelX:
        piKernel = KERNEL_SOBEL_X;
        break;
      case kernelSobelY:
        piKernel = KERNEL_SOBEL_Y;
        break;
      case kernelGauss3x3:
        pfKernel = KERNEL_GAUSS_3x3;
        break;
      case kernelGauss5x5:
        pfKernel = KERNEL_GAUSS_5x5;
        iW=5;
        iH=5;
        break;
      case kernelLaplace:
        piKernel = KERNEL_LAPLACE;
        break;
      default:
        piKernel = KERNEL_ID;
        iW=1;
        iH=1;
        break;        
    }
#endif

  }
  ICLConvolution::ICLConvolution(ICLBase *poKernel):
    pfKernel(0),piKernel(0),iW(poKernel->getWidth()),iH(poKernel->getHeight()),
    bDeleteData(1),eKernel(kernelCustom)
  {
    DEBUG_LOG4("ICLConvolution::ICLConvolution(ICLBase*)");
    if(poKernel->getDepth()==depth8u)
      {
        piKernel = new int[iW*iH]; 
        pfKernel = new float[iW*iH];
        for(int i=0;i<iW*iH;i++)
          {
            piKernel[i]=(poKernel->asIcl8u()->getData(0))[i];
            pfKernel[i]=piKernel[i];
          }
      }
    else
      {
        pfKernel = new float[iW*iH];
        for(int i=0;i<iW*iH;i++)
          {
            pfKernel[i]=(poKernel->asIcl32f()->getData(0))[i];
          }
      }
  }
  ICLConvolution::ICLConvolution(iclfloat *pfKernel, int iW, int iH, int iBufferData):
    pfKernel(iBufferData?new float[iW*iH]:pfKernel),piKernel(0),
    iW(iW),iH(iH),bDeleteData(iBufferData),eKernel(kernelCustom)
  {
    DEBUG_LOG4("ICLConvolution::ICLConvolution(iclfloat*,int,int)");
    if(iBufferData) memcpy(this->pfKernel,pfKernel,iW*iH*sizeof(float));    
  }
  
  ICLConvolution::ICLConvolution(int *piKernel, int iW, int iH):
    pfKernel(new float[iW*iH]),piKernel(new int[iW*iH]),
    iW(iW),iH(iH),bDeleteData(1),eKernel(kernelCustom)
  {
    DEBUG_LOG4("ICLConvolution::ICLConvolution(int*,int,int)");
    memcpy(this->pfKernel,pfKernel,iW*iH*sizeof(float)); 
    for(int i=0;i<iW*iH;i++)
      {
        pfKernel[i]=piKernel[i];
      }
  }
    
  ICLConvolution::~ICLConvolution(){
    DEBUG_LOG4("~ICLConvolution");
    if(bDeleteData){
      if(piKernel)delete piKernel;
      if(pfKernel)delete pfKernel;
    }
  }
  
#ifdef WITH_IPP_OPTIMIZATION

#define CONV_8u(S,D,C,K,KS,A) ippiFilter_8u_C1R(S->ippData8u(c),\
                                               S->ippStep(),    \
                                               D->ippData8u(c), \
                                               D->ippStep(),    \
                                               D->ippRoiSize(), \
                                               K,KS,A,1);

#define CONV32_8u(S,D,C,K,KS,A) ippiFilter32f_8u_C1R(S->ippData8u(c),   \
                                                    S->ippStep(),       \
                                                    D->ippData8u(c),    \
                                                    D->ippStep(),       \
                                                    D->ippRoiSize(),    \
                                                    K,KS,A);

#define CONV_32f(S,D,C,K,KS,A) ippiFilter_32f_C1R(S->ippData32f(c),     \
                                                  S->ippStep(),         \
                                                  D->ippData32f(c),     \
                                                  D->ippStep(),         \
                                                  D->ippRoiSize(),      \
                                                  K,KS,A);              \
  
#else
#define CONV_8u(S,D,C,K,KS,A) (void)S;(void)D;(void)C;(void)K;(void)KS;(void)A;     \
                               ERROR_LOG("convolution not yet implemented without IPP");

#define CONV32_8u(a,b,c,d,e,f) CONV_8u(a,b,c,d,e,f)

#define CONV_32f(a,b,c,d,e,f) CONV_8u(a,b,c,d,e,f)
#endif
  void ICLConvolution::apply(ICLBase *poSrc, ICLBase *poDst)
  {
    DEBUG_LOG4("ICLConvolution::apply(ICLBase *,ICLBase*)");
    
    if(poSrc->getDepth() != poDst->getDepth())
      {
        ERROR_LOG("ICLConvolution::apply: source and destination depth must be equal!");
      }
    if(poSrc->ippRoiSize().width  != poDst->ippRoiSize().width || 
       poSrc->ippRoiSize().height != poDst->ippRoiSize().height)
      {
        ERROR_LOG("ICLConvolution::apply: source and destination roi sizes must be equal!");
      }
    if(poSrc->getChannels()!=poDst->getChannels())
      {
        ERROR_LOG("ICLConvolution::apply: source and destination channel count must be equal!");
      }
    
    IppiPoint oAnchor = { iW/2, iH/2 };
    IppiSize oKernelSize = { iW, iH };
    
    icl_morph_roi_intern(poSrc,-iW/2,-iH/2);    
    
    
    if(poSrc->getDepth() == depth8u)
      {
        if(piKernel)
          {
            for(int c=0;c<poSrc->getChannels();c++)
              {
                CONV_8u(poSrc,poDst,c,piKernel,oKernelSize,oAnchor);
              }
          }
        else
          {
            for(int c=0;c<poSrc->getChannels();c++)
              {
                CONV32_8u(poSrc,poDst,c,pfKernel,oKernelSize,oAnchor);
              }
          }
      }
    else // depth of image is 32f
      {
        for(int c=0;c<poSrc->getChannels();c++)
          {
            CONV_32f(poSrc,poDst,c,pfKernel,oKernelSize,oAnchor);
          }
        
      }
    
    icl_morph_roi_intern(poSrc,iW/2,iH/2);
  }
  
  int ICLConvolution::KERNEL_SOBEL_X[9] = { 1,  0, -1,
                                          2,  0, -2,
                                          1,  0, -1   };
  
  int ICLConvolution::KERNEL_SOBEL_Y[9] = {  1,  2,  1,
                                           0,  0,  0,
                                           -1, -2, -1  };
  
  float ICLConvolution::KERNEL_GAUSS_3x3[9] = { 1.0/16 , 2.0/16, 1.0/16,
                                              2.0/16 , 4.0/16, 2.0/16,
                                              1.0/16 , 2.0/16, 1.0/16  };
  
  float ICLConvolution::KERNEL_GAUSS_5x5[25] = { 2.0/571,  7.0/571,  12.0/571,  7.0/571,  2.0/571,
                                              7.0/571, 31.0/571,  52.0/571, 31.0/571,  7.0/571,
                                             12.0/571, 52.0/571, 127.0/571, 52.0/571, 12.0/571,
                                              7.0/571, 31.0/571,  52.0/571, 31.0/571,  7.0/571,
                                              2.0/571,  7.0/571,  12.0/571,  7.0/571,  2.0/571 };
  
  int ICLConvolution::KERNEL_LAPLACE[9] = { 1, 1, 1,
                                            1,-8, 1,
                                            1, 1, 1} ;
  

   

}
