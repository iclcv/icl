#include <iclImgBase.h>
#ifndef ICL_FILTER_H
#define ICL_FILTER_H

/**
\mainpage ICLFilter package
\section Overview

The ICLFilter package provides the complete filtering functions supported by the ICL. Currently the following subpackages are included in the ICLFilter library:

- <b>Arithmetic</b>: The Arithmetic Class contains a lot of arithmetic functions (like add, mul,etc) working on images

- <b>Canny</b>: The Canny Edge Detector, only available when having IPP. 

- <b>Compare</b>: Compares two images, or one image with an constant pixelwise and saves the binary result in an output image.

- <b>Convolution</b>: The Convolution class provides functionality for any kind of convolution filters.

- <b>GeoTransforms</b>: Contains functions for mirroring and affine transformations.

- <b>IntegralImg</b>: Class for creating integral images.

- <b>LocalThreshold</b>:C.E.: **TODO** document this

- <b>Logical</b>: The Logical Class contains a lot of logical functions (like AND, OR, XOR, etc) working on images with integer Types

- <b>LUT</b>: Class for applying table lookup transformation to Img8u images.

- <b>Median</b>: Class that provides median filter abilities.

- <b>Morphological</b>: Class that provides morphological operations.

- <b>Proximity</b>: Class for computing proximity (similarity) measure between an image and a template (another image).

- <b>Skin</b>: This class implements a Skin color detection algorithm

- <b>Threshold</b>: Class for thresholding operations

- <b>Weighted Sum</b>: Accumulate weighted pixel values of all image channels

- <b>Wiener</b>:  Class for Wiener Filter. Wiener filters remove additive noise from degraded images, to restore a blurred image, only available when having IPP. 


A detailed description of the provided functions in each package is included in
the class description.

*/


namespace icl {
  /// Abstract class interface for filter operations
  /** The Filter class builds a base class for all ICL filter operations. Each
      filter operation is performed on the ROI of the source image only. The
      destination image is <em> always </em> adapted in its parameters to the
      necessary values. We distinguish the following modes:

      - Adapt the destination image in its size and ROI to the source
        image. Hence the destination will have the same size and ROI as the
        source image. Nevertheless the filter operates on the ROI only, 
        leaving the previous content of the destination image unchanged
        in this border region. Hence this mode requires subsequent handling
        of the border.
      - Adapt the destination images size, such that is exactly comprises
        the ROI of the source image. (bClipToROI = true)
      - Only check the destination images parameters and issue a warning
        if not set correctly already. In this case the filter operation itself
        should not be performed. (bCheckOnly = false)

      To this end the Filter class provides variables bCheck and bClipToROI as
      well as several version of prepare () methods which check and adapt the
      destination image if neccessary.
  */
  class Filter {
  public:
     /// change adaption of destination image (see class description)
     void setClipToROI (bool bClipToROI) {this->bClipToROI = bClipToROI;}
     void setCheckOnly (bool bCheckOnly) {this->bCheckOnly = bCheckOnly;}

  protected:
     /// Filter is a base class for other classes and should be instantiated
     Filter() : bClipToROI (true), bCheckOnly (false) {}
     ~Filter() {};

     /// check+adapt destination images parameters against given values
     /// bCheckOnly mode ignores the given imgSize
     bool prepare (ImgBase **ppoDst, depth eDepth, const Size &imgSize, 
                   format eFormat, int nChannels, const Rect& roi, 
                   Time timestamp=Time::null);

     /// check+adapt destination image to properties of given source image
     bool prepare (ImgBase **ppoDst, const ImgBase *poSrc) {
        return prepare (ppoDst, poSrc->getDepth(), chooseSize (poSrc),
                        poSrc->getFormat(), poSrc->getChannels (), chooseROI (poSrc),
                        poSrc->getTime());
     }

     /// check+adapt destination image to properties of given source image
     /// but use explicitly given depth
     bool prepare (ImgBase **ppoDst, const ImgBase *poSrc, depth eDepth) {
        return prepare (ppoDst, eDepth, chooseSize (poSrc),
                        poSrc->getFormat(), poSrc->getChannels (), chooseROI (poSrc),
                        poSrc->getTime());
     }

     /// return to-be-used image size depending on bClipToROI
     const Size chooseSize (const ImgBase *poSrc) {
        return bClipToROI ? poSrc->getROISize () : poSrc->getSize ();
     }
     /// return to-be-used ROI depending on bClipToROI
     const Rect chooseROI (const ImgBase *poSrc) {
        return bClipToROI ? Rect (Point::null, poSrc->getROISize ())
                          : poSrc->getROI();
     }

  protected:
     bool bClipToROI, bCheckOnly;
  };
}
#endif
