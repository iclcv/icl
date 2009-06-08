#ifndef ICL_STOCHASTIC_OPTIMIZER_H
#define ICL_STOCHASTIC_OPTIMIZER_H


namespace icl{

  class StochasticOptimizer {
    public:
    struct Result{
      Result(const float *data=0,float error=0, float startError=0, int steps=0);
      const float *data;
      float error;
      float startError;
      int steps;
    };
    
    StochasticOptimizer(int dataDim=0);
    
    Result optimize(int timeSteps);
    Result optimize(float minError, int maxSteps);
    
    protected:
    virtual float *getData() = 0;
    
    /// must return current error value (>=0)
    virtual float getError(const float *data)=0;
    virtual const float *getNoise(int currentTime, int endTime)=0;
    
    private:
    int m_dataDim;
    
  };
}

#endif
