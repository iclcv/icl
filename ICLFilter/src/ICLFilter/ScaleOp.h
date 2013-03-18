/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ScaleOp.h                      **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLFilter/AffineOp.h>

namespace icl{
  namespace filter{
    
    /// Class to scale images \ingroup UNARY \ingroup AFFINE
    class ScaleOp : public AffineOp{
      public:
      /// Constructor
      ScaleOp (double factorX=0.0, double factorY=0.0, 
               core::scalemode eInterpolate=core::interpolateLIN) :
      AffineOp (eInterpolate) {
        setScale(factorX,factorY);
      }
      
      /// performs a scale
      /**
        @param factorX scale-factor in x-direction
        @param factorY scale-factor in y-direction
        different values for x and y will lead to a dilation / upsetting deformation
      */
      void setScale (double factorX, double factorY) {
        AffineOp::reset (); 
        AffineOp::scale (factorX,factorY);
      }
          
      // apply should still be public
      ///applies the scale
      using AffineOp::apply;
  
      private: // hide the following methods
      using AffineOp::rotate;
      using AffineOp::translate;
    };
  } // namespace filter
}
