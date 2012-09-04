/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/LeastSquareModelFitting.h              **
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

#ifndef ICL_LEAST_SQUARE_MODEL_FITTING_H
#define ICL_LEAST_SQUARE_MODEL_FITTING_H

#include <ICLMath/DynMatrix.h>

namespace icl{
  namespace math{
    
    /// Direct Least Square Fitting Algorithm 
    /** The given algorithm is based on the paper <em><b>Direct Least Square Fitting 
        of Ellipses</b></em> written by<em>Andrew W. Fitzgibbon</em>, <em>Maurizio Milu</em>
        and <em>Robert B. Fischer</em>.
  
        \section ALG The direct least square fitting algorithm
        
        Here, the algorithm is explained, later, some examples will be given.
        
        \subsection MODELS Models
  
        For this algorithm, models are defined by implicit equations f=da=0.
        d = [f1(x), f2(x), ...]^T defines features that are computed on the input data x.
        a = [a1,a2,...] are the linear coefficients. In other words, models
        are expressed as intersections between geometric primitives and the f=0 plane.
        The resulting models always have an extra parameter, which is disambiguated by
        finding solutions where |a|^2 = 1. Actually, this is just a special yet very
        common disambiguation technique. More generally spoken, a^T C a = 1 is used
        where C is the so called <em>constraint matrix</em>. If C is set to the 
        identity matrix, the norm becomes |a|^2=1. In the following, some common
        2D models are discussed.
        
        <b>lines</b>\n
        Straight lines in 2D are expressed by a simple parametric equation. The 
        features d for a given input (x,y) are simply d=(x,y,1)^T. The resulting 
        equation can be seen a scalar product (a1,a2)*(x,y)^T whose result is -a3.
        Geometrically, a line is defined by it's perpendicular vector (a1,a2) and an
        offset to the origin which is -a3.
  
        <b>circles</b>\n
        The standard coordinate-equation for a circle is (x-cx)^2 + (y-cy)^2 = r^2.
        By decoupling the coefficients, we can create a 4 parameter model:\n
        \code 
          a1(x^2 + y^2) + a2 x + a3 y +a4 = 0
        
          where, cx = -a2/(2*a1),
                 cy = -a3/(2*a1)
            and  r = ::sqrt( (a2*a2+a3*a3)/(4*a1*a1) -a4/a1 )
        \endcode
  
        <b>ellipses</b>\n
        General ellipses are expressed by the 6 parameter model
        \code
        a1 x^2 + a2 xy + a3 y^2 +  a4 x + a5 y + a6 = 0;
        \endcode 
        If we want to restrict the ellipses to be non-rotated, the mixed 
        term needs to be removed:
        \code
        a1 x^2 + a2 y^2 +  a3 x + a4 y + a5 = 0;
        \endcode 
        If a1 and a2 are coupled, we get the circle model shown above.
        
        \subsection DLSF Direct Least Square Fitting
     
        Instead of finding a solution da=0, for a single data point, we have a larger
        set of input data D (where the vectors d are the rows of d). Usually, the data is
        noisy. Therefore we try to minimize the error E=|Da|^2. D is the so called 
        design matrix.
        In order to avoid receiving the trivial solution a=0, the constraint matrix C is
        introduced. The minimization is then applied by introducing Lagrange multipliers:
        \code
        minimize:         2 D^T Da - 2 lambda C a = 0
        with subject to                   a^T C a = 1
        \endcode
        By introducing the the scatter matrix S = D^T D, we get a generlized eigen vector
        problem:
        \code
        minimize:         S a - lambda C a = 0
        with subject to            a^T C a = 1
        \endcode
        
        \subsection IMPL Implemented Algorithm
        
        -# create the design matrix D
        -# create the scatter matrix S = D^T D
        -# create the constraint matrix C (usually id)
        -# find eigen vectors and eigen values for S^-1 C
        -# use the eigen vector to the largest eigen value as model 
        
        
        \section EX Examples
        Examples for usage are given in the interactive application
        icl-model-fitting-example located in the ICLQt package. Here, the 
        icl::LeastSquareModelFitting is demonstrated and it is combined with the 
        icl::RansacFitter class in order to show the advantages of using
        the RANSAC algorithm in presence of outliers and noise.
        
  
        For the creation of a LeastSquareModelFitting instance, only the
        model dimension and the creation function for rows of the design
        matrix D. Optionally, a non-ID constraint matrix can be given.
    */
  
    template<class T, class DataPoint>
    class LeastSquareModelFitting{
      public:
      /// fills the give float* with data from the given data point
      /** creates the rows of the design matrix */
      typedef Function<void,const DataPoint&,T*> DesignMatrixGen;
      
      /// model type (defines the model parameters)
      typedef std::vector<T> Model;
      
      private:
      /// model dimension
      int m_modelDim;
  
      /// design matrix row generator
      DesignMatrixGen m_gen;
      
      /// utility valiables
      DynMatrix<T> m_D, m_S, m_Evecs, m_Evals;
      
      /// constraint matrix
      SmartPtr<DynMatrix<T> >m_C;
      
      /// the model parameters
      Model m_model;
      
      public:
      
      /// Empty constructor that creates a dummy instance
      LeastSquareModelFitting(){}
      
      /// constructor with given parameters
      LeastSquareModelFitting(int modelDim, DesignMatrixGen gen, 
                              DynMatrix<T> *constraintMatrix=0):
      m_modelDim(modelDim),m_gen(gen),m_S(modelDim,modelDim), 
      m_C(constraintMatrix),m_model(modelDim){
        
      }
      
      /// computes the error for a given data point
      /** if model is 0, the last fitted model is used */
      icl64f getError(const Model &model,const DataPoint &p){
        std::vector<T> d(m_modelDim);
        m_gen(p,d.data());
        icl64f e = 0;
        for(int i=0;i<m_modelDim;++i) e += d[i] * model[i];
        return fabs(e);
      }
      
      /// fits the model to the given data points and returns optimal parameter set
      /** Internally we use a workaround when the matrix inversion fails due to stability
          problems. If the standard matrix inversion fails, a SVD-based inversion is 
          used */
      Model fit(const std::vector<DataPoint> &points){
        const int M = m_modelDim;
        const int N = (int)points.size();
        
        m_D.setBounds(M,N);
  
        /// create design matrix D
        for(int i=0;i<N;i++){
          m_gen(points[i],m_D.row_begin(i));
        }
        
        /// create the scatter matrix S
        m_D.transp().mult(m_D,m_S);
        
        
        
        DynMatrix<T> Si;
        try{ 
          Si = m_C ? m_S.inv()* (*m_C) : m_S.inv();
        }catch(SingularMatrixException &ex){
          Si = m_C ? m_S.pinv(true)* (*m_C) : m_S.pinv(true); 
        }
        
        try{
          Si.eigen(m_Evecs, m_Evals);
          /// use eigen vector for the largest eigen value
          std::copy(m_Evecs.col_begin(0), m_Evecs.col_end(0), m_model.begin());
        }catch(ICLException &e){
          std::fill(m_model.begin(),m_model.end(),Range<T>::limits().maxVal);
        }
        
        return m_model;
      }
    };
  } // namespace math
}


#endif
