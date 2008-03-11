#ifndef ICL_REGION_DETECTOR_H
#define ICL_REGION_DETECTOR_H

#include <iclImg.h>
#include <iclScanLine.h>
#include <iclRegion.h>
#include <iclRegionPart.h>

namespace icl{
  
  
  /// Region detection class (also known as "Connected Components)"
  /** TODO: document this class */
  class RegionDetector{
    public:

    RegionDetector(unsigned int minSize=0, unsigned int maxSize=2<<20, icl64f minVal=0, icl64f maxVal = 255);
    
    void setRestrictions(unsigned int minSize=0, unsigned int maxSize=2<<20, icl64f minVal=0, icl64f maxVal = 255);
    void setRestrictions(const Range<unsigned int> &sizeRange, const Range<icl64f> &valueRange);

    const std::vector<Region> &detect(const ImgBase *image);
    
    private:
 
    template<class T, typename Compare>
    void detect_intern(const Img<T> &image, Compare cmp);
    void adaptLIM(int w, int h);
    void resetPartList();
    RegionPart *newPart();
    
    std::vector<Region> m_vecBlobData;
    std::vector<RegionPart*> m_vecLIM;
    std::vector<RegionPart*> m_vecParts;
    
    unsigned int m_uiMinSize;    
    unsigned int m_uiMaxSize;
    icl64f m_dMinVal;
    icl64f m_dMaxVal;
    
  };
}

#endif
