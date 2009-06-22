#include <iclStochasticOptimizer.h>
#include <algorithm>
#include <numeric>
#include <vector>
#include <functional>
#include <iclMacros.h>

namespace icl{

  template<class T>
  StochasticOptimizerResult<T>::StochasticOptimizerResult(const T *data,T error, T startError, int steps):
    data(data),error(error),startError(startError),steps(steps){
  }
  
  template<class T>
  StochasticOptimizer<T>::StochasticOptimizer(int dataDim):
    m_dataDim(dataDim){
  }

  template<class T>  
  void StochasticOptimizer<T>::notifyProgress(int,int,int, int, int,const T *, int){}

  template<class T>
  StochasticOptimizerResult<T> StochasticOptimizer<T>::optimize(T minError, int maxSteps){
    reinitialize();
    T *data = getData();
    T error = getError(data);
    T startError = error;
    std::vector<T> noisedData(m_dataDim);
    int t = 0;
    notifyProgress(t,maxSteps,startError,error,error,data,m_dataDim);
    do{
      std::transform(data,data+m_dataDim,getNoise(t,maxSteps),
                     noisedData.data(),std::plus<T>());
      T currError = getError(noisedData.data());

      notifyProgress(t,maxSteps,startError,error,currError,data,m_dataDim);
      if(currError < error){
        error = currError;
        std::copy(noisedData.begin(),noisedData.end(),data);
      }
      if(minError>0 && currError <= minError){
        return Result(data,error,startError,maxSteps-t);
      }
      ++t;
    }while(t<maxSteps);
    return Result(data,error,startError,maxSteps);
  }

  template class StochasticOptimizer<float>;
  template class StochasticOptimizer<double>;
  
}
