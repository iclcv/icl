#ifndef REGIONBASEDBLOBSEARCHER_H
#define REGIONBASEDBLOBSEARCHER_H

#include <iclArray.h>
#include <iclSize.h>
#include <iclPoint.h>
#include <iclRect.h>
#include <iclTypes.h>
#include <map>
#include <iclBlobData.h>
#include <iclRegionFilter.h>
#include <iclFMCreator.h>


namespace icl{

  /** \cond */
  class Converter;          //!< converts images
  class ImgRegionDetector;  //!< seaches connected regions 
  /** \endcond */

  /// Class to detect a set of blobs in an image using a region based approach \ingroup G_RBBS
  /** The basic idea of the ReagionBasedBlobSearcher (RBBS) is simple:
      - create an Image where the homogenous segments comply supposed blobs
      - extract all image regions (with pixel-precision) into a list of regions
      - remove all regions/blobs, which params (size, value, ...) do not match
        to the searched blobs
      - provide access to feature lists of all blobs. Most of the access functions are
        provided in two flawors: a so called Plain Old Data flavor (functions with 
        POD postfix provide blob information in POD types) and default flavor
        (e.g. using Rect and Point structs ). The POD functions will be helpful
        for Frameworks like TDI and NEO;
  
  
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
      since revision 567 all function have been optimized in that way, that image-regions
      are extracted only once.
  **/
  class RegionBasedBlobSearcher{
    public:
    /// Internal used struct to associate FMCreator and RegionFilters
    struct Plugin{
      /// Constructor
      /** @param fmc FMCreator* of this Plugin 
          @param rf RegionFilter *of this Plugin
      **/
      Plugin(FMCreator *fmc=0,RegionFilter *rf=0) : fmc(fmc),rf(rf){}
      
      /// Wrapped FMCreator pointer
      FMCreator *fmc;

      /// Wrapped RegionFitler pointer
      RegionFilter *rf;
    };
    
    /// Create a new RegionBasedBlobSearcher without any FMCreator
    RegionBasedBlobSearcher();

    /// Destructor (delets all contained FMCreators)
    ~RegionBasedBlobSearcher();
    
    /// Main Working function (internally invokes the ImgRegionDetector)
    /** When this function is called, the wrapped ImgRegionDetector is
        invoked to detect all image regions. The result is stored in an 
        internal list of RegionDetectorBlobs. All following calles to 
        access blob information work on this result list. The Region-
        DetectorBlob is optimized to perform as less calculations as
        possible. So all high level blob features are calculated when
        the corresponding getter function is called.
        @param image input image: further getter called refer to the 
                blobs that were detected in this image until set image
                is called again.
    **/
    void extractRegions(const ImgBase *image);
    
    /// access a list of center of gravities
    /** @return "xyxyxy"- ordered int Array containing the data
    **/
    const Array<int> &getCOGsPOD();

    /// access a list of center of gravities (floating point precision)
    /** @return "xyxyxy"- ordered int Array containing the data
    **/
    const Array<float> &getCOGsFloatPOD();


    /// access a list of center of gravities
    /** @return Array of blob center positions 
    **/
    const Array<Point> &getCOGs();
 
    /// access a list of center of gravities (floating point precision)
    /** @return Array of blob center positions 
    **/
    const Array<Point32f> &getCOGsFloat();
    

    /// access to bounding boxes
    /** This function produces resuls only for RegionFilters 
        that use the "special-features".
        @return "xywh,xywh,.."-ordered int Array containing data
    **/
    const Array<int> &getBoundingBoxesPOD();

    /// access to bounding boxes
    /** This function produces resuls only for RegionFilters 
        that use the "special-features".
        @return Array of blobs bounding boxes
    **/
    const Array<Rect> &getBoundingBoxes();
    
    /// Access to pca information
    /** Data order is [axis1-length, axis2-length, axis1-angle, axis2-angle, ...] 
        @return ordered array of float with local pca information
    */
    const Array<float> &getPCAInfoPOD();

    /// extract pca info
    /** @return Array of the found blobs PCA-Information
    */
    const Array<PCAInfo> &getPCAInfo();

    /// Extract an Array of boundary pixel arrays
    /** Array of Array if pixels (xyxyxy...-order) */
    const Array<Array<int> > &getBoundariesPOD();
    
    
    /// Extract an Array of boundary pixel arrays
    /** @return Array of Array of Pixels */
    const Array<Array<Point> > &getBoundaries();

    
    /// Extract an Array of detected blobs boundary lengths
    /** This funtion is in POD-style, so no non-POD function is available
        @return Array of boundary pixle counts 
    **/
    const Array<int> &getBoundaryLengthsPOD();
    
    /// Extract an Array of detected blobs form-factors (see BlobData)
    /** This funtion is in POD-style, so no non-POD function is available
        @return Array of Form-Factors 
    **/
    const Array<float> &getFormFactorsPOD();

    /// Extract an array of detected blob data pointers
    /** The returned blobdata can be used to access all blob available blob features 
        @return const array of internal used blob data Pointers
        */
    const Array<BlobData*> &getBlobData();

    /// add new FMCreator and associated RegionFilter
    /** ownership is passed to the RegionBasedBlobSearcher
        @param fmc new FMCreator to add
        @param rf new RegionFilter to add
    */
    void add(FMCreator* fmc, RegionFilter *rf);

    /// remove FMCreator/RegionFitler tuple by given FMCreator
    /** @param fmc FMCrator* to identify the FMCreator/RegionFilter tuple to remove
        @param release decides whether to release the removed FMCreator and RegionFilter
    **/
    void remove(FMCreator *fmc, bool release=true);

    /// remove FMCreator/RegionFitler tuple by given Regionfilter
    /** @param rf RegionFilter* to identify the FMCreator/RegionFilter tuple to remove
        @param release decides whether to release the removed FMCreator and RegionFilter
    **/
    void remove(RegionFilter *rf, bool release=true);

    /// access a FMCreator/RegionFilter struct by FMCreator identifier
    /** @param fmc FMCreator of the tuple
        @return tuple of FMCreator/Region filter or NULL/NULL tuple if the
                given FMCreator was not found
    **/
    Plugin getPlugin(FMCreator *fmc); 

    /// access a FMCreator/RegionFilter struct by RegionFilter identifier
    /** @param rf RegionFilter of the tuple
        @return tuple of FMCreator/Region filter or NULL/NULL tuple if the
                given RegionFilter was not found
    **/
    Plugin getPlugin(RegionFilter *rf); 

    /// removes all FMCreators
    /** the objects are deleted, as the owner is the RegionBasedBlobSearcher
        @param release decides whether to release the removed FMCreator/RegionFilter
                       tuples or not
    **/
    void removeAll(bool release=true);

    /// returns a list of current feature maps
    const std::vector<const ImgBase*> getFeatureMaps() const;


    private:

    /// returns the current input image in the given format and size
    Img8u *getImage(const Size &size, format fmt, const ImgBase *inputImage);       

    
    /// used converter intstance
    Converter *m_poConverter;   
    
    /// internally used type def for the "format-map"
    typedef std::map<icl::format,Img8u*> fmtmap;

    /// compare function object (for creating a map with a size key)
    struct lessSize {  bool operator()(const icl::Size& lhs, const icl::Size& rhs) const {
      return lhs.width < rhs.width && lhs.height < rhs.height;
    }};
    
    /// internally used type def for the "format-size-map"
    typedef std::map<icl::Size,fmtmap,lessSize> sizemap;
    
    ///map of maps containing converted images
    sizemap m_mmImages;          
    
    /// Array of all currently hold FMCreators and corresponding RegionFilters
    Array<Plugin> m_oFMRF;
    
    /// Wrapped ImgRegionDetector
    ImgRegionDetector *m_poRD;   

    /// Internal buffer of all FMCreators' calculated blob centers
    Array<Array<Point> > m_oCenters;    

    /// Internal buffer of all FMCreators' calculated blob centers
    Array<Array<Point32f> > m_oCentersFloat;    

    /// Internal buffer of all FMCreators' calculated blob bounding boxes
    Array<Array<Rect> > m_oBBs;         
    
    /// Internal buffer of all FMCreators' calculated blob PCAInfos
    Array<Array<PCAInfo> > m_oPCAInfos; 
    
    /// Output buffer for blob centers
    Array<Point> m_oCOGsOut;
    
    /// Output buffer for blob centers (floating point precision)
    Array<Point32f> m_oCOGsFloatOut;
    
    /// Output buffer for blob bounding boxes
    Array<Rect> m_oBBsOut;

    /// Output buffer for blob PCA-Infos
    Array<PCAInfo> m_oPCAInfosOut;
    
    Array<Array<Point> > m_oBoundariesOut;
    
    /// POD-Output buffer for blob centers
    Array<int> m_oCOGsOutPOD;

    /// POD-Output buffer for blob centers (floating point precision)
    Array<float> m_oCOGsFloatOutPOD;

    /// POD-Output buffer for blob bounding boxes
    Array<int> m_oBBsOutPOD;

    /// POD-Output buffer for blob PCA-Infos
    Array<float> m_oPCAInfosOutPOD;

    Array<Array<int> > m_oBoundariesPOD;
    
    Array<int> m_oBoundaryLengthsPOD;
    
    Array<float> m_oFormFactorsPOD;

    Array<BlobData*> m_oInternalData;

    struct FF{
      FF(float f1=0, float f2=0): f1(f1),f2(f2){}
      float f1,f2; 
    };
    Array<FF> m_oScaleFactors; 
    
    /// Currently set input image
    ImgBase *m_poInputImage;     
    


  };
}

#endif
