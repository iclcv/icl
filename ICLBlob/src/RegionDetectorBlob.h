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

  class BlobData;
  
  namespace regiondetector{
    // A RegionDetectorBlob represents an image region with homogenoues value
    
    class RegionDetectorBlob{
      public:
      friend class icl::BlobData;
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
      
      /// returns a list of all boundary pixels (beginning with left most pixel of the first line)
      /** @param imageSize corresponding image size 
          @return list of boundary Pixels
      **/
      std::vector<Point> getBoundary(const Size &imageSize);

      /// retursn the count of all boundary pixels
      /** This function is an optimization: although calling
          getBoundary(size).size() will also return the lenght
          of the boundary, the getBoundaryLength function
          is much more efficient. The better performace is
          is achieved by just counting the border pixels instead 
          of pushing them into a list of points.
          @param imagesSize corresponding image size
          @return count of boundary pixels
      **/
      int getBoundaryLength(const Size &imageSize);
      
      /** returns the forma factor of this blob ( U²/(4*PI*A) )
          @param imageSize corresponding image size
          @return formfactor of this blob
      **/
      float getFormFactor(const Size &imageSize);
      
      int size();
      void show();
      unsigned char val();
      void clear(){
        m_poPixels->clear();
        m_iDirty = 1;
      }
      void update(RegionDetectorBlobPart *r);
      ScanLineList *getPixels(){ return m_poPixels; }

      unsigned char *getImageDataPtr();
      
      static void ensurePixelBufferSize(unsigned int size);
      
      static void showReferenceCounter(){
        printf("RegionDetectorBlob #=%d \n",s_iReferenceCounter);
      }

      private:
      void getStartPixel(int &xStart,int &yStart);
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
