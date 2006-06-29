#include "ICLCore.h"
#include "ICL.h"

namespace ICL{
  
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

  void iclEnsureDepth(ICLBase ** ppoImage, icldepth eDepth)
  {
    if(!*ppoImage)
      {
        printf("warning iclEnshureDepth got NULL-image pointer \n");
        return;
      }
    if((*ppoImage)->getDepth() == eDepth)
      {
        return;
      }
    if(eDepth == depth8u)
      {
        ICL8u *poNewImage = new ICL8u((*ppoImage)->getWidth(),
                                      (*ppoImage)->getHeight(),
                                      (*ppoImage)->getFormat(),
                                      (*ppoImage)->getChannels());
        delete *ppoImage;
        *ppoImage = poNewImage;
      }
    else
      {
        ICL32f *poNewImage = new ICL32f((*ppoImage)->getWidth(),
                                        (*ppoImage)->getHeight(),
                                        (*ppoImage)->getFormat(),
                                        (*ppoImage)->getChannels());
        delete *ppoImage;
        *ppoImage = poNewImage;
      }

  }

  void iclEnsureCompatible(ICLBase **ppoDst, ICLBase *poSrc)
  {
    if(!*ppoDst)
      {
        printf("warning iclEnshureCompatible got NULL-image pointer \n");
        return;
      }
    iclEnsureDepth(ppoDst,poSrc->getDepth());
    (*ppoDst)->setNumChannels(poSrc->getChannels());
    (*ppoDst)->setFormat(poSrc->getFormat());
    (*ppoDst)->resize(poSrc->getWidth(),poSrc->getHeight());
  }

}
