/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/LeastSquareModelFitting2D.h            **
** Module : ICLMath                                                **
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

#ifndef ICL_LEAST_SQUARE_MODEL_FITTING_2D_H
#define ICL_LEAST_SQUARE_MODEL_FITTING_2D_H

#include <ICLMath/LeastSquareModelFitting.h>

namespace icl{
  namespace math{
  
    /// Direct Least Square Fitting specialization for 2D input data
    /** Specialized least square model fitting for 2D data. Also
        some special desing matrix creation methods are provided */
    class LeastSquareModelFitting2D : public LeastSquareModelFitting<double,Point32f>{
      /// super type
      typedef LeastSquareModelFitting<double,Point32f> Super;
      public:
      /// Default constructor for creating dummy instances
      LeastSquareModelFitting2D(){}
      
      /// Constructor with given parameters
      LeastSquareModelFitting2D(int modelDim, DesignMatrixGen gen, 
                           DynMatrix<double> *constraintMatrix = 0):
      Super(modelDim,gen,constraintMatrix){}
      
      /// DesignMatrixGenerator for the 3-parameter line model
      /** @see LeastSquareModelFitting */
      static inline void line_gen(const Point32f &p, double *d){
        d[0] = p.x; 
        d[1] = p.y; 
        d[2] = 1;
      }
      
      /// DesignMatrixGenerator for the 4 parameter circle model
      /** @see LeastSquareModelFitting */
      static inline void circle_gen(const Point32f &p, double *d){
        d[0] = sqr(p.x) + sqr(p.y);
        d[1] = p.x;
        d[2] = p.y;
        d[3] = 1;
      }
  
      /// DesignMatrixGenerator for the 5 parameter restricted ellipse model
      /** @see LeastSquareModelFitting */
      static inline void restr_ellipse_gen(const Point32f &p, double *d){
        d[0] = sqr(p.x);
        d[1] = sqr(p.y);
        d[2] = p.x;
        d[3] = p.y;
        d[4] = 1;
      }
  
      /// DesignMatrixGenerator for the 6 parameter general ellipse model
      /** @see LeastSquareModelFitting */
      static inline void ellipse_gen(const Point32f &p, double *d){
        d[0] = sqr(p.x);
        d[1] = p.x * p.y;
        d[2] = sqr(p.y);
        d[3] = p.x;
        d[4] = p.y;
        d[5] = 1;
      }
      
      inline std::vector<double> fit(const std::vector<Point32f> &points){
        return Super::fit(points);
      }
      inline icl64f getError(const std::vector<double> &model,const Point32f &p) {
        return Super::getError(model,p);
      }
    };
  
  
  } // namespace math
}

#endif
