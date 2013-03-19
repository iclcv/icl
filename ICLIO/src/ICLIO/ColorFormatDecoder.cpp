/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ColorFormatDecoder.cpp                 **
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

#include <ICLCore/CCFunctions.h>
#include <ICLIO/ColorFormatDecoder.h>
#include <ICLIO/JPEGDecoder.h>
#include <ICLUtils/StringUtils.h>

#include <ICLIO/MyrmexDecoder.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
  
    namespace color_format_converter{

      // can be used to grab kinect IR-image via V4L2
      void y10b(const icl8u *rawData,const Size &size, ImgBase **dst, std::vector<icl8u> *buffer=0){
        ensureCompatible(dst,depth16s,size,formatGray);
        icl16s *d = (*dst)->as16s()->begin(0);
        int dim = size.getDim()+1;

	uint32_t buf = 0;
	int bs = 0;
	while(--dim) {
          while (bs < 10) {
            buf = (buf << 8) | *rawData++;
            bs += 8;
          }
          bs -= 10;
          *d++ = (buf >> bs) & 1023;
	}
      }
  
      void myrm(const icl8u *rawData,const Size &size, ImgBase **dst, std::vector<icl8u> *buffer=0){
        static MyrmexDecoder dec;
        dec.decode(reinterpret_cast<const icl16s*>(rawData), size, dst);
      }
        
      void gray(const icl8u *rawData,const Size &size, ImgBase **dst, std::vector<icl8u> *buffer=0){
        ensureCompatible(dst,depth8u,size,formatGray);
        Img8u &image = *(*dst)->as8u();
  
        (void)buffer;
        Img8u tmp(size,1,std::vector<icl8u*>(1,const_cast<icl8u*>(rawData)),false);
        cc(&tmp,&image);
      }
      
      
      void y444(const icl8u* rawData, const Size &size, ImgBase **dst, std::vector<icl8u> *buffer){
        ensureCompatible(dst,depth8u,size,formatRGB);
        Img8u &image = *(*dst)->as8u();
  
  #ifdef HAVE_IPP
        if(buffer){
          buffer->resize(size.getDim()*3);
          ippiYUVToRGB_8u_C3R(rawData,3*size.width,buffer->data(),3*size.width,size);
          interleavedToPlanar(buffer->data(),&image);
          return;
        }
  #endif
  
        icl8u *dstR = image.begin(0);
        icl8u *dstG = image.begin(1);
        icl8u *dstB = image.begin(2);
        
        const icl8u *pSrcEnd = rawData+size.getDim()*3;
        
        // data order uyv uyv ...
        for(const icl8u *pSrc = rawData; pSrc <pSrcEnd ; pSrc+=3){
          int tr,tg,tb;
          cc_util_yuv_to_rgb(pSrc[1],pSrc[0],pSrc[2],tr,tg,tb);
          *dstR++ = tr;
          *dstG++ = tg;
          *dstB++ = tb;
        }
      }
      
      // uses ipp if available and if buf is not null
      void yuyv(const icl8u* yuyv, const Size &size, ImgBase **dst, std::vector<icl8u> *buf=0){
        ensureCompatible(dst,depth8u,size,formatRGB);
        Img8u &image = *(*dst)->as8u();
        /*
  #ifdef HAVE_IPP
        if(buf){
          // 3Times faster on -O4
          buf->resize(size.getDim()*3);
          ippiCbYCr422ToRGB_8u_C2C3R(yuyv,2*size.width, buf->data(),3*size.width, size);
          interleavedToPlanar(buf->data(),&image);
          return;
        }
  #endif
            */
        // interleaved order yuyv
        icl8u *r = image.begin(0), *g = image.begin(1), *b=image.begin(2);
        const int dim = image.getDim()/2;
        for(int i=0;i<dim;++i){
          int tr,tg,tb;
          cc_util_yuv_to_rgb(yuyv[0], yuyv[1], yuyv[3],tr,tg,tb);
          r[2*i] = tr;
          g[2*i] = tg;
          b[2*i] = tb;
          
          cc_util_yuv_to_rgb(yuyv[2], yuyv[1], yuyv[3],tr,tg,tb);
          r[2*i+1] = tr;
          g[2*i+1] = tg;
          b[2*i+1] = tb;
          
          yuyv += 4;
        }
      }
  
  #ifdef HAVE_LIBJPEG    
      void mjpg(const icl8u* data, const Size &size, ImgBase **dst, std::vector<icl8u> *buf = 0){
        try{
          // naive check for a correct jpeg file:
          const unsigned char *p = data;
          ICLASSERT_THROW(*p++ == 0xFF,1); // SOI Marker
          ICLASSERT_THROW(*p++ == 0xD8,2);
          JPEGDecoder::decode(data,4*size.getDim(),dst);
        }catch(std::exception &ex){
          WARNING_LOG("error decoding motion JPEG : " + str(ex.what()) );
        }catch(...){
          WARNING_LOG("error decoding motion JPEG");
        }
      }
  #endif
      void yu12(const icl8u* data, const Size &size, ImgBase **dst, std::vector<icl8u>*){
        ensureCompatible(dst,depth8u,size,formatRGB);
        convertYUV420ToRGB8(data,size,(*dst)->as8u());
      }
      
    }
    ColorFormatDecoder::ColorFormatDecoder():m_dstBuf(0){
      m_functions[FourCC("GRAY").asInt()] = color_format_converter::gray;
      m_functions[FourCC("Y800").asInt()] = color_format_converter::gray;
      m_functions[FourCC("GREY").asInt()] = color_format_converter::gray;
      m_functions[FourCC("YUYV").asInt()] = color_format_converter::yuyv;
      m_functions[FourCC("Y444").asInt()] = color_format_converter::y444;
      m_functions[FourCC("YU12").asInt()] = color_format_converter::yu12;
      m_functions[FourCC("MYRM").asInt()] = color_format_converter::myrm;
      m_functions[FourCC("Y10B").asInt()] = color_format_converter::y10b;
  
  #ifdef HAVE_LIBJPEG
      m_functions[FourCC("MJPG").asInt()] = color_format_converter::mjpg;
  #endif
  
    }
    ColorFormatDecoder::~ColorFormatDecoder(){
      ICL_DELETE(m_dstBuf);
    }
    
    void ColorFormatDecoder::decode(FourCC fourcc, const icl8u *data, const Size &size, ImgBase **dst){
      std::map<icl32u,decoder_func>::iterator it = m_functions.find(fourcc.asInt());
      if(it == m_functions.end()) throw ICLException("ColorFormatDecoder::unable to convert given format " + fourcc.asString());
  
      it->second(data,size,dst,&m_buffer);
    }
  } // namespace io
}
