// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/LeastSquareModelFitting.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  namespace math{

    /// Direct Least Square Fitting specialization for 2D input data
    /** Specialized least square model fitting for 2D data. Also
        some special desing matrix creation methods are provided */
    class LeastSquareModelFitting2D : public LeastSquareModelFitting<double,utils::Point32f>{
      /// super type
      using Super = LeastSquareModelFitting<double, utils::Point32f>;
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
