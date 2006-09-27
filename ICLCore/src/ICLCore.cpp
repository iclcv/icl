#include "ICLCore.h"
#include "Img.h"

using namespace std;

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
          
        case formatChroma:
          return 2;
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
        case formatChroma: return "chroma";
        default: return "undefined format";        
      }
  }
 
  format translateFormat(const string& sFormat)
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
      case 'c': return formatChroma;
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
       (*ppoDst)->setChannels(iChannelCount);
       (*ppoDst)->setFormat(eFormat);
       (*ppoDst)->resize(s);
    }

    if(r){
      (*ppoDst)->setROI(r);
    }else{
      (*ppoDst)->setFullROI();
    }
  }

  void ensureCompatible(ImgI **ppoDst, const ImgI *poSrc) {
     ICLASSERT_RETURN (poSrc);
     ensureCompatible(ppoDst,poSrc->getDepth(),
                      poSrc->getSize(),
                      poSrc->getFormat(),
                      poSrc->getChannels(),
                      poSrc->getROI());
  }

  void ensureCompatibleROI(ImgI **ppoDst, const ImgI *poSrc) {
     ICLASSERT_RETURN (poSrc);
     ensureCompatible(ppoDst,poSrc->getDepth(),
                      poSrc->getROISize(),
                      poSrc->getFormat(),
                      poSrc->getChannels());
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
