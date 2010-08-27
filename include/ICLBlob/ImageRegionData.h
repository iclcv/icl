#ifndef ICL_IMAGE_REGION_DATA_H
#define ICL_IMAGE_REGION_DATA_H

#include <ICLCore/Img.h>
#include <ImageRegionPart.h>
#include <ImageRegion.h>
#include <set>

#include <ICLUtils/StackTimer.h>
#include <ICLBlob/RegionPCAInfo.h>
#include <ICLCore/CornerDetectorCSS.h>
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
        boundingBox(0),cog(0),pcainfo(0),css(0),
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
      CornerDetectorCSS *css; //!< for corner detection
      int boundaryLength;     //!< length of the region boundary

      std::vector<Point> *boundary;         //!< all boundary pixels
      std::vector<Point> *thinned_boundary; //!< thinned boundary pixels
      std::vector<Point> *pixels;           //!< all pixels

    } *simple; //!< simple image region information

    /// contains complex information, 
    struct ComplexInformation{
      inline ComplexInformation():
        directSubRegions(0),allSubRegions(0),parent(0),
        parentTree(0),publicNeighbours(0){}
      inline ~ComplexInformation(){
        if(directSubRegions) delete directSubRegions;
        if(allSubRegions) delete allSubRegions;
        if(parent) delete parent;
        if(parentTree) delete parentTree;
        if(publicNeighbours) delete publicNeighbours;
      }
      std::vector<ImageRegion> *directSubRegions;         //!< directly contained regions   
      std::vector<ImageRegion> *allSubRegions;            //!< (even indirectly) contained regions   
      ImageRegion *parent;                                //!< adjacent surrounding region
      std::vector<ImageRegion> *parentTree;               //!< surround regions
      std::vector<ImageRegion> *publicNeighbours;         //!< adjacent regions
    } *complex; //!< more complex image region information


    /// Utility factory function
    static ImageRegionData *createInstance(ImageRegionPart *topRegionPart, int id, bool createGraphInfo, const ImgBase *image);

    /// Constructor
    inline ImageRegionData(int value, int id, unsigned int segmentSize, bool createGraph,const ImgBase *image):
      value(value),id(id),size(0),image(image),segments(segmentSize),graph(createGraph ? new RegionGraphInfo : 0),
      simple(0),complex(0){}
    
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
