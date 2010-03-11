/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef SCALE_OP_H
#define SCALE_OP_H

#include <ICLFilter/AffineOp.h>

namespace icl{
  
  /// Class to scale images \ingroup UNARY \ingroup AFFINE
  class ScaleOp : public AffineOp{
    public:
    /// Constructor
    ScaleOp (double factorX=0.0, double factorY=0.0, scalemode eInterpolate=interpolateLIN) :
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
    AffineOp::apply;

    private: // hide the following methods
    AffineOp::rotate;
    AffineOp::translate;
  };
}
#endif
