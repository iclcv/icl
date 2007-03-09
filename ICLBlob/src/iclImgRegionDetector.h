#include <iclArray.h>
#include <iclTypes.h>
#include <iclBlobData.h>
#include <iclRange.h>
#ifndef IMG_REGION_DETECTOR_H
#define IMG_REGION_DETECTOR_H


namespace icl{
  /// namespace for ImgRegionDetector specific functions
  namespace regiondetector{
    /// forward declaration for a hidden class
    class RegionDetector;
  }
  
  /// Class for extracting a list of connected image regions from an image
  /** The ImgRegionDetector (IRD) offers the ability for extracting a list of all connected image
      regions in an image:
      The set of regions can be filtered out by specifying an allowed interval for value and size
      of the found regions.\n
      <b>Note:</b> The implemented region detection algorithm is optimized for icl8u image data. 
      So all other incomming image depths are converted internall to icl8u. Additionally the 
      region detection algorithm works on a single icl8u array with no ROI. The input image must
      have a single image channel. If the source image has a specified ROI (different from the 
      hole image size), the images ROI is automatically buffered into a single plain data array.
            
      The IRDs "detect"-function retuns a const Vector of "BlobData"-structs. These stucts can be
      used to get further higher-level region features like the center-of-gravity or count of
      boundary pixels of this blob.
  
      <h2>The implemented algorithm:</h2>
      Details of the implemented algorithm can be read in the:
      Diploma-Thesis of Christof Elbrechter and Jens Schmüdderich
      "Das TDI-Framework für dynamische Lernarchitekturen in intelligenten, interaktiven Systemen"
      Section 4.7
  */
  class ImgRegionDetector{
    public:
    
    /// Creates a new ImgRegionDetector with given size and value interval (compatibility version)
    /** @param minSize minumum pixel count for detected regions
        @param maxSize maximum pixel count for detected regions
        @param minVal minimum value for detected regions (all pixels of a region have the same value)
        @param maxVal maximum value for detected regions        
    **/
    ImgRegionDetector(unsigned int minSize=0, unsigned int maxSize=10000000,icl8u minVal=0, icl8u maxVal=255);

    /// Creates a new ImgRegionDetector with given size and value interval
    /** @param sizeRange range for allowed region pixel count
        @param valueRagen range for allowed region value
    **/
    ImgRegionDetector(const Range<unsigned int> &sizeRange, const Range<icl8u> &valueRange);

    /// Destructor
    ~ImgRegionDetector();

    /// sets new restrictions (compatibility version)
    /** @param minSize minumum pixel count for detected regions
        @param maxSize maximum pixel count for detected regions
        @param minVal minimum value for detected regions (all pixels of a region have the same value)
        @param maxVal maximum value for detected regions        
    **/
    void setRestrictions(int minSize, int maxSize, icl8u minVal, icl8u maxVal);

    /// sets new restrictions
    /** @param sizeRange range for allowed region pixel count
        @param valueRagen range for allowed region value
    **/
    void setRestrictions(const Range<unsigned int> &sizeRange, const Range<icl8u> &valueRange);
    
    /// detects all blobs in the image that match to the given parameters
    /** @param image image to detect regions in. Only single channel images are allowed.
                     <b>NOTE:</b> Non-icl8u images are converted internally to icl8u.
                     (if null, an empty vector is returned)
        @return vector of detected regions*/
    const std::vector<BlobData> &detect(ImgBase *image);
    
    private:
    
    /// wrapped RegionDetector object
    regiondetector::RegionDetector *m_poRD;

    /// Internal buffer for any-to-icl8u conversion
    Img8u *m_poImage8uBuffer;

    /// Internal storage for currently extracted regions
    std::vector<BlobData> m_vecBlobData;
  };
}

#endif
