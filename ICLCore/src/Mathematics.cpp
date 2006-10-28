/*
  Math.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "Mathematics.h"

namespace icl{
  
  // {{{ Random functions: 

  //--------------------------------------------------------------------------
  void randomSeed()
  {
    FUNCTION_LOG("");
    srand48(time(NULL)); 
  } 

  //--------------------------------------------------------------------------
  void randomSeed(long int seedval)
  {
    FUNCTION_LOG("long int");
    srand48(seedval); 
  } 
  
  //--------------------------------------------------------------------------
  float random(float max) 
  {
    FUNCTION_LOG("float");
    return(max * drand48());
  } 
  
  //--------------------------------------------------------------------------
  float random(float min, float max)
  {
    FUNCTION_LOG("float, float");
    return((max - min) * drand48() + min); 
  } 
  
  //--------------------------------------------------------------------------
  float gaussRandom(float limit)
  {
    FUNCTION_LOG("float");
    static int iset=0;
    static float gset;
    float fac,r,v1,v2;
    
    if  (iset == 0) 
    {
      do 
      {
        v1=2*random(limit)-1;
        v2=2*random(limit)-1;
        r=v1*v1+v2*v2;      
      } while (r >= 1);
      
      fac=sqrt(-2*log(r)/r);
      gset=v1*fac;
      iset=1;
      
      return (v2*fac);
    } 
    else 
    {
      iset=0;
      return (gset);
    }
  } 
  
  //--------------------------------------------------------------------------
  void random(vector<float> &rndVec, float limit)
  {
    FUNCTION_LOG("vector<float> &, float");
    for (unsigned int i=0;i<rndVec.size();i++)
      rndVec[i] = random(limit);
  } 
  
  //--------------------------------------------------------------------------
  void gaussRandom(vector<float> &rndVec, float limit)
  {
    FUNCTION_LOG("vector<float> &, float");
    for (unsigned int i=0;i<rndVec.size();i++)
      rndVec[i] = gaussRandom(limit);  
  } 

  // }}}
  
  // {{{ Statistic functions
  //--------------------------------------------------------------------------
  vector<float> mean(ImgI *poImg, int iChannel)
  {
    FUNCTION_LOG("ImgI, int");
    
    vector<float> tmpVec;
    
    switch(poImg->getDepth())
    {
      case depth8u:
        tmpVec = mean(poImg->asImg<icl8u>(), iChannel);
        break;
        
      case depth32f:
        tmpVec = mean(poImg->asImg<icl32f>(), iChannel);
        break;
    }
    
    return tmpVec;
  }
  
  //--------------------------------------------------------------------------
  template <class T> 
  vector<float> mean(Img<T> *poImg, int iChannel)
  {
    FUNCTION_LOG("Img<T>, int");
    
    //---- iVariable initialisation ----
    float fSum = 0;
    vector<float> vecMean;
    
    //---- Compute mean value ----
    if (iChannel < 0) {
      for(int i=0;i<poImg->getChannels();i++) {
        ImgIterator<T> itImg = poImg->getIterator(i);
        
        for(; itImg.inRegion(); itImg++) {
          fSum += *itImg;   
        }
        vecMean.push_back(fSum / poImg->getDim());
        fSum = 0;
      }
    }
    else {

      ImgIterator<T> itImg = poImg->getIterator(iChannel);
      
      for(; itImg.inRegion(); itImg++) {
        fSum += *itImg;   
      }
      vecMean.push_back(fSum / poImg->getDim());
    }
    
    //---- Return value ----
    return vecMean;
  }
  
  template vector<float> mean<icl8u>(Img<icl8u>*, int);
  template vector<float> mean<icl32f>(Img<icl32f>*, int);
  
   //--------------------------------------------------------------------------
  vector<float> variance(ImgI *poImg, int iChannel)
  {
    FUNCTION_LOG("ImgI, int");
        
    vector<float> tmpVec;
    
    switch(poImg->getDepth())
    {
      case depth8u:
        tmpVec = variance(poImg->asImg<icl8u>(), iChannel);
        break;
        
      case depth32f:
        tmpVec = variance(poImg->asImg<icl32f>(), iChannel);
        break;
    }
    
    return tmpVec;
  }
  
  //--------------------------------------------------------------------------
  template <class T> 
  vector<float> variance(Img<T> *poImg, int iChannel)
  {
    FUNCTION_LOG("Img<T>, int");
    
    //---- Initialization ----
    float fTmp = 0;
    vector<float> vecVariance, vecMean;

    //---- Compute variance ----
    if (iChannel < 0) {
      for (int i=0;i<poImg->getChannels();i++) {
        vecMean = mean(poImg,i);
        
        //---- Compute variance ----
        ImgIterator<T> itImg = poImg->getIterator(i);
        
        for(; itImg.inRegion(); itImg++) {
          fTmp += (vecMean[0] - *itImg) * (vecMean[0] - *itImg);
        }
        
        if (poImg->getDim() > 1) {
          vecVariance.push_back(fTmp/(poImg->getDim()-1));
        }
        else {
          vecVariance.push_back(0);
        }
      }
    }
    else {
      vecMean = mean(poImg,iChannel);
      
      //---- Compute variance ----
      ImgIterator<T> itImg = poImg->getIterator(iChannel);
      
      for(; itImg.inRegion(); itImg++) {
        fTmp += (vecMean[0] - *itImg) * (vecMean[0] - *itImg);
      }
      
      if (poImg->getDim() > 1) {
        vecVariance.push_back(fTmp/(poImg->getDim()-1));
      }
      else {
        vecVariance.push_back(0);
      }
    }
    
    return vecVariance;
  }
  
  template vector<float> variance<icl8u>(Img<icl8u>*, int);
  template vector<float> variance<icl32f>(Img<icl32f>*, int);

  //--------------------------------------------------------------------------
  vector<float> stdDeviation(ImgI *poImg, int iChannel)
  {
    FUNCTION_LOG("ImgI, int");
        
    vector<float> tmpVec;
    
    switch(poImg->getDepth())
    {
      case depth8u:
        tmpVec = stdDeviation(poImg->asImg<icl8u>(), iChannel);
        break;
        
      case depth32f:
        tmpVec = stdDeviation(poImg->asImg<icl32f>(), iChannel);
        break;
    }
    
    return tmpVec;
  }

  //--------------------------------------------------------------------------
  template <class T> 
  vector<float> stdDeviation(Img<T> *poImg, int iChannel)
  {
    FUNCTION_LOG("Img<T>, int");
    
    //---- Initialization ----
    float fTmp = 0;
    vector<float> vecDeviation, vecMean;

    //---- Compute standard deviation ----
    if (iChannel < 0) {
      for (int i=0;i<poImg->getChannels();i++) {
        vecMean = mean(poImg,i);
        
        //---- Compute variance ----
        ImgIterator<T> itImg = poImg->getIterator(i);
        
        for(; itImg.inRegion(); itImg++) {
          fTmp += (vecMean[0] - *itImg) * (vecMean[0] - *itImg);
        }
        
        if (poImg->getDim() > 1) {
          vecDeviation.push_back(sqrt(fTmp/(poImg->getDim()-1)));
        }
        else {
          vecDeviation.push_back(0);
        }
      }
    }
    else {
      vecMean = mean(poImg,iChannel);
      
      //---- Compute variance ----
      ImgIterator<T> itImg = poImg->getIterator(iChannel);
      
      for(; itImg.inRegion(); itImg++) {
        fTmp += (vecMean[0] - *itImg) * (vecMean[0] - *itImg);
      }
      
      if (poImg->getDim() > 1) {
        vecDeviation.push_back(sqrt(fTmp/(poImg->getDim()-1)));
      }
      else {
        vecDeviation.push_back(0);
      }
    }
    
    return vecDeviation;
  }
  
  template vector<float> stdDeviation<icl8u>(Img<icl8u>*, int);
  template vector<float> stdDeviation<icl32f>(Img<icl32f>*, int);
  
// }}}
  
} //namespace icl
