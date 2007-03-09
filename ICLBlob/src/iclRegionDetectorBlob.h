#include <iclRegionDetectorTypes.h>
#include <iclRegionDetectorScanLine.h>
#include <iclRegionDetectorBlobPart.h>
#include <iclSize.h>
#include <iclRect.h>
#include <iclPoint.h>
#include <iclArray.h>
#include <iclTypes.h>
#ifndef BLOB_H
#define BLOB_H


namespace icl {

  class BlobData;
  
  namespace regiondetector{
    /// A RegionDetectorBlob represents an image region with homogenoues value
    /** The RegionDetectorBlob complies a fundamental structure in for storing
        found blob data and to computer features for the found blobs.
        The RegionDetectorBlob class provides all low level features that are
        produced in the blob detection mechanism. This knowledge is stored in
        a list of so called ScanLines (see RegionDetector). To work on this 
        low-level data struct, the getScanLines() function can be used. In most
        cases the following high-level blob features provide better information
        to work with:
        - <b>value</b> 
          the uchar value of this blob (all pixels have this value)
        - <b>size</b>
          the pixel count / mass of this blob
        - <b>center of gravity</b> 
          the center of of mass of this blob
        - <b>local pca information</b>
          orientation and length of the axis of highes variance (lenght is calculated as f(sqrt(var))
        - <b>boundary length</b>
          the count of boundary pixels
        - <b>boundary pixel list</b> 
          a list of all boundary pixels beginning with upper left pixel and sorted clock-wise
        - <b>Form factor</b> 
          formular: ( U²/(4*PI*A) )
        
        Because of the data storagement and low level functions, the RegionDetectorBlob class is
        very complex. A simplified access to all high-level blob information is accessible by 
        using the ImgRegionDetector class, which provides a list of so called "BlobData" structs, 
        that wraps the RegionDetectorBlobs data access functions.

        \section sec2 Performance
        The RegionDetectorBlob calculates all information on demand, and stores the calculation 
        results for further access internally. Some of the calculation functions allow to calculate
        other features simultanously without the cost of much additional time. E.g. when calculating
        a list of all boundary pixles, the count of these pixels is implicitly calculated and stored.
    */
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
      void getAllFeatures(Point &center, icl8u &val, Rect &bb, float &l1, float &l2, float&arc1, float &arc2); 
      
      /// returns the value of the blobs pixels
      unsigned char getVal();
      
      /// returns the pixel count of this blob
      int getSize();

      /// returns the center of gravity of this blob
      const Point &getCOG();

      /// returns the bounding box of this blob
      const Rect &getBoundingBox();
      
      /// resturs the pca information for this blob
      /** @return data order [length1, length2, arc1 and arc2] */
      const std::vector<float> &getPCAInfo();
      
      /// returns a list of all boundary pixels (beginning with left most pixel of the first line)
      /** @param imageSize corresponding image size 
          @return list of boundary Pixels
      **/
      const std::vector<Point> &getBoundary(const Size &imageSize);

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
      
      /// returns the forma factor of this blob ( U²/(4*PI*A) )
      /** @param imageSize corresponding image size
          @return formfactor of this blob
      **/
      float getFormFactor(const Size &imageSize);
      
      /// shows the blob center and value to std::out
      void show();
      
      /// clears all data from the blob and sets all states to dirty
      void clear();
      
      /// updates this blob by fetching a blob part structure recursively
      void update(RegionDetectorBlobPart *r);
      
      /// returns the internal scan line list
      const ScanLineList *getScanLines() const { return m_poPixels; }
      
      /// returns a pointer to the corresponding image data origin
      unsigned char *getImageDataPtr();
      
      private:
      
      /// calculate the upper left pixel of this blob
      void getStartPixel(int &xStart,int &yStart);
      
      /// recursively collects scanline from the low-level blob part struct
      void fetch(RegionDetectorBlobPart *r);//recursive
      
      /// updates the COG information on demand
      void calculateCOG();

      /// updates the Size information on demand
      void calculateSize();

      /// updates the Bounding Box information on demand
      void calculateBoundingBox();

      /// updates the PCA information on demand
      void calculatePCA();

      /// updates the Boundary information on demand
      void calculateBoundary(const Size &imageSize);

      /// updates the Boundary Length information on demand
      void calculateBoundaryLength(const Size &imageSize);

      /// updates the FormFactor information on demand
      void calculateFormFactor(const Size &imageSize);

      /// updates the Size, BB and PCA information on demand (TODO: optimize)
      void calculateSizeBBAndPCA();

      
      /// internal flag indicating that the current blob size information is outdated
      bool m_bSizeDirty;

      /// internal flag indicating that the current COG information is outdated
      bool m_bCOGDirty;

      /// internal flag indicating that the current BB information is outdated
      bool m_bBBDirty;

      /// internal flag indicating that the current PCA information is outdated
      bool m_bPCADirty;

      /// internal flag indicating that the current Boundary information is outdated
      bool m_bBoundaryDirty;

      /// internal flag indicating that the current boundary length information is outdated
      bool m_bBoundaryLengthDirty;

      /// internal flag indicating that the current form factor information is outdated
      bool m_bFormFactorDirty;
      
      /// internal storage for current blob center
      Point m_oCOG;  

      /// internal storage for current BB
      Rect m_oBB; 

      /// internal storage for current PCA information
      std::vector<float> m_oPCA;

      /// internal storage for current blob boundary pixels
      std::vector<Point> m_oBoundary; 

      /// internal storage for current boundary length
      int m_iBoundaryLength;

      /// internal storage for current blob size
      int m_iSize;
      
      /// internal storage for current form factor
      float m_fFormFactor;

      /// low level data (list of ScanLines)
      ScanLineList *m_poPixels;
    };
  }
}



#endif
