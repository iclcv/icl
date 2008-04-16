#ifndef ICL_USEFUL_FUNCTIONS_H
#define ICL_USEFUL_FUNCTIONS_H

#include <iclImg.h>
#include <iclRegionDetector.h>

namespace icl{
  
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
      -# for each detected region with value 1, put a Rect centered at
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
  std::vector<Rect> iclMatchTemplate(const Img8u &src, 
                                     const Img8u &templ, 
                                     float significance,
                                     Img8u *buffer=0,
                                     bool clipBuffersToROI=true,
                                     RegionDetector *rd=0,
                                     bool useCrossCorrCoeffInsteadOfSqrDistance=false);

  
  std::vector<Rect> iclMatchTemplate(const Img8u &src, 
                                     const Img8u *srcMask,
                                     const Img8u &templ, 
                                     const Img8u *templMask,
                                     float significance,
                                     Img8u *srcBuffer=0,
                                     Img8u *templBuffer=0,
                                     Img8u *buffer=0,
                                     bool clipBuffersToROI=true,
                                     RegionDetector *rd=0,
                                     bool useCrossCorrCoeffInsteadOfSqrDistance=false);
  
                                      
                                      
                                      

  
}

#endif
