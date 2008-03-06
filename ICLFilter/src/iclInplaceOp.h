#ifndef ICL_INPLACE_OP
#define ICL_INPLACE_OP

#include <iclImgBase.h>

namespace icl{
  class InplaceOp{
    public:
    virtual ImgBase* apply(ImgBase *src)=0;
    
    void setROIOnly(bool roiOnly) { 
      m_bROIOnly=roiOnly; 
    }
    bool getROIOnly() const { 
      return m_bROIOnly; 
    }
    
    private:
    bool m_bROIOnly;
  };
}

#endif
