#ifndef WEIGHTEDSUM_H
#define WEIGHTEDSUM_H

#include <Img.h>
#include <Filter.h>

namespace icl {

   /// Accumulate weighted pixel values of all image channels
   /** Pixels of all channels in source image are weighted 
       by a channel-wise weight and accumulated to the destination image.

       Performance: 1000x1000x10 image
               IPP     C++
       icl8u   175ms   86ms
       icl32f  165ms   85ms
    */
   class WeightedSum : public Filter {
   public:
      void apply (ImgBase *poSrc, Img<icl32f> *poDst, const std::vector<float>& weights);

   private:
#if 0 && defined WITH_IPP_OPTIMIZATION
      std::vector<float> m_oBuffer;
#endif

      template <typename T>
      void compute(const Img<T> *src, Img<icl32f> *dst, 
                   const std::vector<float>& weights);
   };

} // namespace icl

#endif
