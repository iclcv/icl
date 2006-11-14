/*
  Math.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICL_MATH_H
#define ICL_MATH_H

#include "Img.h"
#include "ImgIterator.h"
#include <cmath>

using namespace std;

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
  void random(vector<float> &rndVec, float limit);
  
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
  void gaussRandom(vector<float> &rndVec, float limit);

  /* }}} */
          
  /* {{{ statistic functions */
  //--------------------------------------------------------------------------
  /*!
    \brief Compute the variance of the vector
    \param poImg The input img
    \param iChannel The channel to calculate the variance from    
    \return The variance from the image
  */
  vector<float> variance(ImgBase *poImg , int iChannel=-1);
  
  //--------------------------------------------------------------------------
  /*!
    \brief Compute the variance of the vector
    \param poImg The input img
    \param iChannel The channel to calculate the variance from    
    \return The variance from the image
  */
  template <class T>
  vector<float> variance(Img<T> *poImg , int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the standard deviation of the image
    \param poImg The input img
    \param iChannel The channel to calculate the standard deviation from    
    \return The standard deviation from the image
  */
  vector<float> stdDeviation(ImgBase *poImg , int iChannel=-1);
  
  //--------------------------------------------------------------------------
  /*!
    \brief Compute the standard deviation of the image
    \param poImg The input img
    \param iChannel The channel to calculate the standard deviation from    
    \return The standard deviation from the image
  */
  template <class T>
  vector<float> stdDeviation(Img<T> *poImg , int iChannel=-1);
  
  //--------------------------------------------------------------------------
  /*!
    \brief Compute the mean value from image.
    \param poImg The input img
    \param iChannel The channel to calculate the variance from
    \return The mean value form the image
  */
  vector<float> mean(ImgBase *poImg, int iChannel=-1);

  //--------------------------------------------------------------------------
  /*!
    \brief Compute the mean value from image.
    \param poImg The input img
    \param iChannel The channel to calculate the variance from
    \return The mean value form the image
  */
  template <class T>
  vector<float> mean(Img<T> *poImg, int iChannel=-1);




/* }}} */

} //namespace icl

#endif
