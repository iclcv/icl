#ifndef ICLIMAGE_COLOR_CONVERSION_H
#define ICLIMAGE_COLOR_CONVERSION_H

#include "Img.h"

namespace icl{

/**
This time 8 different color formats in 4 color spaces are suppored.
Color Spaces are:
- Single channel gray images
- RGB 
- HLS (Hue, Lightness, Saturation) also know als HSV-color model width different channel order

All Formats are suppored in the two Img-dephts float (32-Bit) and unsigned char (8Bit)
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
void iclcc(ImgI *poDst, ImgI *poSrc);
  

/// Convert an image of GRAY8-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromGray8(ImgI *poDst, ImgI *poSrc);

/// Convert an image of GRAY32-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromGray32(ImgI *poDst, ImgI *poSrc);

/// Convert an image of RGB8-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromRGB8(ImgI *poDst, ImgI *poSrc);

/// Convert an image of RGB32-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromRGB32(ImgI *poDst, ImgI *poSrc);

/// Convert an image of YUV8-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromYUV8(ImgI *poDst,ImgI *poSrc);

/// Convert an image of YUV32-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromYUV32(ImgI *poDst,ImgI *poSrc);

/// Convert an image of LAB8-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromLAB8(ImgI *poDst,ImgI *poSrc);

/// Convert an image of LAB32-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/

void convertFromLAB32(ImgI *poDst,ImgI *poSrc);
/// Convert an image of RGBA8-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromRGBA8(ImgI *poDst, ImgI *poSrc);

/// Convert an image of RGBA32-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromRGBA32(ImgI *poDst, ImgI *poSrc);

/// Convert an image of HLS8-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromHLS8(ImgI *poDst, ImgI *poSrc);

/// Convert an image of HLS32-Format to any other format
/**
@param poDst destination image
@param poSrc source image
*/
void convertFromHLS32(ImgI *poDst, ImgI *poSrc);

/// Convert an image in YUV420-Format to RGB8 Format (ippi accelerated)
/**
@param poDst destination image
@param pucSrc pointer to source data (data is in YUV420 Format, which is planar and which's U- and V-channel
has half X- and half Y-resolution. The data pointer has iW*iH*1.5 elements)
@param iW image width, associated with the source data pointer
@param iH image hieght, associated with the source data pointer
*/
void convertYUV420ToRGB8(Img8u* poDst, unsigned char *pucSrc, int iW, int iH);


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
