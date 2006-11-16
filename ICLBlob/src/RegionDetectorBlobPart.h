#ifndef REGION_H
#define REGION_H

#include "RegionDetectorTypes.h"
/**
A RegionDetectorBlobPart represents a 
single Region of pixels, with same values, that
was detected in the image. It consists of some 
RegionDetectorScanLines, that belong to
this RegionDectorRegionDetectorBlobPart, and of a list 
of other RegionDectorRegionDetectorBlobParts, that belong to this
RegionDectorRegionDetectorBlobPart.

@see RegionDetectorScanLine
@see RegionDetector
*/
namespace icl{
  namespace regiondetector{
    
    class RegionDetectorBlobPart{
      public:
      inline RegionDetectorBlobPart(){
        rs = new BlobPartList(20);
        ps = new ScanLineList(250);
        inside_other_region = 0;
      }
      inline ~RegionDetectorBlobPart(){
        delete rs;
        delete ps;
      }
      inline void add(RegionDetectorScanLine *p){
        ps->push_back(p);
      }
      inline void add(RegionDetectorBlobPart *r){
        rs->push_back(r);
        r->set_inside_other_region();
      }
      inline void set_inside_other_region(){
        inside_other_region = 1;
      }
      inline int is_inside_other_region(){
        return inside_other_region;
      }
      inline void clear(){
        rs->clear();
        ps->clear();
        inside_other_region = 0;
      }
      
      BlobPartList *rs;
      ScanLineList *ps;
      
      private:
      int inside_other_region;
    };
    
    
  }
}

#endif
