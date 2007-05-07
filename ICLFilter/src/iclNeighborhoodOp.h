#ifndef NEIGHBORHOOD_OP_H
#define NEIGHBORHOOD_OP_H

#include <iclUnaryOp.h>

namespace icl {
  /// unary operators that work on each pixels neighborhood
  /** TODO:: check!!
      The Filter class builds a base class for filter operations employing
      a filter mask which is moved over the ROI of the source image(s), 
      e.g. convolution filters use some convolution masks.
      To this end the class provides members to store the size and anchor
      of the filter mask.

      Special care has to be taken, when applying a filter mask to the border
      of an image, e.g. in case of full-image ROI. In this case the filter
      might access undefined pixel values outside the image, actually causing
      a segfault in most cases.

      Hence, the used ROI size of the source image is <em> shrinked </em> if
      neccessary, such that the filter mask always fits into the image, when
      moved over the ROI. For this purpose the method adaptROI is provided,
      which computes the to-be-used ROI. The ROI of the source image is not
      actually changed, which makes the filter operation thread safe, because
      simultaneous operations running on the source image within different
      threads do not interfere.
  */
  class NeighborhoodOp : public UnaryOp {
    public:
    
    ///Destructor
    virtual ~NeighborhoodOp(){}
    
    protected:
    NeighborhoodOp() : m_oMaskSize(1,1), m_oAnchor (0,0) {}
    NeighborhoodOp(const Size &size) {
      setMask (size);
    }
    
    void setMask(const Size &size) {
        m_oMaskSize = adaptSize(size);
        m_oAnchor   = Point (m_oMaskSize.width/2, m_oMaskSize.height/2);
    }
    void setMask(const Size &size, const Point &anchor){
      m_oMaskSize = adaptSize(size);
      m_oAnchor = anchor;
    }
    void setROIOffset(const Point &offs){
      m_oROIOffset = offs;
    }
    const Size &getMaskSize() const{
      return m_oMaskSize;
    } 
    const Point &getAnchor() const {
      return m_oAnchor;
    }
    const Point &getROIOffset() const{
      return m_oROIOffset;
    }

    /// prepare filter operation: ensure compatible image format and size
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc);
    
    /// prepare filter operation: as above, but with depth parameter
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc, depth eDepht);
    
     /// compute neccessary ROI offset and size
     /** This functions computes the to-be-used ROI for the source image, 
         such that the filter mask of given size (oMaskSize) fits everywhere
         into the image if placed arbitrarily within the ROI.
         The original ROI of the source image is not changed, instead the
         adapted ROI is returned in parameters oROIsize and oROIoffset.
         @param poSrc  image whose ROI is adapted
         @param oROIoffset  new ROI offset
         @param oROIsize    new ROI size
         @return whether a valid ROI remains
     */
     bool computeROI(const ImgBase *poSrc, Point& oROIoffset, Size& oROIsize);
     
     /// this function can be reimplemented e.g to enshure an odd mask width and height
     /** E.g. some implementations of Neighborhood-operation could demand odd or even
         mask size parameters. In this case, this function can be implemented in another
         way. (Example: MedianOp)
         @param size size to ajust
         @return the given size in this base implementation
     **/
     virtual Size adaptSize(const Size &size){ return size; }
     
    protected:
     ///TODO: later private with getter and setter functions
     Size  m_oMaskSize;  //< size of filter mask
     Point m_oAnchor;    //< anchor of filter mask
     Point m_oROIOffset; //< to-be-used ROI offset for source image
  };
}
#endif
