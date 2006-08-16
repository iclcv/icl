#include "Convolution.h"

namespace icl{
  
  /* To improve performance in access to the kernel data, we may create an
     internal buffer of the kernel data. This is done, if the user explicitly
     requests buffering, providing the bBufferData flag in the constructor. In
     this case already the constructor creates new kernel arrays piKernel
     and pfKernel within the methods <em>bufferKernel()</em>. Additionally
     the flag m_bBuffered is set to true.
     Not that a external float pointer can only be buffered as int* piKernel
     if the data can be interpreted as integers.

     If data is not buffered, we create a suitable piKernel or pfKernel buffer
     only on demand within the apply method. eKernelDepth indentifies which
     of the two buffers (pfKernel or piKernel) is external. 

     Because IPP (as well as the fallback implementation) does not provide a method
     to convolute a float image with an int kernel, we always use a float
     kernel in this case, which has to be copied from an external int kernel
     for every application.
  */
  // {{{ fixed convolution masks

  // first element specifies normalization factor
  int Convolution::KERNEL_SOBEL_X_3x3[10] = { 1,
                                              1,  0, -1,
                                              2,  0, -2,
                                              1,  0, -1 };
  
  int Convolution::KERNEL_SOBEL_Y_3x3[10] = {  1, 
                                               1,  2,  1,
                                               0,  0,  0,
                                              -1, -2, -1  };
  
  int Convolution::KERNEL_GAUSS_3x3[10] = { 16,
                                            1, 2, 1,
                                            2, 4, 2,
                                            1, 2, 1 };
  
  int Convolution::KERNEL_GAUSS_5x5[26] = { 571,
                                            2,  7,  12,  7,  2,
                                            7, 31,  52, 31,  7,
                                           12, 52, 127, 52, 12,
                                            7, 31,  52, 31,  7,
                                            2,  7,  12,  7,  2 };
  
  int Convolution::KERNEL_LAPLACE_3x3[10] = { 1,
                                              1, 1, 1,
                                              1,-8, 1,
                                              1, 1, 1} ;
   
  // }}}

  // {{{ isConvertableToInt

  bool Convolution::isConvertableToInt (float *pfData, int iLen)
  {
     // tests if an element of the given float* has decimals
     // if it does: return 0, else 1
     for(int i=0;i<iLen;i++)
        if (pfData[i] != nearbyintf (pfData[i])) return false;
     return true;
  }

  // }}}
  
  // {{{ Constructors / Destructor
   
  // {{{ buffering kernel data

  void Convolution::copyIntToFloatKernel (int iDim) {
     if (!pfKernel) pfKernel = new float[iDim];
     register int   *pi=piKernel+1, *piEnd=pi+iDim;
     register float *pf=pfKernel;
     // first element of piKernel contains normalization factor:
     for (; pi < piEnd; ++pi, ++pf)
        *pf = (float) *pi / (float) *piKernel;
  }

  // initially create buffer array (within constructor only)
  void Convolution::bufferKernel (float *pfKernelExt) {
     int iDim = oMaskSize.width * oMaskSize.height;
     
     pfKernel = new float[iDim];
     std::copy (pfKernelExt, pfKernelExt+iDim, pfKernel);
     if (isConvertableToInt (pfKernelExt, iDim)) {
        // first element contains normalization factor
        piKernel = new int[iDim+1]; piKernel[0] = 1;

        register float *pf=pfKernelExt, *pfEnd=pfKernelExt+iDim;
        register int   *pi=piKernel;
        for (; pf < pfEnd; ++pfEnd, ++pi) *pi = (int) *pf;
     }
  }
  // initially create buffer array (within constructor only)
  void Convolution::bufferKernel (int *piKernelExt) {
     int iDim = oMaskSize.width * oMaskSize.height;

     piKernel = new int[iDim+1];
     std::copy (piKernelExt, piKernelExt+iDim+1, piKernel);
     copyIntToFloatKernel (iDim);
  }

  // }}}

  Convolution::Convolution(kernel eKernel) :
     // {{{ open

     pfKernel(0), piKernel(0), m_bBuffered(true), m_eKernel(eKernel)
  {
     FUNCTION_LOG("");
     
#ifndef WITH_IPP_OPTIMIZATION
     int nSize = 3;
     switch(m_eKernel) {
       case kernelSobelX3x3:
          piKernel = KERNEL_SOBEL_X_3x3; break;
       case kernelSobelY3x3:
          piKernel = KERNEL_SOBEL_Y_3x3; break;
       case kernelGauss3x3:
          piKernel = KERNEL_GAUSS_3x3; break;
       case kernelGauss5x5:
          piKernel = KERNEL_GAUSS_5x5; nSize = 5; break;
       case kernelLaplace3x3:
          piKernel = KERNEL_LAPLACE_3x3; break;
       default:
          ERROR_LOG("unsupported kernel type");
          piKernel = KERNEL_GAUSS_3x3;
          break;     
     }
     setMask (Size(nSize, nSize));

     // create buffer for pfKernel:
     copyIntToFloatKernel (oMaskSize.width * oMaskSize.height);
     // set method pointers to generic convolution methods
     setMethodPointers ();
#else
     pFixed8u = 0; pFixed8uMask = 0; pFixed32f = 0; pFixed32fMask = 0;
     int nSize = 3;
     switch (m_eKernel) {
       case kernelSobelX3x3:
          pFixed8u  = ippiFilterSobelHoriz_8u_C1R;
          pFixed32f = ippiFilterSobelHoriz_32f_C1R;
          break;
       case kernelSobelX5x5:
          // attention: pFixed8uMask stays undefined
          pFixed32fMask = ippiFilterSobelHorizMask_32f_C1R;
          nSize = 5;
          break;
       case kernelSobelY3x3:
          pFixed8u  = ippiFilterSobelVert_8u_C1R;
          pFixed32f = ippiFilterSobelVert_32f_C1R;
          break;
       case kernelSobelY5x5:
          // attention: pFixed8uMask stays undefined
          pFixed32fMask = ippiFilterSobelVertMask_32f_C1R;
          nSize = 5;
          break;
       case kernelGauss5x5:
          nSize = 5;
       case kernelGauss3x3:
          pFixed8uMask = ippiFilterGauss_8u_C1R;
          pFixed32fMask = ippiFilterGauss_32f_C1R;
          break;
       case kernelLaplace5x5:
          nSize = 5;
       case kernelLaplace3x3:
          pFixed8uMask = ippiFilterLaplace_8u_C1R;
          pFixed32fMask = ippiFilterLaplace_32f_C1R;
          break;
       default:
          ERROR_LOG("unsupported kernel type");
          m_eKernel = kernelGauss3x3;
          pFixed8uMask = ippiFilterGauss_8u_C1R;
          pFixed32fMask = ippiFilterGauss_32f_C1R;
          break;     
     }
     // set mask size
     setMask (Size(nSize, nSize));
#endif
  }

  // }}}

  Convolution::Convolution (icl32f *pfKernel, const Size &size,
                            bool bBufferData) :
    // {{{ open
     Filter (size),
     pfKernel(0), piKernel(0), m_bBuffered(bBufferData), 
     m_eKernel(kernelCustom), m_eKernelDepth(depth32f)
  {
     FUNCTION_LOG("");
     
     if (bBufferData) bufferKernel (pfKernel);
     else this->pfKernel = pfKernel;
     setMethodPointers ();
  }

  // }}}
  
  Convolution::Convolution(int *piKernel, const Size &size, 
                           bool bBufferData) :
     // {{{ open
     Filter (size),
     pfKernel(0), piKernel(0), m_bBuffered(bBufferData), 
     m_eKernel(kernelCustom), m_eKernelDepth(depth8u)
  {
     FUNCTION_LOG("");
     
     if (bBufferData) bufferKernel (piKernel);
     else this->piKernel = piKernel;
     setMethodPointers ();
  }

  // }}}

  Convolution::~Convolution(){
     // {{{ open

     FUNCTION_LOG("");

     // free allocated kernel buffers
     if (m_bBuffered) { // can delete both buffers
        delete[] pfKernel;
        // in case of special kernel and no ipp usage
        // piKernel points to static class data
        if (m_eKernel != kernelCustom) delete[] piKernel;
     } else {
        if (m_eKernelDepth == depth8u)  delete[] pfKernel;
        if (m_eKernelDepth == depth32f) delete[] piKernel;
     }
  }

  // }}}

  // }}}
 
#ifdef WITH_IPP_OPTIMIZATION 

  // {{{ generic ipp convolution

  template<>
  void Convolution::ippGenericConv<icl8u, int> (ImgI *poSrc, ImgI *poDst) {
     Img<icl8u> *poS = poSrc->asImg<icl8u>();
     Img<icl8u> *poD = poDst->asImg<icl8u>();
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFilter_8u_C1R (poS->getROIData (c, poD->getROIOffset()), poS->getLineStep(),
                           poD->getROIData (c), poD->getLineStep(), 
                           poD->getROISize(), piKernel+1, oMaskSize, oAnchor, *piKernel);
     }
  }
  template<>
  void Convolution::ippGenericConv<icl8u, float> (ImgI *poSrc, ImgI *poDst) {
     Img<icl8u> *poS = poSrc->asImg<icl8u>();
     Img<icl8u> *poD = poDst->asImg<icl8u>();
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFilter32f_8u_C1R (poS->getROIData (c, poD->getROIOffset()), poS->getLineStep(),
                              poD->getROIData (c), poD->getLineStep(), 
                              poD->getROISize(), pfKernel, oMaskSize, oAnchor);
     }
  }
  template<>
  void Convolution::ippGenericConv<icl32f, float> (ImgI *poSrc, ImgI *poDst) {
     Img<icl32f> *poS = poSrc->asImg<icl32f>();
     Img<icl32f> *poD = poDst->asImg<icl32f>();
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFilter_32f_C1R (poS->getROIData (c, poD->getROIOffset()), poS->getLineStep(),
                            poD->getROIData (c), poD->getLineStep(), 
                            poD->getROISize(), pfKernel, oMaskSize, oAnchor);
     }
  }

  // }}}
   
  // {{{ fixed ipp convolution

  template<typename T>
  void Convolution::ippFixedConv (ImgI *poSrc, ImgI *poDst, 
                                  IppStatus (*pMethod)(const T* pSrc, int srcStep,
                                                       T* pDst, int dstStep, 
                                                       IppiSize roiSize)) {
     Img<T> *poS = poSrc->asImg<T>();
     Img<T> *poD = poDst->asImg<T>();
     
     for(int c=0; c < poSrc->getChannels(); c++) {
        pMethod (poS->getROIData (c, poD->getROIOffset()), poS->getLineStep(),
                 poD->getROIData (c), poD->getLineStep(), 
                 poD->getROISize());
     }
  }
  template<typename T>
  void Convolution::ippFixedConvMask (ImgI *poSrc, ImgI *poDst, 
                                      IppStatus (*pMethod)(const T* pSrc, int srcStep,
                                                           T* pDst, int dstStep, 
                                                           IppiSize roiSize, IppiMaskSize mask)) {
     IppiMaskSize eMaskSize = (IppiMaskSize)(11 * oMaskSize.width);
     Img<T> *poS = poSrc->asImg<T>();
     Img<T> *poD = poDst->asImg<T>();
     
     for(int c=0; c < poSrc->getChannels(); c++) {
        pMethod(poS->getROIData (c, poD->getROIOffset()), poS->getLineStep(),
                poD->getROIData (c), poD->getLineStep(), 
                poD->getROISize(), eMaskSize);
     }
  }

  // }}}

#else

  // {{{ generic fallback convolution
  template<typename ImageT, typename KernelT>
  void Convolution::cGenericConv (ImgI *poSrc, ImgI *poDst)
  {
    Img<ImageT> *poS = poSrc->asImg<ImageT>();
    Img<ImageT> *poD = poDst->asImg<ImageT>();

    // accumulator for each pixel result of Type M
    KernelT buffer; 

    // pointer to the mask
    const KernelT *m;

    for(int c=0; c < poSrc->getChannels(); c++) {
       for(ImgIterator<ImageT> s (poS->getData(c), poS->getSize().width, poD->getROI()),
              d=poD->getROIIterator(c); 
           s.inRegion(); ++s, ++d)
       {
          m = this->getKernel<KernelT>(); buffer = 0;
          for(ImgIterator<ImageT> sR(s,oMaskSize,oAnchor); sR.inRegion(); ++sR, ++m)
          {
             buffer += (*m) * (*sR);
          }
          *d = castResult<ImageT, KernelT>(buffer);
       }
    }
  }

  // }}}

#endif

  // {{{ setMethodPointers

  void Convolution::setMethodPointers () {
#ifdef WITH_IPP_OPTIMIZATION 
     aGenericConvs[depth8u][depth8u]   = &Convolution::ippGenericConv<icl8u,int>;
     aGenericConvs[depth8u][depth32f]  = &Convolution::ippGenericConv<icl8u,float>;
     aGenericConvs[depth32f][depth8u]  = 0;
     aGenericConvs[depth32f][depth32f] = &Convolution::ippGenericConv<icl32f,float>;
#else
     aGenericConvs[depth8u][depth8u]   = &Convolution::cGenericConv<icl8u,int>;
     aGenericConvs[depth8u][depth32f]  = &Convolution::cGenericConv<icl8u,float>;
     aGenericConvs[depth32f][depth8u]  = 0;
     aGenericConvs[depth32f][depth32f] = &Convolution::cGenericConv<icl32f,float>;
#endif     
  }

  // }}}

  void Convolution::apply(ImgI *poSrc, ImgI **ppoDst)
  {
    FUNCTION_LOG("");

    if (!prepare (poSrc, ppoDst)) return;

    /* We must carefully match the image depth to the
       available kernel depth(s).
    */
#ifdef WITH_IPP_OPTIMIZATION 
    if (m_eKernel != kernelCustom) {
       if (poSrc->getDepth () == depth8u) {
          // distinguish between different ipp function interfaces
          if (pFixed8u) this->ippFixedConv<icl8u> (poSrc, *ppoDst, pFixed8u);
          else this->ippFixedConvMask<icl8u> (poSrc, *ppoDst, pFixed8uMask);
       } else {
          // distinguish between different ipp function interfaces
          if (pFixed32f) this->ippFixedConv<icl32f> (poSrc, *ppoDst, pFixed32f);
          else this->ippFixedConvMask<icl32f> (poSrc, *ppoDst, pFixed32fMask);
       }
       return;
    }
#endif

    if (m_bBuffered || 
        poSrc->getDepth () == m_eKernelDepth ||
        m_eKernelDepth == depth32f)
       (this->*(aGenericConvs[poSrc->getDepth()][m_eKernelDepth])) (poSrc, *ppoDst);
    else { 
       // data has to be copied from (external) int* kernel to internal float buffer first
       copyIntToFloatKernel (oMaskSize.width * oMaskSize.height);
       // use float kernel always
       (this->*(aGenericConvs[poSrc->getDepth()][depth32f])) (poSrc, *ppoDst);
    }
  }

} // namespace icl
