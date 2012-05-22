/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/ColorFormatDecoder.h                     **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_COLOR_FORMAT_DECODER_H
#define ICL_COLOR_FORMAT_DECODER_H

#include <ICLCore/Img.h>
#include <ICLIO/FourCC.h>

#include <map>

namespace icl{
  
  /// The ColorFormatDecoder allows for easy conversion of special color formats to RGB
  /** If a capturing device does not support to provide RGB image directly, this
      Decoder is usually used to convert the compressed image data into a simple
      planar RGB image (Img8u). However, also other output depth value are supported. \n
      The decoding routine, is chosen by analyising the source data based on its
      <b>fourcc</b> color code (see: http://en.wikipedia.org/wiki/FourCC and 
      http://v4l2spec.bytesex.org/spec/c2030.htm for more details)
      
      \section EX ICL Specific Extensions
      For supporting the Myrmex Tactile Device, we added an extra
      FourCC code called "MYRM".

      @see FourCC
  */
  class ColorFormatDecoder{
    public:
    // conversion function type
    typedef void (*decoder_func)(const icl8u*,const Size&,ImgBase**,std::vector<icl8u>*);
    
    private:
    std::vector<icl8u> m_buffer; //!< internal buffer  
    std::map<icl32u,decoder_func> m_functions; //!< internal lookup for conversion functions
    ImgBase *m_dstBuf;  //!< optionally used output buffer
    
    public:
    /// create a new instance
    ColorFormatDecoder();
    
    /// Destructor
    ~ColorFormatDecoder();
    
    /// return whether a given format is supported
    inline bool supports(FourCC fourcc){
      return m_functions.find(fourcc.asInt()) != m_functions.end();
    }
    
    /// decodes a given data range to RGB
    void decode(FourCC fourcc, const icl8u *data, const Size &size, ImgBase **dst);

    /// decode, but use the internal buffer as output
    const ImgBase *decode(FourCC fourcc, const icl8u *data, const Size &size){
      decode(fourcc,data,size,&m_dstBuf);
      return m_dstBuf;
    }
    
  };
}

#endif
