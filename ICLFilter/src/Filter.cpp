#include "Filter.h"
#include "Macros.h"

namespace icl {

   /// check+adapt destination images parameters against given values
   bool Filter::prepare (ImgBase **ppoDst, depth eDepth, 
                         const Size &imgSize, format eFormat, Time timestamp, int nChannels, 
                         const Rect& roi) {
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
      }
      return true;
   }

}
