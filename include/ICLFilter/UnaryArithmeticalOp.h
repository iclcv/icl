/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/UnaryArithmeticalOp.h                **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#ifndef UNARY_ARITHMETICAL_H
#define UNARY_ARITHMETICAL_H

#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Img.h>

namespace icl {
  /// Class for Arithmetic Functions  \ingroup UNARY
  /** 
      (nearly all functions: Img8u, Img16s, Img32f: IPP + Fallback, all other Types: Fallback only!)
      The functions AddC, SubC, MulC, DivC, AbsDiff, Sqr, Sqrt, Ln, Exp, Abs are implemented for:
      Img8u IPP+Fallback
      Img16s IPP+Fallback
      Img32f IPP+Fallback
      Img32s Fallback only
      Img64f Fallback only
      The user have to take care about overflows. For example 255+1=0 on icl8u
   */
  class UnaryArithmeticalOp : public UnaryOp {
    public:
    /// this enum specifiy all possible binary arithmetical operations
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
    
    /// Constructor
    UnaryArithmeticalOp(optype t, icl64f val=0):m_eOpType(t), m_dValue(val){}
    
    /// Destructor
    virtual ~UnaryArithmeticalOp(){}
    
    /// performes the arithmetical operation, given in the constructor or by the setOpType method.
    /**
      @param poSrc first operand (image)
      @param ppoDst destination image, to store the result
    */
    virtual void apply(const ImgBase *poSrc, ImgBase **ppoDst);

    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    
    /// sets the second operand, with the source is operated with.
    /**
      @param value the value for the operand 
    */
    void setValue(icl64f value) { m_dValue = value; }

    /// returns the value of the second operand
    /**
      @return  the value of the second operand
    */
    icl64f getValue() const { return m_dValue; }
    
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
    icl64f m_dValue;
  };
} // namespace icl

#endif
