#ifndef BINARY_LOGICAL_H
#define BINARY_LOGICAL_H

#include <iclBinaryOp.h>
#include <iclImg.h>

namespace icl {
  /// Class for logical operations performed on two images. (and, or, xor) \ingroup BINARY
  /**
    Logical operations are only possible on integer types like Img8u, Img16s and Img32s
  */

  class BinaryLogicalOp : public BinaryOp{
    public:
    /// this enum specifiy all possible binary logical operations
    enum optype{
      andOp,
      orOp,
      xorOp
    };
    
    /// Constructor
    /**
      @param t defines the operaion that will be performed by apply
    */
    BinaryLogicalOp(optype t):m_eOpType(t){}
      
    /// Destructor
    virtual ~BinaryLogicalOp(){}
    
    /// performes the logical operation, given in the constructor or by the setOpType method.
    /**
      @param src1 first operand (image)
      @param src2 second operand (image)
      @param dst destination image, to store the result
    */
    virtual void apply(const ImgBase *src1, const ImgBase *src2, ImgBase **dst);

    /// sets the operaion that will be performed by apply
    /**
      @param t defines the operaion that will be performed by apply
    */   
    void setOpType(optype t){ m_eOpType = t;}
    /// returns the operaion that will be performed by apply
    /**
      @return the operaion that will be performed by apply
    */   
    optype getOpType() const { return m_eOpType; }

    private:
    optype m_eOpType;
  };
  
} // namespace icl

#endif
