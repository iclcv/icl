/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/KMeans.h                           **
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

#include <ICLUtils/Random.h>
#include <algorithm>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/CompatMacros.h>

namespace icl{
  namespace math{
    
    
    /// Generic Implementation of the K-Means algorithm
    /** The K-Means algorithms performs vector quantisation in a very
        simple way. Given a set of data pointers xi, and starting with
        a fixed given number of initial centroids, randomly drawn from
        the given data points, it iterates two steps:
        
        -# assign each data point to it's closest centroid 
           (winnter takes all association)
        -# move centroids to the mean of it's associated data points
        
        \section TYPES Vector Types
        
        Supported Vector Types are FixedColVector, FixedRowVector, 
        DynColVector, DynRowVector and Point32f
    */
    template<class Vector, class Scalar>
    class ICL_MATH_API_T KMeans{
      
      /// internal list of centroids
      std::vector<Vector> m_centers;
      
      /// internal buffer
      std::vector<Vector> m_means;
      
      /// numbers of points associated to each centroid
      std::vector<int> m_nums;
      
      /// averate quantisation error for each centroid
      std::vector<Scalar> m_errors;

      /// internal utility function
      static Scalar diff_power_two(const Scalar &a, const Scalar &b){
        Scalar d = a-b;
        return d*d;
      }
      
      /// utility function computing the vector distance
      static inline Scalar dist(const Vector &a, const Vector &b){
        return ::sqrt( std::inner_product(a.begin(),a.end(),b.begin(),Scalar(0), std::plus<Scalar>(), diff_power_two) );
      }
      
      /// sets a vector to null
      static inline void setVectorNull(Vector &v){
        std::fill(v.begin(),v.end(),Scalar(0));
      }
      
      public:

      /// restult type
      struct Result{
        const std::vector<Vector> &centers; //!< final list of centroids
        const std::vector<int> &nums;       //!< final number of points for each centroid
        const std::vector<Scalar> &errors;  //!< average quantisation errors
      };
      
      /// constructor with optionally given number of centers
      inline KMeans(int numCenters=0){
        init(numCenters);
      }
      
      /// deferred intitialization with number of centers
      inline void init(int numCenters){
        m_centers.resize(numCenters);
        m_means.resize(numCenters);
        m_nums.resize(numCenters);
        m_errors.resize(numCenters);
      }

      /// finds the nearest centroid for a given data pointer
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
      
      /// runs the KMeans algorithms on the given data 
      /** if reinitCenters is set to false, the centers of the last
          call to this method are reused */
      template<class RandomAcessIterator>
      Result apply(RandomAcessIterator begin, RandomAcessIterator end, int numSteps = 1000, bool reinitCenters=true){
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
            setVectorNull(m_means[i]);
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

    /** \cond */
    template<>
    ICL_MATH_API float KMeans<utils::Point32f, float>::dist(const utils::Point32f &a, const utils::Point32f &b){
      return a.distanceTo(b);
    }
      
    template<>
    ICL_MATH_API void KMeans<utils::Point32f, float>::setVectorNull(utils::Point32f &p){
      p = utils::Point32f::null;
    }
    /** \endcond */


  }
}
