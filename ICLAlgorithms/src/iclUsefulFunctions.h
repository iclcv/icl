#ifndef ICL_USEFUL_FUNCTIONS_H
#define ICL_USEFUL_FUNCTIONS_H

#include <iclImg.h>
#include <iclImgRegionDetector.h>

namespace icl{
  std::vector<Rect> iclMatchTemplate(const Img8u &src, 
                                     const Img8u &templ, 
                                     float significance,
                                     Img8u *buffer=0,
                                     ImgRegionDetector *rd=0);
                                     
  
  
}

#endif
