#ifndef BLOB_H
#define BLOB_H

#include <RegionDetectorTypes.h>
#include <RegionDetectorScanLine.h>
#include <RegionDetectorBlobPart.h>
#include <Size.h>
#include <Rect.h>
#include <Point.h>
#include <Array.h>
#include <ICLTypes.h>

namespace icl {
  namespace regiondetector{
    // A RegionDetectorBlob represents an image region with homogenoues value
    
    class RegionDetectorBlob{
      public:
      RegionDetectorBlob(RegionDetectorBlobPart *r=0);
      ~RegionDetectorBlob();
      
      void getFeatures(Point &center, icl8u &val);
      
      /**
      getAllFeatures() stores the calculated features as long, as the data
      is not changed due to reset() end update()
      @see reset
      @see update()
      */
      void getAllFeatures(const Size &imageSize, Point &center, icl8u &val, Rect &bb,
                          float &l1, float &l2, float&arc1, float &arc2); 
      int size();
      void show();
      unsigned char val();
      void clear(){
        m_poPixels->clear();
        m_iDirty = 1;
      }
      void update(RegionDetectorBlobPart *r);
      ScanLineList *getPixels(){ return m_poPixels; }
      
      static void ensurePixelBufferSize(unsigned int size);
      
      static void showReferenceCounter(){
        printf("RegionDetectorBlob #=%d \n",s_iReferenceCounter);
      }
      private:
      
      void fetch(RegionDetectorBlobPart *r);//recursive
      
      int m_iDirty;
      Size m_oSize; //iW,iH
      Point m_oMean; // meanx meany
      Rect m_oBB; //bbx, bby, bbw, bbh
      float m_afPCA[4]; //l1, l2, arc1, arc2
      
      ScanLineList *m_poPixels;
      
      static int s_iReferenceCounter;
     
    };
  }
}



#endif
