#include <ICLFilter.h>
#include <ICLMacros.h>

namespace icl {

   /// check+adapt destination images parameters against given values
   bool Filter::prepare (ImgBase **ppoDst, depth eDepth, 
                         const Size &imgSize, format eFormat, int nChannels, 
                         const Rect& roi, Time timestamp) {
      ICLASSERT_RETURN_VAL (ppoDst, false);
      if (bCheckOnly) {
         ImgBase* dst = *ppoDst;
         ICLASSERT_RETURN_VAL (dst && 
                               dst->getDepth() == eDepth && 
                               dst->getChannels () == nChannels &&
                               dst->getFormat () == eFormat &&
                               dst->getROISize () == roi.size(), false);
         dst->setTime(timestamp);
      } else {
         ensureCompatible (ppoDst, eDepth, imgSize, 
                           nChannels, eFormat, roi);
         (*ppoDst)->setTime(timestamp);
      }
      return true;
   }

}
