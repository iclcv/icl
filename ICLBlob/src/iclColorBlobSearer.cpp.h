#include <iclColorBlobSearcher.h>

namespace icl{
  
  template <typename PixelType,typename RatingType,typename BlobRatingType>
  const vector<FoundBlob<BlobRatingType> > &ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::
  search(Img<PixelType> *poImage, Img8u *poMask){
    
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
            for(d=0;d<(int)PixelRatingGroup<PixelType,RatingType>::m_vecPR.size();d++){
              storeResult(d,x,y,PixelRatingGroup<PixelType,RatingType>::m_vecPR[d]->rate(pt0[i],pt1[i],pt2[i]));
            }
          }
        }
      }
    }else{
      for(y =oRoi.y; y<oRoi.bottom();y++){
        for(x = oRoi.x; x<oRoi.right();x++){
          i = x+y*iImageW;
          for(d=0;d<(int)PixelRatingGroup<PixelType,RatingType>::m_vecPR.size();d++){
            storeResult(d,x,y,PixelRatingGroup<PixelType,RatingType>::m_vecPR[d]->rate(pt0[i],pt1[i],pt2[i]));
          }
        }
      }
    }
    evaluateResults(m_vecFoundBlobs);
    return m_vecFoundBlobs;
  }

  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::addPR(PixelRating<PixelType, RatingType> *pr){
    PixelRatingGroup<PixelType,RatingType>::addPR(pr);
    pixelRatingAdded(pr);
  }

  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::removePR(int index){
    if( PixelRatingGroup<PixelType,RatingType>::removePR(index) ){
      pixelRatingRemoved(index);
    }
  }
  
  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::feedback(const FoundBlobVector &results, Img<PixelType> *image){
    (void)results; (void)image;
  }
  
  template <class PixelType,class RatingType,class BlobRatingType>
  ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::~ColorBlobSearcher(){}

  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::pixelRatingAdded(PixelRating<PixelType, RatingType> *pr){
    (void)pr;
  }

  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::pixelRatingRemoved(int index){
    (void)index;
  }

  template <class PixelType,class RatingType,class BlobRatingType>
  void ColorBlobSearcher<PixelType,RatingType,BlobRatingType>::prepareForNewImage(Img<PixelType> *poImage, Img8u *poMask) {
    (void)poImage; (void)poMask;
  }
  
  template class ColorBlobSearcher<unsigned char, bool, float>;
  template class ColorBlobSearcher<float, bool, float>;
}

