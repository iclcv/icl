#include <iclPoint.h>
#include <iclRect.h>
#include <iclTypes.h>
#include <vector>
#include <iclMacros.h>
#include <iclRegionDetectorBlob.h>
#ifndef BLOB_DATA_H
#define BLOB_DATA_H


namespace icl{

  /// data-struct to represent local PCA information
  struct PCAInfo{
    /* {{{ open */

    /// Default Constructor
    PCAInfo(float len1=0, float len2=0, float arc1=0, float arc2=0):
      len1(len1),len2(len2),arc1(arc1),arc2(arc2){}
    
    float len1; /*< length of first major axis */
    float len2; /*< length of second major axis */
    float arc1; /*< angle of the first major axis */
    float arc2; /*< angle of the second major axis (arc1+PI/2)*/

    /// null PCAInfo
    static const PCAInfo null;
  };

  /* }}} */
  
  /// data-struct to represent a scan line
  struct ScanLine{
    /* {{{ open */

    /// Default constructor
    ScanLine(int x=0,int y=0,int len=0):x(x),y(y),len(len){}
    int x; /*< x-offset of this horizontal line */
    int y; /*< y-offset of this horizontal line */
    int len; /*< length of this line */

    /// null ScanLine
    static const ScanLine null;
  };

  /* }}} */


  /// Wrapper sturct for the low level RegionDetectorBlob DataStruct
  class BlobData{
    public:
    /// creates a new BlobData struct
    /** @param poRDB wrapped RegionDetectorBlob 
        @param oImageSize corresponding image size
    **/
    BlobData(regiondetector::RegionDetectorBlob *poRDB=0, const Size &oImageSize=Size::null);

    ~BlobData();
    /// returns the blobs value
    icl8u getVal() const;

    /// returns the blobs COG
    Point getCOG() const;

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
    const std::vector<Point> &getBoundary();

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
        for(int x=pl->getStart();x<=iEnd;pxls.push_back(Point(x++,iY)));
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

    private:
    
    /// wrapped low level blob data
    regiondetector::RegionDetectorBlob *m_poRDB;

    /// correponding image size
    Size m_oImageSize;
  };
  
}


#endif
