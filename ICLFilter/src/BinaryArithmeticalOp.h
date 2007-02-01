#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include <BinaryOp.h>
#include <Img.h>
namespace icl {
  /// TODO short
  /** TODO extended !!
      
      
  */
  class BinaryArithmeticalOp : public BinaryOp{
    public:
    enum optype{
      addOp,
      subOp,
      mulOp,
      divOp
    };
    
    BinaryArithmeticalOp(optype t):m_eOpType(t){}
    virtual ~BinaryArithmeticalOp(){}
    
    virtual void apply(const ImgBase *src1, const ImgBase *src2, ImgBase **dst);
    
    void setOpType(optype t){ m_eOpType = t;}
    optype getOpType() const { return m_eOpType; }

    private:
    optype m_eOpType;
  };
  
} // namespace icl

#endif
