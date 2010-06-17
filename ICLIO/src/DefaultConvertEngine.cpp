/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/DefaultConvertEngine.cpp                     **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
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

#include <ICLIO/DefaultConvertEngine.h>
#include <ICLIO/UnicapDevice.h>
#include <string>
#include <ICLCore/Img.h>
#include <ICLCC/CCFunctions.h>


#include <ICLIO/File.h>
#include <ICLIO/FileGrabber.h>
#include <ICLIO/JPEGDecoder.h>

using namespace std;

namespace icl{

  namespace {
    inline void yuv_to_rgb(const icl32s y,const icl32s u,const icl32s v, icl8u &r, icl8u &g, icl8u &b){
      // {{{ open
      icl32s u2 = 14343*u - 1828717; 
      icl32s v2 = 20231*v - 2579497; 
      
      r = clipped_cast<icl32s,icl8u>(y +  ( ( 290 * v2 ) >> 22 ));
      g = clipped_cast<icl32s,icl8u>(y -  ( ( 100  * u2 + 148 * v2) >> 22 ));
      b = clipped_cast<icl32s,icl8u>(y +  ( ( 518 * u2 ) >> 22 ));
    } 
  }
  
  bool DefaultConvertEngine::isAbleToProvideParams(const ImgParams &desiredParams, depth desiredDepth) const{
    UnicapFormat f = m_poDevice->getCurrentUnicapFormat();
    if(desiredDepth == depth8u && desiredParams.getFormat() == formatRGB && desiredParams.getSize() == f.getSize() ){
      return true;
    }else{
      return false;
    }
  }
 
  void DefaultConvertEngine::cvt(const icl8u *rawData, const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst){
    UnicapFormat f = m_poDevice->getCurrentUnicapFormat();
    string fourcc = f.getFourCC();
    Size size = f.getSize();

    if(desiredParams.getFormat() == formatYUV){
      // do something else !!! .. something faster ??
    }
    
    if(fourcc == "Y444"){ //  YUV444 size 160x120 ORDER: U Y V
      ensureCompatible(ppoDst,depth8u,size,formatRGB);
      
      icl8u *dstR = (*ppoDst)->asImg<icl8u>()->getData(0);
      icl8u *dstG = (*ppoDst)->asImg<icl8u>()->getData(1);
      icl8u *dstB = (*ppoDst)->asImg<icl8u>()->getData(2);
      
      const icl8u *pSrcEnd = rawData+size.getDim()*3;
      for(const icl8u *pSrc = rawData; pSrc <pSrcEnd ;){
        icl8u u = *pSrc++; // nice order !
        icl8u y = *pSrc++;
        icl8u v = *pSrc++;
        yuv_to_rgb(y,u,v,*dstR++,*dstG++,*dstB++);
      }
      
    }else if(fourcc == "UYVY"){ //  YUV422 size = 320x240 || 640x480 ORDER: U Y1 V Y2
      ensureCompatible(ppoDst,depth8u,size,formatRGB);
#ifdef HAVE_IPP
      // 3Times faster on -O4
      m_oCvtBuf.resize(size.getDim()*3);
      ippiCbYCr422ToRGB_8u_C2C3R(rawData,2*size.width, m_oCvtBuf.data(),3*size.width, size);
      interleavedToPlanar(m_oCvtBuf.data(),(*ppoDst)->asImg<icl8u>());
#else
      const icl8u *s = rawData;
      register int width = size.width;
      
      register int cnt = size.getDim();
      register int col = 0;
      register int row = 0;
      register int pixpos = 0;
      register int tar_pos = 0;
      icl8u *dstR = (*ppoDst)->asImg<icl8u>()->getData(0);
      icl8u *dstG = (*ppoDst)->asImg<icl8u>()->getData(1);
      icl8u *dstB = (*ppoDst)->asImg<icl8u>()->getData(2);
      
      for (int i=0; i<cnt; i+=2) {
        pixpos = i*2;
        if(col == width) {
          col -=width;
          row++;
        }
        tar_pos = (row * width) + (width - col);
        yuv_to_rgb(s[pixpos+1],s[pixpos],s[pixpos+2],dstR[cnt-tar_pos],dstG[cnt-tar_pos],dstB[cnt-tar_pos]);
        yuv_to_rgb(s[pixpos+3],s[pixpos],s[pixpos+2],dstR[cnt-tar_pos+1],dstG[cnt-tar_pos+1],dstB[cnt-tar_pos+1]);
        col+=2;
      }
#endif
    }else if(fourcc == "YUYV"){
      ensureCompatible(ppoDst,depth8u,size,formatRGB);

#ifdef HAVE_IPP
      // 3Times faster on -O4
      m_oCvtBuf.resize(size.getDim()*3);
      //ippiCbYCr422ToRGB_8u_C2C3R(rawData,2*size.width, m_oCvtBuf.data(),3*size.width, size);
      ippiYCbCr422ToRGB_8u_C2C3R(rawData,2*size.width, m_oCvtBuf.data(),3*size.width, size);
      interleavedToPlanar(m_oCvtBuf.data(),(*ppoDst)->asImg<icl8u>());
#else
      icl8u *dstR = (*ppoDst)->asImg<icl8u>()->getData(0);
      icl8u *dstG = (*ppoDst)->asImg<icl8u>()->getData(1);
      icl8u *dstB = (*ppoDst)->asImg<icl8u>()->getData(2);
      
      const icl8u *pSrcEnd = rawData+2*size.getDim();
      for(const icl8u *pSrc = rawData; pSrc <pSrcEnd ;){
        icl8u y1 = *pSrc++;
        icl8u u = *pSrc++;
        icl8u y2 = *pSrc++;
        icl8u v = *pSrc++;

        yuv_to_rgb(y1,u,v,*dstR++,*dstG++,*dstB++);
        yuv_to_rgb(y2,u,v,*dstR++,*dstG++,*dstB++);     
      }
#endif
    }else if(fourcc == "Y411"){// YUV411 size = 640x480 ORDER: U Y1 Y2 V Y3 Y4
      ensureCompatible(ppoDst,depth8u,size,formatRGB);

      icl8u *dstR = (*ppoDst)->asImg<icl8u>()->getData(0);
      icl8u *dstG = (*ppoDst)->asImg<icl8u>()->getData(1);
      icl8u *dstB = (*ppoDst)->asImg<icl8u>()->getData(2);
      
      const icl8u *pSrcEnd = rawData+size.getDim()+size.getDim()/2;
      for(const icl8u *pSrc = rawData; pSrc <pSrcEnd ;){
        icl8u u = *pSrc++;
        icl8u y1 = *pSrc++;
        icl8u y2 = *pSrc++;
        icl8u v = *pSrc++;        
        icl8u y3 = *pSrc++;
        icl8u y4 = *pSrc++;

        yuv_to_rgb(y1,u,v,*dstR++,*dstG++,*dstB++);
        yuv_to_rgb(y2,u,v,*dstR++,*dstG++,*dstB++);     
        yuv_to_rgb(y3,u,v,*dstR++,*dstG++,*dstB++);
        yuv_to_rgb(y4,u,v,*dstR++,*dstG++,*dstB++);
      }
    }else if(fourcc == "MJPG"){
#ifdef HAVE_LIBJPEG
      try{
        // naive check for a correct jpeg file:
        const unsigned char *p = rawData;
        ICLASSERT_THROW(*p++ == 0xFF,1); // SOI Marker
        ICLASSERT_THROW(*p++ == 0xD8,2);
        JPEGDecoder::decode(rawData,4*size.getDim(),ppoDst);
      }catch(...){
        ensureCompatible(ppoDst,depth8u,size,formatRGB);        
      }
      return;
#else
      DEBUG_LOG("Motion-JPEG is not supported without jpeg-support!\n"
                "You need to recompile the ICL with libjpeg support");
#endif

    }else if(fourcc == "Y800" || fourcc == "GREY" || fourcc == "GRAY" ){// Gray8Bit size = 640x480
      ensureCompatible(ppoDst,depth8u,size,formatGray);
      copy(rawData,rawData+size.getDim(),(*ppoDst)->asImg<icl8u>()->getData(0));
    }else if(fourcc == "BGR3"){
      ensureCompatible(ppoDst,depth8u,size,formatRGB);
      icl8u *dstR = (*ppoDst)->asImg<icl8u>()->getData(0);
      icl8u *dstG = (*ppoDst)->asImg<icl8u>()->getData(1);
      icl8u *dstB = (*ppoDst)->asImg<icl8u>()->getData(2);
      
      const icl8u *r = rawData;
      const icl8u *rEnd = r + size.getDim()*3;
      while(r<rEnd){
        *dstB++ = *r++;
        *dstG++ = *r++;
        *dstR++ = *r++;
      }
    }else if(fourcc == "BGR4"){
      ensureCompatible(ppoDst,depth8u,size,formatRGB);
      icl8u *dstR = (*ppoDst)->asImg<icl8u>()->getData(0);
      icl8u *dstG = (*ppoDst)->asImg<icl8u>()->getData(1);
      icl8u *dstB = (*ppoDst)->asImg<icl8u>()->getData(2);
      
      const icl8u *r = rawData;
      const icl8u *rEnd = r + size.getDim()*4;
      while(r<rEnd){
        *dstB++ = *r++;
        *dstG++ = *r++;
        *dstR++ = *r++;
        r++;
      }
    }else if(fourcc == "RGB4"){
      ensureCompatible(ppoDst,depth8u,size,formatRGB);
      icl8u *dstR = (*ppoDst)->asImg<icl8u>()->getData(0);
      icl8u *dstG = (*ppoDst)->asImg<icl8u>()->getData(1);
      icl8u *dstB = (*ppoDst)->asImg<icl8u>()->getData(2);
      
      const icl8u *r = rawData;
      const icl8u *rEnd = r + size.getDim()*4;
      while(r<rEnd){
        *dstR++ = *r++;
        *dstG++ = *r++;
        *dstB++ = *r++;
        r++;
      }
    }
  }
}


