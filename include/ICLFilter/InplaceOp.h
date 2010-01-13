#ifndef ICL_INPLACE_OP
#define ICL_INPLACE_OP

#include <ICLCore/ImgBase.h>

namespace icl{
  
  /// Interface class for inplace operators \ingroup INPLACE
  /** Inplace operators work on image pixels directly. Common examples
      are arithmetical expressions like IMAGE *= 2. Useful inplace 
      operations are arithmetical, logical, binary-logical, or table-lookups.
      
      @see ArithmeticalInplaceOp 
      @see LogicalInplaceOp
  */
  class InplaceOp{
    public:

    /// Create a new Inplace op (ROI-only flag is set to true)
    InplaceOp():m_bROIOnly(true){}

    /// apply function transforms source image pixels inplace
    virtual ImgBase* apply(ImgBase *src)=0;
    
    /// setup the operation to work on the input images ROI only or not
    void setROIOnly(bool roiOnly) { 
      m_bROIOnly=roiOnly; 
    }
    
    /// returns whether operator is in "roi-only" mode or not
    bool getROIOnly() const { 
      return m_bROIOnly; 
    }
    
    private:
    /// "roi-only" flag
    bool m_bROIOnly;
  };
}

#endif
