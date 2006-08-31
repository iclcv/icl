#ifndef PIXEL_RATING_GROUP_H
#define PIXEL_RATING_GROUP_H

#include "PixelRating.h"
#include <vector>

namespace icl{
  
  /// Provides abilities for individual combinations of pixleratings
  template<class PixelType,class RatingType>
  class PixelRatingGroup : public PixelRating<PixelType,RatingType>{

    typedef PixelRating<PixelType,RatingType> pixelrating;
    typedef std::vector<pixelrating> PixelRatingVector;
    
    public:
    virtual RatingType rate(PixelType t0, PixelType t1, PixelType t2){
      for(uint i=0;i<m_vecSubDis.size();i++){
        m_vecSubResults[i] = m_vecSubDis[i].rate(t0,t1,t2);
      }
      return combineRatings(m_vecSubResults);
    } 
    virtual RatingType combineRatings(const vector<RatingType> &rvecSubRatings ){
      (void)rvecSubRatings;
      return RatingType();
    }
    
    void addPR(const pixelrating &subRating){
      m_vecSubResults.push_back(0);
      m_vecSubDis.push_back(subRating);
    }
    
    void removePR(int iIndex){
      m_vecSubResults.erase(m_vecSubResults.begin()+iIndex);    
      m_vecSubDis.erase(m_vecSubDis.begin()+iIndex);
    }
    
    private:
    vector<RatingType> m_vecSubResults;
    PixelRatingVector m_vecSubDis;
  };
}

#endif
