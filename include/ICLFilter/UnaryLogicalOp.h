/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/UnaryLogicalOp.h                     **
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

#ifndef UNARY_LOGICAL_H
#define UNARY_LOGICAL_H

#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Img.h>

namespace icl {
   /// Class for bitwise logical operations on pixel values. \ingroup UNARY
   /** 
       (all functions: Img8u, Img32s: IPP + Fallback, Img16s: Fallback only!, No support for other Types)
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
    
    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    
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
