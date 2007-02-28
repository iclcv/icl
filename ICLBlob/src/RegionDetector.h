#ifndef BLOBFINDER_H
#define BLOBFINDER_H

#include <RegionDetectorTypes.h>
#include <RegionDetectorBlobPart.h>
#include <RegionDetectorBlob.h>
#include <RegionDetectorScanLine.h>
#include <Size.h>

namespace icl{
  namespace regiondetector{
    
    /**
    The RegionDetector class is the interface that has to be
    used to find blobs in images
    */
    class RegionDetector{
      public:
      
      /**
      creates a RegionDetector instance, that is prepared for searching for blobs in
      images with size iW x iH and SE aches for all blobs with minimum size 
      iMinSize and value in range iMinValue..iMaxValue
      */
      RegionDetector(int iW, int iH, int iMinSize,int iMaxSize, int iMinValue=0, int iMaxValue=255);
      
      //Desturctor
      ~RegionDetector();
      /**
      searches for blobs in the given image-data. The returned RegionDetectorBlobList must not be
      deleted, as it is owned by the RegionDetector object
      */
      BlobList *find_blobs(unsigned char *pucData);
      
      void setMinSize(int iMinSize);
      void setMaxSize(int iMaxSize);
      void setMinValue(int iMinSize);
      void setMaxValue(int iMaxSize);
      
      Size getSize(){ return Size(m_iW, m_iH); }
      void setSize(const Size &size);
      
      protected:
      
      

      // Lable image
      RegionDetectorBlobPart **m_ppoLim;
      
      int m_iW;
      int m_iH;
      int m_iDim;
      int m_iMinSize;
      int m_iMaxSize;
      int m_iMinValue;
      int m_iMaxValue;
      
      BlobPartMemoryManager *m_poBlobPartMM;
      ScanLineMemoryManager *m_poScanLineMM;
      BlobList *m_poBlobList;
      
    };
    
  }
}

#endif
