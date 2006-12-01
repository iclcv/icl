#ifndef IMG_REGION_DETECTOR_H
#define IMG_REGION_DETECTOR_H

#include <Array.h>
#include <ICLTypes.h>

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
  
      <h2>The implemented algorithm:</h3>
      Details of the implemented algorithm can be read in the:
      Diploma-Thesis of Christof Elbrechter and Jens Schmüdderich
      "Das TDI-Framework für dynamische Lernarchitekturen in intelligenten, interaktiven Systemen"
      Section 4.7
  */
  class ImgRegionDetector{
    public:
    
    /// Creates a new ImgRegionDetector with given size and value interval
    ImgRegionDetector(unsigned int minSize=0, unsigned int maxSize=10000000,icl8u minVal=0, icl8u maxVal=255);

    /// Destructor
    ~ImgRegionDetector();

    /// sets new restrictions
    void setRestrictions(int minSize, int maxSize, icl8u minVal, icl8u maxVal);
    
    /// extracts all regions centers returning them as a list of x,y coordinates
    const Array<int> &detect(ImgBase *image);
    
    /// extracts all regions centers and corresponding values with given destination vectors 
    void detect(ImgBase *image, Array<int> &centers, Array<icl8u> &values);

    /// extracts centers, values, boundind-boxes and major axis from the given image
    /** @param image input image
        @param centers destination vector for the found centers (xyxyxy..)
        @param values destinaiton vector for the regions values
        @param boundingBoxes destination vector for the regions bounding boxes (order xywhyxwh,..)
        @param pcaInfos destination vector for the regions major axis and arcs (order (l1l2arc1arc2,l1l2arc1arc2,...)
    */
    void detect(ImgBase *image, Array<int> &centers, Array<icl8u> &values,
                Array<int> &boundingBoxes, Array<icl32f> &pcaInfos);
    
    
    private:
    regiondetector::RegionDetector *m_poRD;
    Img8u *m_poImage8uBuffer;
    Array<int> m_oCenterBuffer;
  };
}

#endif
