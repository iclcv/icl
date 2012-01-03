/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/SimplexOptimizer.h                    **
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

#ifndef ICL_SIMPLEX_OPTIMIZER_H
#define ICL_SIMPLEX_OPTIMIZER_H

#include <ICLUtils/DynVector.h>
#include <ICLUtils/FixedVector.h>
#include <ICLUtils/Function.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  
  /// Utility structure, that is used as accumulator for results of the SimplexOptimizer class
  template<class T, class Vector=DynColVector<T> >
  struct SimplexOptimizationResult{
    const Vector &x;                      //!< result vector
    const T fx;                           //!< error function value at result vector position
    const int iterations;                 //!< actual count of iterations that were used for optimization
    const std::vector<Vector> &vertices;  //!< end simplex vertices (usually only used for debugging purpose)
  };
  
  
  /// Template based implementation for the Downhill Simplex Optimiztation method
  /** \section __ALGO__ Downhill Simplex Optimiztation Algorithm
      Here, we give a very short pseudo code overview
      <pre>
      input: start position s (in R^D), 
      constants: MAX_ITERATIONS, MIN_ERROR, MIN_DELTA, A, B, G, H
             
      error function: f: R^D -> R
      S <- R+1 simplex nodes (R[i]=s, R[i][i] = S[i]*1.1, or given)
      F <- R+1 function values F[i] = f(S[i])
      
      for i=1 to MAX_ITERATIONS
        best <- min_idx(F)
      
        if F(best) < MIN_ERROR
          break
        endif
      
        worst <- max_idx(F)
        worst2 <- max_idx(F \ F[worst]) # 2nd worst point

        # find the reflection point xg and data center c
        xg <- sum(S \ S[worst])
        c <- xg + S[worst]
        xg = xg / D
      
        # break if error could not be decreased significantly
        if (i>0) and (|c-c_old| < MIN_DELTA)
          break
        else
          c_old <- c
        endif

        # get reflection vector xr using factor A
        xr = xg + A (xg - S[worst])
        fxr = f(xr)
      
        if F[best] <= fxr <= F[worst2]
          # proceed using the reflection
          S[worst] <- xr
          F[worst] <- fxr
        else if fxr < F[best]
          # try an expansion vector xe using factor B
          xe <- xr + B (xr - xg)
          fxe <- f(xe)
          if fxe < fxr
             # use the extension
             S[worst] <- xe
             F[worst] <- fxe
          else
             # use the reflection
             S[worst] <- xr
             F[worst] <- fxr
          endif
        else # fxr > F[worst2]
          # apply contraction xc using constant G
          xc <- xg + G (S[worst] - xg)
          fxc = f(xc)
          if fxc < F[worst]
            # use contraction
            S[worst] = xc
            F[worst] = fxc
          else{ 
            # apply multiple contraction using constant H
            forall j in [0 .. DIM] \ best
               S[j] = S[best] + H (S[j] - S[best])
               F[j] = f(S[j])
            endforall
          endif
        endif
      endfor
      return { S[best], F[best], i, S }


      </pre>
      
      \section __INST__ Explicit Instantiation
      
      The class template is instantated explicitly for all common float and double vector types:
      - icl::DynColVector<float> and icl::DynColVector<double>
      - icl::DynRowVector<float> and icl::DynRowVector<double>
      - icl::DynMatrix<float> and icl::DynMatrix<double> (the entries are used linearily)
      - std::vector<float> and std::vector<double>
      - FixedRowVector<float,D> and FixedRowVector<double,D> where D is 
      - FixedColVector<float,D> and FixedColVector<double,D> where D is 2,3,4 or 6
      - FixedMatrix<float,D,1> and FixedMatrix<double,D,1> where D is 2,3,4 or 6
      - FixedMatrix<float,1,D> and FixedMatrix<double,1,D> where D is 2,3,4 or 6

      \section __DEMOS__ Demo Location
      There are two graphical demos located in the ICLGeom-package due to their dependencies
      to 2D/3D rendering
  */
  template<class T, class Vector=DynColVector<T> >
  class SimplexOptimizer : public Uncopyable{
    struct Data; //!< internal data structure
    Data *m_data;  //!< internal data pointer

    public:
    /// error function type that is used
    typedef Function<T, const Vector&> error_function;
    typedef SimplexOptimizationResult<T,Vector> Result;

    /// creates a new instance with given parameters
    /** @param f error function 
        @param dim vector dimension (please note, for fixed dim vector types,
                                     this must still be set to the right value)
        @param iterations maximum iteration count 
        @param minError minimum error termination criterion 
        @param minDelta, minimum center movement termination criterion 
        @param a reflection factor (default 1.0)
        @param b expansion factor (default 1.0)
        @param g contration factor (default 0.5)
        @param h multiple contraction factor (default 0.5)
    */
    SimplexOptimizer(error_function f, int dim,
                     int iterations=1E5, T minError=1.0E-10, 
                     T minDelta=1.0E-10,
                     T a=1.0, T b=1.0, T g=0.5, T h=0.5);

    /// sets the optimizers dimension (note, that usually the error function must be changed then as well)
    void setDim(int dim);
    
    /// sets the reflection factor
    void setA(T a);

    /// sets the extenxion factor
    void setB(T b);

    /// sets the contraction factor
    void setG(T g);
    
    /// sets the multiple contraction factor
    void setH(T h);
    
    /// sets the maximum iteration termination criterion
    void setIterations(int iterations);
    
    /// sets the minimum error termination criterion
    void setMinError(T minError);
    
    /// sets the minimum delta termination criterion
    void setMinDelta(T minDelta);
    
    /// sets the error function
    void setErrorFunction(error_function f);
    
    /// returns the data dimesions
    int getDim() const;
    
    /// returns the reflection factor
    T getA() const;
    
    /// returns the extension factor
    T getB() const;

    /// returns the contraction factor
    T getG() const;
    
    /// returns the multiple contraction factor
    T getH() const;
    
    /// returns the maximum iteration termination criterion
    int getIterations() const;
    
    /// returns the minimum error termination criterion
    T getMinError() const;
    
    /// returns the minimum delta termination criterion
    T getMinDelta() const;

    /// returns the curren error function
    error_function getErrorFunction() const;
    
    /// runs an optimization using internal parameters starting at given input vector
    /** the initial simplex is created internally using the heuristic shown in the
        \ref __ALGO__ section.
        */
    Result optimize(const Vector &init);
    
    /// rens an optimization using internal parameters using the given initial simplex
    /** the given input simplex must have init[0].dim+1 entries !*/
    Result optimize(const std::vector<Vector> &init);
    
    /// create a default simplex structure
    /** the initial simplex is created internally using the heuristic shown in the
        \ref __ALGO__ section. */
    static std::vector<Vector> createDefaultSimplex(const Vector &init);
  };
} 


#endif
