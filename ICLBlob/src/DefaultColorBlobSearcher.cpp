#include "DefaultColorBlobSearcher.h"


namespace icl{

  // {{{ RGBPixelRating

  class RGBPixelRating : public PixelRating<icl8u,bool>{
  public:
    RGBPixelRating(const icl8u ref[3], const icl8u thresh[3]):
      PixelRating<icl8u,bool>(ref[0],ref[1],ref[2]){
      m_ucThresh0 = thresh[0];
      m_ucThresh1 = thresh[1];
      m_ucThresh2 = thresh[2];
    }
    virtual bool rate(icl8u r, icl8u g, icl8u b){
      return  
        abs(m_ref0-r) < m_ucThresh0 &&
        abs(m_ref1-g) < m_ucThresh1 &&
        abs(m_ref2-b) < m_ucThresh2;
    }
    
  protected:
    icl8u m_ucThresh0, m_ucThresh1, m_ucThresh2;
  };

  // }}}
    
  // {{{ RatingGroupOR

  struct RatingGroupOR : public PixelRatingGroup<icl8u,bool>{
    virtual bool combineRatings(const vector<bool> &ratings){
      for(vector<bool>::const_iterator it = ratings.begin();it!=ratings.end(); ++it){
        if(*it)return true;
      }
      return false;
    }
  };

  // }}}
  
  // {{{ RatingGroupAND

  struct RatingGroupAND : public PixelRatingGroup<icl8u,bool>{
    virtual bool combineRatings(const vector<bool> &ratings){
      for(vector<bool>::const_iterator it = ratings.begin();it!=ratings.end(); ++it){
        if(!(*it))return false;
      }
      return true;
    }
  };

  // }}}

  // {{{ DefaultColorBlobSearcher

  DefaultColorBlobSearcher::DefaultColorBlobSearcher(const Size &imageSize):m_oImageSize(imageSize){
    // {{{ open
  }
  // }}}

  DefaultColorBlobSearcher::~DefaultColorBlobSearcher(){
    // {{{ open

  }

  // }}}
  
  void DefaultColorBlobSearcher::prepareForNewImage(Img8u *poImage, Img8u *poMask){
    // {{{ open

    (void)poImage; (void)poMask;
    for(int i=0;i<getNumPR();i++){
      m_vecXMedianLists[i].clear();
      m_vecYMedianLists[i].clear();
    }
  }

  // }}}
  
  void DefaultColorBlobSearcher::storeResult(int iPRIndex, int x, int y, bool rating){
    // {{{ open

    if(rating){
      m_vecXMedianLists[iPRIndex].add(x);
      m_vecYMedianLists[iPRIndex].add(y);
    }    
  }

  // }}}

  void DefaultColorBlobSearcher::evaluateResults(DefaultColorBlobSearcher::FoundBlobVector &dst ){
    // {{{ open
    dst.clear();
    for(int i=0;i<getNumPR();i++){
      dst.push_back(foundblob(m_vecXMedianLists[i].median(),m_vecYMedianLists[i].median(),i,0));
    }
  }

  // }}}
  
  void DefaultColorBlobSearcher::pixelRatingAdded(const DefaultColorBlobSearcher::pixelrating &pr){
    // {{{ open
    m_vecXMedianLists.push_back(FastMedianList(m_oImageSize.width));
    m_vecYMedianLists.push_back(FastMedianList(m_oImageSize.height));
  }

  // }}}

  void DefaultColorBlobSearcher::pixelRatingRemoved(const DefaultColorBlobSearcher::pixelrating &pr, int index){
    // {{{ open

    (void)pr;
    m_vecXMedianLists.erase(m_vecXMedianLists.begin()+index);
    m_vecYMedianLists.erase(m_vecYMedianLists.begin()+index);
  } 

  // }}}

  const DefaultColorBlobSearcher::FoundBlobVector &DefaultColorBlobSearcher::search(Img8u *poImage, Img8u *poMask){
    // {{{ open

    return ColorBlobSearcher<icl8u,bool,float>::search(poImage,poMask);  
  }

  // }}}

  int DefaultColorBlobSearcher::addNewBlob(const vector<icl8u> &rs, 
                                            const vector<icl8u> &gs,
                                            const vector<icl8u> &bs,     
                                            const icl8u thresholds[3],
                                            DefaultColorBlobSearcher::RatingCombinationType rct){
    // {{{ open
    PixelRatingGroup<icl8u,bool> rg;
    if(rct == rctOR) rg = RatingGroupOR();
    else rg = RatingGroupAND();
                       
    icl8u ref[3];
    for(uint i=0;i<rs.size() && i<gs.size() && i<bs.size(); i++){
      ref[0] = rs[i];
      ref[1] = gs[i];
      ref[2] = bs[i];
      rg.addPR(RGBPixelRating(ref,thresholds));
    }  
    return addPR(rg);
  }

  // }}}

  const Size &DefaultColorBlobSearcher::getImageSize() const {
    // {{{ open
    return m_oImageSize;
  }

  // }}}
  
  void DefaultColorBlobSearcher::setImageSize(const Size &size){
    // {{{ open
    if(m_oImageSize != size){
      m_oImageSize = size;
      uint nPixelRatings = m_vecXMedianLists.size();
      m_vecXMedianLists.clear();
      m_vecYMedianLists.clear();
      for(uint i=0;i<nPixelRatings;i++){
        m_vecXMedianLists.push_back(FastMedianList(m_oImageSize.width));
        m_vecYMedianLists.push_back(FastMedianList(m_oImageSize.height));
      }    
    }
  }

  // }}}

  // }}}

 
}
