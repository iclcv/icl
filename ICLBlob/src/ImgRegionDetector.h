#ifndef IMG_REGION_DETECTOR_H
#define IMG_REGION_DETECTOR_H

#include <Array.h>
#include <ICLTypes.h>

namespace icl{
  namespace regiondetector{
    class RegionDetector;
  }
  
  class ImgRegionDetector{
    public:
    ImgRegionDetector(icl8u minVal=0, icl8u maxVal=255, unsigned int minSize=0, unsigned int maxSize=0);
    ~ImgRegionDetector();

    void setRestrictions(icl8u minVal, icl8u maxVal, int minSize, int maxSize);
    
    const Array<int> &detect(ImgBase *image);
    void detect(ImgBase *image, Array<int> &centers, Array<icl8u> &values);
    void detect(ImgBase *image, Array<int> &centers, Array<icl8u> &values,
                Array<int> &boundingBoxes, Array<icl32f> &pcaInfos);
    
    
    private:
    regiondetector::RegionDetector *m_poRD;
    Img8u *m_poImage8uBuffer;
    Array<int> m_oCenterBuffer;
  };
}

#endif
