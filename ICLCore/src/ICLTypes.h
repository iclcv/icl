#ifndef ICL_TYPES_H
#define ICL_TYPES_H

#ifdef WITH_IPP_OPTIMIZATION
#include <ipp.h>
#endif

namespace icl {
  
#ifdef WITH_IPP_OPTIMIZATION
  /// 32Bit floating point type for the ICL 
  typedef Ipp32f icl32f;

  /// 8Bit unsigned integer type for the ICL
  typedef Ipp8u icl8u;

#else
  /// 32Bit floating point type for the ICL 
  typedef float icl32f;

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
  typedef Img<icl32f> Img32f;

  /// determines the pixel type of an image (8Bit-int or 32Bit-float) 
  enum depth{
    depth8u  = 0, /**< 8Bit unsigned integer values range {0,1,...255} */
    depth32f = 1 /**< 32Bit floating point values */
  };
  
  /// determines the color-format, that is associated with the images channels 
  enum format{
    formatRGB, /**< (red,green,blue) colors pace */
    formatHLS, /**< (hue,lightness,saturation) color space (also know as HSI) */
    formatLAB, /**< (lightness,a*,b*) color space */
    formatYUV, /**< (Y,u,v) color space */
    formatGray, /**< n-channel gray image range of values is [0,255] as default */
    formatMatrix, /**< n-channel image without a specified color space. */
    formatChroma /**< 2 channel chromaticity color space */
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
