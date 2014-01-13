/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ColorFormatDecoder.h                   **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLIO/FourCC.h>

#include <map>

namespace icl{
  namespace io{
    
    /// The ColorFormatDecoder allows for easy conversion of special color formats to RGB
    /** If a capturing device does not support to provide RGB image directly, this
        Decoder is usually used to convert the compressed image data into a simple
        planar RGB image (core::Img8u). However, also other output core::depth value are supported. \n
        The decoding routine, is chosen by analyising the source data based on its
        <b>fourcc</b> color code (see: http://en.wikipedia.org/wiki/FourCC and 
        http://v4l2spec.bytesex.org/spec/c2030.htm for more details)
        
        \section SUP Supported FourCC codes

        * <b>GRAY, GREY or Y800</b> simple 8bit grayscale image (no version, copy only)
        * <b>YUYV</b> encodes 2 rgb-pixels in 4 bytes, ordered Y_1UY_2V, so the first
          pixel is created from Y_1, U and V and the 2nd pixel is created from Y_2, U, and V
        * <b>Y444</b> Simple interleaved YUV-format, data order: U_1,Y_1,V_2,U_2, ...
        * <b>YU12</b> Very common planar format where Y, U and V channels are packed
          in order Y,U,V. The special thing is here, that U and V have only half x- and 
          y-resolution
        * <b>Y10B</b> packed 10 bit gray-scale format (result is put into a 16bit (Img16s) image
        * <b>MYRM</b> Special non standard format used for the Myrmex Tactile Sensor
        * <b>RGGB, GBRG, GRBG, BGGR</b> Bayer filter formats, uncommonly used for webcams
          (note that bayer filters are often used with Firewire cameras, but in the DCGrabber
          backend, the core::BayerConverter is used automatically)
        * <b>MJPG</b> Motion jpeg. Here, each image frame actually contains binary encoded
          jpeg data
        
        \section EX ICL Specific Extensions
        For supporting the Myrmex Tactile Device, we added an extra
        FourCC code called "MYRM".
  
        @see FourCC
    */
    class ICL_IO_API ColorFormatDecoder{
      public:
      // conversion function type
      typedef void (*decoder_func)(const icl8u*,const utils::Size&,core::ImgBase**,std::vector<icl8u>*);
      
      private:
      std::vector<icl8u> m_buffer; //!< internal buffer  
      std::map<icl32u,decoder_func> m_functions; //!< internal lookup for conversion functions
      core::ImgBase *m_dstBuf;  //!< optionally used output buffer
      
      public:
      /// create a new instance
      ColorFormatDecoder();
      
      /// Destructor
      ~ColorFormatDecoder();
      
      /// return whether a given core::format is supported
      inline bool supports(FourCC fourcc){
        return m_functions.find(fourcc.asInt()) != m_functions.end();
      }
      
      /// decodes a given data range to RGB
      void decode(FourCC fourcc, const icl8u *data, const utils::Size &size, core::ImgBase **dst);
  
      /// decode, but use the internal buffer as output
      const core::ImgBase *decode(FourCC fourcc, const icl8u *data, const utils::Size &size){
        decode(fourcc,data,size,&m_dstBuf);
        return m_dstBuf;
      }
      
    };
  } // namespace io
}

