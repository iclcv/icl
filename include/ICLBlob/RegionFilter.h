/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/RegionFilter.h                         **
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

#ifndef REGION_FILTER_H
#define REGION_FILTER_H

#include <ICLUtils/Range.h>
#include <ICLCore/Types.h>
#include <ICLBlob/Region.h>

namespace icl{
   /// discrimimator for a found regions \ingroup G_RBBS
  /** The RegionFilter (RF) class filters a set of given Regions in 2
      steps:
      -# Filter the regions by a size- and value- interval (during region detection
      -# Filter the regions by higher level features like formFactor of pca information.
      
      The devision of the filter process is reasoned in the internal implementation of
      the region extraction algorithm. The used ImgRegionDetector provides a
      direct repulsion of too small, too large, too dark and too bright regions.
      @see RegionBasedBlobSearcher for more details 
  */
  class RegionFilter{
    public:
    /// Creates a new RegionFilter Object with ranges for all features
    /** if one or some of the given ranges is NULL, this features is not used
        for the validation of a blob **/
    RegionFilter(Range<icl8u> *valueRange=0,
                 Range<icl32s> *sizeRange=0,
                 Range<icl32s> *boundaryLengthRange=0,
                 Range<icl32f> *formFactorRange=0,
                 Range<icl32f> *pcaAxisLengthRatioRange=0,
                 Range<icl32f> *pcaFirstMajorAxisAngleRange=0);
    
    /// Destructor
    virtual ~RegionFilter();
    
    /// virtual validation function
    /** The base implementation ensures, that all blob parameters are in
        the given ranges. If they are, it returns true, else false. This 
        function can be specialized to validate blobs more dynamically.
        @param blobData blob data struct to validate 
    **/
    virtual bool validate(const Region &blobData);
    
    /// returns the valid value range for accepted regions
    /** This information is used during the blob detection procedure to
        reject invalid region imediately **/
    virtual const Range<icl8u> &getValueRange();
    
    /// returns the valid size range for accepted regions
    /** This information is used during the blob detection procedure to
        reject invalid region imediately **/
    virtual const Range<icl32s> &getSizeRange();
    
    
    protected:
    Range<icl8u> *m_poValueRange;                     ///!< value range to use as filter
    Range<icl32s> *m_poSizeRange;                     ///!< size range for filtering
    Range<icl32s> *m_poBoundaryLengthRange;           ///!< range for valid boundary length
    Range<icl32f> *m_poFormFactorRange;               ///!< range for valied form factors
    Range<icl32f> *m_poPcaAxisLengthRationRange;      ///!< range for valied pca axis length ratio
    Range<icl32f> *m_poPcaFirstMajorAxisAngleRange;   ///!< range for valied first major axis angle 

  };
}

#endif
