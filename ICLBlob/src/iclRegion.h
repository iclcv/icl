#ifndef ICL_REGION_H
#define ICL_REGION_H

#include <iclSmartPtr.h>
#include <iclPoint32f.h>
#include <iclRect.h>
#include <iclTypes.h>
#include <iclShallowCopyable.h>
#include <vector>
#include "iclRegionPart.h"
#include "iclScanLine.h"
#include "iclRegionPCAInfo.h"


namespace icl{

  /** \cond */
  // this code is only for implementing shallow copyable
  class RegionImpl; 
  struct RegionImplDelOp{
    static void delete_func( RegionImpl* impl);
  };
  /** \endcond */
  
  
  
  /// Region Structure providing region feature information \ingroup G_RD
  /** \section GEN General information
      The Region class provides several region information:
      - size (pixel count of the region
      - value (pixel value of the region; Currently each region is
        defined by connected Pixels with identical intensity values)
      - boundary (list of outer boundary pixels of the region)
      - formfactor (normed ratio between image pixel size and boundary
        length)
      - cog (center of gravity of the region)
      - bounding box (outer bounding box of the region)
      - scanlines (low level access to underlying list of scanlines)
      - pixels (list of all region pixels)
      - local pca information (angles and length of major axis')

      \section INT Internal Information
      Regions are created by a RegionDetector object, which passes a
      RegionPart-Multi-Tree structure to the Regions constructor.
      in addition, the underlying image, the regions value and the
      RegionDetector maxSize parameter is given. If given RegionPart-Multi-
      Tree is traversed from top to the bottom, the current pixel size is 
      computed simultaneously. By this means the recursive collection
      procedure can be stopped immediately if pixel count is larger than
      the maximum region size. \n
      Region size as well as region center of gravity is calculated during
      the tree descent in constructor. All other information are calculated
      only on demand, i.e. if the corresponding getter function is called.
      
      \section INT2 Internal Processing
      Most high level features, such as bounding boxes or PCA information, 
      can be calculated efficiently on the underlying scanline structure.
      Outer binary pixels are calculated on the underlying image.
      
      \section CP Copying
      To avoid deep copies of internal region data, Region class implements ICL's
      ShallowCopyable interface which provides a shallow copied implementation
      class, by using a reference counting smart pointer.
  **/
  class Region : public ShallowCopyable<RegionImpl,RegionImplDelOp>{
    public:
    
    /// Default constructor called by the RegionDetector
    Region(RegionPart *p, int maxSize, icl64f val, const ImgBase *image);
    
    /// returns the pixel count of the Region
    int getSize() const;
    
    /// returns the pixel value of  the region
    icl64f getVal() const;
    
    /// returns the size of boundary pixel array of the Region
    int getBoundaryLength() const;
    
    /// returns the Form-Factor of the Region (Boundary²/4PI*Size)
    float getFormFactor() const;
    
    /// returns the upper left pixel of the Regions (internally used mainly)
    Point getUpperLeftPixel() const;
    
    /// returns the center of gravity of the Region
    const Point32f &getCOG() const;
    
    /// returns the bounding box of the region
    const Rect &getBoundingBox() const;
    
    /// returns local pca information of the region
    const RegionPCAInfo &getPCAInfo() const ;
    
    /// returns list of boundary pixels of the region
    const std::vector<Point> &getBoundary() const;
    
    /// returns list of all region pixels
    const std::vector<Point> &getPixels() const;
    
    /// return low-level list of scanlines
    const std::vector<ScanLine> &getScanLines() const;

    /// draws this region scanline by scanline into a given image 
    /** ROI is not regarded, because the region detectors results are
        not relative to a ROI offset*/
    template<class T>
    void drawTo(Img<T> &image, T val) const;
    
  private:
    /// internally used recursive function for RegionPart-tree traversal
    void collect(RegionPart *p, int maxSize);

    /// internally used boundary computation function
    template<class T>
    void calculateBoundaryIntern(const Img<T> &image) const;
  };
}


#endif
