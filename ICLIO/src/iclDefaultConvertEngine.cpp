#include "iclDefaultConvertEngine.h"
#include "iclUnicapDevice.h"
#include <string>
#include <iclImg.h>
#include <iclCC.h>

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
      // do something else !!!
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
#ifdef WITH_IPP_OPTIMIZATION 
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

      icl8u *dstR = (*ppoDst)->asImg<icl8u>()->getData(0);
      icl8u *dstG = (*ppoDst)->asImg<icl8u>()->getData(1);
      icl8u *dstB = (*ppoDst)->asImg<icl8u>()->getData(2);
      
      const icl8u *pSrcEnd = rawData+size.getDim()+size.getDim()/2;
      for(const icl8u *pSrc = rawData; pSrc <pSrcEnd ;){
        icl8u y1 = *pSrc++;
        icl8u u = *pSrc++;
        icl8u y2 = *pSrc++;
        icl8u v = *pSrc++;

        yuv_to_rgb(y1,u,v,*dstR++,*dstG++,*dstB++);
        yuv_to_rgb(y2,u,v,*dstR++,*dstG++,*dstB++);     
      }
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


