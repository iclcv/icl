#ifndef REGION_DETECTOR_TYPES_H
#define REGION_DETECTOR_TYPES_H

#include <list>
#include <vector>
#include <iclRegionDetectorVector.h>
#include <iclRegionDetectorMemoryManager.h>


#define XBLOB_OPTIMIZED_REGION_LIST
#define XBLOB_OPTIMIZED_PIXLELLINE_LIST
#define XBLOB_OPTIMIZED_BLOB_LIST

namespace icl{
  namespace regiondetector{
    class RegionDetectorBlobPart;
    class RegionDetectorScanLine;
    class RegionDetectorBlob;
    
#ifdef XBLOB_OPTIMIZED_REGION_LIST
    typedef RegionDetectorVector<RegionDetectorBlobPart> BlobPartList;
#else
    typedef std::vector<RegionDetectorBlobPart*> BlobPartList;
#endif
    
#ifdef XBLOB_OPTIMIZED_PIXLELLINE_LIST
    typedef RegionDetectorVector<RegionDetectorScanLine> ScanLineList;
#else
    typedef std::vector<RegionDetectorScanLine*> ScanLineList;
#endif
    
#ifdef XBLOB_OPTIMIZED_BLOB_LIST
    typedef RegionDetectorVector<RegionDetectorBlob> BlobList;
#else
    typedef std::vector<RegionDetectorBlob*> BlobList;
#endif
    
    typedef RegionDetectorMemoryManager<RegionDetectorBlobPart> BlobPartMemoryManager;
    typedef RegionDetectorMemoryManager<RegionDetectorScanLine> ScanLineMemoryManager;
  }
}


#endif
