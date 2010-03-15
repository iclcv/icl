/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLBlob/src/RegionFilter.cpp                           **
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
*********************************************************************/

#include <ICLBlob/RegionFilter.h>
#include <limits>
using namespace std;
namespace icl{

  RegionFilter::RegionFilter(Range<icl8u> *valueRange,
                             Range<icl32s> *sizeRange,
                             Range<icl32s> *boundaryLengthRange,
                             Range<icl32f> *formFactorRange,
                             Range<icl32f> *pcaAxisLengthRatioRange,
                             Range<icl32f> *pcaFirstMajorAxisAngleRange):
    // {{{ open

    m_poValueRange(valueRange),m_poSizeRange(sizeRange),m_poBoundaryLengthRange(boundaryLengthRange),
    m_poFormFactorRange(formFactorRange),m_poPcaAxisLengthRationRange(pcaAxisLengthRatioRange),
    m_poPcaFirstMajorAxisAngleRange(pcaFirstMajorAxisAngleRange){}

  // }}}

  RegionFilter::~RegionFilter(){
    // {{{ open

    if(m_poValueRange) delete m_poValueRange;
    if(m_poSizeRange) delete m_poSizeRange;
    if(m_poBoundaryLengthRange) delete m_poBoundaryLengthRange;
    if(m_poFormFactorRange) delete m_poFormFactorRange;    
    if(m_poPcaAxisLengthRationRange) delete m_poPcaAxisLengthRationRange;    
    if(m_poPcaFirstMajorAxisAngleRange) delete m_poPcaFirstMajorAxisAngleRange; 
  }

  // }}}

  bool RegionFilter::validate(const Region &d){
    // {{{ open
    if(m_poValueRange && !m_poValueRange->contains(d.getVal())) return false;
    if(m_poSizeRange && ! m_poSizeRange->contains(d.getSize())) return false;
    if(m_poBoundaryLengthRange && ! m_poBoundaryLengthRange->contains(d.getBoundaryLength())) return false;
    if(m_poFormFactorRange && ! m_poFormFactorRange->contains(d.getFormFactor())) return false;
    if(m_poPcaAxisLengthRationRange || m_poPcaFirstMajorAxisAngleRange){
      RegionPCAInfo info = d.getPCAInfo();
      if(m_poPcaAxisLengthRationRange){
        float frac = iclMax(info.len1 / info.len2, info.len2 / info.len1);
        if(!m_poPcaAxisLengthRationRange->contains(frac)) return false;
      }
      if(m_poPcaFirstMajorAxisAngleRange && !m_poPcaFirstMajorAxisAngleRange->contains(info.arc1)) return false;
    }
    return true;
  }

  // }}}

  const Range<icl8u> &RegionFilter::getValueRange(){
    // {{{ open

    static Range<icl8u> s_oInfRange(numeric_limits<icl8u>::min(),numeric_limits<icl8u>::max());
    return m_poValueRange ? *m_poValueRange : s_oInfRange;
  }

  // }}}
  const Range<icl32s> &RegionFilter::getSizeRange(){
    // {{{ open

    static Range<icl32s> s_oInfRange(numeric_limits<icl32s>::min(),numeric_limits<icl32s>::max());
    return m_poSizeRange ? *m_poSizeRange : s_oInfRange;
  }

  // }}}


}
