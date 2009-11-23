#ifndef ICL_BLOB_H
#define ICL_BLOB_H

#include <iclDefaultColorBlobSearcher.h>
#include <iclExtrapolator.h>
#include <iclRegionBasedBlobSearcher.h>
#include <iclHungarianAlgorithm.h>
#include <iclMeanShiftTracker.h>

#include <iclPositionTracker.h>
#include <iclVectorTracker.h>
#include <iclRegionDetector.h>

#include <iclSimpleBlobSearcher.h>

#include <iclVQ2D.h>
#include <iclVQClusterInfo.h>
#include <iclVQVectorSet.h>

/** 
    \defgroup G_CBS Color Blob Searcher API (template based)
    \defgroup G_RD Region Detection Package
    \defgroup G_RBBS Region Based Blob Searcher Package
    \defgroup G_PT Position Tracker template class
    \defgroup G_UTILS Utility classes and Functions
    
    \mainpage ICLBlob - A package for detection and tracking of image blobs

    The ICLBlob package contains functions and classes for region and blob detection and
    for tracking. However, the most common component is the icl::RegionDetector class, 
    which performs a parameterized connected component analysis on images. Here are some sample
    application screenshots:

    <TABLE border=0><TR><TD>
    \image html region-inspector.jpg "icl-region-inspector GUI"
    </TD><TD> 
    \image html css-demo.jpg "icl-corner-detection-css-demo GUI"
    </TD></TR></TABLE>
    
    \section BLOB_DEF Blobs and Regions
    At first, we have to define the terms "Blob" and "Region":
    
    <em>
    A "Region" is a set of connected pixels. The set of pixels belonging to a single
    Region has to fulfill a certain criterion of homogeneity (e.g. "all pixels" 
    have exactly the same value"). The term of "connection" must also be defined 
    more precisely in order to avoid misunderstandings. A pixel is connected to all pixels in
    it's neighborhood, which in turn is given by the 4 pixels (to the left, to the 
    right, above and below) next to the reference pixel. 
    The also very common neighborhood that contains the 8 nearest neighbours is currently
    not supported. E.g. a connected component analysis yields a list of Regions.\n\n
    A Blob is a more general local spot in an image. Blobs don't have to be connected
    completely.
    </em>

    
    \section SEC_DETECTION_VS_TRACKING Detection vs. Tracking
    
    We differentiate explicitly between <em>detection</em> and <em>tracking</em>. 
    When regions or blobs are <em>detected</em> in an image, no prior knowledge for the supposed 
    image location is used, i.e. the region/blob is detected in the whole image.\n
    In contrast, blob <em>tracking</em> of blob means, that blobs are tracked in general
    from one time step to another, i.e the former blob location is used as prior guess
    for it's location in the current frame (tracking over time).


    \section BLOB_APPROACHES Different Blob Detection Approaches
    
    The ICLBlob package provides some different blob detection and tracking approaches,
    that are introduced shorty in the following section. For a more detailed insight 
    look at the corresponding class descriptions.

    
    \ref G_CBS \n  
    The Color Blob Searcher API is a template based framework which provides a
    mechanism for a color based blob detection. In this approach each blob is
    determined by a special color, so blobs with identical colors are mixed together.
    Main class is the icl::ColorBlobSearcher template.
        
    \ref G_RD \n
    The icl::RegionDetector performs a connected component analysis on images.
    Regions that can be found must be connected and
    must show identical gray values (color images can not be processed yet). Commonly the
    input image of the RegionDetector's <em>detect(...)-method</em> is a kind of feature
    map that shows only a small number of different gray values (see the class
    documentation for more detail). The set of detected image regions can be restricted by:
    (i) a minimal and maximal gray value and (ii) a minimal and maxmial pixel count
    <b>Note:</b> The algorithm is highly speed-optimized, by using a special kind of 
    self developed memory handling, which avoids memory allocation and deallocation at 
    runtime if possible. Given large images with O(pixel count) regions (e.g. ordinary 
    gray images instead of a feature map) the algorithm may need more physical memory than
    available. A very common pre-processing function may be <b>ICLFilter/LUT::reduceBits(..)</b>.
    In section \ref REGION_DETECTOR_EXAMPLE an example is presented.

    
    \ref G_RBBS \n    
    The icl::RegionBasedBlobSearcher also searches for a set of image blobs, which
    are discriminated <em>here</em> by their spacial location. The <em>connection</em>
    feature is compulsory for blobs. The advantage of this region based approach 
    is, that it is able to detect a large number of image blobs with identical color. 
    The drawback is that detected blobs have no kind of <em>ID</em>, which could be
    used for blob <em>tracking</em>. \n
    The RegionBasedBlobSearcher wraps the icl::RegionDetector and provides an additional
    feature-map creation and region evaluation interface.
    For standard region based blob detection, the icl::SimpleBlobSearcher might be 
    sufficient and much more convenient.
        

    \ref G_PT \n    
    The 3 approaches above all perform Blob or region detection. The icl::PositionTracker or 
    it's generalized version icl::VectorTracker can be used to tracking the resulting regions
    or blobs through time.
    
    \ref G_UTILS \n    
    In this group some additional support classes and functions are provided

    \section REGION_DETECTOR_EXAMPLE Region Detection Example
    <table border=0><tr><td>
    \code
#include <iclCommon.h>
#include <iclRegionDetector.h>
#include <iclColor.h>

// global data (GUI and reference color)
GUI gui("draw[@handle=draw@minsize=16x12]");
Color refcol;

// reference color callback (ref. color is
// updated by mouse-click/drag)
void click_color(const MouseEvent &evt){
  if(evt.isLeft() && evt.getColor().size() == 3){
    std::copy(evt.getColor().begin(),
              evt.getColor().end(),
              refcol.begin());
  }
}

// initialization (create gui and install callback)
void init(){
  gui.show();
  gui["draw"].install(new MouseHandler(click_color));
}

// color distance computation functor
struct DistMap{
  Color refcol;
  DistMap(const Color &color):refcol(color){}
  void operator()(const icl8u src[3],icl8u dst[1]){
    int err = abs(src[0]-refcol[0])
             +abs(src[1]-refcol[1])
             +abs(src[2]-refcol[2]);
    *dst = 255*(err<50);
  }
};

// working loop
void run(){
  gui_DrawHandle(draw);
  static GenericGrabber grabber(FROM_PROGARG("-input"));
  
  // akquire new image
  static Img8u image;
  grabber.grab()->convert(&image);
  
  // apply color distance functor
  static Img8u bin(image.getSize(),1);
  image.reduce_channels<icl8u,3,1,DistMap>(bin,DistMap(refcol));
  
  // create a region detector
  static RegionDetector rd(100,1<<20,255,255);
  const std::vector<icl::Region> &regions = rd.detect(&bin);
  

  // visualization
  draw = image;
  draw->lock();
  draw->reset();
  draw->color(0,100,255);
  for(unsigned int i=0;i<regions.size();++i){
    // obtain region information (boundary pixels here)
    draw->linestrip(regions[i].getBoundary());
  }
  draw->color(255,0,0);
  draw->text("click on reference color",5,225,-1,-1,8);
  draw->unlock();
  draw.update();
}

// default main function
int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input(2)",init,run).exec();
}
    \endcode
    </td><td valign=top>
    \image html online-region-detection-demo.png "icl-online-region-detection-demo screenshot"
    </td></tr></table>

*/



#endif
