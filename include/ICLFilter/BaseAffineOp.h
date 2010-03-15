/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/BaseAffineOp.h                       **
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

#ifndef BASEAFFINE_OP_H
#define BASEAFFINE_OP_H

#include <ICLFilter/UnaryOp.h>

namespace icl{

  /// Abtract base class for arbitrary affine operation classes \ingroup AFFINE \ingroup UNARY
  /** The Base affine class complies an abtract interface class
      for all Filter classes implementing affine operations:
      - Affine (General Affine Transformations using 3x2 Matrix)
      - Rotate
      - Translate
      - Mirror
      - Scale  
  */
  class BaseAffineOp : public UnaryOp{
    public:
    /// Destructor
    virtual ~BaseAffineOp(){}
    
    /// import from super class
    UnaryOp::apply;
  };

}

#endif
