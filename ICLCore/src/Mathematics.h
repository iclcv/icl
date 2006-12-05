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
  
  /* {{{ Random functions */

  //--------------------------------------------------------------------------
  /*!
    \brief Initilaize the random number generator. 
    \param -
    \return -
    \sa void randomSeed(long int)
  */
  void randomSeed();

  //--------------------------------------------------------------------------
  /*!
    \brief Initilaize the random number generator. 
    \param The seed value
    \return -
    \sa void randomSeed()
  */
  void randomSeed(long int seedval);
  
  struct MathematicsRandomSeedInitializer{
    inline MathematicsRandomSeedInitializer(){ randomSeed(); }
  };
  
  //--------------------------------------------------------------------------
  /*!
    \brief Generate a random number to an upper limit. 
    \param max a float argument. The upper limit for the returned number
    \return -
    \sa float gaussRandom(float), 
        void random(vector<float>, float),
        void random(float),
        void gaussRandom(vector<float>, float);
  */
  float random(float max);
  


  // creates a random integer number
  unsigned int randomi(unsigned int max);
  //--------------------------------------------------------------------------
  /*!
    \brief Generate a random number between an lower and upper limit. 
    \param min a float argument. The lower intervall limit
    \param max a float argument. The upper interval limit 
    \return -
    \sa float gaussRandom(float), 
        void random(vector<float>, float),
        float random(float),
        void gaussRandom(vector<float>, float);
  */
  float random(float min, float max);
  
  //--------------------------------------------------------------------------
  /*!
    \brief Generate a gaussian random number to an upper limit. 
    \param limit a float argument. The upper limit for the returned number
    \return -
    \sa float _random(float), 
        void random(vector<float>, float), 
        void gaussRandom(vector<float>, float);
  */
  float gaussRandom(float limit);

  //--------------------------------------------------------------------------
  /*!
    \brief Generate a i-dimensional random vector, with an upper limit for 
    each vector element.
    \param *rndVec a float argument. The destination vector.
    \param iDim an in argument. The dimension of the destination vector
    \param limit a float argument. The upper limit for each element
    \return -
    \sa float generate_random(float), 
        float generate_gauss_random(float), 
        void generate_gauss_random_vec(float*, int, float);
  */
  void random(std::vector<float> &rndVec, float limit);
  
  //--------------------------------------------------------------------------
  /*!
    \brief Generate a i-dimensional gaussian random vector, with an upper 
    limit for each vector element.
    \param *rndVec a float argument. The destination vector.
    \param iDim an in argument. The dimension of the destination vector
    \param limit a float argument. The upper limit for each element
    \return -
    \sa float random(float), 
        float gaussRandom(float), 
        void random(vector<float>, float);
  */
  void gaussRandom(std::vector<float> &rndVec, float limit);

/* }}} */
                              
  /* {{{ statistic functions */

  /* {{{ mean  */

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the mean value from a vector<T>.
    \param vecData The data vector
    \return The mean value form the vector
  */
  template <class T>
  float mean(const T *ptData, int iDim);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the mean value from a vector<T>.
    \param vecData The data vector
    \return The mean value form the vector
  */
  template <class T>
  float mean(const std::vector<T> &vecData);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the mean value from a vector<T>.
    \param vecData The data vector
    \return The mean value form the vector
  */
  template <class T>
  std::vector<float> mean(const Img<T> *poImg, int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the mean value from a vector<T>.
    \param vecData The data vector
    \return The mean value form the vector
  */
  std::vector<float> mean(const ImgBase *poImg, int iChannel=-1);

/* }}} */
  
  /* {{{ variance  */

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the variance value from a vector<T>.
    \param vecData The data vector
    \return The variance value form the vector
  */
  template <class T>
  float variance(const T *ptData, int iDim);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the variance value from a vector<T>.
    \param vecData The data vector
    \return The variance value form the vector
  */
  template <class T>
  float variance(const std::vector<T> &vecData);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the variance value from a vector<T>.
    \param vecData The data vector
    \return The variance value form the vector
  */
  template <class T>
  std::vector<float> variance(const Img<T> *poImg, int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the variance value from a vector<T>.
    \param vecData The data vector
    \return The variance value form the vector
  */
  std::vector<float> variance(const ImgBase *poImg, int iChannel=-1);

/* }}} */

  /* {{{ deviation  */

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the deviation value from a vector<T>.
    \param vecData The data vector
    \return The deviation value form the vector
  */
  template <class T>
  float deviation(const T *ptData, int iDim);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the deviation value from a vector<T>.
    \param vecData The data vector
    \return The deviation value form the vector
  */
  template <class T>
  float deviation(const std::vector<T> &vecData);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the deviation value from a vector<T>.
    \param vecData The data vector
    \return The deviation value form the vector
  */
  template <class T>
  std::vector<float> deviation(const Img<T> *poImg, int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the deviation value from a vector<T>.
    \param vecData The data vector
    \return The deviation value form the vector
  */
  std::vector<float> deviation(const ImgBase *poImg, int iChannel=-1);

/* }}} */

/* }}} */

} //namespace icl

#endif
