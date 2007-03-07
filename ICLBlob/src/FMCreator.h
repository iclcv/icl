#ifndef FMCREATOR_H
#define FMCREATOR_H

#include <Size.h>
#include <vector>
#include <ICLTypes.h>

namespace icl{
  /// interface for Feature Map creators
  /** see RegionBasedBlobSearcher for more details */
  struct FMCreator{

    /// Destructor
    virtual ~FMCreator(){};

    /// returns the demanded image size
    /** @param size size of the source image of the feature-map**/
    virtual Size getSize()=0;


    /// returns the demaned image format
    /** @return format for the source image of the feature-map**/
    virtual format getFormat()=0;
    
    /// create a featur-map from the given image (always 8u)
    /** @param image source image
        @return new feature map
    **/
    virtual Img8u* getFM(Img8u *image)=0;
    
    /// static function, that creates a default FMCreator with given parameters
    /** The new FMCreator will create the feature map using the following algorithm:
        \code
        I = input-image converted to size "imageSize" and format "imageFormat"
        {the example is written for a 3-channel rgb image it is generalizable for abitrary formats}
        [r,g,b] = "refcolor"       
        [tr,tg,tb] = thresholds  
        for all pixel p=(x,y) I do
           dr = abs( I(x,y,0) - r )
           dg = abs( I(x,y,1) - g )
           db = abs( I(x,y,2) - b )
           RESULT(x,y) = (dr<tr && dg<tg && db<tb) * 255;
        done

        \endcode

        @param size image size that is used by the new FMCreator
        @param fmt image format that is used by the new FMCreator
        @param refcolor reference color
        @param thresholds array of color component thresholds
        @return new default FMCreator object
    */
    static FMCreator *getDefaultFMCreator(const Size &size, 
                                          format fmt,
                                          std::vector<icl8u> refcolor,
                                          std::vector<icl8u> thresholds);
  };
}

#endif
