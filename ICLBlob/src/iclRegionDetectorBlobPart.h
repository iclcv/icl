#ifndef REGION_H
#define REGION_H

#include <iclRegionDetectorTypes.h>

namespace icl{
  namespace regiondetector{

    /// Internally used Support class for region parts \ingroup G_RD
    /**
        A RegionDetectorBlobPart represents a 
        single Region of pixels, with identical pixel values, that
        was detected in an image. It consists of some 
        RegionDetectorScanLines, that belong to
        this RegionDectorRegionDetectorBlobPart, and of a list 
        of other RegionDectorRegionDetectorBlobParts, that belong to this
        RegionDectorRegionDetectorBlobPart.
        
        @see RegionDetectorScanLine
        @see RegionDetector
    */
    class RegionDetectorBlobPart{
      public:
      /// create a new blob part
      /** Internally two RegionDetectorVectors are created. One for the list of
          contained child RegionDetectorBlobParts and another one for the list of 
          contained child RegionDetectorScanLine s*/
      inline RegionDetectorBlobPart(){
        rs = new BlobPartList(20);
        ps = new ScanLineList(250);
        inside_other_region = 0;
      }
      /// Destructor
      inline ~RegionDetectorBlobPart(){
        delete rs;
        delete ps;
      }
      /// Add a child RegionDetectorScanLine
      inline void add(RegionDetectorScanLine *p){
        ps->push_back(p);
      }
      /// Add a child RegionDetectorBlobPart
      inline void add(RegionDetectorBlobPart *r){
        rs->push_back(r);
        r->set_inside_other_region();
      }
      /// sets up this BlobPart to be a child
      inline void set_inside_other_region(){
        inside_other_region = 1;
      }
      /// returns whether this blob part is a top level one or a child
      inline int is_inside_other_region(){
        return inside_other_region;
      }
      /// clears the internal lists and resets this blob part to be a top level parent
      inline void clear(){
        rs->clear();
        ps->clear();
        inside_other_region = 0;
      }
      /// Internal list of RegionDetectorBlobParts
      BlobPartList *rs;
      
      /// Internal list of RegionDetectorScanLiness
      ScanLineList *ps;
      
      private:
      int inside_other_region;
    };
    
    
  }
}

#endif
