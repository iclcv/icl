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
    int getBoundaryPointCount(bool thinned=true) const;
    
    /// returns the estimated length of the boundary
    /** This methods gives a much better estimation of the boundary
        length than getBoundaryPointCount(). The change in the estimated
        length should be no larger than 5 to 10 % under rotation of the
        object. \n
        Complexity: linear in boundary length.
    */
    float getBoundaryLength() const;
    
    /// returns the Form-Factor of the Region (Boundary²/4PI*Size)
    /** Please note that the formfactor for smaller regions is 
        always a bit higher as expected for an ideal regions. 
        This problem occurs due to using a 4-neighbourhood for 
        calculating the boundary length of a blob.
        
        The formfactor formula is designed in that way, that
        an ideal circly has a formfactor of 1 and all other --
        less round -- regions have a higher form factor:
        
        - circle 1
        - square 4/PI
        - cuboid (W=2*H) = 4.5/PI
    */
    float getFormFactor() const;

    /// Approximate the region boundary by polygon structure
    /** @see ICLCore :: CornerDetectorCSS for a detailed description of function argmuments
    */
    const std::vector<Point32f> &getBoundaryCorners(float angle_thresh=162.,
                                                    float rc_coeff=1.5, 
                                                    float sigma=3., 
                                                    float curvature_cutoff=100., 
                                                    float straight_line_thresh=0.1) const;
    
    /// returns the upper left pixel of the Regions (internally used mainly)
    Point getUpperLeftPixel() const;
    
    /// returns the center of gravity of the Region
    const Point32f &getCOG() const;
    
    /// returns the bounding box of the region
    const Rect &getBoundingBox() const;
    
    /// returns local pca information of the region
    const RegionPCAInfo &getPCAInfo() const ;
    
    /// returns list of boundary pixels of the region
    const std::vector<Point> &getBoundary(bool thinned=true) const;
    
    /// returns list of all region pixels
    const std::vector<Point> &getPixels() const;
    
    /// return low-level list of scanlines
    const std::vector<ScanLine> &getScanLines() const;
    
    /// accurate detection of blob center using gray image 
    /** This more accurate center of gravity computation function works
        on a gray level image which must describe the region either
        as high or as low gray values. All pixels within the images bounding box
        (enlarged by bbMargin) are weighted with their goodness values
        extracted from the given image. To reduce noise, only values between 
        minThreshold and maxThreshold are used, where values below minThreshold
        are weighted with 0 and values above maxThreshold are clipped to
        maxThreshold. The float-precision weighted sum of all pixel locations
        within the bounding box is returned.\n
        @param grayImage source image with region goodness or 'badness'
                         information
        @param bbMargin amount of bounding box enlarging (in pixels)
                        internally the resulting bounding box is -- of course --
                        clipped to the image rect to avoid seg-faults
        @param minThreshol minimal value for pixels to be taken into account.
                           Pixels of minThreshold and below are not regarded
                           for COG calculation. If darkBlob is true, the minThreshold
                           becomes the upper limit instead
        @param maxThreshold as described above
        @param darkBlob if true, blob pixels must be darker in comparison to the
                        background pixels (default), otherwise, blob pixels must
                        be brighter.
    */
    template<class T>
    const Point32f &getAccurateCOG(const Img<T> &grayImage, int bbMargin=2, 
                                   const T &minThreshold=50, const T &maxThreshold=200,
                                   bool darkBlob=true) const;

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
    
    /// internally computes the thinned boundary
    /** the unthinned (and closed!) boundary must be computed before. */
    void calculateThinnedBoundaryIntern() const;
  };
}


#endif
