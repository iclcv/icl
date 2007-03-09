#include "iclFixedConverter.h"


namespace icl{

  FixedConverter::FixedConverter(const ImgParams &p, depth d, bool applyToROIOnly):
    m_oParams(p),m_oConverter(applyToROIOnly),m_eDepth(d) { }
  
  void FixedConverter::apply(const ImgBase *poSrc, ImgBase **ppoDst){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( ppoDst );
    ensureCompatible(ppoDst,m_eDepth,m_oParams);
    m_oConverter.apply(poSrc,*ppoDst);
  }  
}
