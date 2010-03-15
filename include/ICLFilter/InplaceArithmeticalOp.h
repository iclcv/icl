/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/InplaceArithmeticalOp.h              **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_INPLACE_ARITHMETICAL_OP
#define ICL_INPLACE_ARITHMETICAL_OP

#include <ICLFilter/InplaceOp.h>

namespace icl{
  /// Inplace implementation for arithmetical operations  \ingroup INPLACE
  /** Current supported operations: "+","-","*","/","^2","sqrt2, 
      "ln", "exp" and "abs".\n
      Currently no IPP-Optimization is available      
  */
  class InplaceArithmeticalOp : public InplaceOp{
    public:

    /// Optypes specify the certain operation
    enum optype{
      addOp=0,  /**< add a constant value to each pixel  */
      subOp=1,  /**< substract a constant value from each pixel  */
      mulOp=2,  /**< multiply each pixel by a constant value */
      divOp=3,  /**< divide each pixle by a constant value */
      sqrOp=10, /**< squares each pixel */
      sqrtOp=11,/**< calculates the square root of each pixel*/
      lnOp=12,  /**< calculates the natural logarithm of each pixel */
      expOp=13, /**< calculates the exponential function for each pixel*/
      absOp=14  /**< calculates the absolute value for each pixel */
    };
    
    /// Create new instance with given operator type and optional value
    /** @param t operator type
        @param value 2nd operand for the operations. Some operation like
        "ln" or "abs" do not need this operand, so it can be omitted.
    */
    InplaceArithmeticalOp(optype t, icl64f value=0):
    m_eOpType(t),m_dValue(value){}
    
    /// applys the operation inplace on an input image
    virtual ImgBase *apply(ImgBase *src);
    
    /// returns the current value
    icl64f getValue() const { return m_dValue; }
    
    /// sets the current value
    void setValue(icl64f val){ m_dValue = val; }
    
    /// returns the current operator type
    optype getOpType() const { return m_eOpType; }
    
    /// sets the current operator type
    void setOpType(optype t) { m_eOpType = t; }
    
    private:
    /// operator type
    optype m_eOpType;

    /// value
    icl64f m_dValue;
  };
}

#endif
