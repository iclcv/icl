/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCV/CV.h                                     **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#pragma once

#include <ICLCV/Extrapolator.h>
#include <ICLCV/HungarianAlgorithm.h>
#include <ICLCV/MeanShiftTracker.h>

#include <ICLCV/PositionTracker.h>
#include <ICLCV/VectorTracker.h>
#include <ICLCV/RegionDetector.h>

#include <ICLCV/SimpleBlobSearcher.h>
#include <ICLCore/Img.h>


/** 
    \defgroup G_CBS Color Blob Searcher API (template based)
    \defgroup G_RD Region Detection Package
    \defgroup G_PT Position Tracker template class
    \defgroup G_UTILS Utility classes and Functions
    
    \mainpage ICLCV - A package for Computer Vision Functions and Classes

    TODO!!!
    The ICLCV package contains functions and classes for region and blob detection and
    for tracking. However, the most common component is the icl::RegionDetector class, 
    which performs a parameterized connected component analysis on images. Here are some sample
    application screenshots:

    \image html region-inspector.png "icl-region-inspector GUI"

    
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
    
    The ICLCV package provides some different blob detection and tracking approaches,
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


    \ref G_PT \n    
    These approaches above all perform Blob or region detection. The icl::PositionTracker or 
    it's generalized version icl::VectorTracker can be used to tracking the resulting regions
    or blobs through time.
    
    \ref G_UTILS \n    
    In this group some additional support classes and functions are provided

    \section CSS_CORNERS The Curvature Scale Space
    
    The curvature scale space can be used to extract 2D geometry models from regions.
    <b>Erik:</b> Please add some information here!

    \image html css-demo.jpg "icl-corner-detection-css-demo GUI"


    \section REGION_DETECTOR_EXAMPLE Region Detection Example
    <table border=0><tr><td>
    \code
#include <ICLQt/Common.h>
#include <ICLCV/RegionDetector.h>
#include <ICLFilter/ColorDistanceOp.h>

icl::qt::GUI gui;
GenericGrabber grabber;
RegionDetector rd(100,1E9,255,255);
ColorDistanceOp cd(Color(0,120,240),100);

void mouse(const MouseEvent &e){
  if(e.isLeft()){
    cd.setReferenceColor(e.getColor());
  }
}

void init(){
  grabber.init(pa("-i"));
  gui << Draw().handle("image") << Show();
  gui["image"].install(mouse);
}

void run(){
  DrawHandle draw = gui["image"];
  const core::ImgBase *I = grabber.grab();

  draw = I;

  std::vector<ImageRegion> rs = rd.detect(cd.apply(I));
  for(size_t i=0;i<rs.size();++i){
    draw->linestrip(rs[i].getBoundary());
  }
  draw->render();
}
int main(int n,char **v){
  return ICLApp(n,v,"-input|-i(2)",init,run).exec();
}

    \endcode
    </td><td valign=top>
    \image html icl-online-region-detection-demo-screenshot.png "icl-online-region-detection-demo screenshot"
    </td></tr></table>

*/

namespace icl{
  namespace cv{
    /// template matching using proximity measurement
    /** \section OV Overview
        
        This function matches the given template and returns all rects
        where the template is found with at least the given significance.
        The significance value must be specified in range [0,1] whereby
        significance levels lower then 0.5 involve as a rule too many 
        results.
        
        \section ALG Algorithm
        Template matching is performed by the following steps
        
        -# create a proximity-map of all channels of the src image with
           the according channels in the template image (result is
           buffered in the optionally given image buffer) Range of this
           map is [0,255]
        -# create a single channel binary image from the proximity-map
           using the following rule
           \f[
              BinaryMap(x,y) = \bigvee\limits_{channels\;\; c} ProximityMap(x,y,c) > significance*255
           \f]
        -# detect connected region in the binary map
        -# for each detected region with value 1, put a utils::Rect centered at
           the region center with size of the template (and cropped to the
           image rect) into the result list
        -# return the result list
        
        \section BENCH Performance / Benchmarks
        I applied some exemplary Benchmarks on a 2GHz Core2Duo with the following results:
        
        - source image size 640x480, template size 100x100, 3-channel RGB image\n
          <b>about: 37ms</b>
        - source image size 640x480, template size 100x100, 1-channel gray image\n
          <b>about: 13ms</b>
        - source image size 640x480, template size 10x10, 3-channel RGB image\n
          <b>about: 25ms</b> (not much faster)
        - source image size 640x480, template size 200x200, 3-channel RGB image\n
          <b>about: 35ms</b> (faster than 100x100 because of more omitted border pixels)
  
        The internally used Ipp-Function uses several Threads to apply the proximity 
        measurement call, which means that the function might become much slower on a
        single core machine.
        
        @param src source image where the template should be found in
        @param templ template to search int the src image
        @param significance significance level in range [0,1] (common values are
               0.8 to 0.95)
        @param buffer temporary image buffer (this buffer can be given to the function optionally)
               if given this buffer is exploited internally. The buffers size and channel count
               are adapted automatically
        @param rd Optionally exploited ImgRegionDetector object pointer. If given, 
               this region detector is used, which can speed up Performance in successive calls
               to iclMatchTemplate
    **/
    std::vector<utils::Rect> iclMatchTemplate(const core::Img8u &src, 
                                       const core::Img8u &templ, 
                                       float significance,
                                       core::Img8u *buffer=0,
                                       bool clipBuffersToROI=true,
                                       RegionDetector *rd=0,
                                       bool useCrossCorrCoeffInsteadOfSqrDistance=false);
  
    
    std::vector<utils::Rect> iclMatchTemplate(const core::Img8u &src, 
                                       const core::Img8u *srcMask,
                                       const core::Img8u &templ, 
                                       const core::Img8u *templMask,
                                       float significance,
                                       core::Img8u *srcBuffer=0,
                                       core::Img8u *templBuffer=0,
                                       core::Img8u *buffer=0,
                                       bool clipBuffersToROI=true,
                                       RegionDetector *rd=0,
                                       bool useCrossCorrCoeffInsteadOfSqrDistance=false);
    


  } // namespace cv
} // namespace icl
