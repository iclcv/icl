#ifndef UNARY_OP_H
#define UNARY_OP_H

#include "iclOpROIHandler.h"

namespace icl{
  /// Abstract Base class for I=f(I1,I2) operators
  /** The Binary Operator class provides an abstract class interface for all 
      Operators that are computing a result image of two input images (e.g. 
      pixel-wise "+" or "-" operators.     
      
      \section OPLIST list of unary operators
      The ICLFilter package provides the complete filtering functions supported 
      by the ICL. Currently the following subpackages are included in the ICLFilter library:
      
      - <b>Arithmetic</b>: The Arithmetic Class contains a lot of arithmetic functions 
      (like add, mul,etc) working on images
      
      - <b>Canny</b>: The Canny Edge Detector, only available when having IPP. 
      
      - <b>Compare</b>: Compares two images, or one image with an constant pixelwise 
        and saves the binary result in an output image.
      
      - <b>Convolution</b>: The Convolution class provides functionality for any 
        kind of convolution filters.
      
      - <b>GeoTransforms</b>: Contains functions for mirroring and affine transformations.
      
      - <b>IntegralImg</b>: Class for creating integral images.
      
      - <b>LocalThreshold</b>:C.E.: **TODO** document this
      
      - <b>Logical</b>: The Logical Class contains a lot of logical functions (like 
        AND, OR, XOR, etc) working on images with integer Types
      
      - <b>LUT</b>: Class for applying table lookup transformation to Img8u images.
      
      - <b>Median</b>: Class that provides median filter abilities.
      
      - <b>Morphological</b>: Class that provides morphological operations.
      
      - <b>Proximity</b>: Class for computing proximity (similarity) measure between 
        an image and a template (another image).
      
      - <b>Skin</b>: This class implements a Skin color detection algorithm

      - <b>Threshold</b>: Class for thresholding operations

      - <b>Weighted Sum</b>: Accumulate weighted pixel values of all image channels

      - <b>Wiener</b>:  Class for Wiener Filter. Wiener filters remove additive noise 
        from degraded images, to restore a blurred image, only available when having IPP. 

      
      A detailed description of the provided functions in each package is included in
      the class description.
      */
  class UnaryOp{
    public:
    virtual ~UnaryOp(){}
    /// pure virtual apply function, that must be implemented in all derived classes
    virtual void apply(const ImgBase *operand1, ImgBase **dst)=0;
    
    void setClipToROI (bool bClipToROI) { m_oROIHandler.setClipToROI(bClipToROI); }
    void setCheckOnly (bool bCheckOnly) { m_oROIHandler.setCheckOnly(bCheckOnly); }
    
    bool getClipToROI() const { return m_oROIHandler.getClipToROI(); }
    bool getCheckOnly() const { return m_oROIHandler.getCheckOnly(); }
    
    protected:
    bool prepare (ImgBase **ppoDst, depth eDepth, const Size &imgSize, 
                  format eFormat, int nChannels, const Rect& roi, 
                  Time timestamp=Time::null){
      return m_oROIHandler.prepare(ppoDst, eDepth,imgSize,eFormat, nChannels, roi, timestamp);
    }
    
    /// check+adapt destination image to properties of given source image
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc) {
      return m_oROIHandler.prepare(ppoDst, poSrc);
    }
    
    /// check+adapt destination image to properties of given source image
    /// but use explicitly given depth
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc, depth eDepth) {
      return m_oROIHandler.prepare(ppoDst, poSrc, eDepth);
    }
    
    private:
    OpROIHandler m_oROIHandler;
  };    
}
#endif
