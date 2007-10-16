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
    /** \cond */
    class RegionDetectorBlobPart;
    class RegionDetectorScanLine;
    class RegionDetectorBlob;
    /** \endcond */
#ifdef XBLOB_OPTIMIZED_REGION_LIST
    /// RegionDetectors internally used optimized list of blob parts \ingroup G_RD
    typedef RegionDetectorVector<RegionDetectorBlobPart> BlobPartList;
#else
    typedef std::vector<RegionDetectorBlobPart*> BlobPartList;
#endif
    
#ifdef XBLOB_OPTIMIZED_PIXLELLINE_LIST
    /// RegionDetectors internally used optimized list of scan lines \ingroup G_RD
    typedef RegionDetectorVector<RegionDetectorScanLine> ScanLineList;
#else
    typedef std::vector<RegionDetectorScanLine*> ScanLineList;
#endif
    
#ifdef XBLOB_OPTIMIZED_BLOB_LIST
    /// RegionDetectors internally used optimized list of blobs \ingroup G_RD
    typedef RegionDetectorVector<RegionDetectorBlob> BlobList;
#else
    typedef std::vector<RegionDetectorBlob*> BlobList;
#endif
    
    /// RegionDetectors internally used memory manager for blob parts \ingroup G_RD
    typedef RegionDetectorMemoryManager<RegionDetectorBlobPart> BlobPartMemoryManager;

    /// RegionDetectors internally used memory manager for scan lines \ingroup G_RD
    typedef RegionDetectorMemoryManager<RegionDetectorScanLine> ScanLineMemoryManager;
  }
}


#endif
