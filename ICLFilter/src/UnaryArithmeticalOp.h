#ifndef UNARY_ARITHMETICAL_H
#define UNARY_ARITHMETICAL_H

#include <UnaryOp.h>
#include <Img.h>
namespace icl {
  /// Class for Arithmetic Functions (nearly all functions: Img8u, Img16s, Img32f: IPP + Fallback, all other Types: Fallback only!)
  /** The functions Add, Sub, Mul, Div, AddC, SubC, MulC, DivC, AbsDiff, Sqr, Sqrt, Ln, Exp, Abs, AbsDiffC are implemented for:
      Img8u IPP+Fallback
      Img16s IPP+Fallback
      Img32f IPP+Fallback
      Img32s Fallback only
      Img64f Fallback only
      
      The functions MulScale and MulCScale are implemented for
      Img8u IPP only
      The user have to take care about overflows. For example 255+1=0 on icl8u
   */
  class UnaryArithmeticalOp : public UnaryOp {
    public:
    /// internal type for operation, that should be applied
    enum optype{
      addOp=0,  /**< add a constant value to each pixel  */
      subOp=1,  /**< substract a constant value from each pixel  */
      mulOp=2,  /**< multiply each pixel by a constant value */
      divOp=3,  /**< divide each pixle through a constant value */
      sqrOp=10, /**< spares each pixel */
      sqrtOp=11,/**< calculates the square root of each pixel*/
      lnOp=12,  /**< calculates the natural logarithm of each pixel */
      expOp=13, /**< calculates the exponential function for each pixel*/
      absOp=14  /**< calculates the absolute value for each pixel */
    };
    
    UnaryArithmeticalOp(optype t, icl64f val=0):m_eOpType(t), m_dValue(val){}
    
    virtual ~UnaryArithmeticalOp(){}
    
    virtual void apply(const ImgBase *poSrc, ImgBase **ppoDst);
    
    void setValue(icl64f value) { m_dValue = value; }
    icl64f getValue() const { return m_dValue; }
    void setOpType(optype t){ m_eOpType = t;}
    optype getOpType() const { return m_eOpType; }
    

    private:
    optype m_eOpType;
    icl64f m_dValue;
  };
} // namespace icl

#endif
