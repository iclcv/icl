#include "Convolution.h"
#include "Img.h"

namespace icl {
  
  /* To improve performance in access to the kernel data, we may create an
     internal buffer of the kernel data. This is done, if the user explicitly
     requests buffering, providing the bBufferData flag in the constructor. In
     this case already the constructor creates new kernel arrays piKernel
     and pfKernel within the methods <em>bufferKernel()</em>. Additionally
     the flag m_bBuffered is set to true.
     Note, that an external float pointer can only be buffered as int* piKernel
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
    FUNCTION_LOG("");
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
    FUNCTION_LOG("");
     if (!pfKernel) pfKernel = new float[iDim];
     register int   *pi=piKernel+1, *piEnd=pi+iDim;
     register float *pf=pfKernel;
     // first element of piKernel contains normalization factor:
     for (; pi < piEnd; ++pi, ++pf)
        *pf = (float) *pi / (float) *piKernel;
  }

  // initially create buffer array (within constructor only)
  void Convolution::bufferKernel (float *pfKernelExt) {
    FUNCTION_LOG("");
     int iDim = oMaskSize.width * oMaskSize.height;
     if (!pfKernel) pfKernel = new float[iDim];
     std::copy (pfKernelExt, pfKernelExt+iDim, pfKernel);
     
     if (isConvertableToInt (pfKernelExt, iDim)) {
        // first element contains normalization factor
        if (!piKernel) piKernel = new int[iDim+1]; 
        piKernel[0] = 1;
          
        register float *pf=pfKernelExt, *pfEnd=pfKernelExt+iDim;
        register int   *pi=piKernel+1;
        for (; pf < pfEnd; ++pf, ++pi) {
          *pi = (int) *pf;
        }
     }
  }
  // initially create buffer array (within constructor only)
  void Convolution::bufferKernel (int *piKernelExt) {
    FUNCTION_LOG("");
     int iDim = oMaskSize.width * oMaskSize.height;

     if (!piKernel) piKernel = new int[iDim+1];
     std::copy (piKernelExt, piKernelExt+iDim+1, piKernel);
     copyIntToFloatKernel (iDim);
  }

  // }}}

  // {{{ setKernel

  void Convolution::setKernel (icl32f *pfKernel, const Size& size, bool bBufferData) {
     if (size != oMaskSize ||
         m_eKernel != kernelCustom ||
         m_bBuffered != bBufferData ||
         m_eKernelDepth != depth32f ||
         (this->pfKernel == 0 && this->piKernel == 0)) {
        // major mismatch: do not reuse buffers at all
        releaseBuffers ();

        // set new values
        setMask (size);
        m_eKernel = kernelCustom;
        m_eKernelDepth = depth32f;
        m_bBuffered = bBufferData;

        // buffer data
        if (bBufferData) bufferKernel (pfKernel);
        else this->pfKernel = pfKernel;
     } else if (m_bBuffered) {
        // simply use new kernel pointer
        this->pfKernel = pfKernel;
     } else { // buffered case
        bufferKernel(pfKernel);
     }
  }
  void Convolution::setKernel (int *piKernel, const Size& size, bool bBufferData) {
     if (size != oMaskSize ||
         m_eKernel != kernelCustom ||
         m_bBuffered != bBufferData ||
         m_eKernelDepth != depth8u ||
         (this->pfKernel == 0 && this->piKernel == 0)) {
        // major mismatch: do not reuse buffers at all
        releaseBuffers ();

        // set new values
        setMask (size);
        m_eKernel = kernelCustom;
        m_eKernelDepth = depth8u;
        m_bBuffered = bBufferData;

        // buffer data
        if (bBufferData) bufferKernel (piKernel);
        else this->piKernel = piKernel;
     } else if (m_bBuffered) {
        // simply use new kernel pointer
        this->piKernel = piKernel;
     } else { // buffered case
        bufferKernel(piKernel);
     }
  }

  // }}}

  Convolution::Convolution(kernel eKernel) :
     // {{{ open

     pfKernel(0), piKernel(0), m_bBuffered(true), m_eKernel(eKernel), m_eKernelDepth(depth8u)
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

  Convolution::Convolution () :
     // {{{ open
     FilterMask (Size(INT_MAX, INT_MAX)), // huge kernel size -> prepare returns false
     pfKernel(0), piKernel(0), m_bBuffered(false), 
     m_eKernel(kernelCustom), m_eKernelDepth(depth8u)
  {
     FUNCTION_LOG("");
  }

  // }}}

  Convolution::Convolution (icl32f *pfKernel, const Size &size,
                            bool bBufferData) :
     // {{{ open
     FilterMask (size),
     pfKernel(0), piKernel(0), m_bBuffered(!bBufferData), 
     m_eKernel(kernelCustom), m_eKernelDepth(depth32f)
  {
     FUNCTION_LOG("");
     setKernel(pfKernel, size, bBufferData);
  }

  // }}}
  
  Convolution::Convolution(int *piKernel, const Size &size, 
                           bool bBufferData) :
     // {{{ open
     FilterMask (size),
     pfKernel(0), piKernel(0), m_bBuffered(!bBufferData), 
     m_eKernel(kernelCustom), m_eKernelDepth(depth8u)
  {
     FUNCTION_LOG("");
     setKernel(piKernel, size, bBufferData);
  }

  // }}}

  void Convolution::releaseBuffers () {
     // free allocated kernel buffers
     if (m_bBuffered) { // can delete both buffers
        delete[] pfKernel;
        // in case of special kernel and no ipp usage
        // piKernel points to static class data
        if (m_eKernel == kernelCustom) delete[] piKernel;
     } else {
        if (m_eKernelDepth == depth8u)  delete[] pfKernel;
        if (m_eKernelDepth == depth32f) delete[] piKernel;
     }
     pfKernel = 0; piKernel = 0;
  }

  Convolution::~Convolution(){
     // {{{ open

     FUNCTION_LOG("");
     releaseBuffers();
  }

  // }}}

  // }}}
 
#ifdef WITH_IPP_OPTIMIZATION 

  // {{{ generic ipp convolution

  template<>
  void Convolution::ippGenericConv<icl8u, int> (ImgBase *poSrc, ImgBase *poDst) {
     Img<icl8u> *poS = poSrc->asImg<icl8u>();
     Img<icl8u> *poD = poDst->asImg<icl8u>();
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFilter_8u_C1R (poS->getROIData (c, this->oROIoffset), poS->getLineStep(),
                           poD->getROIData (c), poD->getLineStep(), 
                           poD->getROISize(), piKernel+1, oMaskSize, oAnchor, *piKernel);
     }
  }
  template<>
  void Convolution::ippGenericConv<icl8u, float> (ImgBase *poSrc, ImgBase *poDst) {
     Img<icl8u> *poS = poSrc->asImg<icl8u>();
     Img<icl8u> *poD = poDst->asImg<icl8u>();
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFilter32f_8u_C1R (poS->getROIData (c, this->oROIoffset), poS->getLineStep(),
                              poD->getROIData (c), poD->getLineStep(), 
                              poD->getROISize(), pfKernel, oMaskSize, oAnchor);
     }
  }
  template<>
  void Convolution::ippGenericConv<icl32f, float> (ImgBase *poSrc, ImgBase *poDst) {
     Img<icl32f> *poS = poSrc->asImg<icl32f>();
     Img<icl32f> *poD = poDst->asImg<icl32f>();
     for(int c=0; c < poSrc->getChannels(); c++) {
        ippiFilter_32f_C1R (poS->getROIData (c, this->oROIoffset), poS->getLineStep(),
                            poD->getROIData (c), poD->getLineStep(), 
                            poD->getROISize(), pfKernel, oMaskSize, oAnchor);
     }
  }

  // }}}
   
  // {{{ fixed ipp convolution

  template<typename T>
  void Convolution::ippFixedConv (ImgBase *poSrc, ImgBase *poDst, 
                                  IppStatus (*pMethod)(const T* pSrc, int srcStep,
                                                       T* pDst, int dstStep, 
                                                       IppiSize roiSize)) {
     Img<T> *poS = (Img<T>*) poSrc;
     Img<T> *poD = (Img<T>*) poDst;

     for(int c=0; c < poSrc->getChannels(); c++) {
        pMethod (poS->getROIData (c, this->oROIoffset), poS->getLineStep(),
                 poD->getROIData (c), poD->getLineStep(), 
                 poD->getROISize());
     }
  }
  template<typename T>
  void Convolution::ippFixedConvMask (ImgBase *poSrc, ImgBase *poDst, 
                                      IppStatus (*pMethod)(const T* pSrc, int srcStep,
                                                           T* pDst, int dstStep, 
                                                           IppiSize roiSize, IppiMaskSize mask)) {
     IppiMaskSize eMaskSize = (IppiMaskSize)(11 * oMaskSize.width);
     Img<T> *poS = (Img<T>*) poSrc;
     Img<T> *poD = (Img<T>*) poDst;
     
     for(int c=0; c < poSrc->getChannels(); c++) {
        pMethod(poS->getROIData (c, this->oROIoffset), poS->getLineStep(),
                poD->getROIData (c), poD->getLineStep(), 
                poD->getROISize(), eMaskSize);
     }
  }

  // }}}

#else

  // {{{ generic fallback convolution
  template<typename ImageT, typename KernelT>
  void Convolution::cGenericConv (ImgBase *poSrc, ImgBase *poDst)
  {
    Img<ImageT> *poS = (Img<ImageT>*) poSrc;
    Img<ImageT> *poD = (Img<ImageT>*) poDst;

    // accumulator for each pixel result of Type M
    KernelT buffer; 

    // pointer to the mask
    const KernelT *m;

    for(int c=0; c < poSrc->getChannels(); c++) {
       for(ImgIterator<ImageT> s (poS->getData(c), poS->getSize().width, 
                                  Rect (this->oROIoffset, poD->getROISize())),
              d=poD->getROIIterator(c); 
           s.inRegion(); ++s, ++d)
       {
          m = this->getKernel<KernelT>(); buffer = 0;
          for(ImgIterator<ImageT> sR(s,oMaskSize,oAnchor); sR.inRegion(); ++sR, ++m)
          {
             buffer += (*m) * (*sR);
          }
          *d = Cast<KernelT, ImageT>::cast(buffer);
       }
    }
  }

  // }}}

#endif

  // {{{ static MethodPointers aGenericConvs

  // array of image- and kernel-type selective generic convolution methods
  void (Convolution::*Convolution::aGenericConvs[2][2])(ImgBase *poSrc, ImgBase *poDst) = {
#ifdef WITH_IPP_OPTIMIZATION 
     {&Convolution::ippGenericConv<icl8u,int>,    // 8u - 8u
      &Convolution::ippGenericConv<icl8u,float>},  // 8u - 32f
     {0,                                          // 32f - 8u
      &Convolution::ippGenericConv<icl32f,float>}  // 32f - 32f
#else
     {&Convolution::cGenericConv<icl8u,int>,    // 8u - 8u
      &Convolution::cGenericConv<icl8u,float>},  // 8u - 32f
     {0,                                        // 32f - 8u
      &Convolution::cGenericConv<icl32f,float>}  // 32f - 32f
#endif     
  };

  // }}}

  // {{{ Convolution::apply (ImgBase *poSrc, ImgBase **ppoDst)

  void Convolution::apply(ImgBase *poSrc, ImgBase **ppoDst)
  {
    FUNCTION_LOG("");

    if (!prepare (ppoDst, poSrc)) return;

    /* We must carefully match the image depth to the
       available kernel depth(s).
    */
#ifdef WITH_IPP_OPTIMIZATION 
    if (m_eKernel != kernelCustom) {
       if (poSrc->getDepth () == depth8u) {
          // distinguish between different ipp function interfaces
          if (pFixed8u) this->ippFixedConv<icl8u> (poSrc, *ppoDst, pFixed8u);
          else if (pFixed8uMask) this->ippFixedConvMask<icl8u> (poSrc, *ppoDst, pFixed8uMask);
          else ERROR_LOG ("IPP fixed filter not implemented for depth8u");
       } else {
          // distinguish between different ipp function interfaces
          if (pFixed32f) this->ippFixedConv<icl32f> (poSrc, *ppoDst, pFixed32f);
          else if (pFixed32fMask) this->ippFixedConvMask<icl32f> (poSrc, *ppoDst, pFixed32fMask);
          else ERROR_LOG ("IPP fixed filter not implemented for depth32f");
       }
       return;
    }
#endif

    if (poSrc->getDepth () == m_eKernelDepth || m_eKernelDepth == depth32f) {
       (this->*(aGenericConvs[poSrc->getDepth()][m_eKernelDepth])) (poSrc, *ppoDst);
    } else { // 32f image and int* kernel case, which is not supported directly
       if (!m_bBuffered) {
          // data has to be copied from (external) int* kernel to internal float buffer first
          copyIntToFloatKernel (oMaskSize.width * oMaskSize.height);
       }
       // use float kernel always
       (this->*(aGenericConvs[poSrc->getDepth()][depth32f])) (poSrc, *ppoDst);
    }
  }

  // }}}


  // {{{ DynamicConvolution

  DynamicConvolution::DynamicConvolution (const ImgBase* poKernel) : 
     Convolution ()
  {
     poKernelBuf = new icl::Img<icl32f>(Size(3,3), 1);
     if (poKernel) setKernel (poKernel);
  }
  
  DynamicConvolution::~DynamicConvolution () {
     delete poKernelBuf;
  }

  void DynamicConvolution::setKernel (const ImgBase* poKernel) {
     ICLASSERT_RETURN(poKernel->getChannels() > 0);

     // resize kernel buffer if necessary
     if (poKernel->getROISize() != poKernelBuf->getSize())
        poKernelBuf->setSize (poKernel->getROISize());

     // copy data from poKernel's ROI to poKernelBuf
     if (poKernel->getDepth () == depth8u) {
        deepCopyChannelROI<icl8u,icl32f> 
           (poKernel->asImg<icl8u>(), 0, poKernel->getROIOffset(), poKernel->getROISize(),
            poKernelBuf, 0, Point::zero, poKernelBuf->getSize());
     } else {
        deepCopyChannelROI<icl32f,icl32f> 
           (poKernel->asImg<icl32f>(), 0, poKernel->getROIOffset(), poKernel->getROISize(),
            poKernelBuf, 0, Point::zero, poKernelBuf->getSize());
     }
     Convolution::setKernel (poKernelBuf->getData(0), poKernelBuf->getSize(), false);
  }

  // }}}

} // namespace icl
