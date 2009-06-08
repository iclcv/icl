#ifndef ICL_STOCHASTIC_OPTIMIZER_H
#define ICL_STOCHASTIC_OPTIMIZER_H

namespace icl{

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
  */
  class StochasticOptimizer {
    public:
    /// Result structure
    struct Result{
      /// creates a new result structure (internally used only)
      Result(const float *data=0,float error=0, float startError=0, int steps=0);
      const float *data; //!< resulting optimized data vector
      float error;       //!< reached minimum error
      float startError;  //!< first error measurement (reached with initial data vector)
      int steps;         //!< steps iterated!
    };
    
    /// create a stochastic optimizer with given data dimension
    StochasticOptimizer(int dataDim=0);
    
    /// start optimization process with given step count
    Result optimize(int timeSteps);
    
    /// start optimization process with given step count and minimal error stop-criterion
    Result optimize(float minError, int maxSteps);
    
    protected:
    /// must return the current data vector
    /** data is un-const, as it is changed in each step if the reached error is less
        then the current best error. Returned pointer must have at least the length of
        dataDim passed to the constructor */
    virtual float *getData() = 0;
    
    /// must return current error value (>=0)
    /** returns the error measurement dependent on given data vector */
    virtual float getError(const float *data)=0;
    
    /// returns a noise vector (of size dataDim, which was passed to the constructor)
    /** optionally, the noise-strength might depend on the current time-progress. Therefore,
        currentTime and endTime is also passed to this functions */
    virtual const float *getNoise(int currentTime, int endTime)=0;
    
    /// this function is called before the optimization is started
    /** Internally data must be initialized */
    virtual void reinitialize() = 0;
    
    /// a pure utility function, which can be implemented in derived classes to notify optization progress (somehow)
    virtual void notifyProgress(int t, int numSteps, int startError, int currBestError, int currError,const  float *data, int dataDim);

    private:
    /// internal data-dimension variable
    int m_dataDim;
  };
}

#endif
