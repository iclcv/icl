#ifndef DEF_COLOR_BLOB_SEARCHER_H
#define DEF_COLOR_BLOB_SEARCHER_H

#include <iclColorBlobSearcher.h>
#include <iclPixelRatingGroup.h>
#include <iclFastMedianList.h>
#include <stdlib.h>
#include <vector>



namespace icl{

   
  class DefaultColorBlobSearcher : public ColorBlobSearcher<icl8u,bool,float>{
    public:
    enum RatingCombinationType { rctOR, rctAND };
    enum CenterEstimationType { cetMean, cetMedian };

    DefaultColorBlobSearcher(const Size &imageSize);
    virtual ~DefaultColorBlobSearcher();
    
    typedef vector<FastMedianList> fmlVec;
    typedef vector<CenterEstimationType> cetVec;

    int addSubSearcher(const vector<icl8u> &rs,
                       const vector<icl8u> &gs, 
                       const vector<icl8u> &bs,
                       const icl8u thresholds[3],
                       RatingCombinationType rct=rctOR,
                       CenterEstimationType cet=cetMedian);

    /// just passing to the parent class
    virtual const FoundBlobVector &search(Img8u *poImage, Img8u *poMask);

    /// returns the current image size of this 
    const Size &getImageSize() const;
    void setImageSize(const Size &size);
    protected:
    
    virtual void prepareForNewImage(Img8u *poImage, Img8u *poMask);
    virtual void storeResult(int iPRIndex, int x, int y, bool rating);
    virtual void evaluateResults(FoundBlobVector &resultDestination);

    virtual void pixelRatingAdded(pixelrating *pr);
    virtual void pixelRatingRemoved(int index);
    
    private:
    
    Size m_oImageSize;
    fmlVec m_vecXMedianLists;
    fmlVec m_vecYMedianLists;
    cetVec m_vecCet;
    
  };
}
#endif

