#ifndef ICLCORE_H
#define ICLCORE_H

#include "ICLMacros.h"

/// The ICL-namespace
/**
This namespace is dedicated for ICLCore- and
all additional Computer-Vision packages, that 
are based on the ICLCore classes.
**/
namespace ICL {
  
  /// 32Bit floating point type for the ICL 
  typedef float iclfloat;

  /// 8Bit unsigned integer type for the ICL 
  typedef unsigned char iclbyte;
  
  /// determines the pixeltype of an image (8Bit-int or 32Bit-float) 
  enum icldepth{
    depth8u, /// 8Bit unsigned integer values range {0,1,...255}
    depth32f /// 32Bit floatingpoint values 
  };
  
  /// determines the color-format, that is assiciated with the images channels 
  enum iclformat{
    formatRGB, /// (red,green,blue)-colorspace
    formatHLS, /// (hue,lightness,saturation)-colorspace (also know as HSI)
    formatLAB, /// (lightness,a*,b*)-colorspace
    formatYUV, /// (Y,u,v)-colorspace
    formatGray, /// n-channel gray image range of values is [0,255] as default
    formatMatrix /// n-channel image without a specified colorspace.
  };

  /// for scaling of ICL images theses functions are provided
  enum iclscalemode{
    interpolateNon, /// no interpolation (image will become black)
    interpolateNN, /// nearest neightbour interpolation
    interpolateBL, /// bilinear interpolation
    interpolateAV /// region-average interpolation
  };
  /* {{{ Global functions */

  /// determines the count of channels, for each colorformat
  /** @param eFormat source format which channel count should be returned
      @return channel count of format eFormat
  **/
  int iclGetChannelsOfFormat(iclformat eFormat);

  /// call iclGetDepth<T> inside of an ICL function to getAssociated Depth as int
  /**
  @return depth associated with the Type value
  **/
  template<class T> 
  static icldepth iclGetDepth(){
    return depth8u;
  }

  /// specialized function for depth8u
  template<> 
  static icldepth iclGetDepth<iclbyte>(){
    return depth8u;
  }
  
  /// specialized function for depth32f
  template<> 
  static icldepth iclGetDepth<iclfloat>(){
    return depth32f;
  }
  
}

#endif
