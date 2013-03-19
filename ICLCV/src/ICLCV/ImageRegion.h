/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/ImageRegion.h                          **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/


#pragma once

#include <ICLUtils/ShallowCopyable.h>
#include <ICLCore/Img.h>
#include <ICLCV/RegionPCAInfo.h>
#include <ICLCV/LineSegment.h>
#include <ICLUtils/Any.h>

namespace icl{
  namespace cv{
    /** \cond */
    struct ImageRegionPart;
    struct ImageRegionData;
    /** \endcond */
  
  
    /// ImageRegion Structure providing region feature information \ingroup G_RD
    /** \section GEN General information
        The Region class provides several region information:
        - size (pixel count of the region
        - value (pixel value of the region; Currently each region is
          defined by connected Pixels with identical intensity values)
        - boundary (list of outer boundary pixels of the region, in several versions) 
        - formfactor (normed ratio between image pixel size and boundary
          length)
        - cog (center of gravity of the region)
        - bounding box (outer bounding box of the region)
        - line segments (low level access to underlying list of line segments)
        - pixels (list of all region pixels)
        - local PCA information (angles and length of major axis')
        - (optionally) region-graph information: (what region contains which other regions)
  
        \section INT Internal Information
        Valid ImageRegion-instances are only created by a RegionDetector instance. 
        
        
        \section DEF Deferred Calculation of Features
        Almost all features are computed on demand only. This means, that e.g. the center of
        gravity is only calculated if the corresponding getter-function getCOG() is called.
        The results of all features are automatically stored internally. By this means, all features
        must only be computed once, even if the corresponding getter function is called several times.
        
        \section GRAPH Region Graph Information
        Some ImageRegion features are only available if the parent RegionDetector was
        set up to compute so called region graph information. From this, an ImageRegion
        structure can compute the following features:
        * neighbour regions
        * sub regions
        * surrounding regions (parent region and parent region tree) 
  
        If one of these functions is called in a case where no region graph information
        is available, an exception will be thrown.
        
        \section INT2 Internal Processing
        Most high level features, such as bounding boxes or PCA information, 
        can be calculated efficiently on the underlying LineSegment structure.
        Only this list of boundary pixels are calculated on basis of the underlying image.
        
        \section CP Copying
        To avoid deep copies of internal region data, the ImageRegion is a very simple wrapper
        of an underlying data structure called ImageRegionData. Even though a pointer to this
        structure can be obtained using the ImageRegion::data() function, is is strongly recommended
        to use the ImageRegions interface only. Access to the internal structure is only for 
        debugging.
    **/
  
    struct ImageRegion {
  
      /// Internally handled data (not shallow copied, but simply linked to the real data that is managed by the RegionDetector
      ImageRegionData *m_data;
      
      /// internal utility function
      template<class T> void calculateBoundaryIntern(const core::Img<T> &image) const;
  
      /// another utility function
      void calculateThinnedBoundaryIntern() const;
  
      /// utility function that returns the upper left pixel of the region
      utils::Point getUpperLeftPixel()const;
      
      public:
  
      /// Null-construktor
      ImageRegion(ImageRegionData *data=0):
        m_data(data){}
  
      /// Allows the RegionDetector to access private functions/methods
      friend class RegionDetector;
      
      /// returns whether this image region is not null
      /** a parent region e.g. can be null */
      inline operator bool() const { return m_data; }
      
      /// returns the internal data (do not use!)
      ImageRegionData *data() { return m_data; }
      
      /// returns the internal data (do not use!) (const)
      const ImageRegionData *data() const { return m_data; }
  
      /// samples the region into a given image
      void sample(core::ImgBase *image, int color);
  
      /// samples the region into a given image
      void sample(core::ImgBase *image, const std::vector<int> &channelColors);
      
      /// returns the pixel count of the Region
      int getSize() const;
      
      /// returns the pixel value of  the region
      int getVal() const;
      
      /// returns a unique region ID
      int getID() const;
  
      /// returns the region's center of gravity
      utils::Point32f getCOG() const;
  
      /// retuns the internal list of line segments
      const std::vector<LineSegment> &getLineSegments() const;
      
      /// returns the region's bounding box
      const utils::Rect &getBoundingBox() const;
  
      /// returns information about the regions spacial PCA
      const RegionPCAInfo &getPCAInfo() const;
  
      /// returns all boundary pixels of this region
      const std::vector<utils::Point> &getBoundary(bool thinned=true) const;
  
      /// returns the size of boundary pixel array of the ImageRegion
      int getBoundaryPointCount(bool thinned) const;
  
      /// returns the estimated length of the boundary
      /** This methods gives a much better estimation of the boundary
          length than getBoundaryPointCount(). The change in the estimated
          length should be no larger than 5 to 10 % under rotation of the
          object. \n
          Complexity: linear in boundary length.
      */
      float getBoundaryLength() const;
  
      /// Approximate the region boundary by polygon structure
      /** @see ICLCore :: CornerDetectorCSS for a detailed description of function argmuments
          *NEW* parameters can now be adjusted by the configurable interface of the
          parent RegionDetector instance
      */
      const std::vector<utils::Point32f> &getBoundaryCorners() const;
  
      
      /// returns the Form-Factor of the Region (Boundary²/4PI*utils::Size)
      /** Please note that the formfactor for smaller regions is 
          always a bit higher as expected for an ideal regions. 
          This problem occurs due to using a 4-neighbourhood for 
          calculating the boundary length of an ImageRegion;
          
          The form factor formula is designed in that way, that
          an ideal circly has a formfactor of 1 and all other --
          less round -- regions have a higher form factor:
          
          - circle 1
          - square 4/PI
          - cuboid (W=2*H) = 4.5/PI
      */
      float getFormFactor() const;
  
      /// returns a list of all pixels, of this regions
      /** Most of the time, this is very unefficient. It is strongly recommended to use
          getLineSegments() instead */
      const std::vector<utils::Point> &getPixels() const;
      
      /// sets all pixel positions in given image to val in channel 0 of image
      void drawTo(const core::ImgBase *image, icl64f val) const;
      
      /// highlights the region in the image using the given color
      /** please note, that color must contain at least one element for each channel in the given image */
      void drawToColor(const core::ImgBase *image, const icl64f *color) const;
  
      /// returns a list of all fully contained image regions 
      /** This function is only provided if region graph information is available (see \ref GRAPH) 
          
          \section SUB_REGIONS What is a Sub Region?
          
          ImageRegion A contains ImageRegion B if
          -# B is a directly adjacent of A (B is in A's neighbour-list)
          -# There is no path from B (from ImageRegion to adjacent ImageRegion) 
             to a border-region that does not cross A
      */
      const std::vector<ImageRegion> &getSubRegions(bool directOnly=true) const throw (utils::ICLException);
      
      /// returns the parent regions (which might be NULL)
      /** This function is only provided if region graph information is available (see \ref GRAPH) */
      const ImageRegion &getParentRegion() const throw (utils::ICLException);
      
      /// returns the parent region and the parent's parent and so on
      /** If a parent is null, the list ends immediately. In particular, this list can be empty
          This function is only provided if region graph information is available (see \ref GRAPH) 
      */
      const std::vector<ImageRegion> &getParentTree() const throw (utils::ICLException);
  
      /// returns a list of all adjacent regions
      /** This function is only provided if region graph information is available (see \ref GRAPH) **/
      const std::vector<ImageRegion> &getNeighbours() const throw (utils::ICLException);
      
      /// returns whether this ImageRegion is adjacent to the image border
      bool isBorderRegion() const throw (utils::ICLException);
  
      /// returns whether a given pixel position is part of the image region
      bool contains(const utils::Point &p) const;
      
      /// shows the region tree and neighbours (for debugging)
      void showTree() const;
      
      /// associates meta data with this region (also const)
      /** Note, this method is explicitly const since an un-const version
          would make the users simply copy the region before setting the
          meta data value. As all other properties, also the meta data is
          stored in the internal ImageRegionData structure. Therefore,
          also the meta-data is hold in the internally managed and shallowly
          copied data structure.
      */
      void setMetaData(const utils::Any &any) const;
      
      /// returns the meta data associated with this region
      const utils::Any &getMetaData() const;
   };
    
  } // namespace cv
}

