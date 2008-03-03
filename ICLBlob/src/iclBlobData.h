#ifndef BLOB_DATA_H
#define BLOB_DATA_H

#include <iclPoint.h>
#include <iclPoint32f.h>
#include <iclRect.h>
#include <iclTypes.h>
#include <vector>
#include <iclMacros.h>
#include <iclRegionDetectorBlob.h>


namespace icl{

  /// data-struct to represent local PCA information \ingroup G_RD
  struct PCAInfo{
    /* {{{ open */

    /// Default Constructor
    PCAInfo(float len1=0, float len2=0, float arc1=0, float arc2=0, int cx=0, int cy=0):
      len1(len1),len2(len2),arc1(arc1),arc2(arc2),cx(cx),cy(cy){}
    /// length of first major axis
    float len1; 
    /// length of second major axis
    float len2; 
    /// angle of the first major axis
    float arc1; 
    /// angle of the second major axis (arc1+PI/2)
    float arc2; 
    /// x center of the region
    int cx;
    /// y center of the region
    int cy;

    /// null PCAInfo
    static const PCAInfo null;
  };

  /* }}} */
  
  /// data-struct to represent a scan line \ingroup G_RD
  struct ScanLine{
    /* {{{ open */

    /// Default constructor
    ScanLine(int x=0,int y=0,int len=0):x(x),y(y),len(len){}
    int x; ///< x-offset of this horizontal line 
    int y; ///< y-offset of this horizontal line 
    int len; ///< length of this line

    /// null ScanLine
    static const ScanLine null;
  };

  /* }}} */


  /// Wrapper sturct for the low level RegionDetectorBlob DataStruct \ingroup G_RD
  class BlobData{
    public:
    /// creates a new BlobData struct
    /** @param poRDB wrapped RegionDetectorBlob 
        @param oImageSize corresponding image size
    **/
    BlobData(regiondetector::RegionDetectorBlob *poRDB=0, const Size &oImageSize=Size::null);

    /// Destructor
    ~BlobData();
    /// returns the blobs value
    icl8u getVal() const;

    /// returns the blobs COG
    Point getCOG() const;

    /// returns the blobs COG with floating point precision
    Point32f getCOGFloat() const;

    /// returns the blobs pixel count
    int getSize() const;
    
    /// returns the blobs boundary length (size of boundary pixels)
    int getBoundaryLength() const;
    
    /// returns the blobs form factor (see RegionDetectorBlob)
    float getFormFactor() const;
   
    /// returns the blobs bounding box
    Rect getBoundingBox() const;
    
    /// returns the blobs pca information
    PCAInfo getPCAInfo() const;
  
    /// returns a list of boundary pixels for this blob
    const std::vector<Point> &getBoundary() const;

    /// returs the list of all pixels of this blob
    inline std::vector<Point> getPixels() const{
      /* {{{ open */

      FUNCTION_LOG("");
      ICLASSERT_RETURN_VAL(m_poRDB,std::vector<Point>());
      std::vector<Point> pxls;
      icl::regiondetector::ScanLineList &sll = *(m_poRDB->m_poPixels);
      for(icl::regiondetector::ScanLineList::iterator it=sll.begin();it!=sll.end();++it){
        icl::regiondetector::RegionDetectorScanLine *pl = (*it);
        const int iY=pl->y(),iEnd = pl->getEnd();
        for(int x=pl->getStart();x<=iEnd;pxls.push_back(Point(x++,iY))) ;
      }
      return pxls;
    }

    /* }}} */

    /// Returns a list of ScanLines for this blob
    inline std::vector<ScanLine> getScanLines() const{
      /* {{{ open */

      FUNCTION_LOG("");
      ICLASSERT_RETURN_VAL(m_poRDB,std::vector<ScanLine>());
      std::vector<ScanLine> lines;
      icl::regiondetector::ScanLineList &sll = *(m_poRDB->m_poPixels);
      for(icl::regiondetector::ScanLineList::iterator it=sll.begin();it!=sll.end();++it){
        icl::regiondetector::RegionDetectorScanLine *pl = (*it);
        lines.push_back(ScanLine(pl->getStart(),pl->y(),pl->getEnd()));
      }
      return lines;
    }

    /* }}} */

    /// Returns the underlying handle
    const regiondetector::RegionDetectorBlob* getRegionDetectorBlobHandle() const{
      /* {{{ open */

      return m_poRDB;
    }

    /* }}} */

    /// draws this blob into an image
    void drawTo(ImgBase *image, icl64f val=255) const;
    
    inline bool operator<(const BlobData &other) const{
      /* {{{ open */

      return this->getSize() < other.getSize();
    }

    /* }}} */
    private:
    
    /// wrapped low level blob data
    regiondetector::RegionDetectorBlob *m_poRDB;

    /// correponding image size
    Size m_oImageSize;
  };
  
}


#endif
