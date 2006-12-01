#ifndef LUT_H
#define LUT_H

#include "Filter.h"
#include "Img.h"
#include <vector>

namespace icl {
   class LUT : public Filter {
   public:
     static void simple(Img8u *src, Img8u *dst, const std::vector<icl8u>& lut); 
     static void reduceBits(Img8u *src, Img8u *dst, icl8u levels);
   };
} // namespace icl

#endif
