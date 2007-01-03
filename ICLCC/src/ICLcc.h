#ifndef ICLIMAGE_COLOR_CONVERSION_H
#define ICLIMAGE_COLOR_CONVERSION_H

#include <Img.h>

namespace icl{

  /** ICLCC the Color Conversion Package of the ICL
  The main class of the ICLCC package is The <b>Converter</b> which 
  provides a very convenient color conversion mechanism.
  \sa Converter

  In addition to the Converter class, direct conversion routines are
  defined, and accessible directy in the icl namespace.
  \sa icl
  */

/**
This time 8 different color formats in 4 color spaces are suppored.
Color Spaces are:
- Single channel gray images
- RGB 
- HLS (Hue, Lightness, Saturation) also know als HSV-color model width different channel order

All formats are suppored in the two Img-dephts float (32-Bit) and unsigned char (8Bit)
*/


///@name Color Conversion functions
//@{

/// Color-Conversion function
/**
The image formats are not checked, so an error will occur, if poDst or
poSrc have not compatible formats.
@param poDst destination image
@param poSrc source image
*/
void iclcc(ImgBase *poDst, ImgBase *poSrc);
  

/// Convert an image of GRAY8-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromGray8(ImgBase *poDst, ImgBase *poSrc);

/// Convert an image of GRAY32-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromGray32(ImgBase *poDst, ImgBase *poSrc);

/// Convert an image of RGB8-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromRGB8(ImgBase *poDst, ImgBase *poSrc);

/// Convert an image of RGB32-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromRGB32(ImgBase *poDst, ImgBase *poSrc);

/// Convert an image of YUV8-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromYUV8(ImgBase *poDst,ImgBase *poSrc);

/// Convert an image of YUV32-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromYUV32(ImgBase *poDst,ImgBase *poSrc);

/// Convert an image of LAB8-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromLAB8(ImgBase *poDst,ImgBase *poSrc);

/// Convert an image of LAB32-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/

void convertFromLAB32(ImgBase *poDst,ImgBase *poSrc);
/// Convert an image of RGBA8-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromRGBA8(ImgBase *poDst, ImgBase *poSrc);

/// Convert an image of RGBA32-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromRGBA32(ImgBase *poDst, ImgBase *poSrc);

/// Convert an image of HLS8-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromHLS8(ImgBase *poDst, ImgBase *poSrc);

/// Convert an image of HLS32-format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromHLS32(ImgBase *poDst, ImgBase *poSrc);

/// Convert an image in YUV420-format to RGB8 format (ippi accelerated)
/**
@param poDst destination image
@param pucSrc pointer to source data (data is in YUV420 format, which is planar and which's U- and V-channel
has half X- and half Y-resolution. The data pointer has iW*iH*1.5 elements)
@param s image size 
*/
void convertYUV420ToRGB8(Img8u* poDst, unsigned char *pucSrc, const Size &s);

/// Convert an 4 channel Img8u into Qts ARGB32 interleaved format 
/** @param pucDst destination data pointer of size
                  poSrc->getDim()*4
    @param poSrc source image with 4 channels
*/ 
 void convertToARGB32Interleaved(unsigned char *pucDst, Img8u *poSrc);

 /// Convert an 4 channel Img32f into Qts ARGB32 interleaved format 
 /** This function will first convert the given Img32f poSrc into the 
     buffer image poBuffer. Then it will call the above method, to convert the
     buffer data into pucDst. If the buffer is not valid, the method will
     return immediately.
     @param pucDst destination data pointer of size
                   poSrc->getDim()*4
     @param poSrc source image with 4 channels
     @param poBuffer buffer to use for internal depth conversion.
 */ 
 void convertToARGB32Interleaved(unsigned char *pucDst, Img32f *poSrc, Img8u *poBuffer);


 /// Converts a planar Img<S> image into its interleaved representations by mixing the channels
 /** This function is highly optimized, because it is needed whenever we need interleaved images
 
 */
 template<class S, class D>
 void planarToInterleaved(const Img<S> *src, D* dst);

 template<class S, class D>
 void interleavedToPlanar(const S *src,const Size &srcSize, int srcChannels,  Img<D> *dst);


 //void planarToInterleaved(const Img8u *src, icl8u *dst,const Point ROIoffset =Point(0,0));
 //void planarToInterleaved(const Img32f *src, icl32f *dst,const Point ROIoffset =Point(0,0));

 //void interleavedToPlanar(const icl8u *src, const Size &srcSize, int srcChannels,  Img8u *dst);
 //void interleavedToPlanar(const icl32f *src, const Size &srcSize, int srcChannels,  Img32f *dst);

//@}
///@name Function for Converting into HLS-Colorspace (Hue, Lightnes, Saturation)
//@{

/// convert a single (h,l,s)-Tupel into r,g,b values (slow!)
/**
call init_table before calling this function
@param r source red value
@param g source green value
@param b source blue value
@param h destination reference for the extracted hue-value
@param l destination reference for the extracted lightness-value
@param s destination reference for the extracted saturation-value
*/
void rgb_to_hls(const unsigned char &r,const unsigned char &g,const unsigned char &b, 
                unsigned char &h, unsigned char &l, unsigned char &s);

/// convert a single (h,l,s)-Tupel into r,g,b values (slow!)
/**
call init_table before calling this function
@param r source red value
@param g source green value
@param b source blue value
@param h destination reference for the extracted hue-value
@param l destination reference for the extracted lightness-value
@param s destination reference for the extracted saturation-value
*/
void rgb_to_hls(const float &r,const float &g,const float &b, 
                unsigned char &h, unsigned char &l, unsigned char &s);

/// convert a single (h,l,s)-Tupel into r,g,b values (slow!)
/**
call init_table before calling this function
@param r source red value
@param g source green value
@param b source blue value
@param h destination reference for the extracted hue-value
@param l destination reference for the extracted lightness-value
@param s destination reference for the extracted saturation-value
*/
void rgb_to_hls(const unsigned char &r,const unsigned char &g,const unsigned char &b, 
                float &h, float &l, float &s);

/// convert a single (h,l,s)-Tupel into r,g,b values (slow!)
/**
call init_table before calling this function
@param r source red value
@param g source green value
@param b source blue value
@param h destination reference for the extracted hue-value
@param l destination reference for the extracted lightness-value
@param s destination reference for the extracted saturation-value
*/
void rgb_to_hls(const float &r,const float &g,const float &b, 
                float &h, float &l, float &s);

/// initialize hls-lookup table
/**
Calculates an intern lookup table for Hue values. If this function is not called
all rgb_to_hls function will calculate wrong hue-values
*/
void init_table();
//@}

}

#endif
