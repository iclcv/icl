/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/StochasticOptimizer.cpp                   **
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

#include <ICLUtils/StochasticOptimizer.h>
#include <algorithm>
#include <numeric>
#include <vector>
#include <functional>
#include <ICLUtils/Macros.h>

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
