#include <RegionFilter.h>
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

  bool RegionFilter::validate(const BlobData &d){
    // {{{ open

    if(m_poValueRange && !m_poValueRange->in(d.getVal())) return false;
    if(m_poSizeRange && ! m_poSizeRange->in(d.getSize())) return false;
    if(m_poBoundaryLengthRange && ! m_poBoundaryLengthRange->in(d.getBoundaryLength())) return false;
    if(m_poFormFactorRange && ! m_poFormFactorRange->in(d.getFormFactor())) return false;
    if(m_poPcaAxisLengthRationRange || m_poPcaFirstMajorAxisAngleRange){
      PCAInfo info = d.getPCAInfo();
      if(m_poPcaAxisLengthRationRange){
        float frac = std::max(info.len1 / info.len2, info.len2 / info.len1);
        if(!m_poPcaAxisLengthRationRange->in(frac)) return false;
      }
      if(m_poPcaFirstMajorAxisAngleRange && !m_poPcaFirstMajorAxisAngleRange->in(info.arc1)) return false;
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
