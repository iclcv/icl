#ifndef ICLBLOB_H
#define ICLBLOB_H

/** 
\mainpage ICLBlob - A package for detection and tracking of image blobs

\section BlobSec Blobs?
An image blob is a connected region of pixels, fulfilling a 
homogeneity criteria, like "all pixels have exactly the same color".
The postulation of pairwise connection of the blobs can be softened,
which leads to a more general definition of a blob as an image location
with certain features as mean value, center position, a bounding box, or
its spacial major axis and arcs. 

Common classes of this package are:
- ColorBlobSearcher and the DefaultColorBlobSearcher
- PixelRating and PixelRatingGroup
- ImgRegionDetector
- RegionBasedBlobSearcher with FMCreator and RegionFilter

\section AppSec Approaches
The ICLBlob package provides <b>3</b> different implementations of
blob detection frameworks, which differ in the way different blobs
are discriminated:
-# The <b>ColorBlobSeacher</b> searches for a set of blobs in an image using a
   reference-color and threshold representation for each blob. Blobs are 
   discriminated by their color; Blobs with identical colors are mixed
   together. ( Common classes are: <b>PixelRating, PixelRatingGroup,
   ColorBlobSeacher and DefaultColorBlobSearcher </b>)
-# The RegionBasedBlobSearcher also searches for a set of image blobs, which
   are discriminated <em>here</em> by their spacial location. The <em>connection</em>
   feature is compulsory for blobs. The advantage of this region based approach 
   is, that it is able to detect a large number of image blobs with identical color. 
   The drawback is the detected blobs have no kind of <em>ID</em>, which could be
   used for blob tracking. (Common classes are: <b>RegionBasedBlobSearcher, FMCreator, 
   and RegionFilter </b>)
-# The <b>ImgRegionDetector</b> provides low level functionalities for the detection of
   <em>connected</em> image regions. The regions that can be found must be connected and
   must show identical gray values (color images can not be tackled yet). Commonly the
   input image of the ImgRegionDetectors <em>detect(...)-call</em> is a kind of feature
   map that shows only a small number of different gray values (see the classes
   documentation for more detail). The set of detected image regions can be restricted by:
   1st: a min. and max.gray value and 2nd: a min. and max pixel count
   <b>Note:</b> The algorithm is highly speed-optimized, by using a special kind of 
   self implemented memory handling, which avoids memory allocation and deallocation at 
   runtime if possible. Given large images with O(pixel count) regions (e.g. ordinary 
   gray images instead of a feature map) the algorithm may need more physical memory than
   available. (Common classes are: <b>ImgRegionDetector</b> and the 
   <b>ICLFilter/LUT::reduceBits(..)</b>  function)

*/



#endif
