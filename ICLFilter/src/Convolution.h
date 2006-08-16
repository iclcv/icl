#ifndef ICLCONVOLUTION_H
#define ICLCONVOLUTION_H

#include "Filter.h"
#include "Img.h"

namespace icl{
  
  /// Img-class for Image convolution
  /**
  The Convolution class provides functionality for generic linear image filter 
  procedures. To guarantee compatibility to the IPP optimized functions, also the 
  C++ fallback implementations are working with the IPPs ROI-conventions:
      
  <h2>Conventions (IPP)</h2>
  Let the source image image size be (W,H) and the mask size (M,N), then
  the convolution operation can only be performed on some inner rect of
  the source image. This inner rect is defined by the source images ROI-size.
  To ease using of iclConv, the ROI-size of the source image is made smaller
  before the operation and reset to its former size afterwards.
  The amount of pixels, that must be <i>eroded</i> before applying the convolution
  on the source image depends on the size of the mask, that is used.
  As the mask is anchored at the center-point, the ROI size is reduced by 
  (M/2,N/2) and enlarged by the same values after the operation.
  The destination image must be of the same size as this temporary ROI, and
  is resized automatically if it has a different size. Furthermore the channel 
  count and the depth of source and destination image must be equal. 
  These values are automatically adapted for the destination image if not matching.
  
  <h2>Efficiency (IPP-Optimized)</h2>
  All possible filter operations can be divided in 4 cases, depending on the
  source and destination images depths and the depth of the used filter kernel.
  Possible image depths are depth8u and depth32f (the two supported types of
  Img-images). Supported kernel depths are depth32f and <b>32-bit signed 
  integer</b> (called depth32s) (depth8u-kernels, will be converted 
  internally into this depth). Note the differences of the following cases: 
  
  <h3>case images: depth8u </h3>
  In this case, an integer kernel is preferred. That means, that an integer
  kernel will be used, if it's available. This will speed up performance:
  TODO: percent?

  <h3>case images: depth32f </h3>
  In this case, a float kernel is preferred. If it is not available, the
  fallback integer-kernel must be used. As convolution operations of float
  images with integer kernels are not supported by the IPP, the kernel is
  converted internally into a temporary float-buffer, which is released after
  the convolution operation. This will speed up performance in comparison
  with the fallback C-implementation by a factor about 10.

  <h3>Benchmarks</h3>
  The IPP-optimized functions are <b>VERY</b> fast in comparison to the 
  fallback C++ implementations. The optimized 3x3 convolution functions
  provided by the IPP are more then 20 times faster. Here are some benchmarks:
  - arbitrary 3x3-convolution 1000x1000 single channel image (IPP-OPTIMIZED)
     - iclbyte images & int kernel <b>~11.6ms</b>
     - iclfloat images & int kernel <b>~11.1ms</b>
     - iclbyte images & iclfloat kernel <b>~13.5ms</b>
     - iclfloat-image & iclfloat kernel <b>~11.3ms</b>
  - fixed 3x3 convolution 1000x1000 single channel sobelx (IPP-OPTIMIZED)
     - iclbyte images & int mask <b>~4ms (!!!)</b>
     - iclbyte images & iclfloat mask <b>~8ms (!!!)</b>
  - arbitrary 3x3-convolution 1000x1000 single channel image (C++-Fallback)
     - iclbyte images & int kernel <b>~56ms</b> (further implem. ~81ms) ???
     - iclfloat images & int kernel <b>~76ms</b> (further implem. ~370ms)
     - iclbyte images & iclfloat kernel <b>~135ms</b> (further implem. ~230ms)
     - iclfloat-image & iclfloat kernel <b>~60ms</b> (further implem. ~60ms)
  
  <h2>Buffering Kernels</h2>
  In some applications the Convolution object has to be created
  during runtime. If the filter-kernel is created elsewhere, and it
  is persistent over the <i>lifetime</i> of the Convolution object,
  it may not be necessary to copy the kernel deeply into an Convolution-
  internal buffer. To make the Convolution object just using a
  given kernel pointer, an additional flag <b>iBufferData</b> can be set
  in two Constructors.
  */
  class Convolution : public Filter{
    public:
    /// this enum contains several predefined convolution kernels
    /** <h3>kernelSobleX</h3>
        The sobel x filter is a combined filter. It performs a symmetrical
        border filter operation in x-direction, followed by a smoothing
        operation in y-direction:
        <pre>

                        ---------     ---        
                       | 1  0 -1 |   | 1 |    ---------
        kernelSobelX = | 2  0 -2 | = | 2 | * | 1  0 -1 |
                       | 1  0 -1 |   | 1 |    ---------
                        ---------     ---
        </pre>           
    
        <h3>kernelSobelY</h3>
        The sobel y filter is essentially equal to the sobel y filter. The
        border detection will run in y-direction, and the smoothing x-direction.

        <pre>

                        ---------     ---        
                       | 1  2  1 |   | 1 |    ---------
        kernelSobelY = | 0  0  0 | = | 0 | * | 1  2  1 |
                       |-1 -2 -1 |   |-1 |    ---------
                        ---------     ---
        </pre>               
    
        <h3>kernelGauss3x3</h3>
        This is a 3x3-Pixel approximation of a 2D Gaussian. It is separable into
        smoothing filters in x- and y-direction.
        
        <pre>

                                 ---------     ---        
                                | 1  2  1 |   | 1 |    ---------
        kernelGauss3x3 = 1/16 * | 2  4  2 | = | 2 | * | 1  2  1 | * (1/4) *(1/4) 
                                | 1  2  1 |   | 1 |    ---------
                                 ---------     ---
        </pre>               

        <h3>kernelGauss5x5</h3>
        This is a 5x5-Pixel approximation of a 2D Gaussian. It is separable into
        smoothing filters in x- and y-direction.
        
        <pre>

                                  ----------------- 
                                 | 2   7  12  7  2 |
                                 | 7  31  52 31  7 |
        kernelGauss5x5 = 1/571 * |12  52 127 52 12 |
                                 | 7  31  52 31  7 |
                                 | 2   7  12  7  2 |
                                  ----------------- 

        </pre>               

        <h3>kernelLaplace</h3>
        The Laplacian kernel is a discrete approximation of the 2nd derivation
        of a 2D function.

        <pre>

                         --------- 
                        | 1  1  1 |
        kernelLaplace = | 1 -8  1 |
                        | 1  1  1 |
                         --------- 
        </pre>    
    
      
    */
    enum Kernel { 
      kernelGauss3x3,  /*!< 3x3 approximation of a Gaussian */
      kernelGauss5x5,  /*!< 5x5 approximation of a Gaussian */
      kernelSobelX3x3, /*!< 3x3 sobel x filter */
      kernelSobelX5x5, /*!< 5x5 sobel x filter */
      kernelSobelY3x3, /*!< 3x3 sobel y filter */
      kernelSobelY5x5, /*!< 5x5 sobel y filter */
      kernelLaplace3x3, /*!< 3x3 approximation of the 2nd derivation */
      kernelLaplace5x5, /*!< 5x5 approximation of the 2nd derivation */
      kernelCustom  /*!< used for all other user defined kernels */ 
    };
    
    /// create Convolution object with a fixed predefined filter type
    /** @param eKernel determines the filter type 
        (kernelCustom is not allowed here!)
    */
    Convolution(Kernel eKernel);

    /// create Convolution object from ROI of given image
    /** If the given kernel is of depth8u, then its data will be buffered 
        internally both as depth32f and depth32s (32-bit signed integer).

        Otherwise (kernel: depth32f) it is buffered in the internal depth32f buffer.
        If it is possible to use signed integer values also (if the given float
        buffer contains no floats with decimals) then an additional depth32s buffer
        is created, and filled with the casted values. In case of applying the
        filter on depth8u images, this will speed up performance.
        @param poKernel custom convolution kernel
    */
    Convolution(iclfloat *pfKernel, const Size& size, bool bBufferData=true);

    /// Creates an Convolution object with the given custom kernel
    /** This constructor behaves essentially like the above one.
        The first element of piKernel is assumed to contain a normalization 
        factor, by which the scalar product of kernel and image mask is divided.
        Usually it is the sum of purely possitive kernel entries or it equals 1.

        @param piKernel convolution kernel data
        @param size kernel mask size
        @param bBufferData flag that indicates, if given data should be 
        buffered internally. By default given data will be buffered.
    */
    Convolution(int *piKernel, const Size& size, bool bBufferData=true);

    /// Destructor
    virtual ~Convolution();
    
    /// performs the convolution operation on the image
    /** The destination image is automatically set up to 
        correct size and its channel count is set to the
        source images channel count.
        The size of the destination image becomes:
        <pre>
        width = src.width-kernel.width/2
        height = src.height-kernel.height/2
        </pre>
        @param poSrc source image
        @param poDst destination image
    */
    virtual void apply(ImgI *poSrc, ImgI **ppoDst);
    
    private:

    /// internal storage for the sobel-x filter kernel
    static int KERNEL_SOBEL_X_3x3[10];

    /// internal storage for the sobel-y filter kernel
    static int KERNEL_SOBEL_Y_3x3[10];

    /// internal storage for the gauss 3x3 filter kernel
    static int KERNEL_GAUSS_3x3[10];

    /// internal storage for the gauss 5x5 filter kernel
    static int KERNEL_GAUSS_5x5[26];

    /// internal storage for the Laplace filter kernel
    static int KERNEL_LAPLACE_3x3[10];
  
    /// storage of the kernel data
    float *pfKernel;
    /// storage of the kernel data
    int   *piKernel;
    
    /// indicates that data is buffered
    bool   m_bBuffered;

    /// kernel type
    Kernel m_eKernel;
    Depth  m_eKernelDepth;

    /// checks whether float array can be interpreted as int
    bool isConvertableToInt (float *pfData, int iLen);
    /// copy external int kernel to internal float buffer
    void copyIntToFloatKernel (int iDim);
    /// create kernel buffers
    void bufferKernel (float *pfKernel);
    void bufferKernel (int *piKernel);

    /// array of image- and kernel-type selective generic convolution methods
    void (Convolution::*aGenericConvs[2][2])(ImgI *poSrc, ImgI *poDst); 
    /// set those method pointers
    void setMethodPointers ();

#ifdef WITH_IPP_OPTIMIZATION 
    template<typename ImgT, typename KernelT>
       void ippGenericConv (ImgI *poSrc, ImgI *poDst);
    template<typename T>
       void ippFixedConv (ImgI *poSrc, ImgI *poDst,
                          IppStatus (*pMethod)(const T* pSrc, int srcStep,
                                               T* pDst, int dstStep, IppiSize));
    template<typename T>
       void ippFixedConvMask (ImgI *poSrc, ImgI *poDst,
                              IppStatus (*pMethod)(const T* pSrc, int srcStep,
                                                   T* pDst, int dstStep, 
                                                   IppiSize, IppiMaskSize));

    /// function pointer for ipp fixed convolution, depth8u image
    IppStatus (*pFixed8u)(const Ipp8u* pSrc, int srcStep,
                              Ipp8u* pDst, int dstStep, 
                              IppiSize roiSize);
    /// function pointer for ipp fixed convolution, depth8u image, mask size parameter
    IppStatus (*pFixed8uMask)(const Ipp8u* pSrc, int srcStep,
                                  Ipp8u* pDst, int dstStep, 
                                  IppiSize roiSize, IppiMaskSize mask);
    /// function pointer for ipp fixed convolution, depth32f image
    IppStatus (*pFixed32f)(const Ipp32f* pSrc, int srcStep,
                           Ipp32f* pDst, int dstStep, 
                           IppiSize roiSize);
    /// function pointer for ipp fixed convolution, depth32f image, mask size parameter
    IppStatus (*pFixed32fMask)(const Ipp32f* pSrc, int srcStep,
                               Ipp32f* pDst, int dstStep, 
                               IppiSize roiSize, IppiMaskSize mask);

#else
    template<typename ImgT, typename KernelT>
       void cGenericConv (ImgI *poSrc, ImgI *poDst);

    template<typename KernelT> const KernelT* getKernel() const;
    template<typename ImgT, typename KernelT> ImgT castResult(const KernelT value) {
       return static_cast<ImgT>(value);
    }
#endif
  };

#ifndef WITH_IPP_OPTIMIZATION 
  template<> const int* Convolution::getKernel<int>()     const {return piKernel;}
  template<> const float* Convolution::getKernel<float>() const {return pfKernel;}

/*
  template<> 
  iclbyte Convolution::castResult<iclbyte, int> (const int value) const {
     return static_cast<iclbyte>(value < 0 ? 0 : value > 255 ? 255 : value);
  }
  template<>
  iclbyte Convolution::castResult<iclbyte,float> (const float value) const {
     return static_cast<iclbyte>(value < 0. ? 0 : value > 255. ? 255 : value);
  }
*/
#endif
}

#endif
