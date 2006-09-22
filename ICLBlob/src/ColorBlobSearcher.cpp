#include "ColorBlobSearcher.h"

namespace icl{
  
  template <typename PixelType,typename RatingType,typename BlobRatingType>
  const vector<FoundBlob<BlobRatingType> > &ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::search(Img<PixelType> *poImage, Img8u *poMask){
    
    ICLASSERT_RETURN_VAL( poImage->getChannels() == 3 , m_vecFoundBlobs );
    icl8u *mask = 0;
    if(poMask){
      mask = poMask->getData(0);
      ICLASSERT_RETURN_VAL( poImage->getSize() == poMask->getSize() , m_vecFoundBlobs );
    }
    
    prepareForNewImage(poImage,poMask);
    
    PixelType *pt0 = poImage->getData(0);
    PixelType *pt1 = poImage->getData(1);
    PixelType *pt2 = poImage->getData(2);

    Rect oRoi = poImage->getROI();
    
    int x,y,i,d;
    int iImageW = poImage->getSize().width;
    if(mask){
      for(y =oRoi.y; y<oRoi.bottom();y++){
        for(x = oRoi.x; x<oRoi.right();x++){
          i = x+y*iImageW;
          if(mask[i]){
            for(d=0;d<(int)m_vecPixelRatings.size();d++){
              storeResult(d,x,y,m_vecPixelRatings[d].rate(pt0[i],pt1[i],pt2[i]));
            }
          }
        }
      }
    }else{
      for(y =oRoi.y; y<oRoi.bottom();y++){
        for(x = oRoi.x; x<oRoi.right();x++){
          i = x+y*iImageW;
          for(d=0;d<(int)m_vecPixelRatings.size();d++){
            storeResult(d,x,y,m_vecPixelRatings[d].rate(pt0[i],pt1[i],pt2[i]));
          }
        }
      }        
    }
    evaluateResults(m_vecFoundBlobs);
    
    return m_vecFoundBlobs;
  }

  template <class PixelType,class RatingType,class BlobRatingType>
  int ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::addPR(const PixelRating<PixelType, RatingType> &pr){
    m_vecPixelRatings.push_back(pr);
    pixelRatingAdded(pr);
    m_vecFoundBlobs.resize(m_vecPixelRatings.size());
    return m_vecPixelRatings.size()-1;
  }

  template <class PixelType,class RatingType,class BlobRatingType>
  const std::vector<PixelRating<PixelType,RatingType> >&
  ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::getAllPixelRatings() const{
    return m_vecPixelRatings;
  }

  
  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::removePR(int iIndex){
    if((int)m_vecPixelRatings.size()>iIndex){
      PixelRating<PixelType,RatingType> pr = m_vecPixelRatings[iIndex];
      m_vecPixelRatings.erase(m_vecPixelRatings.begin()+iIndex);
      pixelRatingRemoved(pr,iIndex);
      m_vecFoundBlobs.resize(m_vecPixelRatings.size());
    }
  }
  
  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::feedback(const FoundBlobVector &roLastResults, Img<PixelType> *poImage){
    (void)roLastResults; (void)poImage;
  }
  
  template <class PixelType,class RatingType,class BlobRatingType>
  ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::~ColorBlobSearcher(){}

  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::pixelRatingAdded(const PixelRating<PixelType, RatingType> &pr){
    (void)pr;
  }

  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::pixelRatingRemoved(const PixelRating<PixelType, RatingType> &pr, int index){
    (void)pr;(void)index;
  }

  template <class PixelType,class RatingType,class BlobRatingType>
  int ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::getNumPR() const {
    return  (int)(m_vecPixelRatings.size());
  }
  
  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::prepareForNewImage(Img<PixelType> *poImage, Img8u *poMask) {
    (void)poImage; (void)poMask;
  }
  
  template class ColorBlobSearcher<unsigned char, bool, float>;
  template class ColorBlobSearcher<float, bool, float>;
}

