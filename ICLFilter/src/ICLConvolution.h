#ifndef ICLCONVOLUTION_H
#define ICLCONVOLUTION_H

#include "ICLFilter.h"
#include "ICL.h"

namespace icl{
  
  /// ICL-class for Image convolution
  /**
  The ICLConvolution class provides functionality for generic linear image filter 
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
  In case of different channel count, the channel count of the destination image
  is adapted to the channel count of the source image. If the depth of source 
  and destination do not match, an error message is shown, and the program will 
  be aborted with exit(-1).
  
  <h2>Efficiency (IPP-Optimized)</h2>
  All possible filter operations can be divided in 4 cases, depending on the
  source and destination images depths and the depth of the used filter kernel.
  Possible image depths are depth8u and depth32f (the two supported types of
  ICL-images). Supported kernel depths are depth32f and <b>32-bit signed 
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
  konverted internally into a temporary float-buffer, which is released after
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
     - iclbyte images & int kernel <b>~56ms</b> (further implem. ~81ms)
     - iclfloat images & int kernel <b>~76ms</b> (further implem. ~370ms)
     - iclbyte images & iclfloat kernel <b>~135ms</b> (further implem. ~230ms)
     - iclfloat-image & iclfloat kernel <b>~60ms</b> (further implem. ~60ms)
  
  <h2>Buffering Kernels</h2>
  In some applications the ICLConvolution object has to be created
  during runtime. If the filter-kernel is created elsewhere, and it
  is persistent over the <i>lifetime</i> of the ICLConvolution object,
  it may not be necessary to copy the kernel deeply into an ICLConvolution-
  internal buffer. To make the ICLConvolution object just using a
  given kernel pointer, an additional flag <b>iBufferData</b> can be set
  in two Constructors.
  */
  class ICLConvolution : public ICLFilter{
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
    enum iclkernel { 
      kernelSobelX, /*!< sobel x filter */
      kernelSobelY, /*!< sobel y filter */
      kernelGauss3x3, /*!< 3x3-approximation of a Gaussian */
      kernelGauss5x5, /*!< 5x5-approximation of a Gaussian */
      kernelLaplace, /*!< approximation of the 2nd derivation */
      kernelCustom  /*!< used for all other user defined kernels */ 
    };
    
    /// Creates an ICLConvolution object with a fixed predefined filter type
    /** @param eKernel determines the filter type (kernelCustom is not allowed here!)
    */
    ICLConvolution(iclkernel eKernel);

    /// Creates an ICLConvolution object with given custom convolution kernel
    /** If the given kernel is of depth8u, then its data and size will be buffered
        as well in depth32f, as in 32-bit signed integer (depth32s) representation.
        Otherwise (kernel: depth32f) it is buffered in the internal depth32f buffer.
        If it is possible to use signed integer values also (if the given float
        buffer contains no floats with decimals) then an additional depth32s buffer
        is created, and filled with the casted values. In case of applying the
        filter on depth8u images, this will speed up performance.
        @param poKernel custom convolution kernel
    */
    ICLConvolution(ICLBase *poKernel);

    /// Creates an ICLConvolution object with the given custom kernel
    /** This constructor behaves essentially as the above one, up till the
        convolution kernel is given in POD (plain old data) types.
        Furthermore it has an additional parameter, whiches default may
        be overridden with "0". In this case no internal <i>snapshot</i>
        of the giver kernel data is made. This will speed up the construction
        time of the object rapidly. The user has to take care, that
        the kernel data is persistent as long as apply calls on the 
        ICLConvoluion objects are preformed. The given pointer is not
        owned by the ICLConvolution object, so it will not delete it
        at the destruction time.
        @param pfKernel convolution kernel data
        @param iW convolution kernel width
        @param iH convolution kernel height
        @param iBufferData flag that indicates, if given data should be 
                           buffered internally. By default given data will
                           be buffered.
    */
    ICLConvolution(iclfloat *pfKernel, int iW, int iH, int iBufferData=1);

    /// Creates an ICLConvolution object with the given custom kernel
    /** This constructor behaves essentially like the above one.
        @param piKernel convolution kernel data
        @param iW convolution kernel width
        @param iH convolution kernel height
        @param iBufferData flag that indicates, if given data should be 
                           buffered internally. By default given data will
                           be buffered.
    */
    ICLConvolution(int *piKernel, int iW, int iH,int iBufferData=1);

    /// Destructor
    virtual ~ICLConvolution();
    
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
    virtual void apply(ICLBase *poSrc, ICLBase *poDst);
    
    private:

    /// internal storage for the sobel-x filter kernel
    static int KERNEL_SOBEL_X[9];

    /// internal storage for the sobel-y filter kernel
    static int KERNEL_SOBEL_Y[9];

    /// internal storage for the gauss 3x3 filter kernel
    static float KERNEL_GAUSS_3x3[9];

    /// internal storage for the gauss 5x5 filter kernel
    static float KERNEL_GAUSS_5x5[25];

    /// internal storage for the Laplace filter kernel
    static int KERNEL_LAPLACE[9];
  
    /// internal storage for the kernels depth32f data
    float *pfKernel;
    
    /// internal storage for the kernels depth32s data
    int *piKernel;
    
    /// internal storage of the filter width
    int iW;

    /// internal storage of the filter height
    int iH;

    /// internal flag, that indicates, if the contained data is owned by this object
    bool bDeleteData;
    
    /// current kernel type
    iclkernel eKernel;
  };
  
}

#endif
