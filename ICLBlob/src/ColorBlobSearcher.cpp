/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/src/ColorBlobSearcher.cpp                      **
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

#include <ICLBlob/ColorBlobSearcher.h>

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

