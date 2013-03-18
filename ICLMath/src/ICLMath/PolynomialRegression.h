/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/PolynomialRegression.h             **
** Module : ICLMath                                                **
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

#include <ICLMath/DynMatrix.h>
#include <vector>
#include <string>

namespace icl{
  
  namespace math{
    
    /** \cond */
    
    /// utility class
    template<class T>
    struct PolynomialRegressionAttrib{ 
      virtual T compute(const T *row) const = 0; 
      virtual std::string toString() const = 0;
    };
    /** \endcond */
    
    
    /// Generic Implementation of the Polynomial Regression algorithm
    /** Polynomial regression applies least square regression 
        based on a polynomial that is computed on the input variables.
        It minimized p(x) a = y, for a given polynomial function p(x).
        x can be a scalar of a vector. The same is true for y. Each
        entry of x and y resp. is assued to be one row of the input
        matrices xs and ys.
        
        \section RES Result 

        The resulting PolynomialRegression::Result instance can be
        used to compute the output of the a given set of input variables
        simultaneously. It contains the optimal solution of a, which
        is used to compute y = p(x) a.


        \section FUNC The Polynomial Function definition
        
        The PolynomialRegression class uses a rather very simple but intuitive
        syntax for the definition of the used polynomial. Here is an
        example that contains all allowed parts: "1 + x0 + x1 + x0*x1 + x1^2 + x2^2"
        White spaces are ignored; tokens are defined by the "+"-delimiter.
        Tokens can be of form 
        - K, where K is a constant (can be float, however actually only "1" makes sence here)
        - xI, where I is an integer input variable index
        - xI*xJ*... , where I and J and other are integer input varialbe indices
        - xI^K, where K is a constant (can be float and int)
        
    */
    template<class T>
    class PolynomialRegression{
      
      /// internally used matrxi type
      typedef DynMatrix<T> Matrix;
      
      /// internally used type
      typedef PolynomialRegressionAttrib<T> Attrib;
      
      public:
      
      /// result type
      class Result{
        /// parent class
        friend class PolynomialRegression;

        /// list of attribute functions
        std::vector<const Attrib*> m_attribs;
        
        /// function definition given
        std::string m_function;
        
        /// set of estimated parameters
        mutable Matrix m_params; 
        
        /// internal buffers
        mutable Matrix m_xbuf, m_resultBuf;
        
        /// maximum x-row index used in the attribute list
        mutable int m_attribMaxIndex;
        
        /// sets up the result by the given function
        void setup(const std::string &function);
        
        public:
        
        /// empty default constructor
        inline Result(){}
        
        /// constructor with given xml file saved with the save method
        Result(const std::string &xmlfilename);
        
        /// set of parameters (on row for each output dimension)
        const Matrix &getParams() const { return m_params; }
        
        /// applys the parameters to given set of inputs
        /** inputs are assumed to be the rows of xs!
            The operator computes p(xs) * a,
            Where p(xs) is the matrix that contains the attribut
            values for each input row of xs in its rows.
            
            The output matrix contains the outputs as rows
            
            The input matrix needs to have at least m_attribMaxIndex+1 
            columns
        */
        const Matrix &operator()(const Matrix &xs) const;
        
        /// returns the dummy attrib instances
        std::vector<const Attrib*> getAttribs() const { return m_attribs; }
        
        /// creates a human readable string representation of the result
        std::string toString(const std::vector<std::string> &rowLabels=std::vector<std::string> ()) const;
        
        /// saves the result to an xml-file
        void save(const std::string &xmlFileName) const;
      };
      
      protected:
      
      /// internal buffer
      mutable Matrix m_buf;
      
      /// internal result buffer (always returned as const-reference)
      Result m_result;
      

      public:

      /// create instance with given function generator
      PolynomialRegression(const std::string &function);
      
      
      /// computes the polynomial regression parameters
      /** The input matrices xs and ys must have the same number of rows, and
          xs needs to have at least m_attribMaxIndex+1 columns. The input dimension
          is given by the number of columns in xs, the output dimension
          is defined by the number of columns in ys. Each row of xs and ys resp.
          defines a single input/output pair that is used for the internal 
          least-square based optimization 
          
          Optionally, the internally computed pseudo inverse that solves p(xs) a = ys,
          which is p(xs).pinv() can be computed using an SVD-based approach. This
          is usually slower, but more stable and less prone to singular-matrix-exceptions.
      */
      const Result &apply(const Matrix &xs, const Matrix &ys, bool useSVD=false);
      
      /// returns the interpreted function string
      std::string getFunctionString() const;
    };

  }
}
