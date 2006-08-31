#ifndef DEF_BLOB_SEARCHER_H
#define DEF_BLOB_SEARCHER_H

#include "ColorBlobSearcher.h"
#include "PixelRatingGroup.h"
#include "FastMedianList.h"
#include <stdlib.h>
#include <vector>


namespace icl{
  
  class DefaultBlobSearcher : public ColorBlobSearcher8u<bool>{
    public:
    enum RatingCombinationType { rctOR, rctAND };
    
    DefaultBlobSearcher(const Size &imageSize);
    virtual ~DefaultBlobSearcher();
    
    typedef vector<FastMedianList> fmlVec;

    void addNewBlob(const vector<icl8u> &rs,
                    const vector<icl8u> &gs, 
                    const vector<icl8u> &bs,
                    const icl8u thresholds[3],
                    RatingCombinationType rct); 


    protected:
    
    virtual void prepareForNewImage(Img8u *poImage, Img8u *poMask);
    virtual void storeResult(int iPRIndex, int x, int y, bool rating);
    virtual void evaluateResults(FoundBlobVector &roResultDestination);

    virtual void pixelRatingAdded(const pixelrating &pr);
    virtual void pixelRatingRemoved(const pixelrating &pr, int index);
    
    private:
    
    Size m_iImageSize;
    fmlVec m_vecXMedianLists;
    fmlVec m_vecYMedianLists;

    class RGBPixelRating : public PixelRating<icl8u,bool>{
      public:
      RGBPixelRating(const icl8u ref[3], const icl8u thresh[3]);
      virtual bool rate(icl8u r, icl8u g, icl8u b);
      
      protected:
      icl8u m_aucThresholds[3];
    };
    
    class RatingGroupOR : public PixelRatingGroup<icl8u,bool>{
      public:
      virtual bool combineRatings(const vector<bool> &rvecSubRatings );
    };
    
    class RatingGroupAND : public PixelRatingGroup<icl8u,bool>{
      public:
      virtual bool combineRatings(const vector<bool> &rvecSubRatings );
    };
  };
}
#endif

