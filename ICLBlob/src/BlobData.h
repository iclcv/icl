#ifndef BLOB_DATA_H
#define BLOB_DATA_H

#include <Point.h>
#include <Rect.h>
#include <ICLTypes.h>
#include <vector>
#include <Macros.h>
#include <RegionDetectorBlob.h>

namespace icl{

  /// struct to represent local PCA information
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
  
  /// struct to represent a scan line
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


  
  class BlobData{
    public:
    BlobData(regiondetector::RegionDetectorBlob *poRDB=0, const Size &oImageSize=Size::null);
    
    icl8u getVal() const;
    Point getCenter() const;
    int getSize() const;
    int getBoundaryLength() const;
    float getFormFactor() const;
   
    Rect getBoundingBox() const;
    PCAInfo getPCAInfo() const;
    
    inline std::vector<Point> getBoundary(){
      /* {{{ open */

      FUNCTION_LOG("");
      ICLASSERT_RETURN_VAL(m_poRDB,std::vector<Point>());
      return m_poRDB->getBoundary(m_oImageSize);
    }

    /* }}} */

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
    regiondetector::RegionDetectorBlob *m_poRDB;
    Size m_oImageSize;
  };
  
}


#endif
