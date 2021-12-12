/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/InplaceLogicalOp.h             **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/InplaceOp.h>

namespace icl{
  namespace filter{

    /// Filter class for logical in-place operations  \ingroup INPLACE
    /** The InplaceLogicalOp class provides functionalities for
        arbitrary logical in-place operations on images. The operator
        can be set to implement a certain operation using a given
        optype value. Logical (non-bit-wise) operations result in
        images of value 0 or 255.\n
        Operation list can be split into two sections:

        - pure logical operations (AND OR XOR and NOT)
        - bit-wise operations (bit-wise-AND bit-wise-OR bit-wise-XOR and bit-wise-NOT)

        Pure Logical operations are available for all types; bit-wise operations
        make no sense on floating point data, hence these operations are available
        for integer types only.

        Supported operator types (implementation on pixel value P and operator value
        V in braces)
        - <b>andOp</b> "logical and" ((P&&V)*255)
        - <b>orOp</b> "logical or" ((P||V)*255)
        - <b>xorOp</b> "logical and" ((!!P xor !!V)*255)
        - <b>notOp</b> "logical not" ((!P)*255) operator value is not used in this case
        - <b>binAndOp</b> "binary and" (P&V) [integer types only]
        - <b>binOrOp</b> "binary or" ((P|V) [integer types only]
        - <b>binXorOp</b> "binary and" (P^V) [integer types only]
        - <b>binNotOp</b> "binary not" (~P) operator value is not used in this case
          [integer types only]

        \section IPP-Optimization

        IPP-Optimization is possible, but not yet implemented.

    */
    class ICLFilter_API InplaceLogicalOp : public InplaceOp{
      public:

      enum optype{
        andOp=0,   ///< logical "and"
        orOp=1,    ///< logical "or"
        xorOp=2,   ///< logical "xor"
        notOp=3,   ///< logical "not"
        binAndOp=4,///< binary "and" (for integer types only)
        binOrOp=5, ///< binary "or" (for integer types only)
        binXorOp=6,///< binary "xor" (for integer types only)
        binNotOp=7 ///< binary "not" (for integer types only)
      };


      /// Creates a new InplaceLogicalOp instance with given optype and value
      InplaceLogicalOp(optype t, icl64f value=0):
      m_eOpType(t),m_dValue(value){}

      /// applies this operation in-place on given source image
      virtual core::ImgBase *apply(core::ImgBase *src);

      /// returns current value
      icl64f getValue() const { return m_dValue; }

      /// set current value
      void setValue(icl64f val){ m_dValue = val; }

      /// returns current optype
      optype getOpType() const { return m_eOpType; }

      /// set current optype
      void setOpType(optype t) { m_eOpType = t; }

      private:

      /// optype
      optype m_eOpType;

      /// value
      icl64f m_dValue;
    };
  } // namespace filter
}
