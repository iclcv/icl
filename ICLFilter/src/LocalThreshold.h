#ifndef LOCAL_THRESHOLD_H
#define LOCAL_THRESHOLD_H

#include "Filter.h"
#include "Size.h"
#include "IntegralImg.h"
#include <vector>


namespace icl{
  
  class LocalThreshold : public Filter{
    public:
    LocalThreshold(unsigned int maskSize=10, int globalThreshold=0);
    void apply(ImgBase *src, ImgBase **dst);

    void setMaskSize(unsigned int maskSize);
    void setGlobalThreshold(int globalThreshold);
    
    private:
    unsigned int m_uiMaskSize;
    int m_iGlobalThreshold;
    
    ImgBase *m_poROIImage;

    unsigned int m_uiROISizeImagesMaskSize;

    IntegralImg::IntImg32s m_oROISizeImage;
    IntegralImg::IntImg32s m_oIntegralImage;
    IntegralImg::IntImg32f m_oIntegralImageF;
  };
  
}  
#endif
