/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/TranslateOp.h                  **
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
#include <ICLFilter/AffineOp.h>

namespace icl{
  namespace filter{

    /// Class to translate images \ingroup UNARY \ingroup AFFINE
    /** TODO: currently the translation effect is compensated by the AffineOp's
        re-centering mechanism*/
    class ICLFilter_API TranslateOp : public AffineOp {
      public:
      /// Constructor
      TranslateOp (double dX=0.0, double dY=0.0, core::scalemode eInterpolate=core::interpolateLIN) :
        AffineOp (eInterpolate) {
          setTranslation(dX,dY);
          setAdaptResultImage(false);
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
      using AffineOp::apply;

      private: // hide the following methods
      using AffineOp::rotate;
      using AffineOp::scale;
      using AffineOp::setAdaptResultImage;
    };
  } // namespace filter
}
