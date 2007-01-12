#ifndef ICL_TYPES_H
#define ICL_TYPES_H

#ifdef WITH_IPP_OPTIMIZATION
#include <ipp.h>
#endif

namespace icl {
  
#ifdef WITH_IPP_OPTIMIZATION
  /// 64Bit floating point type for the ICL
  typedef Ipp64f icl64f;

  /// 32Bit floating point type for the ICL 
  typedef Ipp32f icl32f;

  /// 32bit signed integer type for the ICL
  typedef Ipp32s icl32s;

  /// 16bit signed integer type for the ICL (range [-32767, 32768 ])
  typedef Ipp16s icl16s;
  
  /// 8Bit unsigned integer type for the ICL
  typedef Ipp8u icl8u;

#else
  /// 64Bit floating point type for the ICL
  typedef double icl64f;

  /// 32Bit floating point type for the ICL 
  typedef float icl32f;

  /// 32bit signed integer type for the ICL
  typedef int icl32s;
  
  /// 16bit signed integer type for the ICL (range [-32767, 32768 ])
  typedef short int icl16s;

  /// 8Bit unsigned integer type for the ICL 
  typedef unsigned char icl8u;

#endif

  //forward declaration for the Image interface
  class ImgBase;

  /// forward declaration of the Img-class
  template<class T> class Img;

  /// typedef for 8bit integer images
  typedef Img<icl8u> Img8u;

  /// typedef for 32bit float images
  typedef Img<icl16s> Img16s;

  /// typedef for 8bit integer images
  typedef Img<icl32s> Img32s;

  /// typedef for 32bit float images
  typedef Img<icl32f> Img32f;

  /// typedef for 64bit float images
  typedef Img<icl64f> Img64f;

  /// determines the pixel type of an image (8Bit-int or 32Bit-float) 
  enum depth{
    depth8u  = 0, /**< 8Bit unsigned integer values range {0,1,...255} */
    depth16s = 1, /**< 16Bit signed integer values */  
    depth32s = 2, /**< 32Bit signed integer values */
    depth32f = 3, /**< 32Bit floating point values */
    depth64f = 4, /**< 64Bit floating point values */
    depthLast = depth64f
  };
  
  /// determines the color-format, that is associated with the images channels 
  enum format{
    formatGray   = 0, /**< 1-channel gray image, range of values is [0,255] as default */
    formatRGB    = 1, /**< (red,green,blue) colors pace */
    formatHLS    = 2, /**< (hue,lightness,saturation) color space (also know as HSI) */
    formatYUV    = 3, /**< (Y,u,v) color space */
    formatLAB    = 4, /**< (lightness,a*,b*) color space */
    formatChroma = 5, /**< 2 channel chromaticity color space */
    formatMatrix = 6, /**< n-channel image without a specified color space. */
    formatLast = formatMatrix
  };
  

#ifdef WITH_IPP_OPTIMIZATION
  enum scalemode{
    interpolateNN=IPPI_INTER_NN,      /**< nearest neighbor interpolation */
    interpolateLIN=IPPI_INTER_LINEAR, /**< bilinear interpolation */
    interpolateRA=IPPI_INTER_SUPER    /**< region-average interpolation */
  };
#else
  /// for scaling of Img images theses functions are provided
  enum scalemode{
    interpolateNN,  /**< nearest neighbor interpolation */
    interpolateLIN, /**< bilinear interpolation */
    interpolateRA   /**< region-average interpolation */
  };
#endif

  /// for flipping of images
  enum axis{
#ifdef WITH_IPP_OPTIMIZATION
    axisHorz=ippAxsHorizontal, /**> horizontal image axis */
    axisVert=ippAxsVertical,   /**> vertical image axis */
    axisBoth=ippAxsBoth        /**> flip both axis */
#else
    axisHorz, /**> horizontal image axis */
    axisVert, /**> vertical image axis */
    axisBoth  /**> flip both axis */
#endif
  };
}

#endif //ICL_TYPES_H
