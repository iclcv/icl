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
#define ICL_INSTANTIATE_DEPTH(T) \
    case depth ## T: compute (poSrc->asImg<icl ## T>(), poDst, weights); break;

  void WeightedSum::apply (ImgBase *poSrc, Img<icl32f> *poDst, 
                           const std::vector<float>& weights) {
    ImgBase *poDstBase = static_cast<ImgBase*>(poDst);
    if (!Filter::prepare (&poDstBase, depth32f, Filter::chooseSize (poSrc),
                          formatMatrix, 1, chooseROI (poSrc), poSrc->getTime())) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default:
        ICL_INVALID_DEPTH;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
}
