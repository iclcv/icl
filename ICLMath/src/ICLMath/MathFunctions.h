/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/MathFunctions.h                    **
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

#include <vector>
#include <algorithm>
#include <cmath>
#ifdef HAVE_IPP
#include <ipps.h>
#endif

#include <ICLUtils/Random.h>

namespace icl {
  namespace math{
    /// Calculate the euclidian distance of two vectors v1 and v2 \ingroup MATH
    /** @param v1Begin first element of v1
        @param v1End   end of v1 (points the first element behind v1)
        @param v2Begin first element of v2
        @return The euclidian distance |v1-v2|
    */
    template <class ForwardIterator> 
    inline float euclidian(ForwardIterator v1Begin, ForwardIterator v1End,
                           ForwardIterator v2Begin) {
      float fSum = 0.0, fDiff;
      for (; v1Begin != v1End; ++v1Begin, ++v2Begin) {
        fDiff = (*v1Begin-*v2Begin);
        fSum += fDiff*fDiff;
      }
      return ::sqrt(fSum);
    }
    
    /// Calculate the euclidian distance of points a and b \ingroup MATH
    /** @param a The first point
        @param b The second point
        @return The distance of point a and b
    */
    template <class T>
    inline float euclidian(const std::vector<T> &a, const std::vector<T> &b) {
      ICLASSERT_RETURN_VAL(a.size() == b.size(), float(0));
      return euclidian (a.begin(), a.end(), b.begin());
    }
    
    /// computes the mean value of a data range \ingroup MATH
    /** @param begin start iterator 
        @param end end iterator 
        IPP-optimized for float and double */
    template <class ForwardIterator>
    inline double mean(ForwardIterator begin, ForwardIterator end){
      if(begin == end) return 0;
      double sum = 0;
      int num = 0;
      while(begin != end){
        sum += *begin++;
        num++;
      }
      return sum / num;
    }

    /** \cond */
#ifdef HAVE_IPP
    template<> inline double mean<const icl32f*>(const icl32f *begin,const icl32f *end){
      icl32f m = 0;
      ippsMean_32f(begin,end-begin,&m,ippAlgHintAccurate);
      return m;
    }
    template<> inline double mean<const icl64f*>(const icl64f *begin,const icl64f *end){
      icl64f m = 0;
      ippsMean_64f(begin,end-begin,&m);
      return m;
    }
#endif
    /** \endcond */
    
    
  
    /// Compute the variance of a given data range with given mean value \ingroup MATH
    /** @param begin start iterator
        @param end end iterator
        @param mean mean value of the range
        @param empiricMean if true, sum of square distances is devidec by n-1 else by n
        */
    template <class ForwardIterator>
    inline double variance(ForwardIterator begin, ForwardIterator end, double mean, bool empiricMean=true){
      if(begin == end) return 0;
      register double sum = 0;
      register double d = 0;
      int num = 0;
      while(begin != end){
        d = *begin - mean;
        sum += d*d;
        ++begin;
        num++;
      }
      return sum/(empiricMean&&num>1 ? num - 1 : num); 
    }
  
    /// Compute the variance of a given data range \ingroup MATH
    /** @param begin start ForwardIterator
        @param end end ForwardIterator
        */
    template <class ForwardIterator>
    inline double variance(ForwardIterator begin, ForwardIterator end){
      return variance(begin,end,mean(begin,end),true);
    }
    
    
    /// Compute std-deviation of a data set with given mean (calls sqrt(variance(..))
    /** @param begin start iterator
        @param end end iterator
        @param mean given mean value
        @param empiricMean if true, sum of square distances is devidec by n-1 else by n
    */
    template <class ForwardIterator>
    inline double stdDeviation(ForwardIterator begin, ForwardIterator end, double mean, bool empiricMean=true){
      return ::sqrt(variance(begin,end,mean,empiricMean));
    }
  
    /// Compute std-deviation of a data set
    /** @param begin start iterator
        @param end end iterator
        */
    template <class ForwardIterator>
    inline double stdDeviation(ForwardIterator begin, ForwardIterator end){
      return ::sqrt(variance(begin,end));
    }
  
  
    /// Calculates mean and standard deviation of given data range simultanously
    /** @param begin start iterator
        @param end end iterator
        @return pair p with p.first = mean and p.second = stdDev
    */
    template<class ForwardIterator>
    inline std::pair<double,double> meanAndStdDev(ForwardIterator begin,ForwardIterator end){
      std::pair<double,double> md;
      md.first = mean(begin,end);
      md.second = stdDeviation(begin,end,md.first,true);
      return md;
    }
  
                                 
  } // namespace math
} //namespace icl

