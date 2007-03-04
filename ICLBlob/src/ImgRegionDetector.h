#ifndef IMG_REGION_DETECTOR_H
#define IMG_REGION_DETECTOR_H

#include <Array.h>
#include <ICLTypes.h>
#include <BlobData.h>
#include <Range.h>

namespace icl{
  /// namespace for ImgRegionDetector specific functions
  namespace regiondetector{
    /// forward declaration for a hidden class
    class RegionDetector;
  }
  
  /// Class for extracting a list of connected image regions from an image
  /** TODO "detecte with pixel accuracy"
      The ImgRegionDetector (IRD) offers the ability for extracting a list of all connected image
      regions in an image:
      The set of regions can be filtered out by selection an interval for the allowed value and size
      of the found regions:
  
      <h2>The implemented algorithm:</h2>
      Details of the implemented algorithm can be read in the:
      Diploma-Thesis of Christof Elbrechter and Jens Schmüdderich
      "Das TDI-Framework für dynamische Lernarchitekturen in intelligenten, interaktiven Systemen"
      Section 4.7
  */
  class ImgRegionDetector{
    public:
    
    /// Creates a new ImgRegionDetector with given size and value interval
    ImgRegionDetector(unsigned int minSize=0, unsigned int maxSize=10000000,icl8u minVal=0, icl8u maxVal=255);
    ImgRegionDetector(const Range<unsigned int> &sizeRange, const Range<icl8u> &valueRange);

    /// Destructor
    ~ImgRegionDetector();

    /// sets new restrictions
    void setRestrictions(int minSize, int maxSize, icl8u minVal, icl8u maxVal);
    void setRestrictions(const Range<unsigned int> &sizeRange, const Range<icl8u> &valueRange);
    
    /// detects all blobs in the image that match to the given parameters
    const std::vector<BlobData> &detect(ImgBase *image);
    
    private:
    regiondetector::RegionDetector *m_poRD;
    Img8u *m_poImage8uBuffer;
    std::vector<BlobData> m_vecBlobData;
  };
}

#endif
