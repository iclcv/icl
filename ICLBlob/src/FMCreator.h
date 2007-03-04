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
    virtual Size getSize()=0;

    /// returns the demaned image format
    virtual format getFormat()=0;
    
    /// create a featur-map from the given image (always 8u)
    virtual Img8u* getFM(Img8u *image)=0;
    
    /// static function, that creates a default FMCreator with given parameters
    static FMCreator *getDefaultFMCreator(const Size &size, 
                                          format fmt,
                                          std::vector<icl8u> refcolor,
                                          std::vector<icl8u> thresholds);
  };
}

#endif
