#include "ICLCore.h"
#include "ICL.h"

namespace icl{
  
  ICLBase *iclNew(icldepth eDepth, int iWidth, int iHeight, iclformat eFormat, int iChannels)
  {
    if(eDepth == depth8u)
      {
        return new ICL8u(iWidth,iHeight,eFormat,iChannels);
      }
    else
      {
        return new ICL32f(iWidth,iHeight,eFormat,iChannels);
      }
  }

  int iclGetChannelsOfFormat(iclformat eFormat)
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
          return 1;
          break;
          
        case formatMatrix:
          return 1;
          break;
          
        default:
          return 1;
      }
  }

  string iclTranslateFormat(iclformat eFormat)
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
 
  iclformat iclTranslateFormat(string sFormat)
  {
    if(sFormat.length()<=0)
      {
        printf("warning iclTranslateFormatString(string) got \"\"-string\n ");
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

  void iclEnsureDepth(ICLBase ** ppoImage, icldepth eDepth)
  {
    if(!*ppoImage)
      {
        *ppoImage = iclNew(eDepth,1,1,formatMatrix);
      }
    if((*ppoImage)->getDepth() != eDepth)
      {
        ICLBase *poNew = iclNew(eDepth,
                                (*ppoImage)->getWidth(),
                                (*ppoImage)->getHeight(),
                                (*ppoImage)->getFormat(),
                                (*ppoImage)->getChannels());
        
        delete *ppoImage;
        *ppoImage = poNew;     
      }
  }

  void iclEnsureCompatible(ICLBase **ppoDst, ICLBase *poSrc)
  {
    if(!poSrc)
      {
        printf("error in iclEnsureCompatible: source image is NULL!\n"); exit(-1);
      }
    if(!*ppoDst)
      {
        *ppoDst = iclNew(poSrc->getDepth(),
                         poSrc->getWidth(),
                         poSrc->getHeight(),
                         poSrc->getFormat(),
                         poSrc->getChannels());
      }
    iclEnsureDepth(ppoDst,poSrc->getDepth());
    (*ppoDst)->setNumChannels(poSrc->getChannels());
    (*ppoDst)->setFormat(poSrc->getFormat());
    (*ppoDst)->resize(poSrc->getWidth(),poSrc->getHeight());
  }
}
