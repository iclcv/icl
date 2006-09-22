#ifndef DEF_COLOR_BLOB_SEARCHER_H
#define DEF_COLOR_BLOB_SEARCHER_H

#include "ColorBlobSearcher.h"
#include "PixelRatingGroup.h"
#include "FastMedianList.h"
#include <stdlib.h>
#include <vector>


namespace icl{

   
  class DefaultColorBlobSearcher : public ColorBlobSearcher<icl8u,bool,float>{
    public:
    enum RatingCombinationType { rctOR, rctAND };
    
    DefaultColorBlobSearcher(const Size &imageSize);
    virtual ~DefaultColorBlobSearcher();
    
    typedef vector<FastMedianList> fmlVec;

    int addNewBlob(const vector<icl8u> &rs,
                   const vector<icl8u> &gs, 
                   const vector<icl8u> &bs,
                   const icl8u thresholds[3],
                   RatingCombinationType rct=rctOR); 

    // just passing to the parent class
    virtual const FoundBlobVector &search(Img8u *poImage, Img8u *poMask);

    // returns the current image size of this 
    const Size &getImageSize() const;
    void setImageSize(const Size &size);
    protected:
    
    virtual void prepareForNewImage(Img8u *poImage, Img8u *poMask);
    virtual void storeResult(int iPRIndex, int x, int y, bool rating);
    virtual void evaluateResults(FoundBlobVector &roResultDestination);

    virtual void pixelRatingAdded(const pixelrating &pr);
    virtual void pixelRatingRemoved(const pixelrating &pr, int index);
    
    private:
    
    Size m_oImageSize;
    fmlVec m_vecXMedianLists;
    fmlVec m_vecYMedianLists;

    
  };
}
#endif

