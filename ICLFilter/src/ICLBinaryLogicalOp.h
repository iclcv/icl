#include <ICLBinaryOp.h>
#include <ICLImg.h>
#ifndef BINARY_LOGICAL_H
#define BINARY_LOGICAL_H

namespace icl {
  /// TODO short
  /** TODO extended !!
      
      
  */
  class BinaryLogicalOp : public BinaryOp{
    public:
    enum optype{
      andOp,
      orOp,
      xorOp
    };
    
    BinaryLogicalOp(optype t):m_eOpType(t){}
    virtual ~BinaryLogicalOp(){}
    
    virtual void apply(const ImgBase *src1, const ImgBase *src2, ImgBase **dst);
    
    void setOpType(optype t){ m_eOpType = t;}
    optype getOpType() const { return m_eOpType; }

    private:
    optype m_eOpType;
  };
  
} // namespace icl

#endif
