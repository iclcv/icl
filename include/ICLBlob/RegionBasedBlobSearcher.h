/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/RegionBasedBlobSearcher.h              **
** Module : ICLBlob                                                **
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

#ifndef REGIONBASEDBLOBSEARCHER_H
#define REGIONBASEDBLOBSEARCHER_H

#include <ICLUtils/Size.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Rect.h>
#include <ICLCore/Types.h>
#include <map>
#include <ICLBlob/Region.h>
#include <ICLBlob/RegionPCAInfo.h>
#include <ICLBlob/RegionFilter.h>
#include <ICLBlob/FMCreator.h>
#include <ICLUtils/Uncopyable.h>


namespace icl{

  /** \cond */
  class Converter;          //!< converts images
  class RegionDetector;     //!< seaches connected regions 
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
  class RegionBasedBlobSearcher : public Uncopyable{
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
    /** @return "xyxyxy"- ordered int std::vector containing the data
    **/
    const std::vector<int> &getCOGsPOD();

    /// access a list of center of gravities (floating point precision)
    /** @return "xyxyxy"- ordered int std::vector containing the data
    **/
    const std::vector<float> &getCOGsFloatPOD();


    /// access a list of center of gravities
    /** @return std::vector of blob center positions 
    **/
    const std::vector<Point> &getCOGs();
 
    /// access a list of center of gravities (floating point precision)
    /** @return std::vector of blob center positions 
    **/
    const std::vector<Point32f> &getCOGsFloat();
    

    /// access to bounding boxes
    /** This function produces resuls only for RegionFilters 
        that use the "special-features".
        @return "xywh,xywh,.."-ordered int std::vector containing data
    **/
    const std::vector<int> &getBoundingBoxesPOD();

    /// access to bounding boxes
    /** This function produces resuls only for RegionFilters 
        that use the "special-features".
        @return std::vector of blobs bounding boxes
    **/
    const std::vector<Rect> &getBoundingBoxes();
    
    /// Access to pca information
    /** Data order is [axis1-length, axis2-length, axis1-angle, axis2-angle, ...] 
        @return ordered array of float with local pca information
    */
    const std::vector<float> &getPCAInfoPOD();

    /// extract pca info
    /** @return std::vector of the found blobs PCA-Information
    */
    const std::vector<RegionPCAInfo> &getPCAInfo();

    /// Extract an std::vector of boundary pixel arrays
    /** std::vector of std::vector if pixels (xyxyxy...-order) */
    const std::vector<std::vector<int> > &getBoundariesPOD();
    
    
    /// Extract an std::vector of boundary pixel arrays
    /** @return std::vector of std::vector of Pixels */
    const std::vector<std::vector<Point> > &getBoundaries();

    
    /// Extract an std::vector of detected blobs boundary lengths
    /** This funtion is in POD-style, so no non-POD function is available
        @return std::vector of boundary pixle counts 
    **/
    const std::vector<int> &getBoundaryLengthsPOD();
    
    /// Extract an std::vector of detected blobs form-factors (see BlobData)
    /** This funtion is in POD-style, so no non-POD function is available
        @return std::vector of Form-Factors 
    **/
    const std::vector<float> &getFormFactorsPOD();

    /// Extract an array of detected blob data pointers
    /** The returned blobdata can be used to access all blob available blob features 
        @return const array of internal used blob data Pointers
        */
    const std::vector<Region*> &getRegions();

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
    
    /// std::vector of all currently hold FMCreators and corresponding RegionFilters
    std::vector<Plugin> m_oFMRF;
    
    /// Wrapped ImgRegionDetector
    RegionDetector *m_poRD;   

    /// Internal buffer of all FMCreators' calculated blob centers
    std::vector<std::vector<Point> > m_oCenters;    

    /// Internal buffer of all FMCreators' calculated blob centers
    std::vector<std::vector<Point32f> > m_oCentersFloat;    

    /// Internal buffer of all FMCreators' calculated blob bounding boxes
    std::vector<std::vector<Rect> > m_oBBs;         
    
    /// Internal buffer of all FMCreators' calculated blob PCAInfos
    std::vector<std::vector<RegionPCAInfo> > m_oPCAInfos; 
    
    /// Output buffer for blob centers
    std::vector<Point> m_oCOGsOut;
    
    /// Output buffer for blob centers (floating point precision)
    std::vector<Point32f> m_oCOGsFloatOut;
    
    /// Output buffer for blob bounding boxes
    std::vector<Rect> m_oBBsOut;

    /// Output buffer for blob PCA-Infos
    std::vector<RegionPCAInfo> m_oPCAInfosOut;
    
    std::vector<std::vector<Point> > m_oBoundariesOut;
    
    /// POD-Output buffer for blob centers
    std::vector<int> m_oCOGsOutPOD;

    /// POD-Output buffer for blob centers (floating point precision)
    std::vector<float> m_oCOGsFloatOutPOD;

    /// POD-Output buffer for blob bounding boxes
    std::vector<int> m_oBBsOutPOD;

    /// POD-Output buffer for blob PCA-Infos
    std::vector<float> m_oPCAInfosOutPOD;

    std::vector<std::vector<int> > m_oBoundariesPOD;
    
    std::vector<int> m_oBoundaryLengthsPOD;
    
    std::vector<float> m_oFormFactorsPOD;

    std::vector<Region*> m_oInternalData;

    struct FF{
      FF(float f1=0, float f2=0): f1(f1),f2(f2){}
      float f1,f2; 
    };
    std::vector<FF> m_oScaleFactors; 
    
    /// Currently set input image
    ImgBase *m_poInputImage;     
    


  };
}

#endif
