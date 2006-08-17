#include "ICLCore.h"
#include "Img.h"

namespace icl{
  
  ImgI *imgNew(depth eDepth, 
               const Size& s,
               format eFormat, 
               int iChannels,
               const Rect &oROI)
  {
    ImgI *poNew = 0;
    if(eDepth == depth8u)
      {
        poNew =  new Img8u(s,eFormat,iChannels);
      }
    else
      {
        poNew = new Img32f(s,eFormat,iChannels);
      } 
    
    if(oROI) poNew->setROI(oROI);
    return poNew;
  }

  int getChannelsOfFormat(format eFormat)
  {
    switch (eFormat)
      {
        case formatRGB:
        case formatHLS:
        case formatLAB:
        case formatYUV:
          return 3;
          break;
          
        case formatGray:
        case formatMatrix:
        default:
          return 1;
          break;
      }
  }

  string translateFormat(format eFormat)
  {
    switch(eFormat)
      {
        case formatRGB: return "rgb";
        case formatHLS: return "hls";
        case formatLAB: return "lab";
        case formatYUV: return "yuv";
        case formatGray: return "gray";
        case formatMatrix: return "matrix";
        default: return "undefined format";        
      }
  }
 
  format translateFormat(string sFormat)
  {
    if(sFormat.length()<=0)
      {
        ERROR_LOG("warning iclTranslateFormatString(string) got \"\"-string");
        return formatMatrix;
      }
    switch(sFormat[0])
    {
      case 'r': return formatRGB;
      case 'h': return formatHLS;
      case 'l': return formatLAB;
      case 'y': return formatYUV;
      case 'g': return formatGray;
      case 'm': return formatMatrix;
      default: return formatMatrix;
    }
  }
  
  void ensureDepth(ImgI ** ppoImage, depth eDepth)
  {
    if(!*ppoImage)
    {
      *ppoImage = imgNew(eDepth);
    }
    if((*ppoImage)->getDepth() != eDepth)
    {
      ImgI *poNew = imgNew(eDepth,
                           (*ppoImage)->getSize(),
                           (*ppoImage)->getFormat(),
                           (*ppoImage)->getChannels(),
                           (*ppoImage)->getROI());
      
      delete *ppoImage;
      *ppoImage = poNew;     
    }
  }
  
  void ensureCompatible(ImgI **ppoDst,
                        depth eDepth,
                        const Size &s,
                        format eFormat,
                        int iChannelCount,
                        const Rect &r)
  {
    if(!*ppoDst)
    {
      *ppoDst = imgNew(eDepth,s,eFormat,iChannelCount);
    }
    else 
    {
       ensureDepth(ppoDst,eDepth);
       if (iChannelCount < 0) iChannelCount = getChannelsOfFormat(eFormat);
       (*ppoDst)->setNumChannels(iChannelCount);
       (*ppoDst)->setFormat(eFormat);
       (*ppoDst)->resize(s);
    }

    if(r){
      (*ppoDst)->setROI(r);
    }else{
      (*ppoDst)->setFullROI();
    }
  }

  void ensureCompatible(ImgI **ppoDst, ImgI *poSrc) {
     if(!poSrc) { ERROR_LOG ("source image is NULL"); }
     ensureCompatible(ppoDst,poSrc->getDepth(),
                         poSrc->getSize(),
                         poSrc->getFormat(),
                         poSrc->getChannels(),
                         poSrc->getROI());
  }


  int getSizeOf(depth eDepth)
  {
    switch(eDepth)
    {
      case depth8u:
        return sizeof(icl8u);
        break;
      case depth32f:
        return sizeof(icl32f);
        break;
      default:
        ERROR_LOG("Unknown icl depth");
        return 0;
    }
  }
} //namespace
