#include "Convolution.h"

namespace icl{
  
  // {{{ fixed convolution masks

  int Convolution::KERNEL_SOBEL_X[9] = { 1,  0, -1,
                                            2,  0, -2,
                                            1,  0, -1   };
  
  int Convolution::KERNEL_SOBEL_Y[9] = {  1,  2,  1,
                                             0,  0,  0,
                                            -1, -2, -1  };
  
  float Convolution::KERNEL_GAUSS_3x3[9] = { 1.0/16 , 2.0/16, 1.0/16,
                                                2.0/16 , 4.0/16, 2.0/16,
                                                1.0/16 , 2.0/16, 1.0/16  };
  
  float Convolution::KERNEL_GAUSS_5x5[25] = { 2.0/571,  7.0/571,  12.0/571,  7.0/571,  2.0/571,
                                                 7.0/571, 31.0/571,  52.0/571, 31.0/571,  7.0/571,
                                                12.0/571, 52.0/571, 127.0/571, 52.0/571, 12.0/571,
                                                 7.0/571, 31.0/571,  52.0/571, 31.0/571,  7.0/571,
                                                 2.0/571,  7.0/571,  12.0/571,  7.0/571,  2.0/571 };
  
  int Convolution::KERNEL_LAPLACE[9] = { 1, 1, 1,
                                            1,-8, 1,
                                            1, 1, 1} ;

  // }}}

  // {{{ icl_is_convertable_to_int_intern
  int icl_is_convertable_to_int_intern(float *pfData, int iLen)
  {
    // tests if an element of the given float* has decimals
    // if it does: return 0, else 1
    for(int i=0;i<iLen;i++)
      {
        if(pfData[i] != (float)((int)pfData[i])) return 0;
      }    
    return 1;
  }

  // }}}
  
  // {{{ Constructors / Destructor

  Convolution::Convolution(iclkernel eKernel):
    // {{{ open

    pfKernel(0),piKernel(0),bDeleteData(0),eKernel(eKernel){
    DEBUG_LOG4("Convolution::Convolution(iclkernel)");

#ifndef WITH_IPP_OPTIMIZATION
    setMask (3,3);
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
        setMask (5,5);
        break;
      case kernelLaplace:
        piKernel = KERNEL_LAPLACE;
        break;
      default:
        ERROR_LOG("unsupported kernel type");
        break;        
    }
#endif
  }

  // }}}

  Convolution::Convolution(ImgI *poKernel):
    // {{{ open
    Filter (poKernel->getWidth(), poKernel->getHeight()),
    pfKernel(0),piKernel(0),bDeleteData(1),eKernel(kernelCustom)
  {
    DEBUG_LOG4("Convolution::Convolution(ImgI*)");
    int iDim = oMaskSize.width * oMaskSize.height;
    if(poKernel->getDepth()==depth8u)
      {
        piKernel = new int[iDim]; 
        pfKernel = new float[iDim];
        for(int i=0;i<iDim;i++)
          {
            piKernel[i]=(poKernel->asIcl8u()->getData(0))[i];
            pfKernel[i]=piKernel[i];
          }
      }
    else
      {
        pfKernel = new float[iDim];
        if(icl_is_convertable_to_int_intern(poKernel->asIcl32f()->getData(0),iDim))
          {
            piKernel = new int[iDim];
          }
        for(int i=0;i<iDim;i++)
          {
            pfKernel[i]=(poKernel->asIcl32f()->getData(0))[i];
            if(piKernel)piKernel[i]=(int)pfKernel[i];
          }
      }
  }

  // }}}

  Convolution::Convolution(iclfloat *pfKernel, int iW, int iH, int iBufferData):
    // {{{ open
    Filter (iW, iH),
    pfKernel(0),piKernel(0),bDeleteData(iBufferData),eKernel(kernelCustom)
  {
    DEBUG_LOG4("Convolution::Convolution(iclfloat*,int,int)");
    if(iBufferData){
      int iDim = oMaskSize.width * oMaskSize.height;
      this->pfKernel = new float[iDim];
      memcpy(this->pfKernel,pfKernel,iDim*sizeof(float));    
      if(icl_is_convertable_to_int_intern(pfKernel,iDim))
        {
          piKernel = new int[iDim];
          for(int i=0;i<iDim;i++)
            {
              piKernel[i]=(int)pfKernel[i];
            }
        }     
    }else{
      this->pfKernel = pfKernel;
    }
  }

  // }}}
  
  Convolution::Convolution(int *piKernel, int iW, int iH, int iBufferData):
    // {{{ open
    Filter (iW, iH),
    pfKernel(0),piKernel(0),bDeleteData(iBufferData),eKernel(kernelCustom)
  {
    DEBUG_LOG4("Convolution::Convolution(int*,int,int)");
    if(iBufferData){
        int iDim = oMaskSize.width * oMaskSize.height;
        this->piKernel = new int[iDim];
        this->pfKernel = new float[iDim];
        memcpy(this->piKernel,piKernel,iDim*sizeof(int));    
        for(int i=0;i<iDim;i++){
          pfKernel[i]=(float)piKernel[i];
        }
      }else{
        this->piKernel = piKernel;
      }
  }

  // }}}
    
  Convolution::~Convolution(){
    // {{{ open

    DEBUG_LOG4("~Convolution");
    if(bDeleteData){
      if(piKernel)delete piKernel;
      if(pfKernel)delete pfKernel;
    }
  }

  // }}}

  // }}}
 
  // {{{ C++ - generic convolution
  template<class ImageT, class KernelT, class BufferT> void 
  generic_conv(ImgI *poSrcIn, ImgI *poDstIn, KernelT* pmMask, int iKernelW, int iKernelH, int c)
  {
    Img<ImageT> *poS = reinterpret_cast<Img<ImageT>*>(poSrcIn);
    Img<ImageT> *poD = reinterpret_cast<Img<ImageT>*>(poDstIn);

    // accumulator for each pixel result of Type M
    BufferT buffer; 

    // pointer to the mask
    KernelT *m;
    for(ImgIterator<ImageT> s=poS->begin(c), d=poD->begin(c) ; s.inRegion() ; s++, d++)
      {
        m = pmMask;
        buffer = 0;
        for(ImgIterator<ImageT> sR(s,iKernelW,iKernelH) ; sR.inRegion(); sR++, m++)
          {
            buffer += (*m) * (*sR);
          } 
        *d =  static_cast<ImageT>(buffer<0 ? 0 : buffer > 255 ? 255 : buffer);
      }
  }

  // }}}

  // {{{ C_CONV-calls

#define C_CONV_8u(S,D,C,K,KS) generic_conv<iclbyte,int,int>(S,D,K,KS.width,KS.height,C);
#define C_CONV32_8u(S,D,C,K,KS) generic_conv<iclbyte,iclfloat,iclfloat>(S,D,K,KS.width,KS.height,C);
#define C_CONV_32f(S,D,C,K,KS) generic_conv<iclfloat,iclfloat,iclfloat>(S,D,K,KS.width,KS.height,C);
#define C_CONV8_32f(S,D,C,K,KS) generic_conv<iclfloat,int,iclfloat>(S,D,K,KS.width,KS.height,C);

  // }}}

  // {{{ IPP_CONV-calls

#define IPP_CONV_8u(S,D,C,K,KS,A) ippiFilter_8u_C1R(S->roiData8u(c),\
                                               S->ippStep(),        \
                                               D->roiData8u(c),     \
                                               D->ippStep(),        \
                                               D->getROISize(),     \
                                               K,KS,A,1);

#define IPP_CONV32_8u(S,D,C,K,KS,A) ippiFilter32f_8u_C1R(S->roiData8u(c),\
                                                    S->ippStep(),        \
                                                    D->roiData8u(c),     \
                                                    D->ippStep(),        \
                                                    D->getROISize(),     \
                                                    K,KS,A);

#define IPP_CONV_32f(S,D,C,K,KS,A) ippiFilter_32f_C1R(S->roiData32f(c), \
                                                  S->ippStep(),         \
                                                  D->roiData32f(c),     \
                                                  D->ippStep(),         \
                                                  D->getROISize(),      \
                                                  K,KS,A);              \

  // }}}

  // {{{ "fixed" ippi-convolution calls

#ifdef WITH_IPP_OPTIMIZATION

  // parameter list for an ipp call without IppiMaskSize arg
#define PARAM_LIST_A(DEPTH)         \
      poSrc->roiData ## DEPTH(c, &poDst->getROIOffset()),   \
      poSrc->ippStep(),             \
      poDst->roiData ## DEPTH(c),   \
      poDst->ippStep(),             \
      poSrc->getROISize()          
  // parameter list for an ipp call WITH IppiMaskSize arg
#define PARAM_LIST_B(DEPTH)         \
      PARAM_LIST_A(DEPTH),          \
      oMaskSize
      
  // the hole code for a single case xxx:{...} statement
#define A_CASE(THECASE,FILTER,MSIZE,PLIST_TYPE)                                 \
  case Convolution::kernel ## THECASE:{                                      \
    IppiMaskSize oMaskSize = (IppiMaskSize)(11*MSIZE);                          \
    (void)oMaskSize;                                                            \
    if(eDepth == depth8u){                                                      \
      for(int c=0;c<poSrc->getChannels();c++){                                  \
        ippiFilter ## FILTER ## _8u_C1R(PARAM_LIST_ ## PLIST_TYPE(8u));         \
      }                                                                         \
    }else{                                                                      \
      for(int c=0;c<poSrc->getChannels();c++){                                  \
        ippiFilter ## FILTER ## _32f_C1R(PARAM_LIST_ ## PLIST_TYPE(32f));       \
      }                                                                         \
    }                                                                           \
    return;                                                                     \
  }
  
  // internally used function, that applies ipp-optimized "fixed" convolution operations on images
  void ipp_fixed_conv(ImgI *poSrc, ImgI *poDst, icldepth eDepth, Convolution::iclkernel eKernel)
  {
    switch(eKernel)
      {   
        A_CASE(SobelX,SobelHoriz,3,A);
        A_CASE(SobelY,SobelVert,3,A);
        A_CASE(Gauss3x3,Gauss,3,B);
        A_CASE(Gauss5x5,Gauss,5,B);
        A_CASE(Laplace,Laplace,3,B);

        default:
          ERROR_LOG("unsupported kernel type");
          return;
      }
  }
#undef A_CASE
#undef PARAM_LIST_A
#undef PARAM_LIST_B
#endif

  // }}}

  // {{{ apply(ImgI*, ImgI*)
  ImgI* Convolution::apply(ImgI *poSrc, ImgI *poDst)
  {
    FUNCTION_LOG("");
   
    // {{{ prepare destination image

    poDst = prepare (poSrc, poDst);
    if (!adaptROI (poSrc, poDst)) return poDst;

    printf("---------------info in ICLConv:apply--------------\n");
    poSrc->print("src image");
    poDst->print("dst image");
    printf("Kernel = %d x %d \n",oMaskSize.width,oMaskSize.height);
    printf("--------------------------------------------------\n");

    // }}}

    // {{{ check for "fixed" kernels

#ifdef WITH_IPP_OPTIMIZATION
    if(eKernel != kernelCustom){
      ipp_fixed_conv(poSrc,poDst,poSrc->getDepth(),eKernel);
      return poDst;
    }
#endif

    // }}}

    if(poSrc->getDepth() == depth8u)
      // {{{ [depth8u-case:] open

      {
        if(piKernel)
          {
            for(int c=0;c<poSrc->getChannels();c++)
              {
#ifdef WITH_IPP_OPTIMIZATION
                IPP_CONV_8u(poSrc,poDst,c,piKernel,oMaskSize,oAnchor);
#else
                C_CONV_8u(poSrc,poDst,c,piKernel,oMaskSize);
#endif
              }
          }
        else // use float kernel as fallback
          {
            for(int c=0;c<poSrc->getChannels();c++)
              {
#ifdef WITH_IPP_OPTIMIZATION
                IPP_CONV32_8u(poSrc,poDst,c,pfKernel,oMaskSize,oAnchor);
#else
                C_CONV32_8u(poSrc,poDst,c,pfKernel,oMaskSize);
#endif
              }
          }
      }

    // }}}
    else 
      // {{{ [depth32f-case:] open

      {
        if(pfKernel)
          {
            for(int c=0;c<poSrc->getChannels();c++)
              {
#ifdef WITH_IPP_OPTIMIZATION
                IPP_CONV_32f(poSrc,poDst,c,pfKernel,oMaskSize,oAnchor);
#else
                C_CONV_32f(poSrc,poDst,c,pfKernel,oMaskSize);
#endif
              }
          }
        else // fallback to C-implementatino is 10 times slower! -> tmp conversion of the kernel data
          {
#ifdef WITH_IPP_OPTIMIZATION
            int iDim = oMaskSize.width * oMaskSize.height;
            float *pfKernelTmp = new float[iDim];
            for(int i=0;i<iDim;i++)
              {
                pfKernelTmp[i]=piKernel[i];
              }
#endif            
            for(int c=0;c<poSrc->getChannels();c++)
              {
#ifdef WITH_IPP_OPTIMIZATION
                IPP_CONV_32f(poSrc,poDst,c,pfKernelTmp,oMaskSize,oAnchor);
#else
                C_CONV8_32f(poSrc,poDst,c,piKernel,oMaskSize);  
#endif
              }
#ifdef WITH_IPP_OPTIMIZATION
            delete pfKernelTmp;
#endif
          }
        
      }

    // }}}
    
    return poDst;
  }

  // }}}

}
