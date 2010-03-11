/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLCC module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_CC_FUNCTIONS_H  
#define ICL_CC_FUNCTIONS_H

#include <ICLCore/Types.h>
#include <ICLUtils/Size.h>
#include <string>

namespace icl{
  
  /// Color conversion from src to the dst image
  /** All color conversions of the ICL are tackled by this function.
      see the ICLCC mainpage for its behavior, features and performance.
      The destination images size is adapted to the source images size,
      as cc does not provide implicit scaling; Use the Converter class,
      also located in the ICLCC package, for color conversion with implicit
      scaling instead.
  */
  void cc(const ImgBase *src, ImgBase *dst, bool roiOnly=false);
  
  /// returns whether a lookup table was already created for src and dst format
  /** @param srcFmt source format
      @param dstFmt destination format
  **/
  bool lut_available(format srcFmt, format dstFmt);  

  /// Internally creates a lookup table to accelerate conversion between given formats
  /** Take care: <b>Each LUT uses up to 48MB of system memory</b> 
      @param srcFmt source format
      @param dstFmt destination format
  **/
  void createLUT(format srcFmt, format dstFmt);
  
  /// releases the internal lookup table created with createLUT
  /**  @param srcFmt source format
       @param dstFmt destination format
  **/
  void releaseLUT(format srcFmt, format dstFmt);

  /// releases all lookup tables that were created with createLUT
  void releaseAllLUTs();
  
  /// Internal used type, that describes an implementation type of a specific color conversion function
  enum ccimpl{
    ccAvailable   = 0, /**< conversion is supported natively/directly */
    ccEmulated    = 1, /**< conversion is supported using the bridge format RGB */
    ccAdapted     = 2, /**< conversion is actually not possible but although performed ( like XXX to matrix )*/
    ccUnavailable = 3, /**< conversion is not implemented yet, but possible */
    ccImpossible  = 4  /**< conversion does not make sense (like croma to RGB )*/
  };
  
  /// translates a ccimpl enum into a string representation
  /** The returned string for ccAvailable is "available" (...) */
  std::string translateCCImpl(ccimpl i);

  /// translates the string represenation of a
  ccimpl translateCCImlp(const std::string &s);

  /// returns the ccimpl state to a conversion from srcFmt to dstFmt
  ccimpl cc_available(format srcFmt, format dstFmt);


  /// Convert an image in YUV420-format to RGB8 format (ippi accelerated)
  /**
  @param poDst destination image
  @param pucSrc pointer to source data (data is in YUV420 format, which is planar and which's U- and V-channel
  has half X- and half Y-resolution. The data pointer has iW*iH*1.5 elements)
  @param s image size 
  */
  void convertYUV420ToRGB8(const unsigned char *pucSrc, const Size &s, Img8u* poDst);

  /// Convert an 4 channel Img8u into Qts ARGB32 interleaved format 
  /** @param pucDst destination data pointer of size
  poSrc->getDim()*4
  @param poSrc source image with 4 channels
  */ 
  //void convertToARGB32Interleaved(const Img8u *poSrc, unsigned char *pucDst);
  
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
  //void convertToARGB32Interleaved(const Img32f *poSrc, Img8u *poBuffer, unsigned char *pucDst);
  
  
  /// Converts a planar Img<S> images ROI  into its interleaved representations by mixing the channels
  /** This function is highly optimized, because it is needed whenever we need interleaved images
      @param src source image image
      @param dst destination data pointer
      @param dstLineStep optinal linestep of the destination image. This must be given, if it differs from
                         the source images lineStep multiplied by the source images channel count
  */
  template<class S, class D>
  void planarToInterleaved(const Img<S> *src, D* dst, int dstLineStep=-1);
  
  /// Converts interleaved image data into planar representation 
  /** The source data is transformed into the destination images ROI 
      @param src data pointer
      @param dst image pointer
      @param srcLineStep optionally given src linestep size
  */
  template<class S, class D>
  void interleavedToPlanar(const S *src, Img<D> *dst, int srcLineStep=-1);

  /// converts given (r,g,b) pixel into the yuv format
  void cc_util_rgb_to_yuv(const icl32s r, const icl32s g, const icl32s b, icl32s &y, icl32s &u, icl32s &v);
  
  /// converts given (y,u,v) pixel into the rgb format
  void cc_util_yuv_to_rgb(const icl32s y,const icl32s u,const icl32s v, icl32s &r, icl32s &g, icl32s &b);
  
  /// converts given (r,g,b) pixel into the hls format
  void cc_util_rgb_to_hls(const icl32f r255,const icl32f g255,const icl32f b255, icl32f &h, icl32f &l, icl32f &s);

  /// converts given (h,l,s) pixel into the rgb format
  void cc_util_hls_to_rgb(const icl32f h255, const icl32f l255, const icl32f sl255, icl32f &r, icl32f &g, icl32f &b);

  /// converts given (r,g,b) pixel into the RG-chroma format
  void cc_util_rgb_to_chroma(const icl32f r, const icl32f g, const icl32f b, icl32f &chromaR, icl32f &chromaG);
}

#endif
