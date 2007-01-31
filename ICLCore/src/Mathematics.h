/*
  Math.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICL_MATH_H
#define ICL_MATH_H

#include <Img.h>
#include <ImgIterator.h>
#include <vector>
#include <cmath>

namespace icl {
  
  /* {{{ random functions */

  //--------------------------------------------------------------------------
  /*!
    @brief Initilaize the random number generator. 
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
    @brief Initilaize the random number generator. 
    @sa void randomSeed(long int)
  */
  inline void randomSeed() {randomSeed(Time::now().toMicroSeconds());}

  
  struct MathematicsRandomSeedInitializer{
    inline MathematicsRandomSeedInitializer(){ randomSeed(); }
  };

  //--------------------------------------------------------------------------
  /*!
    @brief Generates random numbers between 0 and 1. 
    @param -
    @return random number
    @sa double random()
  */
  inline double random() {
#ifdef WIN32
     // this generates quite poor random numbers, because we RAND_MAX is 32767 only
     return static_cast<double>(rand()) / (1.0 + static_cast<double>(RAND_MAX));
#else
     return drand48();
#endif
  }
  
  //--------------------------------------------------------------------------
  /*!
    @brief Generate a random number to an upper limit. 
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
    @brief Generate a random number between a lower and upper limit. 
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
    @brief Creates a non-negative random number to an upper limit max
    @param max The upper limit for the returned number
    @return The random number
  */
  inline unsigned int random(unsigned int max) {
    FUNCTION_LOG("unsigned int");
    unsigned int val = static_cast<unsigned int>(floor(random (static_cast<double>(max)+1.0)));
    return std::min(val, max);
  }
   
  //--------------------------------------------------------------------------
  /*!
    @brief Generate a gaussian random number to an upper limit. 
    @param limit a float argument. The upper limit for the returned number
    @return -
    @sa float random(float), 
        void random(vector<float>, float), 
        void gaussRandom(vector<float>, float);
  */
  double gaussRandom(double limit);

  //--------------------------------------------------------------------------
  /*!
    @brief Initialize the elements of a std::vector by values of
    an generator function taking no arguments, e.g. random()
  */
  template <class T>
  void initVector(std::vector<T> &vec, double (*gen)()) {
     FUNCTION_LOG("vector<T> &, Generator");
     std::generate (vec.begin(), vec.end(), gen);
  }
  /*!
    @brief Initialize the elements of a std::vector by values of
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
    @brief Calculate the euclidian distance of points a and b
    @param a The first point
    @param b The second point
    @return The distance of point a and b
  */
  template <class ForwardIterator> 
  float euclidian(ForwardIterator first, ForwardIterator last,
                  ForwardIterator second) {
     float fSum = 0.0, fDiff;
     for (; first != last; ++first, ++second) {
        fDiff = (*second-*first);
        fSum += fDiff*fDiff;
     }
     return sqrt(fSum);
  }

  /*!
    @brief Calculate the euclidian distance of points a and b
    @param a The first point
    @param b The second point
    @return The distance of point a and b
  */
  template <class T>
     float euclidian(const std::vector<T> &a, const std::vector<T> &b) {
     ICLASSERT (a.size() == b.size());
     return euclidian (a.begin(), a.end(), b.begin());
  }
     
/* }}} */
                              
  /* {{{ statistic functions */

  /* {{{ mean  */

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the mean value from a vector<T>.
    @param iDim an in argument. The dimension of the destination vector
    @param ptData The data vector
    @return The mean value form the vector
  */
  template <class T>
  float mean(const T *ptData, int iDim);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the mean value from a vector<T>.
    @param vecData The data vector
    @return The mean value form the vector
  */
  template <class T>
  float mean(const std::vector<T> &vecData);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the mean value from a vector<T>.
    @param poImg The data Image
    @param iChannel The number of channels
    @return The mean value form the vector
  */
  template <class T>
  std::vector<float> mean(const Img<T> *poImg, int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the mean value from a vector<T>.
    @param poImg The data Image
    @param iChannel The number of channels
    @return The mean value form the vector
  */
  std::vector<float> mean(const ImgBase *poImg, int iChannel=-1);

/* }}} */
  
  /* {{{ variance  */

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the variance value from a vector<T>.
    @param ptData The data vector
    @param iDim an in argument. The dimension of the destination vector
    @return The variance value form the vector
  */
  template <class T>
  float variance(const T *ptData, int iDim);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the variance value from a vector<T>.
    @param vecData The data vector
    @return The variance value form the vector
  */
  template <class T>
  float variance(const std::vector<T> &vecData);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the variance value from a vector<T>.
    @param poImg The data vector
    @param iChannel The number of channels
    @return The variance value form the vector
  */
  template <class T>
  std::vector<float> variance(const Img<T> *poImg, int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the variance value from a vector<T>.
    @param poImg The data vector
    @param iChannel The number of channels
    @return The variance value form the vector
  */
  std::vector<float> variance(const ImgBase *poImg, int iChannel=-1);

/* }}} */

  /* {{{ deviation  */

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the deviation value from a vector<T>.
    @param ptData The data vector
    @param iDim an in argument. The dimension of the destination vector
    @return The deviation value form the vector
  */
  template <class T>
  float deviation(const T *ptData, int iDim);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the deviation value from a vector<T>.
    @param vecData The data vector
    @return The deviation value form the vector
  */
  template <class T>
  float deviation(const std::vector<T> &vecData);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the deviation value from a vector<T>.
    @param poImg The data vector
    @param iChannel The number of channels
    @return The deviation value form the vector
  */
  template <class T>
  std::vector<float> deviation(const Img<T> *poImg, int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    @brief Compute the deviation value from a vector<T>.
    @param poImg The data vector
    @param iChannel The number of channels
    @return The deviation value form the vector
  */
  std::vector<float> deviation(const ImgBase *poImg, int iChannel=-1);

/* }}} */

/* }}} */

} //namespace icl

#endif
