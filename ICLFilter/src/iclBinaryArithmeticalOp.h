#ifndef BINARY_ARITHMETICAL_H
#define BINARY_ARITHMETICAL_H

#include <iclBinaryOp.h>
#include <iclImg.h>

namespace icl {
  /// Class for arithmetic operations performed on two images. (add, sub, mul, div)
  /**
    Performance notes: The functions are implemented for all 5 ICL datatypes, but only
    Img8u, Img16s and Img32f are IPP-accelerated!      
  */
  class BinaryArithmeticalOp : public BinaryOp{
    public:
    /// this enum specifiy all possible binary arithmetical operations
    enum optype{
      addOp,
      subOp,
      mulOp,
      divOp
    };
    /// Constructor
    /**
      @param t defines the operaion that will be performed by apply
    */    
    BinaryArithmeticalOp(optype t):m_eOpType(t){}
    /// Destructor
    virtual ~BinaryArithmeticalOp(){}
    
    /// performes the arithmetical operation, given in the constructor or by the setOpType method.
    /**
      @param poSrc1 first operand (image)
      @param poSrc2 second operand (image)
      @param poDst pointer to the destination image, to store the result
    */
    virtual void apply(const ImgBase *poSrc1,const ImgBase *poSrc2, ImgBase **poDst);

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
