#ifndef UNARY_LOGICAL_H
#define UNARY_LOGICAL_H

#include <UnaryOp.h>
#include <Img.h>
namespace icl {
   /// Class for bitwise logical operations on pixel values.  (all functions: Img8u, Img32s: IPP + Fallback, Img16s: Fallback only!, No support for other Types)
   /** 
       Supported operations include And, Or, Xor, Not. Clearly all logical operations
       are only supported on integer typed images, i.e. icl8u.
   */

  class UnaryLogicalOp : public UnaryOp {
    public:
    /// internal type for operation, that should be applied
    enum optype{
      andOp=0,  /**< add a constant value to each pixel  */
      orOp=1,  /**< substract a constant value from each pixel  */
      xorOp=2,  /**< multiply each pixel by a constant value */
      notOp=3  /**< divide each pixle through a constant value */
    };
    
    UnaryLogicalOp(optype t, icl32s val=0):m_eOpType(t), m_dValue(val){}
    
    virtual ~UnaryLogicalOp(){}
    
    virtual void apply(const ImgBase *poSrc, ImgBase **ppoDst);
    
    void setValue(icl32s value) { m_dValue = value; }
    icl32s getValue() const { return m_dValue; }
    void setOpType(optype t){ m_eOpType = t;}
    optype getOpType() const { return m_eOpType; }
    

    private:
    optype m_eOpType;
    icl32s m_dValue;
  };
} // namespace icl

#endif
