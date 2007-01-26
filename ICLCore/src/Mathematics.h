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
#include <cmath>

namespace icl {
  
  /* {{{ random functions */

  //--------------------------------------------------------------------------
  /*!
    @brief Initilaize the random number generator. 
    @param -
    @return -
    @sa void randomSeed(long int)
  */
  void randomSeed();

  //--------------------------------------------------------------------------
  /*!
    @brief Initilaize the random number generator. 
    @param seedval The seed value
    @return -
    @sa void randomSeed()
  */
  void randomSeed(long int seedval);
  
  struct MathematicsRandomSeedInitializer{
    inline MathematicsRandomSeedInitializer(){ randomSeed(); }
  };
  
  //--------------------------------------------------------------------------
  /*!
    @brief Generate a random number to an upper limit. 
    @param max a float argument. The upper limit for the returned number
    @return -
    @sa float gaussRandom(float), 
        void random(vector<float>, float),
        void random(float),
        void gaussRandom(vector<float>, float);
  */
  inline float random(float max) {
    FUNCTION_LOG("float");
    return(max * drand48());
  }

  //--------------------------------------------------------------------------
  /*!
    @brief Creates a non negativ random number to an upper limit max
    @param max The upper limit for the returned number
    @return The random number
  */
  inline unsigned int randomi(unsigned int max) {
    FUNCTION_LOG("unsigned int");
    float f = random(max+1);
    if(f == max+1){
      return static_cast<unsigned int>(max);
    }else{
      return static_cast<unsigned int>(floor(f));
    }
  }
  
  //--------------------------------------------------------------------------
  /*!
    @brief Generate a random number between an lower and upper limit. 
    @param min a float argument. The lower intervall limit
    @param max a float argument. The upper interval limit 
    @return -
    @sa float gaussRandom(float), 
        void random(vector<float>, float),
        float random(float),
        void gaussRandom(vector<float>, float);
  */
  inline float random(float min, float max) {
    FUNCTION_LOG("float, float");
    return((max - min) * drand48() + min); 
  }
  
  //--------------------------------------------------------------------------
  /*!
    @brief Generate a gaussian random number to an upper limit. 
    @param limit a float argument. The upper limit for the returned number
    @return -
    @sa float _random(float), 
        void random(vector<float>, float), 
        void gaussRandom(vector<float>, float);
  */
  float gaussRandom(float limit);

  //--------------------------------------------------------------------------
  /*!
    @brief Generate a i-dimensional random vector, with an upper limit for 
    each vector element.
    @param *rndVec a float argument. The destination vector.
    @param limit a float argument. The upper limit for each element
    @return -
    @sa float generate_random(float), 
        float generate_gauss_random(float), 
        void generate_gauss_random_vec(float*, int, float);
  */
  template <class T>
  void random(std::vector<T> &rndVec, T limit) {
    FUNCTION_LOG("vector<T> &, T");
    for (unsigned int i=0;i<rndVec.size();i++) {
      rndVec[i] = static_cast<T>(random(static_cast<float>(limit)));
    }
  }

  //--------------------------------------------------------------------------
  /*!
    @brief Generate a i-dimensional gaussian random vector, with an upper 
    limit for each vector element.
    @param *rndVec a float argument. The destination vector.
    @param limit a float argument. The upper limit for each element
    @return -
    @sa float random(float), 
        float gaussRandom(float), 
        void random(vector<float>, float);
  */
  template <class T>
  void gaussRandom(std::vector<T> &rndVec, T limit) {
    FUNCTION_LOG("vector<T> &, T");
    for (unsigned int i=0;i<rndVec.size();i++)
      rndVec[i] = static_cast<T>(gaussRandom(static_cast<float>(limit)));  
  }

/* }}} */
                              
  /* {{{ distance functions */

  /*!
    @brief Calculate the eucledean distance of point a and b
    @param a The first 2D point
    @param b The second 2D point
    @return The distance of point a and b
  */
  template <class T> 
  float euclidian(const std::vector<T> &a, const std::vector<T> &b);
    
  /*!
    @brief Calculate the eucledean distance of point a and b
    @param a The first 2D point
    @param b The second 2D point
    @param iDim The dimension of a, b
    @return The distance of point a and b
  */
  template <class T>
  float euclidian(const T *a, const T *b, unsigned int iDim);

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
