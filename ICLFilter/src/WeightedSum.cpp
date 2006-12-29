#include <WeightedSum.h>

namespace icl {

   // {{{ C++ fallback

   template <typename T>
   void WeightedSum::compute(const Img<T> *src, Img<icl32f> *dst, 
                             const std::vector<float>& weights) {
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == (int)weights.size() );
      ICLASSERT_RETURN( dst->getChannels() == 1);
 
      ConstImgIterator<T> itSrc = src->getROIIterator(0);
      ImgIterator<icl32f> itDst = dst->getROIIterator(0);

      float w = weights[0];
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
         *itDst = *itSrc * w;
      }

      for (int c=src->getChannels()-1; c > 0; c--) {
         w = weights[c];
         itSrc = src->getROIIterator(c);
         itDst = dst->getROIIterator(0);
         for(;itSrc.inRegion(); ++itSrc, ++itDst){
            *itDst += *itSrc * w;
         }
      }
   }

   // }}}

  void WeightedSum::apply (ImgBase *poSrc, Img<icl32f> *poDst, 
                           const std::vector<float>& weights) {
    ImgBase *poDstBase = static_cast<ImgBase*>(poDst);
    if (!Filter::prepare (&poDstBase, depth32f, Filter::chooseSize (poSrc),
                          formatMatrix, 1, chooseROI (poSrc), poSrc->getTime())) return;
    switch (poSrc->getDepth()) {
      case depth8u:  compute (poSrc->asImg<icl8u>(), poDst, weights); break;
      case depth16s: compute (poSrc->asImg<icl16s>(), poDst, weights); break;
      case depth32s: compute (poSrc->asImg<icl32s>(), poDst, weights); break;
      case depth32f: compute (poSrc->asImg<icl32f>(), poDst, weights); break;
      case depth64f: compute (poSrc->asImg<icl64f>(), poDst, weights); break;
      default:
        ICL_INVALID_DEPTH;
    }
  }
}
