#ifndef COLOR_BLOB_SEARCHER_H
#define COLOR_BLOB_SEARCHER_H

#include "PixelRatingGroup.h"
#include "FoundBlob.h"
#include <vector>
#include "Macros.h"
#include "Img.h"


namespace icl{

  using std::vector;
  
  /// interface for a single color blob searcher
  /** The ColorBlobSearcher Interface provides as well the functionality
      for different Blob searching Algorithms (working pixelwise and
      reference color based), as a dynamic interface for custom 
      implementations.
  
      <h2>Slice Model</h2>
      The detection of image blobs is brocken apart into slice model,
      for a generalizing abstraction that is <em>as dynamic as possible</em>. (~~~)
  <pre>
  +--------------------------------------------------------+
  | .. Higher level combination of different               |
  |      ColorBlobSearchers ...                            |
  |                                                        |
  |                                                        |
  |                                                        |
  |                                                        |
  |                                                        |
  |                                                        |
  +========================================================+
  | ColorBlobSearcher<P,R,B>:                              |
  | This 3rd layers interface provides functions to evalu- |
  | ate the results of a set of PixelRating(Groups). To    |
  | search for blobs in a given image, it will iterate     |
  | line by line over the images ROI (leaving out masked   |
  | pixels) and store the results of each PixelRating by   | 
  | calling the abstract function: storeResults(...).      |
  | after the iteration cycle over the images pixels,      |
  | another abstract function combineResults is called.    |
  | By implementing these two abstract functions, the pro- |
  | grammer is able to construct a large viriety of diffe- |
  | rent searching algorithms.                             |
  +========================================================+
  | PixelRatingGroup<T>: public PixelRating<T>             |
  | This higher level class defines the interface for      |
  | grouping a set of PixelRating<T>s together and combine |
  | their results with an abritrary combination function.  |
  | As a PixelRatingGroup itself inherits the PixelRating  |
  | interface, PixelRatingGroups may contain other Groups. |
  | When the group is asked for a rating of a given Pixel, |
  | it will collect the results of all contained Pixel-    |
  | Ratings and return the combined Results, computed by   |
  | the defined "combine"-function.                        |
  +========================================================+
  | PixelRating<T>                                         |
  | The abstract PixelRating class defines an interface for|
  | different reference color based rating functions. The  |
  | co-domain of an implemented functions is determined by |
  | the template parameter T. In simple cases T might be   |
  | bool, so the rating implements a binary discriminator, |
  | which decides if pixels are "good" or "not good". By   |
  | using floats as rating types, it is furthermore        |
  | possible to pass a higher level rating to the above    |
  | layers.                                                |
  +--------------------------------------------------------+

  </pre>
  
  */
  template <class PixelType,class RatingType,class BlobRatingType>
  class ColorBlobSearcher : public PixelRatingGroup<PixelType,RatingType>{
    public:
    typedef FoundBlob<BlobRatingType> foundblob;
    typedef vector<foundblob> FoundBlobVector;
    typedef PixelRating<PixelType,RatingType> pixelrating;
    typedef std::vector<pixelrating*> PixelRatingVector;
   
    virtual ~ColorBlobSearcher();
    virtual const FoundBlobVector &search(Img<PixelType> *poImage, Img8u *poMask);

    virtual void addPR(pixelrating *p);
    virtual void removePR(int index);
    
    protected:
    virtual void prepareForNewImage(Img<PixelType> *poImage, Img8u *poMask);
    virtual void storeResult(int iPRIndex, int x, int y, RatingType rating)=0;
    virtual void evaluateResults(FoundBlobVector &destination)=0;
    virtual void feedback(const FoundBlobVector &results, Img<PixelType> *poImage);
    
    virtual void pixelRatingAdded(pixelrating *pr);
    virtual void pixelRatingRemoved(int index);
    
    private:
    
    FoundBlobVector m_vecFoundBlobs;
  };
}

#endif
