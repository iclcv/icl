#include <iclStochasticOptimizer.h>
#include <algorithm>
#include <numeric>
#include <vector>
#include <functional>

namespace icl{

  StochasticOptimizer::Result::Result(const float *data,float error, float startError, int steps):
    data(data),error(error),startError(startError),steps(steps){
  }


  StochasticOptimizer::StochasticOptimizer(int dataDim):
    m_dataDim(dataDim){
    
  }

  StochasticOptimizer::Result StochasticOptimizer::optimize(int maxSteps){
    return optimize(-1,maxSteps);
  }
  

  StochasticOptimizer::Result StochasticOptimizer::optimize(float minError, int maxSteps){
    float *data = getData();
    float error = getError(data);
    float startError = error;
    std::vector<float> noisedData(m_dataDim);
    int t = 0;
    do{
      std::transform(data,data+m_dataDim,getNoise(t,maxSteps),
                     noisedData.data(),std::plus<float>());
      float currError = getError(noisedData.data());
      if(currError < error){
        error = currError;
        std::copy(noisedData.begin(),noisedData.end(),data);
      }
      if(minError>0 && currError <= minError){
        return Result(data,error,startError,maxSteps-t);
      }
    }while(t<maxSteps);
    return Result(data,error,startError,maxSteps);
  }
}
