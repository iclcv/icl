/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/StochasticOptimizer.cpp            **
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

#include <ICLUtils/Macros.h>
#include <ICLMath/StochasticOptimizer.h>
#include <algorithm>
#include <numeric>
#include <vector>
#include <functional>

namespace icl{
  namespace math{
  
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
  
    template class ICLMath_API StochasticOptimizer<float>;
    template class ICLMath_API StochasticOptimizer<double>;
    
  } // namespace math
}
