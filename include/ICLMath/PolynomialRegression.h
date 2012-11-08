/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/PolynomialRegression.h                 **
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

#pragma once

#include <ICLMath/DynMatrix.h>

namespace icl{
  
  namespace math{
    
    /** \cond */
    template<class T>
    struct PolynomialRegressionAttrib{ 
      virtual T compute(const T *row) const = 0; 
      virtual std::string toString() const = 0;
    };
    /** \endcond */
    
    
    template<class T>
    class PolynomialRegression{
      typedef DynMatrix<T> Matrix;
      typedef PolynomialRegressionAttrib<T> Attrib;
      
      class Result{
        friend class PolynomialRegression;

        std::vector<const Attrib*> m_attribs;
        mutable Matrix m_params; // !< estimated parameters
        mutable Matrix m_xbuf, m_resultBuf;
        public:

        const Matrix &getParams() const { return m_params; }
        const Matrix &operator()(const Matrix &xs) const;
      };
      
      protected:
      mutable Matrix m_buf;
      Result m_result;
      
      /// create instance with given function generator
      /** function = f(x1,x2,x3,x4) = 1 + x1*x1 + x1^3 + x2  
          This is a comma seperated list of tokens
          - konstant value k
          - xi*xj
          - xi^e where e is the exponent
          */
      PolynomialRegression(const std::string &function);
      
      
      /// computes the polynomial regression parameters
      const Result &apply(const Matrix &xs, const Matrix &ys);
      
      /// returns the interpreted function string
      std::string getFunctionString() const;
    };
  }
}
