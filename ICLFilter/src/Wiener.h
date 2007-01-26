#ifndef WIENER_H
#define WIENER_H

#include <FilterMask.h>

namespace icl {
   class ImgBase;
  
   /// Class for Wiener Filter
   /** Wiener filters are commonly used in image processing applications to
       remove additive noise from degraded images, to restore a blurred image.
   */
   class Wiener : public FilterMask {
   public:

      /// Constructor that creates a wiener filter object, with specified mask size
      /** @param maskSize of odd width and height
          Even width or height is increased to next higher odd value.
      */
      Wiener (const Size &maskSize);

      /// Filters an image using the Wiener algorithm.
      /** @param fNoise: noise level in range [0,1] 
        @param poSrc Source image
        @param ppoDst Destination image
      */
      void apply (const ImgBase *poSrc, ImgBase **ppoDst, icl32f fNoise);

#ifdef WITH_IPP_OPTIMIZATION
   protected:
      template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, IppiSize, IppiPoint, icl32f*, icl8u*)>
         void ippiWienerCall (const ImgBase *src, ImgBase *dst, icl32f fNoise);
#endif

   private:
      /// array of image-depth selective filter methods
      static void (Wiener::*aMethods[depthLast+1])(const ImgBase *poSrc, ImgBase *poDst, float); 

      std::vector<icl8u> vBuffer;
   };
} // namespace icl

#endif // WIENER_H
