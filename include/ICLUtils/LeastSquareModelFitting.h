/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/LeastSquareModelFitting.h             **
** Module : ICLUtils                                               **
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

#include <ICLUtils/DynMatrix.h>

namespace icl{
  
  /// Direct Least Square Fitting Algorithm (not working yet!)
  /** TODO: make it working :-) */
  template<class T, class DataPoint>
  class LeastSquareModelFitting{
    public:
    /// fills the give float* with data from the given data point
    /** creates the rows of the design matrix */
    typedef Function<void,const DataPoint&,T*> DesignMatrixGen;
    
    /// model type (defines the model parameters)
    typedef std::vector<T> Model;
    
    private:
    int m_modelDim;
    DesignMatrixGen m_gen;
    DynMatrix<T> m_D, m_S, m_Evecs, m_Evals;
    SmartPtr<DynMatrix<T> >m_C;
    Model m_model;
    
    public:
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

  
  class LeastSquareModelFitting2D : public LeastSquareModelFitting<double,Point32f>{
    typedef LeastSquareModelFitting<double,Point32f> Super;
    public:
    LeastSquareModelFitting2D(int modelDim, DesignMatrixGen gen, 
                         DynMatrix<double> *constraintMatrix = 0):
    Super(modelDim,gen,constraintMatrix){}
    
    /// DesignMatrixGenerator for the 3-parameter line model
    static inline void line_gen(const Point32f &p, double *d){
      d[0] = p.x; 
      d[1] = p.y; 
      d[2] = 1;
    }
    
    /// DesignMatrixGenerator for the 4 parameter circle model
    static inline void circle_gen(const Point32f &p, double *d){
      d[0] = sqr(p.x) + sqr(p.y);
      d[1] = p.x;
      d[2] = p.y;
      d[3] = 1;
    }

    /// DesignMatrixGenerator for the 5 parameter restricted ellipse model
    static inline void restr_ellipse_gen(const Point32f &p, double *d){
      d[0] = sqr(p.x);
      d[1] = sqr(p.y);
      d[2] = p.x;
      d[3] = p.y;
      d[4] = 1;
    }

    /// DesignMatrixGenerator for the 6 parameter general ellipse model
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
}


#endif
