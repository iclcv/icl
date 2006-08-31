#include "DefaultBlobSearcher.h"


namespace icl{

  // {{{ DefaultBlobSearcher

  DefaultBlobSearcher::DefaultBlobSearcher(const Size &imageSize):m_iImageSize(imageSize){
    // {{{ open  }  // }}}

  DefaultBlobSearcher::~DefaultBlobSearcher(){
    // {{{ open  }  // }}}
  
  void DefaultBlobSearcher::prepareForNewImage(Img8u *poImage, Img8u *poMask){
    // {{{ open    (void)poImage; (void)poMask;    for(int i=0;i<getNumPR();i++){      m_vecXMedianLists[i].clear();      m_vecYMedianLists[i].clear();    }  }  // }}}
  
  void DefaultBlobSearcher::storeResult(int iPRIndex, int x, int y, bool rating){
    // {{{ open    if(rating){      m_vecXMedianLists[iPRIndex].add(x);      m_vecYMedianLists[iPRIndex].add(y);    }      }  // }}}

  void DefaultBlobSearcher::evaluateResults( DefaultBlobSearcher::FoundBlobVector &dst ){
    // {{{ open    for(int i=0;i<getNumPR();i++){      dst[i]= foundblob(m_vecXMedianLists[i].median(),m_vecYMedianLists[i].median(),i,0);    }  }  // }}}
  
  void DefaultBlobSearcher::pixelRatingAdded(const DefaultBlobSearcher::pixelrating &pr){
    // {{{ open    m_vecXMedianLists.push_back(FastMedianList(m_iImageSize.width));    m_vecYMedianLists.push_back(FastMedianList(m_iImageSize.height));  }  // }}}

  void DefaultBlobSearcher::pixelRatingRemoved(const DefaultBlobSearcher::pixelrating &pr, int index){
    // {{{ open    (void)pr;    m_vecXMedianLists.erase(m_vecXMedianLists.begin()+index);    m_vecYMedianLists.erase(m_vecYMedianLists.begin()+index);  }   // }}}

  void DefaultBlobSearcher::addNewBlob(const vector<icl8u> &rs, 
                                       const vector<icl8u> &gs,
                                       const vector<icl8u> &bs,     
                                       const icl8u thresholds[3],
                                       DefaultBlobSearcher::RatingCombinationType rct){
    // {{{ open    PixelRatingGroup<icl8u,bool> rg;    if(rct == rctOR) rg = RatingGroupOR();    else rg = RatingGroupAND();                           icl8u ref[3];    for(uint i=0;i<rs.size() && i<gs.size() && i<bs.size(); i++){      ref[0] = rs[i];      ref[1] = gs[i];      ref[2] = bs[i];      rg.addPR(RGBPixelRating(ref,thresholds));    }      }  // }}}



  // }}}

  
  // {{{ DefaultBlobSearcher::RGBPixelRating  DefaultBlobSearcher::RGBPixelRating::RGBPixelRating(const icl8u ref[3], const icl8u thresh[3]):    PixelRating<icl8u,bool>(ref[0],ref[1],ref[2]){    m_aucThresholds[0] = thresh[0];    m_aucThresholds[1] = thresh[1];    m_aucThresholds[2] = thresh[2];  }    bool DefaultBlobSearcher::RGBPixelRating::rate(icl8u r, icl8u g, icl8u b){    return r-m_ref0 < m_aucThresholds[0] &&           g-m_ref1 < m_aucThresholds[1] &&           b-m_ref2 < m_aucThresholds[2];  }  // }}}
  
  // {{{ DefaultBlobSearcher::RatingGroupOR  bool DefaultBlobSearcher::RatingGroupOR::combineRatings(const vector<bool> &ratings){    for(vector<bool>::const_iterator it = ratings.begin();it!=ratings.end(); ++it){      if(*it)return true;    }    return false;  }  // }}}

  // {{{ DefaultBlobSearcher::RatingGroupAND  bool DefaultBlobSearcher::RatingGroupAND::combineRatings(const vector<bool> &ratings){    for(vector<bool>::const_iterator it = ratings.begin();it!=ratings.end(); ++it){      if(!(*it))return false;    }    return true;  }  // }}}

}
