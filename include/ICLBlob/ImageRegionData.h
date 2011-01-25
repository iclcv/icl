/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/ImageRegionData.h                      **
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

#ifndef ICL_IMAGE_REGION_DATA_H
#define ICL_IMAGE_REGION_DATA_H

#include <ICLCore/Img.h>
#include <ICLUtils/Any.h>
#include <ICLBlob/ImageRegionPart.h>
#include <ICLBlob/ImageRegion.h>
#include <ICLUtils/StackTimer.h>
#include <ICLBlob/RegionPCAInfo.h>
#include <ICLCore/CornerDetectorCSS.h>

#include <set>

namespace icl{

  /// Utility class for shallow copied data of image region class  \ingroup G_RD
  /** Note: a nested class of ImageRegion is not possible as we need forward 
      declarations of this class. Nested classes cannot be 'forward-declared' */
  class ImageRegionData{
    typedef ImageRegionData IRD;
  public:
    friend class RegionDetector; 
    friend class ImageRegion;     
    friend bool region_search_border(std::set<IRD*>&,IRD*); 
    friend void collect_subregions_recursive(std::set<IRD*>&,IRD*);
    friend bool is_region_contained(IRD*,IRD*);
    friend bool region_search_outer_bb(const Rect&,std::set<IRD*>&,IRD*);
  
  private:
    /// image pixle value
    int  value; 
    
    /// Region-ID
    int id;

    /// pixel-count
    mutable int size;

    /// underlying image
    const ImgBase *image;

    /// list of line segments
    std::vector<LineSegment> segments;
    
    /// meta data, that can be associated with a region structure
    Any meta;

    /// structure for representing region-graph information
    struct RegionGraphInfo{
      /// Constructor
      RegionGraphInfo():isBorder(false),parent(0){}

      /// is the region connected to the border
      bool isBorder;
      
      // region graph information
      std::set<ImageRegionData*> neighbours;

      // child regions
      std::vector<ImageRegionData*> children;

      /// parent region
      ImageRegionData *parent;              
    } *graph; //!< optional information about the region graph

    // structure for representing simple region information
    struct SimpleInformation{
      inline SimpleInformation():
        boundingBox(0),cog(0),pcainfo(0),
        boundaryLength(0),boundary(0),
        thinned_boundary(0),pixels(0){}
      inline ~SimpleInformation(){
        if(boundingBox) delete boundingBox;
        if(cog) delete cog;
        if(pcainfo) delete pcainfo;
        if(boundary) delete boundary;
        if(thinned_boundary) delete thinned_boundary;
        if(pixels) delete pixels;
      }
      Rect *boundingBox;      //!< bounding rectangle
      Point32f *cog;          //!< center of gravity
      RegionPCAInfo *pcainfo; //!< spacial PCA information
      int boundaryLength;     //!< length of the region boundary

      std::vector<Point> *boundary;         //!< all boundary pixels
      std::vector<Point> *thinned_boundary; //!< thinned boundary pixels
      std::vector<Point> *pixels;           //!< all pixels

    } *simple; //!< simple image region information

    struct CSSParams{
      float angle_thresh;
      float rc_coeff;
      float sigma;
      float curvature_cutoff;
      float straight_line_thresh;
      std::vector<Point32f> resultBuffer;

      bool isOk(CornerDetectorCSS *css) const{
        return css->getAngleThreshold() == angle_thresh &&
        css->getRCCoeff() == rc_coeff &&
        css->getSigma() == sigma &&
        css->getCurvatureCutoff() == curvature_cutoff &&
        css->getStraightLineThreshold() == straight_line_thresh;
      }

      void setFrom(CornerDetectorCSS *css){
        angle_thresh = css->getAngleThreshold();
        rc_coeff = css->getRCCoeff();
        sigma = css->getSigma();
        curvature_cutoff = css->getCurvatureCutoff();
        straight_line_thresh = css->getStraightLineThreshold();
      }
    };

    /// contains complex information, 
    struct ComplexInformation{
      inline ComplexInformation():
        directSubRegions(0),allSubRegions(0),parent(0),
        parentTree(0),publicNeighbours(0),cssParams(0){}
      inline ~ComplexInformation(){
        if(directSubRegions) delete directSubRegions;
        if(allSubRegions) delete allSubRegions;
        if(parent) delete parent;
        if(parentTree) delete parentTree;
        if(publicNeighbours) delete publicNeighbours;
        if(cssParams) delete cssParams;
      }
      std::vector<ImageRegion> *directSubRegions;         //!< directly contained regions   
      std::vector<ImageRegion> *allSubRegions;            //!< (even indirectly) contained regions   
      ImageRegion *parent;                                //!< adjacent surrounding region
      std::vector<ImageRegion> *parentTree;               //!< surround regions
      std::vector<ImageRegion> *publicNeighbours;         //!< adjacent regions
      CSSParams *cssParams;
    } *complex; //!< more complex image region information

    
    CornerDetectorCSS *css; //!< for corner detection

    /// Utility factory function
    static ImageRegionData *createInstance(CornerDetectorCSS *css, ImageRegionPart *topRegionPart, int id, bool createGraphInfo, const ImgBase *image);

    /// Constructor
    inline ImageRegionData(CornerDetectorCSS *css, int value, int id, unsigned int segmentSize, bool createGraph,const ImgBase *image):
      value(value),id(id),size(0),image(image),segments(segmentSize),graph(createGraph ? new RegionGraphInfo : 0),
    simple(0),complex(0),css(css){}
    
    /// Destructor
    inline ~ImageRegionData(){
      if(graph) delete graph;
      if(simple) delete simple;
      if(complex) delete complex;
    }
    
    // utility function (only if linkTable is not given)
    inline void link(ImageRegionData *a){
      if(this != a){
        if(a->graph->neighbours.size() < graph->neighbours.size()){
          if(a->graph->neighbours.insert(this).second){
            graph->neighbours.insert(a);
          }
        }else{
          if(graph->neighbours.insert(a).second){
            a->graph->neighbours.insert(this);
          }
        }
      }
    }
    
    /// adds a new child region
    inline void addChild(ImageRegionData *a){
      graph->children.push_back(a);
      a->graph->parent = this;
    }
    
    /// for debugging only
    void showTree(int indent=0) const;

    /// for debugging only 
    void showWithNeighbours() const;
    
    /// utility function
    inline ComplexInformation *ensureComplex(){
      if(!complex) complex = new ComplexInformation;
      return complex;
    }
    
    /// utility function
    inline SimpleInformation *ensureSimple(){
      if(!simple) simple = new SimpleInformation;
      return simple;
    }
  };
  
}

#endif
