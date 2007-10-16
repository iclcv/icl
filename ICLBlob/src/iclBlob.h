#ifndef ICLBLOB_H
#define ICLBLOB_H

/** 
    \defgroup G_CBS Color Blob Searcher API (template based)
    \defgroup G_RD Region Detection Package
    \defgroup G_RBBS Region Based Blob Searcher Package
    \defgroup G_PT Position Tracker template class
    \defgroup G_UTILS Utility classes and Functions
    
    \mainpage ICLBlob - A package for detection and tracking of image blobs
    
    \section BLOB_DEF Blobs?
    At first, we have to define the term "Blob". In literature, many most different
    definition of image blobs can be found. Hence we give our own definition:
    
    <em>
    A "Blob" is a set of connected pixels. The set of pixels belonging to a single
    Blob has to fulfill a certain criterion of homogeneity (e.g. "all pixels" 
    have exactly the same value"). The term of "connection" must also be defined 
    more exactly, to avoid misunderstandings. A pixel is connected to all pixels in
    its neighborhood, which in turn is given by the 4 pixels (to the left, to the 
    right, above and below) next to the reference pixel. Although a very common
    neighborhood defined by the 8 nearest pixels to a reference pixel is often used,
    we resign of it because of its higher computational complexity. 
    </em>

    
    \section SEC_DETECTION_VS_TRACKING Blobs vs. Regions and Detection vs. Tracking
    
    In the following some pretended synonyms are used very specificly:
    <b>"blob detection"</b>, <b>"image region extraction"</b> and <b>"blob tracking"</b>.
    To avoid confusions, we also define these terms precisely:
    - <b>blob detection:</b> "detection" means "without a temporal apriori knowledge".
      When we talk about the detection of Blobs in an image, we start from the assumption
      that we have <b>no</b> special apriori knowledge of where we expect a certain blob
      to be located in the image.
    - <b>blob tracking</b> in opposite to "blob detection", the term "blob tracking" 
      implicated a time concept (-> tracking over time). Tracking is performed in general
      by the following procedure. In the first step, where we have no apriori knowledge
      of the location of blobs in the image, we perform simple blob detection. In the
      following time/processing steps, we compute a prediction of the blob center and
      use this as apriori knowledge. By this means, we have to search for a certain blob
      only in the vicinity of the predicted blob center rather that in the whole image.
    - <b>region detection</b> A second term sometimes used synonymous to the word "blob"
      is "region". This is correct, as we defined a blob to be a "region" with some 
      special features. In our region detection API, we assume a regions criterion of
      homogenity to be the one mentioned above: <em>all pixel within a certain region
      have exactly the same pixel value</em>.
    

    \section BLOB_APPROACHES Different Blob Detection Approaches
    
    The ICLBlob package provides some different blob detection and tracking approaches,
    that are introduced shorty in the following section. For a more detailed insight 
    look at the corresponding class descriptions.

    
    \ref G_CBS \n  
    The Color Blob Searcher API is a template based framework which provides a
    mechanism for a color based blob detection. In this approach each blob is
    determined by a special color, so blobs with identical colors are mixed together.
        
    \ref G_RD \n
    The <b>ImgRegionDetector</b> provides low level functionalities for the detection of
    <em>connected</em> image regions. The regions that can be found must be connected and
    must show identical gray values (color images can not be processed yet). Commonly the
    input image of the ImgRegionDetectors <em>detect(...)-call</em> is a kind of feature
    map that shows only a small number of different gray values (see the classes
    documentation for more detail). The set of detected image regions can be restricted by:
    1st: a min. and max.gray value and 2nd: a min. and max pixel count
    <b>Note:</b> The algorithm is highly speed-optimized, by using a special kind of 
    self implemented memory handling, which avoids memory allocation and deallocation at 
    runtime if possible. Given large images with O(pixel count) regions (e.g. ordinary 
    gray images instead of a feature map) the algorithm may need more physical memory than
    available. A very common function may be <b>ICLFilter/LUT::reduceBits(..)</b>.

    
    \ref G_RBBS \n    
    The RegionBasedBlobSearcher also searches for a set of image blobs, which
    are discriminated <em>here</em> by their spacial location. The <em>connection</em>
    feature is compulsory for blobs. The advantage of this region based approach 
    is, that it is able to detect a large number of image blobs with identical color. 
    The drawback is the detected blobs have no kind of <em>ID</em>, which could be
    used for blob <em>tracking</em>. \n
    The RegionBasedBlobSearcher wraps the ImgRegionDetector and provides an additional
    feature-map creation and region evaluation interface.
        

    \ref G_PT \n    
    The 3 approaches above all perform Blob or region detection. The Position tracker can
    be used together with these ones to track detected blobs over time.

    
    \ref G_UTILS \n    
    In this group some additional support classes and functions are provided

*/



#endif
