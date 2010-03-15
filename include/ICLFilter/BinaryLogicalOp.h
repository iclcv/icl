/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/BinaryLogicalOp.h                    **
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

#ifndef BINARY_LOGICAL_H
#define BINARY_LOGICAL_H

#include <ICLFilter/BinaryOp.h>
#include <ICLCore/Img.h>

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

    /// import apply symbol from parent class
    BinaryOp::apply;

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
