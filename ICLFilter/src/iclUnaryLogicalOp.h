#ifndef UNARY_LOGICAL_H
#define UNARY_LOGICAL_H

#include <iclUnaryOp.h>
#include <iclImg.h>

namespace icl {
   /// Class for bitwise logical operations on pixel values.  (all functions: Img8u, Img32s: IPP + Fallback, Img16s: Fallback only!, No support for other Types)
   /** 
       Supported operations include And, Or, Xor, Not. Clearly all logical operations
       are only supported on integer typed images, i.e. icl8u.
   */

  class UnaryLogicalOp : public UnaryOp {
    public:
    /// this enum specifiy all possible unary logical operations
    enum optype{
      andOp=0,  /**< add a constant value to each pixel  */
      orOp=1,  /**< substract a constant value from each pixel  */
      xorOp=2,  /**< multiply each pixel by a constant value */
      notOp=3  /**< divide each pixle through a constant value */
    };
    /// Constructor
    UnaryLogicalOp(optype t, icl32s val=0):m_eOpType(t), m_dValue(val){}
    
    /// Destructor
    virtual ~UnaryLogicalOp(){}

    /// performes the logical operation, given in the constructor or by the setOpType method.
    /**
      @param poSrc first operand (image)
      @param ppoDst pointer to the destination image, to store the result
    */      
    virtual void apply(const ImgBase *poSrc, ImgBase **ppoDst);
    
    /// sets the second operand, with the source is operated with.
    /**
      @param value the value for the operand 
    */
    void setValue(icl32s value) { m_dValue = value; }

    /// returns the value of the second operand
    /**
      @return  the value of the second operand
    */
    icl32s getValue() const { return m_dValue; }

    /// changes the operator type
    /**
      @see optype
      @param t operator type
    */
    void setOpType(optype t){ m_eOpType = t;}

    /// returns the operator type
    /**
      @see optype
      @return operator type
    */
    optype getOpType() const { return m_eOpType; }
    

    private:
    optype m_eOpType;
    icl32s m_dValue;
  };
} // namespace icl

#endif
