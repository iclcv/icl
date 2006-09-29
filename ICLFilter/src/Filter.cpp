#include "Filter.h"
#include "Macros.h"

namespace icl {

   /// check+adapt destination images parameters against given values
   bool Filter::prepare (ImgI **ppoDst, depth eDepth, 
                         const Size &imgSize, format eFormat, int nChannels, 
                         const Rect& roi) {
      ICLASSERT_RETURN_VAL (ppoDst, false);
      if (bCheckOnly) {
         ImgI* dst = *ppoDst;
         ICLASSERT_RETURN_VAL (dst && 
                               dst->getDepth() == eDepth && 
                               dst->getChannels () == nChannels &&
                               dst->getFormat () == eFormat &&
                               dst->getROISize () == roi.size(), false);
      } else {
         ensureCompatible (ppoDst, eDepth, imgSize, eFormat, 
                           nChannels, roi);
      }
      return true;
   }
   
   /// check+adapt destination images parameters against values from source image
   bool Filter::prepare (ImgI **ppoDst, const ImgI *poSrc) {
      const Rect& roi = bClipToROI ? Rect (Point::zero, poSrc->getROISize ())
         : poSrc->getROI();
      return prepare (ppoDst, poSrc->getDepth(), poSrc->getSize(),
                      poSrc->getFormat(), poSrc->getChannels (), roi);
   }

}
