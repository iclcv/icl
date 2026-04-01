// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Macros.h>

namespace icl {
  namespace core{

    //forward declaration for the Image interface \ingroup TYPES
    class ImgBase;

    /// forward declaration of the Img-class \ingroup TYPES
    template<class T> class Img;

    /// typedef for 8bit integer images \ingroup TYPES
    using Img8u = Img<icl8u>;

    /// typedef for 16bit integer images \ingroup TYPES
    using Img16s = Img<icl16s>;

    /// typedef for 8bit integer images \ingroup TYPES
    using Img32s = Img<icl32s>;

    /// typedef for 32bit float images \ingroup TYPES
    using Img32f = Img<icl32f>;

    /// typedef for 64bit float images \ingroup TYPES
    using Img64f = Img<icl64f>;

    /// determines the pixel type of an image (8Bit-int or 32Bit-float) \ingroup TYPES
    enum depth{
      depth8u  = 0, /**< 8Bit unsigned integer values range {0,1,...255} */
      depth16s = 1, /**< 16Bit signed integer values */
      depth32s = 2, /**< 32Bit signed integer values */
      depth32f = 3, /**< 32Bit floating point values */
      depth64f = 4, /**< 64Bit floating point values */
      depthLast = depth64f
    };

    /// determines the color-format, that is associated with the images channels \ingroup TYPES
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


  #ifdef ICL_HAVE_IPP
    /// for scaling of Img images theses functions are provided \ingroup TYPES
    enum scalemode{
      interpolateNN=IPPI_INTER_NN,      /**< nearest neighbor interpolation */
      interpolateLIN=IPPI_INTER_LINEAR, /**< bilinear interpolation */
      interpolateRA=IPPI_INTER_SUPER    /**< region-average interpolation */
    };
  #else
    /// for scaling of Img images theses functions are provided \ingroup TYPES
    enum scalemode{
      interpolateNN,  /**< nearest neighbor interpolation */
      interpolateLIN, /**< bilinear interpolation */
      interpolateRA   /**< region-average interpolation */
    };
  #endif

    /// for flipping of images \ingroup TYPES
    enum axis{
  #ifdef ICL_HAVE_IPP
      axisHorz=ippAxsHorizontal, /**> horizontal image axis */
      axisVert=ippAxsVertical,   /**> vertical image axis */
      axisBoth=ippAxsBoth        /**> flip both axis */
  #else
      axisHorz, /**> horizontal image axis */
      axisVert, /**> vertical image axis */
      axisBoth  /**> flip both axis */
  #endif
    };
    /// getDepth<T> returns the depth enum associated to type T \ingroup GENERAL
    template<class T> inline depth getDepth();

    /** \cond */
#define ICL_INSTANTIATE_DEPTH(T)                                        \
    template<> inline depth getDepth<icl ## T>() { return depth ## T; }
    ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    /** \endcond */

    /// determines the count of channels for each color format \ingroup GENERAL
    ICLCore_API int getChannelsOfFormat(format fmt);

    /// return sizeof value for the given depth type \ingroup GENERAL
    ICLCore_API unsigned int getSizeOf(depth eDepth);

  } // namespace core
}
