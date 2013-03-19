/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/Homography2D.h                     **
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

#include <ICLMath/FixedMatrix.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  namespace math{
    
    /// Utility structure that represents a 2D homography (implemented for float and double)
    /** Basically, a 2D homography implements a transformation between 2 paralellograms 
        Given a set of at least 4 points in parallellogram A and the same number of 
        corresponding points in parallelogram B, the homography (affine 2D transformation
        between space A and space B is defined as follows:
        
        \f$H\f$ must transform each point \f$a_i\f$ (given wrt. space A) to it's corresponding
        point \f$b_i\f$. Please refer to Wikipedia or the class implementation for
        more details:
        
        @section ALG Algorithms
        The Homography2D class provides two different algorithms: a  faster simple one, and a
        slightly slower, but more elaborated one. The simple algorithm creates a simpler
        matrix whose last row becomes (0,0,1)^T. In some cases, this is enough, which is why,
        this algorithm is provided even though, it does not lead to perfect results.
        
        @subsection SIMPLE The Simple Algorithm
    
        The homography is used to transform a set of homogeneous 2D source points
        \f$ \{a_i\} \;\;i \in [0,n[ \f$ into a set of 2D destination points
        \f$ \{b_i\} \;\;i \in [0,n[ \f$. 
        Since we search for a linear transformation, This transformation H is modelled by
        a 3x3 matrix whose last element is fixed to 1. We call the rows of H X,Y,L, which
        leads to the targeted equation:
        \f[
        H a_i = b_i \;\;\; \forall i \in [0,n[
        \f]
        The most simple approach is to stack all \f$a_i\f$ and all \f$b_i\f$ horizontally
        which leads to the equation
        \f[
        H A = B
        \f]
        \f[
        H (a_0 a_1 a_2 ... a_{n-1}) = (b_0 b_1 b_2 ... b_{n-1})
        \f]
        Obviously, this can be solved using a standard pseudoinverse approach
        \f[
        H = B A^{-1}
        \f]
        This approach does somehow optimize the problem, but it does not really touch
        the last row of H. Therefore, the results of this simple approach are sometimes
        not good enough. Please continue with \ref ADV
        
  
        
        \subsection ADV The Advanced Algorithm
        
        In order to also fill the last row of H with optimal values, the originating problem
        must be reformulated in matrix notation in a different way:\n
        Again, we start with
  
        \f[
        H a_i = b_i \;\;\; \forall i \in [0,n[
        \f]
  
        For a single \f$a_i\f$ (we call it a and the counter part b resp.), we always have two
        formulas, one for the x- and one for the y-component.
        
        \f[
        H ( a_x a_y 1 )^T = (b_x b_y 1)^T
        \f]
  
        Decomposing H to its rows X,Y and L, this can be reformulated as
        
        \f[
        \frac{X a}{L a} = b_x    \;\;\;\;\;and\;\;\;\;\;\;    \frac{Y a}{L a} = b_y
        \f]
  
        \f[
        \Leftrightarrow \frac{X a}{b_x} = La    \;\;\;\;\;and\;\;\;\;\;\;    \frac{Y a}{b_y} = La
        \f]
  
        \f[
        \Leftrightarrow \frac{X a}{b_x} - La = 0    \;\;\;\;\;and\;\;\;\;\;\;    \frac{Y a}{b_y} - La = 0
        \f]
  
        \f[
        \Leftrightarrow \frac{X a}{b_x} - L_x a_x - L_y a_y - 1 = 0    \;\;\;\;\;and\;\;\;\;\;\;    \frac{Y a}{b_y} - L_x a_x - L_y a_y - 1 = 0
        \f]
  
        \f[
        \Leftrightarrow \frac{X a}{b_x} - L_x a_x - L_y a_y = 1    \;\;\;\;\;and\;\;\;\;\;\;    \frac{Y a}{b_y} - L_x a_x - L_y a_y = 1
        \f]
  
        By multiplying with \f$ b_x \f$ (\f$ b_y \f$ resp.), we get
  
        \f[
        \Leftrightarrow X a - L_x b_x a_x - L_y a_y b_x = b_x    \;\;\;\;\;and\;\;\;\;\;\;    Y a - L_x a_x b_y - L_y a_y b_y = b_y
        \f]
        
        These two equations can be expressed in a single huge matrix expression  \f$ M h = r \f$ where M is a 2n by 8 matrix, 
        and \f$ h = (X^T Y^T L_x L_y)^T \f$ and r are 2n-dimensional row vectors. M and r are build as follows. For each input/output tuple
        \f$(a_i,b_i)\f$, two rows of M are created using the following scheme:
        
        \f[
        M=\left(\begin{array}{cccccccc} 
        a_x & a_y & 1 & 0 & 0 & 0 & -a_x b_x & -a_y b_x  \\
        0 & 0 & 0 & a_x & a_y & 1 & -a_x b_y & -a_y b_y  \\
        ...
        \end{array}\right)
        \f]
        
        The result vector r is just filled with the target values 
        \f[ 
        r = ( b_x b_y ... )^T
        \f]
        
        Finally the matrix equation \f$ M h = r \f$ is evaluated with respect to h by
        \f[
        h = M^{-1} r
        \f]
        And h's elements are put back into the homography matrix H in a row-wise manner. Remember that The last elememt
        of H was set fixed to 1. <b>Please note:</b> Internally, the matrix equation \f$ M h = r \f$ is solved using
        an SVD (@see ICLMath/DynMatrix::solve) based solver (the much faster lu-decomposition based solver does not provide useful results)
    */
    template<class T>
    struct GenericHomography2D : public FixedMatrix<T,3,3>{
      /// super class typedef for shorter super-class references
      typedef FixedMatrix<T,3,3> Super;
      
      /// Internally used algorithm type
      enum Algorithm{
        Simple,   //!< use the simple algorithm (@see @ref ALG)
        Advanced, //!< use the advanced algorithm (@see @ref ALG)
      };
      
      /// Empty constructor
      GenericHomography2D(){}
      
      /// Constructor from given two point sets of size n>=4
      GenericHomography2D(const utils::Point32f *pAs, const utils::Point32f *pBs, int n=4, Algorithm algo = Advanced);
  
  
      /// applies a given homography matrix
      static inline utils::Point32f apply_homography(const FixedMatrix<float,3,3> &H, const utils::Point32f &p){
        float az = H(0,2)*p.x + H(1,2) * p.y + H(2,2);
        return utils::Point32f(( H(0,0)*p.x + H(1,0) * p.y + H(2,0) ) / az,
                               ( H(0,1)*p.x + H(1,1) * p.y + H(2,1) ) / az );
      }
  
      /// applies the homography 
      inline utils::Point32f apply(const utils::Point32f &p) const{
        return apply_homography(*this,p);
      }
  
      /// applies a given homography matrix
      static inline utils::Point apply_homography_int(const FixedMatrix<float,3,3> &H, const utils::Point &p){
        float az = H(0,2)*p.x + H(1,2) * p.y + H(2,2);
        return utils::Point(round(( H(0,0)*p.x + H(1,0) * p.y + H(2,0) ) / az),
                     round(( H(0,1)*p.x + H(1,1) * p.y + H(2,1) ) / az) );
      }
  
      /// applies the homography 
      inline utils::Point32f apply_int(const utils::Point32f &p) const{
        return apply_homography_int(*this, p);
      }
    };
    
    /// default homography 2D type definition (usually fload depth is enough)
    typedef GenericHomography2D<float> Homography2D;
    
  
  } // namespace math
}

