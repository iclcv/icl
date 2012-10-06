/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/VectorQuantisation.h                   **
** Module : ICLMath                                                **
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

#pragma once

#include <ICLUtils/Random.h>
#include <algorithm>

namespace icl{
  namespace math{
    
    template<class Vector, class Scalar>
    class GenericVectorQuantisation{
      
      std::vector<Vector> m_centers;
      std::vector<Vector> m_means;
      std::vector<int> m_nums;
      std::vector<Scalar> m_errors;

      static Scalar diff_power_two(const Scalar &a, const Scalar &b){
        Scalar d = a-b;
        return d*d;
      }
      
      static inline Scalar dist(const Vector &a, const Vector &b){
        return ::sqrt( std::inner_product(a.begin(),a.end(),b.begin(),Scalar(0), std::plus<Scalar>(), diff_power_two) );
      }
      
      public:

      struct Result{
        const std::vector<Vector> &centers;
        const std::vector<int> &nums;
        const std::vector<Scalar> &errors;
      };
      
      inline GenericVectorQuantisation(int numCenters=0){
        init(numCenters);
      }
      
      inline void init(int numCenters){
        m_centers.resize(numCenters);
        m_means.resize(numCenters);
        m_nums.resize(numCenters);
        m_errors.resize(numCenters);
      }

      inline int findNN(const Vector &v, Scalar &minDist){
        int minIdx = 0;
        minDist = dist(v,m_centers[0]);
        for(size_t i=1;i<m_centers.size();++i){
          Scalar currDist = dist(v,m_centers[i]);
          if(currDist < minDist){
            minDist = currDist;
            minIdx = i;
          }
        }
        return minIdx;
      }
      
      template<class RandomAcessIterator>
      Result run(RandomAcessIterator begin, RandomAcessIterator end, int numSteps = 1000, bool reinitCenters=true){
        Scalar minDist = 0;
        
        if(reinitCenters){
          URandI r((int)(end-begin)-1);
          for(size_t i=0;i<m_centers.size();++i){
            int ri = r;
            m_centers[i] = *(begin+ri); 
          }
        }
        
        for(int step=0;step<numSteps;++step){
          // empty accumulators
          for(size_t i=0;i<m_means.size();++i){
            std::fill(m_means[i].begin(),m_means[i].end(),Scalar(0));
            m_nums[i] = Scalar(0);
            m_errors[i] = Scalar(0);
          }

          // estimate means
          for(RandomAcessIterator it=begin;it != end; ++it){
            const int nn = findNN(*it,minDist);
            m_means[nn] += *it;
            m_nums[nn] ++;
            m_errors[nn] += minDist;
          }
          
          for(size_t i=0;i<m_means.size();++i){
            if(m_nums[i]){
              m_centers[i] = m_means[i] * (Scalar(1.0)/m_nums[i]);
              m_errors[i] /= m_nums[i];
            }// empty voronoi tessels are not moved
          }
        }
        
        Result r= { m_centers, m_nums, m_errors };
        return r;
      }
    };
  }
}
