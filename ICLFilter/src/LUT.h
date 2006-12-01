#ifndef LUT_H
#define LUT_H

#include "Filter.h"
#include "Img.h"
#include <vector>

namespace icl {

  /// class for applying table lookup transformation to Img8u images
   class LUT : public Filter {
   public:
     /// simple lut transformation dst(p) = lut(src(p))
     static void simple(Img8u *src, Img8u *dst, const std::vector<icl8u>& lut); 

     /// specialization of a lut transformation to reduce the number colors levels image a given image
     /** TODO: some more details */
     static void reduceBits(Img8u *src, Img8u *dst, icl8u levels);
   };
} // namespace icl

#endif
