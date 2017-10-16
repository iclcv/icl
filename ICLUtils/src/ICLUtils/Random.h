/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Random.h                         **
** Module : ICLUtils                                               **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/ClippedCast.h>
#include <cmath>
#include <cstdlib>
#include <vector>

#ifdef WIN32
  #undef max
#endif

namespace icl{
  namespace utils{
    /// Initilaize the random number generator. \ingroup RANDOM
    /** @param seedval The seed value (e.g. time(0) ...)
    */
    inline void randomSeed(long int seedval) {
  #ifdef WIN32
      srand(seedval);
  #else
      srand48(seedval);
  #endif
    }

    /// Initilaize the random number generator (with Time::now().toMicroSeconds()).\ingroup RANDOM
    inline void randomSeed() { randomSeed(Time::now().toMicroSeconds()); }

    /// Object based random seed caller \ingroup RANDOM
    /** Calls randomSeed() at construction time */
    struct RandomSeeder{
      inline RandomSeeder(){randomSeed();}
    };

    /// Generates random numbers in range [0,1]  \ingroup RANDOM
    inline double random(double max = 1) {
  #ifdef WIN32
      // this generates quite poor random numbers, because RAND_MAX = 32767
      return max*(static_cast<double>(rand()) / (1.0 + static_cast<double>(RAND_MAX)));
  #else
      return max*drand48();
  #endif
    }

    /// Generate a random number in range [min,max] \ingroup RANDOM
    /** @param min a float argument. The lower intervall bound
        @param max a float argument. The upper interval bound
    */
    inline double random(double min, double max) {
      return ((max - min) * random() + min);
    }

    /// equivalent to random (r.minVal,r.maxVal)
    template<class T>
    inline double random(const Range<T> &r){
      return random((double)r.minVal,(double)r.maxVal);
    }

    ///Creates a non-negative random number in range [0,max] \ingroup RANDOM
    /** @param max The upper limit for the returned number
    */
    inline unsigned int random(unsigned int max) {
      unsigned int val = static_cast<unsigned int>(floor(random (static_cast<double>(max)+1.0)));
      return iclMin(val, max);
    }

  #if 0
    // removed due to lack of usebility: use std::fill(i.begin(c),i.end(c),URand(range)) instead

    /// fill an image with uniform distributed random values in the given range \ingroup RANDOM
    /** @param poImage image to fill with random values (NULL is not allowed)
        @param range for the random value
        @param roiOnly decides whether to apply the operation on the whole image or on its ROI only
    **/
    ICLUtils_API void random(ImgBase *poImage, const Range<double> &range=Range<double>(0,255), bool roiOnly=true);

    /// fill an image with gauss-distributed random values with given mean, variance and min/max value \ingroup RANDOM
    /** @param poImage image to fill with random values (NULL is not allowed)
        @param mean mean value for all gauss distributed random variables
        @param var variance for all gauss distributed random variables
        @param minAndMax clipping range for all variables
        @param roiOnly decides whether to apply the operation on the whole image or on its ROI only
    **/
    ICLUtils_API void gaussRandom(ImgBase *poImage, double mean, double var, const Range<double> &minAndMax, bool roiOnly=true);
  #endif
    /// Generate a gaussian random number with given mean and variance \ingroup RANDOM
    /** @param mean mode of the gaussian
        @param var variance of the gaussian
        @return gaussian distributed variable
        @sa double(double,double,const Range<double>&),
    **/
    ICLUtils_API double gaussRandom(double mean, double var);

    /// Generate a gaussian random number with given mean and variance and clips the result to a range \ingroup RANDOM
    /** @param mean mode of the gaussian
        @param var variance of the gaussian
        @param range clipping range for the returned value
        @return gaussian distributed variable clipped to range range
        @sa double(double,double,const Range<double>&),
    **/
    inline double gaussRandom(double mean, double var, const Range<double> &range){
      return clip<double>( gaussRandom(mean,var), range.minVal, range.maxVal);
    }

    /// lightweight Random generator class for uniform random distributions
    /** URand obeject can be used like 'normal double values'. Each time
        some other variable is assigned by it, it returns a random value.
        By this means, e.g. STL-containers can be filled/created with random numbers
        \code
        std::vector<double> foo(100,URand());
        std::fill(foo.begin(),foo.end(),URand(0,42));
        \endcode
        But do not try to fill an Image with random numbers like this:
        \code
        Img64f image(Size::VGA,1);
        image.clearAllChannels(URand());
        \endcode
        Here: URand is only evaluatet at the function interface, so the
        image is filled with a single random value;
     */
    class URand{
      Range64f range;
      public:
      /// Range [0,1]
      inline URand():range(0,1){};

      /// Given range
      inline URand(icl64f min, icl64f max):range(min,max){}

      /// Given range
      inline URand(const Range64f &range):range(range){};

      /// returns random(this->min,this->max)
      inline operator icl64f() const { return random(range.minVal,range.maxVal); }
    };

    /// lightweight Random generator class for uniform random distributions in positive integer domain
    /** @see URand*/
    class URandI{
      unsigned int max;
      public:
      /// Create with given max value
      inline URandI(unsigned int max):max(max){}

      /// returns random(this->max)
      inline operator unsigned int() const{ return random(max); }
    };

    /// lightweight Random generator class for gaussian distributed numbers
    /** @see URand*/
    class GRand{
      icl64f mean,var;
      public:
      /// Create with optionally given mean and variance
      inline GRand(icl64f mean=0, icl64f var=1.0):mean(mean),var(var){}

      /// returns gaussRandom(this->mean,this->var)
      inline operator icl64f() const { return gaussRandom(mean,var); }
    };

    /// lightweight Random generator class for gaussian distributed numbers clipped to a given range
    /** @see URand*/
    class GRandClip{
      icl64f mean,var;
      Range64f range;
      public:
      /// Create with optionally given mean and variance
      inline GRandClip(icl64f mean, icl64f var,const Range64f &range):mean(mean),var(var),range(range){}

      /// returns gaussRandom(this->mean,this->var)
      inline operator icl64f() const { return gaussRandom(mean,var,range); }
    };


  /* }}} */

    inline std::vector<int> get_random_index_subset(int containerSize,
                                                    int subsetSize)
      throw (ICLException){
      if(subsetSize > containerSize){
        throw ICLException("get_random_index_subset: subsetsize must be <= containerSize");
      }
      std::vector<int> s(containerSize);
      for(int i=0;i<containerSize;++i) s[i] = i;
      std::random_shuffle(s.begin(), s.end());
      return std::vector<int>(s.begin(), s.begin()+subsetSize);
    }

    template<class T>
    inline std::vector<T> get_random_subset(const std::vector<T> &s, int subsetSize)
      throw (ICLException){
      std::vector<int> indices = get_random_index_subset(s.size(), subsetSize);
      std::vector<T> subset(subsetSize);
      for(int i=0;i<subsetSize;++i){
        subset[i] = s[indices[i]];
      }
      return subset;
    }

    template<class T>
    inline void get_random_subset(const std::vector<T> &s, int subsetSize,
                                  std::vector<T> &subset)
      throw (ICLException){
      std::vector<int> indices = get_random_index_subset(s.size(), subsetSize);
      subset.resize(subsetSize);
      for(int i=0;i<subsetSize;++i){
        subset[i] = s[indices[i]];
      }
    }

    template<class T>
    inline void get_random_subset(const std::vector<T> &s, int subsetSize,
                                  std::vector<T> &subset, std::vector<int> &indices)
      throw (ICLException){
      indices = get_random_index_subset(s.size(), subsetSize);
      subset.resize(subsetSize);
      for(int i=0;i<subsetSize;++i){
        subset[i] = s[indices[i]];
      }
    }



  } // namespace utils
}

