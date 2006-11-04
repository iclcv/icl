#include "WeightedSum.h"

namespace icl {
#undef WITH_IPP_OPTIMIZATION // slower than fallback
#ifdef WITH_IPP_OPTIMIZATION
   // {{{ IPP version

   /* function accumulating weighted channel into output buffer */
   inline void ippiAccumulate (const icl8u *src, icl32f *buf, int srcStep, float w,
                               icl32f *dst, int dstStep, const Size& roiSize) {
      int bufStep = roiSize.width*sizeof(icl32f);
      // convert icl8u into icl32f buffer
      ippiConvert_8u32f_C1R(src, srcStep, buf, bufStep, roiSize);
      // inline multiply by weight within buffer
      ippiMulC_32f_C1IR (w, buf, bufStep, roiSize);
      // add result to accumulation buffer
      ippiAdd_32f_C1IR(buf, bufStep, dst, dstStep, roiSize);
   }
   /* function accumulating weighted channel into output buffer */
   inline void ippiAccumulate (const icl32f *src, icl32f *buf, int srcStep, float w,
                               icl32f *dst, int dstStep, const Size& roiSize) {
      int bufStep = roiSize.width*sizeof(icl32f);
      // multiply by weight and write to buffer
      ippiMulC_32f_C1R (src, srcStep, w, buf, bufStep, roiSize);
      // add result to accumulation buffer
      ippiAdd_32f_C1IR(buf, bufStep, dst, dstStep, roiSize);
   }

   template <typename T>
   void WeightedSum::compute(const Img<T> *src, Img<icl32f> *dst, 
                             const std::vector<float>& weights) {
      const Size& size = dst->getROISize ();
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize()  == size );
      ICLASSERT_RETURN( src->getChannels() == (int)weights.size() );
      ICLASSERT_RETURN( dst->getChannels() == 1);
    
      // reserve memory for depth/multiply buffer
      m_oBuffer.reserve (size.width * size.height);

      // clear destination = accumulation buffer
      ippiSet_32f_C1R(0.0, dst->getData(0), dst->getLineStep(), dst->getSize());

      for (int c=src->getChannels()-1; c >= 0; --c) {
         ippiAccumulate (src->getROIData(c), &m_oBuffer[0], src->getLineStep(), weights[c], 
                         dst->getROIData(0), dst->getLineStep(), size);
      }
   }
   // }}}
#else
   // {{{ C++ fallback

   template <typename T>
   void WeightedSum::compute(const Img<T> *src, Img<icl32f> *dst, 
                             const std::vector<float>& weights) {
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == (int)weights.size() );
      ICLASSERT_RETURN( dst->getChannels() == 1);
 
      ImgIterator<T>      itSrc = const_cast<Img<T>*>(src)->getROIIterator(0);
      ImgIterator<icl32f> itDst = dst->getROIIterator(0);

      float w = weights[0];
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
         *itDst = *itSrc * w;
      }

      for (int c=src->getChannels()-1; c > 0; c--) {
         w = weights[c];
         itSrc = const_cast<Img<T>*>(src)->getROIIterator(c);
         itDst = dst->getROIIterator(0);
         for(;itSrc.inRegion(); ++itSrc, ++itDst){
            *itDst += *itSrc * w;
         }
      }
   }

   // }}}
#endif

   void WeightedSum::apply (ImgI *poSrc, Img<icl32f> *poDst, 
                            const std::vector<float>& weights) {
      if (!Filter::prepare ((ImgI**) &poDst, depth32f, Filter::chooseSize (poSrc),
                            formatMatrix, 1, chooseROI (poSrc))) return;
      switch (poSrc->getDepth()) {
         case depth8u:  compute (poSrc->asImg<icl8u>(), poDst, weights); break;
         case depth32f: compute (poSrc->asImg<icl32f>(), poDst, weights); break;
      }
   }

}
