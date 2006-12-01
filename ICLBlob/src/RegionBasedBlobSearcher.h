#ifndef REGIONBASEDBLOBSEARCHER_H
#define REGIONBASEDBLOBSEARCHER_H
#include <Array.h>
#include <Size.h>
#include <Point.h>
#include <Rect.h>
#include <ICLTypes.h>
#include <map>


namespace icl{
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
    virtual icl8u getMinVal()=0;
    virtual icl8u getMaxVal()=0;
    virtual bool needSpecialFeatures(){ return false; }
    virtual bool ok(icl8u value, int x, int y){
      (void)value; (void)x; (void)y; 
      return false;
    }
    virtual bool ok(icl8u value, int x, int y, int *bb, icl32f *pca){
      (void)value; (void)x; (void)y; (void)bb; (void)pca;
      return false;
    }
    static RegionFilter *getDefaultRegionFilter(unsigned int minSize, unsigned int maxSize,
                                                icl8u minVal, icl8u maxVal, 
                                                bool specialFeaturesEnabled);
                                                
  };

  /// interface for Feature Map creators
  /** see RegionBasedBlobSearcher for more details */
  struct FMCreator{
    virtual ~FMCreator(){};
    virtual Size getSize()=0;
    virtual format getFormat()=0;
    virtual Img8u* getFM(Img8u *image)=0;
    virtual RegionFilter *getFilter()=0;


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
     The RegionBasedBlobSearcher (RBBS) provides an easy to use interface to tackle all
     the above mentioned meanisms. To allow inexperienced users a quick entrance, the 
     member function addDefaultFMCreator(..) can be used to add a simple color map
     based FMCreator-object. 
  
  */
  class RegionBasedBlobSearcher{
    public:
    
    /// Create a new RegionBasedBlobSearcher without any FMCreator
    RegionBasedBlobSearcher();

    /// Destructor (delets all contained FMCreators)
    ~RegionBasedBlobSearcher();
    
    /// 1st working method: extract centers by using all FMCreators
    const Array<int> &getCenters(ImgBase *image);

    /// 2nd working method: extract bounding boxes by using all FMCreators
    /** This function produces valid resuls only for RegionFilters 
        that use the "special-features" */
    const Array<int> &getBoundingBoxes(ImgBase *image);

    /// add new FMCreator
    /** ownership is passed to the RegionBasedBlobSearcher*/
    void add(FMCreator* fmc);

    /// remove FMCreator
    /** ownership is passed back to the caller*/
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
    */
    void addDefaultFMCreator(const Size &imageSize,
                             format imageFormat,
                             std::vector<icl8u> refcolor,
                             std::vector<icl8u> thresholds,
                             unsigned int minBlobSize,
                             unsigned int maxBlobSize,
                             bool enableSpecialFeatures = true);
                             


    private:
    // ::functions::
    
    /// returns the current input image in the given format and size
    Img8u *getImage(const Size &size, format fmt);       

    /// 1st: use all fms to create a list of regions
    void extractRegions();
    
    /// 3rd: unify dublicated regions
    void unifyRegions();
    
    // ::member data::

    // layer 1 converter
    Converter *m_poConverter;                            //!< used converter instance
    std::map<Size,std::map<format,Img8u*> > m_mmImages;  //!< map of map of conveted images
        
    // FM-Creators
    Array<FMCreator*> m_oFMCreators;    //!< array of all fm-creators
    
    // RegionDetector
    ImgRegionDetector *m_poRD;          //!< detects connected regions
    Array<Array<Point> > m_oPoints;     //!< buffer of all center lists
    Array<Array<Rect> > m_oRects;       //!< buffer of all b.b. lists
    
    
    /// inupt and ouput data
    ImgBase *m_poInputImage;            //!< input image
    Array<int> m_oOutputBuffer;         //!< output-buffer (used for centers and b.b.)

  };
}

#endif
