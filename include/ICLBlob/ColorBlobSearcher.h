/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/ColorBlobSearcher.h                    **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter, Robert Haschke                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef COLOR_BLOB_SEARCHER_H
#define COLOR_BLOB_SEARCHER_H

#include <ICLBlob/PixelRatingGroup.h>
#include <ICLBlob/FoundBlob.h>
#include <vector>
#include <ICLUtils/Macros.h>
#include <ICLCore/Img.h>


namespace icl{

  /// abstract interface for a single color blob searcher \ingroup G_CBS
  /** The ColorBlobSearcher Interface provides as well the functionality
      for different Blob searching Algorithms (working pixelwise and
      reference color based), as a dynamic interface for custom 
      implementations.
  
      <h2>Slice Model</h2>
      The detection of image blobs is brocken apart into slice model,
      for a generalizing abstraction that is <em>as dynamic as possible</em>. 
      
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
    
    /// internal type definition
    typedef FoundBlob<BlobRatingType> foundblob;

    /// internal type definition
    typedef std::vector<foundblob> FoundBlobVector;

    /// internal type definition
    typedef PixelRating<PixelType,RatingType> pixelrating;

    /// internal type definition
    typedef std::vector<pixelrating*> PixelRatingVector;
   
    /// Destructor
    virtual ~ColorBlobSearcher();
    
    /// extracts all blobs
    virtual const FoundBlobVector &search(Img<PixelType> *poImage, Img8u *poMask);

    /// inserts a new  pixel rating a last index (passing ownership of p to this class)
    virtual void addPR(pixelrating *p);
    
    /// removes a pixel rating at given index
    virtual void removePR(int index);
    
    protected:
    /// internally used function ( can be reimplemented )
    virtual void prepareForNewImage(Img<PixelType> *poImage, Img8u *poMask);

    /// internally used function ( can be reimplemented )
    virtual void storeResult(int iPRIndex, int x, int y, RatingType rating)=0;

    /// internally used function ( can be reimplemented )
    virtual void evaluateResults(FoundBlobVector &destination)=0;

    /// internally used function ( can be reimplemented )
    virtual void feedback(const FoundBlobVector &results, Img<PixelType> *poImage);
    
    /// internally used function ( can be reimplemented )
    virtual void pixelRatingAdded(pixelrating *pr);

    /// internally used function ( can be reimplemented )
    virtual void pixelRatingRemoved(int index);
    
    private:
    /// internal storage of the current set of found blobs
    FoundBlobVector m_vecFoundBlobs;
  };
}

#endif
