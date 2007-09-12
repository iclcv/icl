#ifndef ICL_MATH_H
#define ICL_MATH_H

#include <iclImg.h>
#include <iclImgIterator.h>
#include <vector>
#include <cmath>
/*
  Math.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/



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
    return std::min(val, max);
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
  void initVector(std::vector<T> &vec, double (*gen)()) {
     FUNCTION_LOG("vector<T> &, Generator");
     std::generate (vec.begin(), vec.end(), gen);
  }

  /*!
    @brief Initialize the elements of a std::vector by values of \ingroup MATH
    an generator function taking one argument, e.g. randomGauss(max)
  */
  template <class T>
  void initVector(std::vector<T> &vec, double (*f)(double), double arg) {
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
  float euclidian(ForwardIterator v1Begin, ForwardIterator v1End,
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
  float euclidian(const std::vector<T> &a, const std::vector<T> &b) {
     ICLASSERT_RETURN_VAL(a.size() == b.size(), float(0));
     return euclidian (a.begin(), a.end(), b.begin());
  }
     
/* }}} */
                              
  /* {{{ statistic functions */

  /* {{{ mean  */

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the mean value from a vector<T>. \ingroup MATH
    @param iDim an in argument. The dimension of the destination vector
    @param ptData The data vector
    @return The mean value form the vector
  */
  template <class T>
  float mean(const T *ptData, int iDim);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the mean value from a vector<T>. \ingroup MATH
    @param vecData The data vector
    @return The mean value form the vector
  */
  template <class T>
  float mean(const std::vector<T> &vecData);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the mean value from a vector<T>. \ingroup MATH
    @param poImg The data Image
    @param iChannel The number of channels
    @return The mean value form the vector
  */
  template <class T>
  std::vector<float> mean(const Img<T> *poImg, int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the mean value from a vector<T>. \ingroup MATH
    @param poImg The data Image
    @param iChannel The number of channels
    @return The mean value form the vector
  */
  std::vector<float> mean(const ImgBase *poImg, int iChannel=-1);

/* }}} */
  
  /* {{{ variance  */

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the variance value from a vector<T>. \ingroup MATH
    @param ptData The data vector
    @param iDim an in argument. The dimension of the destination vector
    @return The variance value form the vector
  */
  template <class T>
  float variance(const T *ptData, int iDim);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the variance value from a vector<T>. \ingroup MATH
    @param vecData The data vector
    @return The variance value form the vector
  */
  template <class T>
  float variance(const std::vector<T> &vecData);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the variance value from a vector<T>. \ingroup MATH
    @param poImg The data vector
    @param iChannel The number of channels
    @return The variance value form the vector
  */
  template <class T>
  std::vector<float> variance(const Img<T> *poImg, int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the variance value from a vector<T>. \ingroup MATH
    @param poImg The data vector
    @param iChannel The number of channels
    @return The variance value form the vector
  */
  std::vector<float> variance(const ImgBase *poImg, int iChannel=-1);

/* }}} */

  /* {{{ deviation  */

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the deviation value from a vector<T>. \ingroup MATH
    @param ptData The data vector
    @param iDim an in argument. The dimension of the destination vector
    @return The deviation value form the vector
  */
  template <class T>
  float deviation(const T *ptData, int iDim);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the deviation value from a vector<T>. \ingroup MATH
    @param vecData The data vector
    @return The deviation value form the vector
  */
  template <class T>
  float deviation(const std::vector<T> &vecData);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the deviation value from a vector<T>. \ingroup MATH
    @param poImg The data vector
    @param iChannel The number of channels
    @return The deviation value form the vector
  */
  template <class T>
  std::vector<float> deviation(const Img<T> *poImg, int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the deviation value from a vector<T>. \ingroup MATH
    @param poImg The data vector
    @param iChannel The number of channels
    @return The deviation value form the vector
  */
  std::vector<float> deviation(const ImgBase *poImg, int iChannel=-1);

/* }}} */

/* }}} */

} //namespace icl

#endif
