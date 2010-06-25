/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/TranslateOp.h                        **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef TRANSLATE_OP_H
#define TRANSLATE_OP_H

#include <ICLFilter/AffineOp.h>

namespace icl{
  
  /// Class to translate images \ingroup UNARY \ingroup AFFINE
  /** TODO: currently the translation effect is compensated by the AffineOp's 
      re-centering mechanism*/
  class TranslateOp : public AffineOp {
    public:
    /// Constructor
    TranslateOp (double dX=0.0, double dY=0.0, scalemode eInterpolate=interpolateLIN) :
      AffineOp (eInterpolate) {
        setTranslation(dX,dY);
      }
    
    /// performs a translation
    /**
      @param dX pixels to translate in x-direction
      @param dY pixels to translate in y-direction
    */

    void setTranslation (double dX, double dY) {
      AffineOp::reset (); 
      AffineOp::translate (dX,dY);
    }
    
    // apply should still be public
    
    ///applies the translation
    AffineOp::apply;

    private: // hide the following methods
    AffineOp::rotate;
    AffineOp::scale;
  };
}
#endif
