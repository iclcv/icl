#include "ICLConvolution.h"

namespace icl{

  // {{{ typedef of Ippi<Point|Size> if  WITH_IPP_OPTIMIZATION is not defined

#ifndef WITH_IPP_OPTIMIZATION
  typedef struct IppiPoint_ {int x,y;} IppiPoint;
  typedef struct IppiSize_ {int width,height;} IppiSize;
#endif

  // }}}
  
  int icl_is_convertable_to_int_intern(float *pfData, int iLen)
    // {{{ open
  {
    // tests if an element of the given float* has decimals
    // if it does: return 0, else 1
    for(int i=0;i<iLen;i++)
      {
        if(pfData[i] == (float)((int)pfData[i])) return 0;
      }    
    return 1;
  }

  // }}}
  
  // {{{ Constructors / Destructor

  ICLConvolution::ICLConvolution(iclkernel eKernel):
    // {{{ open

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
        ERROR_LOG("unsupported kernel type");
        break;        
    }
#endif
  }

  // }}}

  ICLConvolution::ICLConvolution(ICLBase *poKernel):
    // {{{ open

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
        if(icl_is_convertable_to_int_intern(poKernel->asIcl32f()->getData(0),iW*iH))
          {
            piKernel = new int[iW*iH];
          }
        for(int i=0;i<iW*iH;i++)
          {
            pfKernel[i]=(poKernel->asIcl32f()->getData(0))[i];
            if(piKernel)piKernel[i]=(int)pfKernel[i];
          }
      }
  }

  // }}}

  ICLConvolution::ICLConvolution(iclfloat *pfKernel, int iW, int iH, int iBufferData):
    // {{{ open

    pfKernel(0),piKernel(0), iW(iW),iH(iH),bDeleteData(iBufferData),eKernel(kernelCustom)
  {
    DEBUG_LOG4("ICLConvolution::ICLConvolution(iclfloat*,int,int)");
    if(iBufferData){
      this->pfKernel = new float[iW*iH];
      memcpy(this->pfKernel,pfKernel,iW*iH*sizeof(float));    
      if(icl_is_convertable_to_int_intern(pfKernel,iW*iH))
        {
          piKernel = new int[iW*iH];
          for(int i=0;i<iW*iH;i++)
            {
              piKernel[i]=(int)pfKernel[i];
            }
        }     
    }else{
      this->pfKernel = pfKernel;
    }
  }

  // }}}
  
  ICLConvolution::ICLConvolution(int *piKernel, int iW, int iH, int iBufferData):
    // {{{ open

    pfKernel(0),piKernel(0),
    iW(iW),iH(iH),bDeleteData(iBufferData),eKernel(kernelCustom)
  {
    DEBUG_LOG4("ICLConvolution::ICLConvolution(int*,int,int)");
    memcpy(this->pfKernel,pfKernel,iW*iH*sizeof(float)); 
      if(iBufferData){
        this->piKernel = new int[iW*iH];
        this->pfKernel = new float[iW*iH];
        memcpy(this->piKernel,piKernel,iW*iH*sizeof(int));    
        for(int i=0;i<iW*iH;i++){
          pfKernel[i]=(float)piKernel[i];
        }
      }else{
        this->piKernel = piKernel;
      }
  }

  // }}}
    
  ICLConvolution::~ICLConvolution(){
    // {{{ open

    DEBUG_LOG4("~ICLConvolution");
    if(bDeleteData){
      if(piKernel)delete piKernel;
      if(pfKernel)delete pfKernel;
    }
  }

  // }}}

  // }}}
 
  // {{{ help functions

  // C++-fall back function for generic convolution 
  template<class T, class M> void 
  generic_conv(ICLBase *poSrcIn, ICLBase *poDstIn, M* pmMask, int iKernelW, int iKernelH, int iChannel)
     // {{{ open

  {
    ICL<T> *poSrc = reinterpret_cast<ICL<T>*>(poSrcIn);
    ICL<T> *poDst = reinterpret_cast<ICL<T>*>(poDstIn);

    // accumulator for each pixel result
    M buf; 

    // pixel distances from the kernel anchor to the kernel borders
    int iTop = (int)floor((float)iKernelH/2);
    int iLeft = (int)floor((float)iKernelW/2);
    int iBottom = iKernelH-iTop-1;
    int iRight = iKernelW-iLeft-1;

    // move the kernel into the kernel center
    pmMask+=iLeft+iTop*iKernelW;
    
    // buffer som tmp variables 
    int iW = poSrc->getWidth();
    T *ptSrc = poSrc->getData(iChannel);

    // apply convolution on the source image
    for(ICLIterator<T> p_dst = poDst->begin(iChannel), p_src = poSrc->begin(iChannel);
        p_dst!=poDst->end(iChannel);
        p_dst++,p_src++)
      {
        buf = 0;
        for(int x=-iLeft;x<=iRight;x++){
          for(int y=-iTop;y<=iBottom;y++){
            buf += (M)ptSrc[p_src.x+x+(p_src.y+y)*iW]*(M)pmMask[x+y*iKernelW]; 
          }
        }
        
        // this is not necessary i think
        buf = buf<0 ? 0 : buf > 255 ? 255 : buf;
        *p_dst = static_cast<T>(buf);
      }    
  }

  // }}}

  // {{{ C_CONV-calls

#define C_CONV_8u(S,D,C,K,KS) generic_conv<iclbyte,int>(S,D,K,KS.width,KS.height,C);
#define C_CONV32_8u(S,D,C,K,KS) generic_conv<iclbyte,iclfloat>(S,D,K,KS.width,KS.height,C);
#define C_CONV_32f(S,D,C,K,KS) generic_conv<iclfloat,iclfloat>(S,D,K,KS.width,KS.height,C);
#define C_CONV8_32f(S,D,C,K,KS) generic_conv<iclfloat,int>(S,D,K,KS.width,KS.height,C);

  // }}}

  // {{{ IPP_CONV-calls

#define IPP_CONV_8u(S,D,C,K,KS,A) ippiFilter_8u_C1R(S->ippData8u(c),\
                                               S->ippStep(),        \
                                               D->ippData8u(c),     \
                                               D->ippStep(),        \
                                               D->ippROISize(),     \
                                               K,KS,A,1);

#define IPP_CONV32_8u(S,D,C,K,KS,A) ippiFilter32f_8u_C1R(S->ippData8u(c),\
                                                    S->ippStep(),        \
                                                    D->ippData8u(c),     \
                                                    D->ippStep(),        \
                                                    D->ippROISize(),     \
                                                    K,KS,A);

#define IPP_CONV_32f(S,D,C,K,KS,A) ippiFilter_32f_C1R(S->ippData32f(c), \
                                                  S->ippStep(),         \
                                                  D->ippData32f(c),     \
                                                  D->ippStep(),         \
                                                  D->ippROISize(),      \
                                                  K,KS,A);              \

  // }}}

  // {{{ code to apply the "fixed" convolution methods of the IPPI

#ifdef WITH_IPP_OPTIMIZATION

  // parameter list for an ipp call without IppiMaskSize arg
#define PARAM_LIST_A(DEPTH)         \
      poSrc->ippData ## DEPTH(c),   \
      poSrc->ippStep(),             \
      poDst->ippData ## DEPTH(c),   \
      poDst->ippStep(),             \
      poSrc->ippROISize()          
  // parameter list for an ipp call WITH IppiMaskSize arg
#define PARAM_LIST_B(DEPTH)         \
      PARAM_LIST_A(DEPTH),          \
      oMaskSize
      
  // the hole code for a single case xxx:{...} statement
#define A_CASE(THECASE,FILTER,MSIZE,PLIST_TYPE)                                 \
  case ICLConvolution::kernel ## THECASE:{                                      \
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
  void ipp_fixed_conv(ICLBase *poSrc, ICLBase *poDst, icldepth eDepth, ICLConvolution::iclkernel eKernel)
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

  // }}}

  void ICLConvolution::apply(ICLBase *poSrc, ICLBase *poDst)
    // {{{ open

  {
    DEBUG_LOG4("ICLConvolution::apply(ICLBase *,ICLBase*)");
   
    // {{{ prepare destination image

    if(poSrc->getDepth() != poDst->getDepth())
      {
        ERROR_LOG("ICLConvolution::apply: source and destination depth must be equal!");
      }

#ifdef WITH_IPP_OPTIMIZATION
    IppiPoint oAnchor = { iW/2, iH/2 };
#endif

    IppiSize oKernelSize = { iW, iH };
    
    morphROI(poSrc,-iW/2,-iH/2);    
    
    int iSrcRoiW, iSrcRoiH;
    poSrc->getROISize(iSrcRoiW,iSrcRoiH);

    poDst->renew(iSrcRoiW,iSrcRoiH,poSrc->getChannels());  

    // }}}

    // {{{ check for "fixed" kernels

#ifdef WITH_IPP_OPTIMIZATION
    if(eKernel != kernelCustom){
      ipp_fixed_conv(poSrc,poDst,poSrc->getDepth(),eKernel);
      morphROI(poSrc,iW/2,iH/2);
      return;
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
                IPP_CONV_8u(poSrc,poDst,c,piKernel,oKernelSize,oAnchor);
#else
                C_CONV_8u(poSrc,poDst,c,piKernel,oKernelSize);
#endif
              }
          }
        else // use float kernel as fallback
          {
            for(int c=0;c<poSrc->getChannels();c++)
              {
#ifdef WITH_IPP_OPTIMIZATION
                IPP_CONV32_8u(poSrc,poDst,c,pfKernel,oKernelSize,oAnchor);
#else
                C_CONV32_8u(poSrc,poDst,c,pfKernel,oKernelSize);
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
                IPP_CONV_32f(poSrc,poDst,c,pfKernel,oKernelSize,oAnchor);
#else
                C_CONV_32f(poSrc,poDst,c,pfKernel,oKernelSize);
#endif
              }
          }
        else // fallback to C-implementatino is 10 times slower! -> tmp conversion of the kernel data
          {
#ifdef WITH_IPP_OPTIMIZATION
            float *pfKernelTmp = new float[iW*iH];
            for(int i=0;i<iW*iH;i++)
              {
                pfKernelTmp[i]=piKernel[i];
              }
#endif            
            for(int c=0;c<poSrc->getChannels();c++)
              {
#ifdef WITH_IPP_OPTIMIZATION
                IPP_CONV_32f(poSrc,poDst,c,pfKernelTmp,oKernelSize,oAnchor);
#else
                C_CONV8_32f(poSrc,poDst,c,piKernel,oKernelSize);  
#endif
              }
#ifdef WITH_IPP_OPTIMIZATION
            delete pfKernelTmp;
#endif
          }
        
      }

    // }}}
    
    morphROI(poSrc,iW/2,iH/2);
  }

  // }}}
  
  // {{{ fixed convolution masks

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

  // }}}

   

}
