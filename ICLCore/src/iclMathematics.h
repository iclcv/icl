#ifndef ICL_MATH_H
#define ICL_MATH_H

#include <iclImg.h>
#include <iclImgIterator.h>
#include <vector>
#include <algorithm>
#include <cmath>
#ifdef WITH_IPP_OPTIMIZATION
#include <ipps.h>
#endif


namespace icl {
  
  /* {{{ random functions */

  //--------------------------------------------------------------------------
  /*!
    @brief Initilaize the random number generator. \ingroup MATH
    @param seedval The seed value
    @return -
    @sa void randomSeed()
  */
  inline void randomSeed(long int seedval) {
#ifdef WIN32
	  srand(seedval);
#else
	  srand48(seedval);
#endif
  }

  //--------------------------------------------------------------------------
  /*!
    @brief Initilaize the random number generator.\ingroup MATH 
    @sa void randomSeed(long int)
  */
  inline void randomSeed() {randomSeed(Time::now().toMicroSeconds());}

  
  struct MathematicsRandomSeedInitializer{
    inline MathematicsRandomSeedInitializer(){ randomSeed(); }
  };

  //--------------------------------------------------------------------------
  /*!
    @brief Generates random numbers between 0 and 1. \ingroup MATH
    @param -
    @return random number
    @sa double random()
  */
  inline double random() {
#ifdef WIN32
     // this generates quite poor random numbers, because RAND_MAX = 32767
     return static_cast<double>(rand()) / (1.0 + static_cast<double>(RAND_MAX));
#else
     return drand48();
#endif
  }
  
  //--------------------------------------------------------------------------
  /*!
    @brief Generate a random number to an upper limit. \ingroup MATH
    @param max a float argument. The upper limit for the returned number
    @return -
    @sa float gaussRandom(float), 
        void  random(vector<float>, float),
        void  random(float),
        void  gaussRandom(vector<float>, float);
  */
  inline double random(double max) {
     FUNCTION_LOG("float");
     return max * random();
  }

  //--------------------------------------------------------------------------
  /*!
    @brief Generate a random number between a lower and upper limit. \ingroup MATH
    @param min a float argument. The lower intervall limit
    @param max a float argument. The upper interval limit 
    @return -
    @sa float gaussRandom(float), 
        void random(vector<float>, float),
        float random(float),
        void gaussRandom(vector<float>, float);
  */
  inline double random(double min, double max) {
    FUNCTION_LOG("float, float");
    return ((max - min) * random() + min); 
  }
 
  //--------------------------------------------------------------------------
  /*!
    @brief Creates a non-negative random number to an upper limit max \ingroup MATH
    @param max The upper limit for the returned number
    @return The random number
  */
  inline unsigned int random(unsigned int max) {
    FUNCTION_LOG("unsigned int");
    unsigned int val = static_cast<unsigned int>(floor(random (static_cast<double>(max)+1.0)));
    return iclMin(val, max);
  }
   
  /// fill an image with uniform distributed random values in the given range \ingroup MATH
  /** @param poImage image to fill with random values (NULL is not allowed) 
      @param range for the random value 
      @param roiOnly decides whether to apply the operation on the whole image or on its ROI only 
  **/
  void random(ImgBase *poImage, const Range<double> &range=Range<double>(0,255), bool roiOnly=true);

  /// fill an image with gauss-distributed random values with given mean, variance and min/max value \ingroup MATH
  /** @param poImage image to fill with random values (NULL is not allowed) 
      @param mean mean value for all gauss distributed random variables
      @param var variance for all gauss distributed random variables
      @param minAndMax clipping range for all variables
      @param roiOnly decides whether to apply the operation on the whole image or on its ROI only 
  **/
  void gaussRandom(ImgBase *poImage, double mean, double var, const Range<double> &minAndMax, bool roiOnly=true);

  /// Generate a gaussian random number with given mean and variance \ingroup MATH
  /** @param mean mode of the gaussian
      @param var variance of the gaussian
      @return gaussian distributed variable
      @sa double(double,double,const Range<double>&), 
  **/
  double gaussRandom(double mean, double var);

  /// Generate a gaussian random number with given mean and variance and clips the result to a range \ingroup MATH
  /** @param mean mode of the gaussian
      @param var variance of the gaussian
      @param range clipping range for the returned value
      @return gaussian distributed variable clipped to range range
      @sa double(double,double,const Range<double>&), 
  **/
  inline double gaussRandom(double mean, double var, const Range<double> &range){
    return icl::clip( gaussRandom(mean,var), range.minVal, range.maxVal);
  }

  //--------------------------------------------------------------------------
  /*!
    @brief Initialize the elements of a std::vector by values of \ingroup MATH
    an generator function taking no arguments, e.g. random()
  */
  template <class T>
  inline void initVector(std::vector<T> &vec, double (*gen)()) {
     FUNCTION_LOG("vector<T> &, Generator");
     std::generate (vec.begin(), vec.end(), gen);
  }

  /*!
    @brief Initialize the elements of a std::vector by values of \ingroup MATH
    an generator function taking one argument, e.g. randomGauss(max)
  */
  template <class T>
  inline void initVector(std::vector<T> &vec, double (*f)(double), double arg) {
     FUNCTION_LOG("vector<T> &, Function, arg");
     for (typename std::vector<T>::iterator it=vec.begin(), end=vec.end();
          it != end; ++it) {
        *it = f(arg);
     }
  }

/* }}} */
                              
  /* {{{ distance functions */

  /*!
    @brief Calculate the euclidian distance of two vectors v1 and v2 \ingroup MATH
    @param v1Begin first element of v1
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

  /*!
    @brief Calculate the euclidian distance of points a and b \ingroup MATH
    @param a The first point
    @param b The second point
    @return The distance of point a and b
  */
  template <class T>
  inline float euclidian(const std::vector<T> &a, const std::vector<T> &b) {
     ICLASSERT_RETURN_VAL(a.size() == b.size(), float(0));
     return euclidian (a.begin(), a.end(), b.begin());
  }
     
/* }}} */
                              
  /* {{{ statistic functions */

  /* {{{ mean  */

  /// compute mean value of a data range \ingourp MATH
  /** @param begin start iterator 
      @param end end iterator*/
  template <class ForwardIterator>
  inline double mean(ForwardIterator begin, ForwardIterator end){
    if(!(begin-end)) return 0;
    double sum = 0;
    while(begin != end) sum += *begin++;
    return sum / (end-begin);
  }
  
#ifdef WITH_IPP_OPTIMIZATION
  template<> inline double mean<const icl32f*>(const icl32f *begin,const icl32f *end){
    icl32f m = 0;
    // More fast: ippAlgHintFast
    ippsMean_32f(begin,end-begin,&m,ippAlgHintAccurate);
    return m;
  }
  template<> inline double mean<const icl64f*>(const icl64f *begin,const icl64f *end){
    icl64f m = 0;
    ippsMean_64f(begin,end-begin,&m);
    return m;
  }
  // Scalfactor version not used ippsMean_16s_Sfs(const Ipp16s* pSrc, int len, Ipp16s* pMean, int scaleFactor)
  // Scalfactor version not used ippsMean_32s_Sfs(const Ipp32s* pSrc, int len, Ipp32s* pMean, int scaleFactor)
#endif
  
  

  /// Computes the mean value of a ImgBase* ingroup MATH
  /** IPP-Optimized for icl32f and icl64f
      @param poImg input image
      @param iChannel channel index (-1 for all channels)
      @return mean value of image or image channel (optionally: roi)
  */
  std::vector<double> mean(const ImgBase *poImg, int iChannel=-1, bool roiOnly=false);

/* }}} */
  
  /* {{{ variance  */



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
    while(begin != end){
      d = *begin - mean;
      sum += d*d;
      ++begin;
    }
    return d/(empiricMean&&end-begin>1 ? end-begin - 1 : end-begin); 
  }

  /// Compute the variance of a given data range \ingroup MATH
  /** @param begin start ForwardIterator
      @param end end ForwardIterator
  */
  template <class ForwardIterator>
  inline double variance(ForwardIterator begin, ForwardIterator end){
    return variance(begin,end,mean(begin,end),true);
  }


  /// Compute the variance value of an image a with given mean \ingroup MATH
  /** @param poImg input imge
      @param mean vector with channel means
      @param empiricMean if true, sum of square distances is devidec by n-1 else by n
      @param iChannel channel index (-1 for all channels)
      @return The variance value form the vector
  */
  std::vector<double> variance(const ImgBase *poImg, const std::vector<double> &mean, bool empiricMean=true,  int iChannel=-1, bool roiOnly=false);
  
  /// Compute the variance value of an image a \ingroup MATH
  /** @param poImg input imge
      @param iChannel channel index (-1 for all channels)
      @return The variance value form the vector
      */
  std::vector<double> variance(const ImgBase *poImg, int iChannel=-1, bool roiOnly=false); 



/* }}} */

  /* {{{ standard-deviation  */

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

  /// Compute the std::deviation of an image
  /** @param poImage input image
      @param iChannel channel index (all channels if -1)
  */
  std::vector<double> stdDeviation(const ImgBase *poImage, int iChannel=-1, bool roiOnly = false);

  /// Compute the std::deviation of an image with given channel means
  /** @param poImage input image
      @param iChannel channel index (all channels if -1)
  */
  std::vector<double> stdDeviation(const ImgBase *poImage, const std::vector<double> mean, bool empiricMean=true, int iChannel=-1, bool roiOnly = false);


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

  /// Calculates mean and standard deviation of given image simultanously
  /** @param image input image
      @param iChannel image channel if < 0 all channels are used
      @return vector v of pairs p with p.first = mean and p.second = stdDev v[i] containing i-th channel's results
  */
  std::vector< std::pair<double,double> > meanAndStdDev(const ImgBase *image, int iChannel=-1, bool roiOnly = false);
  

/* }}} */

/* }}} */
                               
  /* {{{ histogramm functions */

  /// computes the color histogramm of given image channel                               
  std::vector<int> channelHisto(const ImgBase *image,int channel, int levels=256, bool roiOnly=false);
  
  /// computes the color histogramm of given image
  std::vector<std::vector<int> > hist(const ImgBase *image, int levels=256, bool roiOnly=false);

  /* }}} */                             
                               
} //namespace icl

#endif
