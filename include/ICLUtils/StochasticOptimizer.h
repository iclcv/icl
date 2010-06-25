/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/StochasticOptimizer.h                 **
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

#ifndef ICL_STOCHASTIC_OPTIMIZER_H
#define ICL_STOCHASTIC_OPTIMIZER_H

namespace icl{

  /// Utility structure for the stochastic optimizer class
  template<class T>
  struct StochasticOptimizerResult{
      /// creates a new result structure (internally used only)
    StochasticOptimizerResult(const T *data=0,T error=0, T startError=0, int steps=0);
    const T *data; //!< resulting optimized data vector
    T error;       //!< reached minimum error
    T startError;  //!< first error measurement (reached with initial data vector)
    int steps;         //!< steps iterated!
  };
  
  /// The StochasticOptimizer is a tiny frame-work for simple stochastic optimization processes
  /** Stochastic optimization is minimization of an error function, which depends on a set of 
      latent variables (here: called data). The naive approach origins from starting configuration,
      which must be created by the pure virtual reinitialize function. 
      Now in each optimization step, the current data vector is changed slightly using an additive
      noise vector which must be provided by the also pure virtual getNoise-function. If the error
      arising from the changed data is less then the current minimal error, then the change data
      vector use hold, otherwise the last change of data is reverted.
      This procedure is iterated until either a maximum number of iterations is reached or until
      a given minimal error is reached.
      
      **NEW** now this class is a template (defined for float and double)
  */
  template<class T=float>
  class StochasticOptimizer {
    public:
    /// Result structure
    typedef StochasticOptimizerResult<T> Result;
    
    /// create a stochastic optimizer with given data dimension
    StochasticOptimizer(int dataDim);
    
    /// start optimization process with given step count
    Result optimize(int maxTimeSteps){
      return optimize(-1,maxTimeSteps);
    }
    
    /// start optimization process with given step count and minimal error stop-criterion
    Result optimize(T minError, int maxSteps);
    
    protected:
    /// must return the current data vector
    /** data is un-const, as it is changed in each step if the reached error is less
        then the current best error. Returned pointer must have at least the length of
        dataDim passed to the constructor */
    virtual T *getData() = 0;
    
    /// must return current error value (>=0)
    /** returns the error measurement dependent on given data vector */
    virtual T getError(const T *data)=0;
    
    /// returns a noise vector (of size dataDim, which was passed to the constructor)
    /** optionally, the noise-strength might depend on the current time-progress. Therefore,
        currentTime and endTime is also passed to this functions */
    virtual const T *getNoise(int currentTime, int endTime)=0;
    
    /// this function is called before the optimization is started
    /** Internally data must be initialized */
    virtual void reinitialize() = 0;
    
    /// a pure utility function, which can be implemented in derived classes to notify optization progress (somehow)
    virtual void notifyProgress(int t, int numSteps, int startError, 
                                int currBestError, int currError,const  T *data, int dataDim);

    private:
    /// internal data-dimension variable
    int m_dataDim;
  };
}

#endif
