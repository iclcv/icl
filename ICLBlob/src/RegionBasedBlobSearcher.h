#ifndef REGIONBASEDBLOBSEARCHER_H
#define REGIONBASEDBLOBSEARCHER_H
#include <Array.h>
#include <Size.h>
#include <Point.h>
#include <Rect.h>
#include <ICLTypes.h>
#include <BlobData.h>
#include <map>


namespace icl{

  
  /// struct to use a Size struct as std::map - key
  struct lessSize {
     bool operator()(const icl::Size& lhs, const icl::Size& rhs) const {
        return lhs.width < rhs.width && lhs.height < rhs.height;
     }
  };
  
  class Converter; // converts all images
 
  class ImgRegionDetector;  // seaches connected regions 
  class RegionDetectorBlob; // a found blob and all params
  
  /// discrimimator for a found regions
  /** The RegionFilter (RF) class filters a set of given Regions in two
      steps:
      -# Filter the regions by a size- and value- interval
      -# Filter the regions by position, value, and optionally higher level features
         like the bounding box and the major axis and corresponding arcs.
      
      The devision of the filter process is reasoned in the internal implementation of
      the region extraction algorithm. The used ImgRegionDetector provides a
      direct repulsion of too small, too large, too dark and too bright regions.
      @see RegionBasedBlobSearcher for more details 
  */
  struct RegionFilter{

    // Destructor
    virtual ~RegionFilter(){}

    /// defines the minimum size of a blob
    virtual unsigned int getMinSize()=0;

    /// defines the maximum size of a blob
    virtual unsigned int getMaxSize()=0;
    
    /// defines the minimum value of a blob
    virtual icl8u getMinVal()=0;

    /// defines the maximum value of a blob
    virtual icl8u getMaxVal()=0;

    /// defines if this RegionFilter needs BoundingBoxes and PCAInfo to filter blobs
    virtual bool needSpecialFeatures(){ return false; }
    
    /// filter funtion for no special features case
    virtual bool ok(icl8u value, const Point &p){
      (void)value; (void)p;
      return false;
    }
    
    /// filter function for special feature case
    virtual bool ok(icl8u value, const Point &p,const Rect &bb, const PCAInfo &pca){
      (void)value; (void)p; (void)bb; (void)pca;
      return false;
    }
    
    /// static function that returns a default regionfilter with given parameters
    static RegionFilter *getDefaultRegionFilter(unsigned int minSize, unsigned int maxSize,
                                                icl8u minVal, icl8u maxVal, 
                                                bool specialFeaturesEnabled);
                                                
  };

  /// interface for Feature Map creators
  /** see RegionBasedBlobSearcher for more details */
  struct FMCreator{
    /// Destructor
    virtual ~FMCreator(){};

    /// returns the demanded image size
    virtual Size getSize()=0;

    /// returns the demaned image format
    virtual format getFormat()=0;
    
    /// create a featur-map from the given image
    virtual Img8u* getFM(Img8u *image)=0;
    
    /// returns it corresponging region-filter
    virtual RegionFilter *getFilter()=0;


    /// static function, that creates a default FMCreator with given parameters
    static FMCreator *getDefaultFMCreator(const Size &size, 
                                          format fmt,
                                          std::vector<icl8u> refcolor,
                                          std::vector<icl8u> thresholds,
                                          RegionFilter *rf);
  };


  /// Class to detect a set of blobs in an image using a region based approach
  /** The basic idea of the ReagionBasedBlobSearcher (RBBS) is simple:
      - create an Image where the homogenous segments comply supposed blobs
      - extract all image regions (with pixel-precision) into a list of regions
      - remove all regions/blobs, which params (size, value, ...) do not match
        to the searched blobs
      - use the bounding boxes, or the centers of the remaining regions as blob info
  
      The RBBS complies a more convenient wrapper of the ImgRegionDetector class.
  
      <h2>Details</h2>
      The above description poses three quetions:
      -# How can we create a feature map ?
      -# How can we reject bad regions ?
      -# How can we handle all that together ?
      
      <h3>How can we create a feature map and How can we reject bad regions </h3>
      The RegionBasedBlobSearcher (RBBS) provides an interface for adding and removing
      so called FMCreators (see FMCreator). Each of this objects can tell the 
      parent RBBS which certain image format and size it needs to create a new feature 
      map. In addition the FMCreator offers an own so called RegionFilter, which is
      another user-definable class interface. Each region, extracted from the 
      freature map of the FMCreator by the RBBS, is given to the corresponding RegionFilter
      to decide if the regions has to be accepted or rejected. Some RegionFilters are able
      to decide this only by some the simple features: x-position, y-position and value of
      the region; Other will additionally factor in so advanced features: The Regions
      bounding box and the regions major axis' and arcs. 
      
      <h3>How can we handle all that together</h3>
      The RegionBasedBlobSearcher (RBBS) provides an easy-to-use interface to tackle all
      the above mentioned mechanisms. To allow inexperienced users a quick entrance, the 
      member function addDefaultFMCreator(..) can be used to add a simple color map
      based FMCreator-object. 
      
      
      <H2>POD and not POD Functions</H2>
      Since revision 544, all getCenters(..), etc. function are available with and
      without "POD" (<b>P</b>lain <b>O</b>ld <b>D</b>ata) postfix. If lower revisions,
      only the "POD"-function signatures were available (but without a POSTFIX).
      Although the interface of the RegionBasedBlobSearcher has changed a little, the
      Internal data handling and storagement has been completely redisigned. This had
      become necessary as the data manangement of the wrapped ImgRegionDetector has
      been adapted in an earlier revision update. \n
      Internally, the data is stored in "not-POD" structures (Point,Rect and PCAInfo),
      so the PODFunctions' performance has been slowed down.\n
      The POD-Functions are provided for Frameworks as Neo/NST or TDI: The Unit/Module
      interfaces can handle POD-Data natively.

      <H2>Performance Hints</H2>
      All getter function will extract all features from the given image that are necessary
      directly:
      \code
      Array<Point> centers = RBBS.getCenters(image);
      Array<Rect> bbs = RBBS.getBoundingBoxes(image);
      \endcode
      This code cause the RBBS to extract blob regions twice. Use getAllFeatures(..) 
      to avoid this.
  **/
  class RegionBasedBlobSearcher{
    public:
    
    /// Create a new RegionBasedBlobSearcher without any FMCreator
    RegionBasedBlobSearcher();

    /// Destructor (delets all contained FMCreators)
    ~RegionBasedBlobSearcher();
    
    
    /// extract centers by using all FMCreators (POD-Version)
    /** @param image input image
        @return "xyxyxy"- ordered int Array containing the data
    **/
    const Array<int> &getCentersPOD(ImgBase *image);

    /// extract centers by using all FMCreators
    /** @param image input image
        @return Array of blob center positions 
    **/
    const Array<Point> &getCenters(ImgBase *image);

    /// extract bounding boxes by using all FMCreators (POD-Version)
    /** This function produces resuls only for RegionFilters 
        that use the "special-features".
        @param image input image
        @return "xywh,xywh,.."-ordered int Array containing data
    **/
    const Array<int> &getBoundingBoxesPOD(ImgBase *image);

    /// extract bounding boxes by using all FMCreators
    /** This function produces resuls only for RegionFilters 
        that use the "special-features".
        @param image input image
        @return Array of blobs bounding boxes
    **/
    const Array<Rect> &getBoundingBoxes(ImgBase *image);
    
    /// extract pca info (POD-Version)
    /** Data order is [axis1-length, axis2-length, axis1-angle, axis2-angle, ...] 
        @param image input image
        @return ordered array of float with local pca information
    */
    const Array<float> &getPCAInfoPOD(ImgBase *image);

    /// extract pca info
    /** Data order is [axis1-length, axis2-length, axis1-angle, axis2-angle, ...] 
        @param image input image
        @return Array of the found blobs PCA-Information
    */
    const Array<PCAInfo> &getPCAInfo(ImgBase *image);

    /// Combined working method, extracting all information at once (POD-Version)
    /** The given destination Array are cleared automatically before use
        @param image input image
        @param centers destination vector for centers 
        @param boundingBoxes destination vector for bounding boxes
        @param pcaInfos destination vector for pcaInfos
    **/
    void detectAllPOD(ImgBase *image, Array<int> &centers, Array<int> &boundingBoxes, Array<float> &pcaInfos); 


    /// Combined working method, extracting all information at once
    /** The given destination Array are cleared automatically before use
        @param image input image
        @param centers destination vector for centers 
        @param boundingBoxes destination vector for bounding boxes
        @param pcaInfos destination vector for pcaInfos
    **/
    void detectAll(ImgBase *image, Array<Point> &centers, Array<Rect> &boundingBoxes, Array<PCAInfo> &pcaInfos); 
    
    /// add new FMCreator
    /** ownership is passed to the RegionBasedBlobSearcher
        @param fmc new FMCreator to add
    */
    void add(FMCreator* fmc);

    /// remove FMCreator
    /** ownership is passed back to the caller
        @param fmc FMCrator ptr. to remove
    **/
    void remove(FMCreator *fmc);

    /// removes all FMCreators
    /** the objects are deleted, as the owner is the RegionBasedBlobSearcher*/
    void removeAll();

    /// adds an anonymous DefaultFMCreator with given params
    /** Because of the its anomymity, the FMCreator is owned by the
        RegionBasedBlobSearcher
        The new FMCreator will create the feature map using the following algorithm:
        <pre>
        I = input-image converted to size "imageSize" and format "imageFormat"
        {the example is written for a 3-channel rgb image it is generalizable for abitrary formats}
        [r,g,b] = "refcolor"       
        [tr,tg,tb] = thresholds  
        for all pixel p=(x,y) I do
           dr = abs( I(x,y,0) - r )
           dg = abs( I(x,y,1) - g )
           db = abs( I(x,y,2) - b )
           RESULT(x,y) = (dr<tr && dg<tg && db<tb) * 255;
        done
        </pre>

        @param imageSize image size that is used by the new FMCreator
        @param imageFomrat image format that is used by the new FMCreator
        @param refcolor reference color
        @param thresholds array of color component thresholds
        @param minBlobSize minimun pixel count for detected blobs
        @param minBlobSize maximun pixel count for detected blobs
        @param enableSpecialFeatures flag whether to enable "special features" (see above)
    */
    void addDefaultFMCreator(const Size &imageSize,
                             format imageFormat,
                             std::vector<icl8u> refcolor,
                             std::vector<icl8u> thresholds,
                             unsigned int minBlobSize,
                             unsigned int maxBlobSize,
                             bool enableSpecialFeatures = true);
                             


    private:
    
    /// returns the current input image in the given format and size
    Img8u *getImage(const Size &size, format fmt);       

    /// uses all fmcs to create a list of regions
    void extractRegions();
    
    /// unify dublicated regions (not yet implemented!)
    void unifyRegions();
    
    /// used converter intstance
    Converter *m_poConverter;   
    
    /// internally used type def for the "format-map"
    typedef std::map<icl::format,Img8u*> fmtmap;

    /// internally used type def for the "format-size-map"
    typedef std::map<icl::Size,fmtmap,lessSize> sizemap;
    
    ///map of maps containing converted images
    sizemap m_mmImages;          
    
    /// Array of all currently hold FMCreators
    Array<FMCreator*> m_oFMCreators;
    
    /// Wrapped ImgRegionDetector
    ImgRegionDetector *m_poRD;   

    /// Internal buffer of all FMCreators' calculated blob centers
    Array<Array<Point> > m_oCenters;    

    /// Internal buffer of all FMCreators' calculated blob bounding boxes
    Array<Array<Rect> > m_oBBs;         
    
    /// Internal buffer of all FMCreators' calculated blob PCAInfos
    Array<Array<PCAInfo> > m_oPCAInfos; 
    
    /// Output buffer for blob centers
    Array<Point> m_oCentersOut;
    
    /// Output buffer for blob bounding boxes
    Array<Rect> m_oBBsOut;

    /// Output buffer for blob PCA-Infos
    Array<PCAInfo> m_oPCAInfosOut;
    
    /// POD-Output buffer for blob centers
    Array<int> m_oCentersOutPOD;

    /// POD-Output buffer for blob bounding boxes
    Array<int> m_oBBsOutPOD;

    /// POD-Output buffer for blob PCA-Infos
    Array<float> m_oPCAInfosOutPOD;
    
    /// Currently set input image
    ImgBase *m_poInputImage;     

  };
}

#endif
