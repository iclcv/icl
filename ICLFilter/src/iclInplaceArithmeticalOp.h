#ifndef ICL_INPLACE_ARITHMETICAL_OP
#define ICL_INPLACE_ARITHMETICAL_OP

#include "iclInplaceOp.h"

namespace icl{
  class InplaceArithmeticalOp : public InplaceOp{
    public:

    enum optype{
      addOp=0,  /**< add a constant value to each pixel  */
      subOp=1,  /**< substract a constant value from each pixel  */
      mulOp=2,  /**< multiply each pixel by a constant value */
      divOp=3,  /**< divide each pixle through a constant value */
      sqrOp=10, /**< squares each pixel */
      sqrtOp=11,/**< calculates the square root of each pixel*/
      lnOp=12,  /**< calculates the natural logarithm of each pixel */
      expOp=13, /**< calculates the exponential function for each pixel*/
      absOp=14  /**< calculates the absolute value for each pixel */
    };
    
    InplaceArithmeticalOp(optype t, icl64f value=0):
    m_eOpType(t),m_dValue(value){}
    
    
    virtual ImgBase *apply(ImgBase *src);
    
    icl64f getValue() const { return m_dValue; }
    void setValue(icl64f val){ m_dValue = val; }
    
    optype getOpType() const { return m_eOpType; }
    void setOpType(optype t) { m_eOpType = t; }
    
    private:
    optype m_eOpType;
    icl64f m_dValue;
  };
}

#endif
