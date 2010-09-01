/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/RegionDetector.h                       **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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

#ifndef ICL_REGION_DETECTOR_H
#define ICL_REGION_DETECTOR_H

#include <ICLUtils/Uncopyable.h>
#include <ICLCore/ImgBase.h>
#include <ICLBlob/ImageRegion.h>

#include <vector>

namespace icl{
  
  /// Complex utility class for detection of connected image components \ingroup G_RD
  /** The RegionDetector extracts all connected components from a given input image
      
      \section NEIGHBOURHOOD Pixel Neighbourhood
      Currently, only a 4-Neighbourhood for pixels is supported. This means, that
      Image-Regions have to be adjacent either horizontally or vertically. Pixels 
      with an identical value, that are only connected diagonally are not connected.
      
      \section DEPTHS Supported Image Depths
      As internally, adjacent pixels are compared using the default '==' processing
      of float- or double-images is currently not supported. Furthermore, it is
      also very uncommon to search for connected components on floating-point images.

      \section MULTI_CHANNEL Multi-Channel Images
      In short: multi-channel images are not supported at all. Strictly speaking
      only the channel with index 0 is used, all other remain untouched.
      
      \section ROI ROI Support
      As the actual detection of image regions is much more expensive than a deep
      image copy, ROI support is emulated internally by extracting the image ROI
      from the source image beforehand. However all pixel positions are automatically 
      shifter by the original ROI offset. By this means pixel locations are still 
      relative to the given input images origin.
      
      \section OWNERSHIP Image Ownership
      The RegionDetector instance does not take ownership of the given input image.
      However, the input image needs to remain unchanged as long as the 
      result of the RegionDetector::detect-function is used. Currently, only the
      ImageRegions's ImageRegion::getBoundary() and variants of this method actually
      demand the image to be unchanged, however, future implementations will not
      guarantee this.
      
      \section IMAGE_REGION The ImageRegion Structure
      The ImageRegion structure that is returned by the RegionDetector::detect-function
      Is a utility wrapper around it's data structure that is called ImageRegionData.
      Please do not use ImageRegionData directly, but only by using the ImageRegion
      interface. ImageRegions can easily be copied (just a single pointer copy) --
      the internal data is always managed by the parent RegionDetector instance.
      
      \section CONSTRAINTS Detection Constraints
      There are two direct constraints, that can be defined for filtering the set
      of all image regions: 
      - size (minSize,maxSize) defines an interval for the pixel-count of valid regions.
      - value (minValue,maxValue) defines and interval for the pixel-value of valid regions
      
      <b>Note:</b> Internally, all regions are always detected and automatically filtered
      afterwards. By this means, RegionDetector::detect only returns regions, that
      match the given size and value constraints but also all neighbours and sub-regions
      of these are also available even if they do not match the filter criterion.
      
      \section REGION_GRAPH Region Graph Information
      RegionDetector provides the functionality to perform a simple connected component
      analysis as well as to compute higher level information about the region structure.
      In particular, it can be set up to create a so called <em>region graph</em>.
      Which defines basically, which regions are adjacent. This information can then
      be used to compute a region hierarchy that defines which regions do contain which
      other regions. The most expensive step is to define the region neighbours of each
      region. Therefore, the creation of this additinal information has to be activated
      explicitly. If the RegionDetector was not set up to create the region graph (either
      via a constructor argument or using the function 
      RegionDetector::setCreateGraph(bool)), this information is not available, and 
      the following region-information are not available:
      * region neighbours
      * sub-regions 
      * parent regions

      \section ALGO The Algorithm
      We use a custom designed and highly optimized algorithm for connected component analysis.
      In a first step: The image is run-length encoded. All further steps are performed on
      the resulting LineSegment representation.
      
      The Algorithm is split into 6 parts:
      -# run length encoding
      -# region analysis (create region-parts)
      -# region joining (collect sup-parts of top-level parts in order to obtain regions)
      -# region linking (only if a region graph is created) set up region neighbours 
      -# setting up border regions (only if a region graph is created)
      -# region filtering (filter regions by using given size and value constraint)
      -# further processing on individual regions
      
      \subsection RLE Run Length Encoding
      
      In a first preprocessing step, the input image is run length encoded. Each image line
      is then no longer represented as a set of <em>width</em> pixel values but as a 
      sequence of so called LineSegments each defined by pixel value, x,y, and xend. For this step
      a RunLengthEncoder instances is used. Note, that run length encoding is highly
      optimized in case of using icl8u-images. Here, the most essential step (search for
      the first pixel value that is different from the current one) is accelerated by
      checking 4 next pixels at once.
      
      \subsection RA Region Analysis
      In this processing step, each 2 successive image lines (represented as sequences of
      LineSegments) are processed. Internally, we used a utility structure called 
      WorkingLineSegment for run length encoding. This utility class can also hold additinal
      information which is a pointer to an ImageRegionPart in this step. If two
      line segments A and B with identical values are adjacent (using 4-neighbourhood), 
      they are both added to A's ImageRegionPart and B's ImageRegionPart is set to 
      A's ImageRegionPart. Occasionally, A and B are already set up with an ImageRegionPart.
      In this case, B's ImageRegionPart is added to A's ImageRegionPart and B's 
      ImageRegionPart is set to be no longer on top level. Only top level ImageRegionParts
      are transformed to complete image regions in the next step.\n
      After this step, we have a set of ImageRegionParts. Some of this parts are still top
      level, others are not and need to be added to their top-most ImageRegionPart.
      
      \subsection JO Region Joining
      Here, a ImageRegion (stricly speaking ImageRegionData-structure) is created from 
      each top level ImageRegionParts T. During the creation process, all  
      WorkingLineSegments of T and all contained ImageRegionParts are collected recursively. 
      Furthermore, the internally used WorkingLineSegments do no longer 
      need information about their parent ImageRegionPart. Therefore, their data pointer 
      is set to their parent ImageRegionData structure.\n
      After this step, we already have a set of all image regions.
      
      \subsection LINKING Region Linking
      If a region graph needs to be created, this step is used to detect which regions
      are adjacent. Again, we use the internal WorkingLineSegment representation. For 
      each pair adjacent WorkingLineSegments A and B (4-neighbourhood) A's linked 
      ImageRegionData is set to be a neighbour of B's ImageRegionData and vice versa.
      After this step, each ImageRegionData contains a set of all of it's adjacent
      ImageRegionData structures.
      
      \subsection BORDERS Setting up Border Regions
      For the computation of region containment, regions that are adjacent to the image
      borders are special as these regions are not contained by other regions. Here,
      we again use the internal WorkingLineSegment representation to set up the 
      isBorder-flag of all ImageRegionData structures that touch the image border to true.

      \subsection FILTERING Filtering Regions
      Lastly, all ImageRegionData structures are filtered w.r.t. the given size and
      value constraints.
  */
  class RegionDetector : public Uncopyable{
    struct Data;  //!< internal data structure
    Data *m_data; //!< internal data pointer
    
    public:
    /// first constructor with given flag for creation of the region graph
    /** Note: at default, the region graph is not created */
    RegionDetector(bool createRegionGraph);

    /// 2nd constructor with given constraints and region-graph creation flag
    RegionDetector(int minSize=1, int maxSize=2<<20, int minVal=0, int maxVal=255, bool createRegionGraph=false);

    /// Destructor
    ~RegionDetector();

    /// set up new constraints
    void setConstraints(int minSize, int maxSize, int minVal, int maxVal);
    
    /// set up the region-graph creation flag
    void setCreateGraph(bool on);

    /// main apply function that is used to detect an images image-regions
    /** As explained in \ref DEPTHS, this function is only valid for icl8u, icl16s and icl32s images */
    const std::vector<ImageRegion> &detect(const ImgBase *image);
    
    /// Utility function that returns the image regions that contains a given position (e.g. from mouse input)
    /** click always refers to the last detect call. If no region contains the given point (e.g. because
        it is outside the image rectangle), a null-region is returned. */
    const ImageRegion click(const Point &pos);
    
    private:
    
    /// Internally used utility function that extracts the input images ROI if necessary
    void useImage(const ImgBase *image) throw (ICLException);
    
    /// creates region-parts 
    /** see \ref RA */
    void analyseRegions();

    /// combines ImageRegionParts
    /** see \ref JO */
    void joinRegions();
    
    /// detects region neighbours
    /** see \ref LINKING */
    void linkRegions();
    
    /// detects border regions
    /** see \ref BORDERS */
    void setUpBorders();
    
    /// filters regions by given size and value constraints
    /** see \ref FILTERING */
    void filterRegions();
    
  };

}


#endif
