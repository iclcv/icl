#ifndef BLOBFINDER_H
#define BLOBFINDER_H

#include <iclRegionDetectorTypes.h>
#include <iclRegionDetectorBlobPart.h>
#include <iclRegionDetectorBlob.h>
#include <iclRegionDetectorScanLine.h>
#include <iclSize.h>

namespace icl{
  namespace regiondetector{
    
    /// Low-Level working class for image region detection \ingroup G_RD
    /**
        At lowest level, region detection in the ICL is performed using the RegionDetector class. The 
        region detector is able to process icl8u images only. Higher level classes like the
        ImgRegionDetector just wrap the RegionDetector class and extend its abilities. 
        
        \section _ALG Algorithm
        The RegionDetector searches the input image for connected regions with identical pixel value. Each 
        pixel is connected only to the 4 nearest neighbor pixels to the left, to the right, above and below.
        Internally the RegionDetector searches the image line by line and pixel by pixel.

        The whole algorithm can be seen in my Master Thesis "Das TDI-Framework für dynamische Lernarchitekturen
        in intelligenten, interaktiven Systemem"
    */
    class RegionDetector{
      public:
      
      /// Create a new instance with given parameters
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
      
      /// set the minimal pixel count for detected regions
      void setMinSize(int iMinSize);
      
      /// set the maximal pixel count for detected regions
      void setMaxSize(int iMaxSize);

      /// set the minimal pixel value for detected regions
      void setMinValue(int iMinSize);

      /// set the maximal pixel value for detected regions
      void setMaxValue(int iMaxSize);
      
      /// returns the current image processing size
      Size getSize(){ return Size(m_iW, m_iH); }

      /// sets the size of to be processed images
      void setSize(const Size &size);
      
      protected:

      // Internally used label image
      RegionDetectorBlobPart **m_ppoLim;
      
      int m_iW;          ///!< image width
      int m_iH;          ///!< image height
      int m_iDim;        ///!< image dim = width * height
      int m_iMinSize;    ///!< min region size
      int m_iMaxSize;    ///!< max region size
      int m_iMinValue;   ///!< min region value
      int m_iMaxValue;   ///!< max region value
      
      /// Internally used memory manager for blob parts
      BlobPartMemoryManager *m_poBlobPartMM;
      
      /// Internally used memory manager for scan lines
      ScanLineMemoryManager *m_poScanLineMM;

      /// Internally stored region data
      BlobList *m_poBlobList;
      
    };
    
  }
}

#endif
