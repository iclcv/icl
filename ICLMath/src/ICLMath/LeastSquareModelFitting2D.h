/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/LeastSquareModelFitting2D.h        **
** Module : ICLMath                                                **
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

#include <ICLMath/LeastSquareModelFitting.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  namespace math{
  
    /// Direct Least Square Fitting specialization for 2D input data
    /** Specialized least square model fitting for 2D data. Also
        some special desing matrix creation methods are provided */
    class ICL_MATH_API LeastSquareModelFitting2D : public LeastSquareModelFitting<double,utils::Point32f>{
      /// super type
      typedef LeastSquareModelFitting<double, utils::Point32f> Super;
      public:
      /// Default constructor for creating dummy instances
      LeastSquareModelFitting2D(){}
      
      /// Constructor with given parameters
      LeastSquareModelFitting2D(int modelDim, DesignMatrixGen gen, 
                           DynMatrix<double> *constraintMatrix = 0):
      Super(modelDim,gen,constraintMatrix){}
      
      /// DesignMatrixGenerator for the 3-parameter line model
      /** @see LeastSquareModelFitting */
      static inline void line_gen(const utils::Point32f &p, double *d){
        d[0] = p.x; 
        d[1] = p.y; 
        d[2] = 1;
      }
      
      /// DesignMatrixGenerator for the 4 parameter circle model
      /** @see LeastSquareModelFitting */
      static inline void circle_gen(const utils::Point32f &p, double *d){
        d[0] = utils::sqr(p.x) + utils::sqr(p.y);
        d[1] = p.x;
        d[2] = p.y;
        d[3] = 1;
      }
  
      /// DesignMatrixGenerator for the 5 parameter restricted ellipse model
      /** @see LeastSquareModelFitting */
      static inline void restr_ellipse_gen(const utils::Point32f &p, double *d){
        d[0] = utils::sqr(p.x);
        d[1] = utils::sqr(p.y);
        d[2] = p.x;
        d[3] = p.y;
        d[4] = 1;
      }
  
      /// DesignMatrixGenerator for the 6 parameter general ellipse model
      /** @see LeastSquareModelFitting */
      static inline void ellipse_gen(const utils::Point32f &p, double *d){
        d[0] = utils::sqr(p.x);
        d[1] = p.x * p.y;
        d[2] = utils::sqr(p.y);
        d[3] = p.x;
        d[4] = p.y;
        d[5] = 1;
      }
      
      inline std::vector<double> fit(const std::vector<utils::Point32f> &points){
        return Super::fit(points);
      }
      inline icl64f getError(const std::vector<double> &model, const utils::Point32f &p) {
        return Super::getError(model,p);
      }
    };
  
  
  } // namespace math
}

