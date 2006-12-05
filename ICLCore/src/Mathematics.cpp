/*
  Math.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include <Mathematics.h>

using namespace std;

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
  
  unsigned int randomi(unsigned int max){
    FUNCTION_LOG("unsigned int");
    float f = random(max+1);
    if(f==max+1){
      return static_cast<unsigned int>(max);
    }else{
      return static_cast<unsigned int>(std::floor(f));
    }
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

  // {{{ statistic functions

  // {{{ mean

  //--------------------------------------------------------------------------
  template <class T> 
  float mean(const T *pfData, int iDim)
  {
    FUNCTION_LOG("");
    if (iDim < 1) {return -1;}
    
    //---- iVariable initialisation ----
    float fSum = 0;
    
    //---- Compute mean value ----
    for(int i=0; i<iDim;i++) {
      fSum += *(pfData+i);   
    }
    
    //---- Return value ----
    return (fSum / iDim);
  }
  
  template float mean<unsigned char> (const unsigned char*, int);
  template float mean<int> (const int*, int);
  template float mean<float> (const float*, int);

  //--------------------------------------------------------------------------
  template <class T> 
  float mean(const vector<T> &vecData)
  {
    FUNCTION_LOG("");
    
    //---- Compute mean value ----
    return mean(&(vecData[0]), vecData.size());
  }
  
  template float mean<unsigned char> (const vector<unsigned char> &);
  template float mean<int> (const vector<int> &);
  template float mean<float> (const vector<float> &);

  //--------------------------------------------------------------------------
  template <class T> 
  vector<float> mean(const Img<T> *poImg, int iChannel)
  {
    FUNCTION_LOG("");
    
    //---- iVariable initialisation ----
    vector<float> vecMean;
    
    //---- Compute mean value ----
    if (iChannel < 0) {
      for(int i=0;i<poImg->getChannels();i++) {
        vecMean.push_back(mean((const T*) poImg->getDataPtr(i),
                               poImg->getDim()));
        SUBSECTION_LOG("Mean for channel " << i << " = " << vecMean[i]);
      }
    }
    else {
      vecMean.push_back(mean((const T*) poImg->getDataPtr(iChannel),
                             poImg->getDim()));
      SUBSECTION_LOG("Mean for channel " << iChannel << " = " << vecMean[0]);
    }
    
    //---- Return value ----
    return vecMean;
  }
  
  template vector<float> mean<icl8u>(const Img<icl8u>*, int);
  template vector<float> mean<icl32f>(const Img<icl32f>*, int);

  //--------------------------------------------------------------------------
  vector<float> mean(const ImgBase *poImg, int iChannel)
  {
    FUNCTION_LOG("");
    
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
    
    // Return
    return tmpVec;
  }

// }}}

  // {{{ variance

  //--------------------------------------------------------------------------
  template <class T> 
  float variance(const T *pfData, int iDim)
  {
    FUNCTION_LOG("");
    if (iDim < 1) { return -1;}

    //---- Variable initialisation ----
    float fSum = 0, fMean;
    
    //---- Compute variance ----
    fMean = mean(pfData, iDim);
    for(int i=0; i<iDim;i++) {
      fSum += (fMean - *(pfData+i)) * (fMean - *(pfData+i));  
    }
    
    //---- Return value ----
    return ( fSum / (iDim-1) );
  }
  
  template float variance<unsigned char> (const unsigned char*, int);
  template float variance<int> (const int*, int);
  template float variance<float> (const float*, int);
  
  //--------------------------------------------------------------------------
  template <class T> 
  float variance(const vector<T> &vecData)
  {
    FUNCTION_LOG("");
    
    // Compute variance
    return mean(&(vecData[0]), vecData.size());
  }

  template float variance<unsigned char> (const vector<unsigned char> &);
  template float variance<int> (const vector<int> &);
  template float variance<float> (const vector<float> &);

  //--------------------------------------------------------------------------
  template <class T> 
  vector<float> variance(const Img<T> *poImg, int iChannel)
  {
    FUNCTION_LOG("");
    
    //---- iVariable initialisation ----
    vector<float> vecVariance;
    
    //---- Compute variance value ----
    if (iChannel < 0) {
      for(int i=0;i<poImg->getChannels();i++) {
        vecVariance.push_back(variance((const T*) poImg->getDataPtr(i),
                                       poImg->getDim()));
        SUBSECTION_LOG("Variance for channel " << i << " = " << 
                       vecVariance[i]);
      }
    }
    else {
      vecVariance.push_back(variance((const T*) poImg->getDataPtr(iChannel),
                                     poImg->getDim()));
      SUBSECTION_LOG("Variance for channel " << iChannel << " = " << 
                     vecVariance[0]);
    }
    
    //---- Return value ----
    return vecVariance;
  }
  
  template vector<float> variance<icl8u>(const Img<icl8u>*, int);
  template vector<float> variance<icl32f>(const Img<icl32f>*, int);

  //--------------------------------------------------------------------------
  vector<float> variance(const ImgBase *poImg, int iChannel)
  {
    FUNCTION_LOG("");
    
    vector<float> vecVar;
    
    switch(poImg->getDepth())
    {
      case depth8u:
        vecVar = variance(poImg->asImg<icl8u>(), iChannel);
        break;
        
      case depth32f:
        vecVar = variance(poImg->asImg<icl32f>(), iChannel);
        break;
    }
    
    // Return
    return vecVar;
  }

// }}}
 
  // {{{ deviation

  //--------------------------------------------------------------------------
  template <class T> 
  float deviation(const T *pfData, int iDim)
  {
    FUNCTION_LOG("");
    if (iDim < 1) { return -1;}
    
    // Variable initialisation
    float fVariance = 0;
    
    // Compute variance
    fVariance = variance(pfData, iDim);

    // Return
    if (fVariance < 0) {return 0;}
    return sqrt(variance(pfData, iDim));
  }
  
  template float deviation<unsigned char> (const unsigned char*, int);
  template float deviation<int> (const int*, int);
  template float deviation<float> (const float*, int);

  //--------------------------------------------------------------------------
  template <class T> 
  float deviation(const vector<T> &vecData)
  {
    FUNCTION_LOG("");
    
    // Compute deviation
    return mean(&(vecData[0]), vecData.size());
  }

  template float deviation<unsigned char> (const vector<unsigned char> &);
  template float deviation<int> (const vector<int> &);
  template float deviation<float> (const vector<float> &);

  //--------------------------------------------------------------------------
  template <class T> 
  vector<float> deviation(const Img<T> *poImg, int iChannel)
  {
    FUNCTION_LOG("");
    
    //---- iVariable initialisation ----
    vector<float> vecDeviation;
    
    //---- Compute deviation value ----
    if (iChannel < 0) {
      for(int i=0;i<poImg->getChannels();i++) {
        vecDeviation.push_back(deviation((const T*) poImg->getDataPtr(i),
                                       poImg->getDim()));
        SUBSECTION_LOG("Deviation for channel " << i << " = " << 
                       vecDeviation[i]);
      }
    }
    else {
      vecDeviation.push_back(deviation((const T*) poImg->getDataPtr(iChannel),
                                     poImg->getDim()));
      SUBSECTION_LOG("Deviation for channel " << iChannel << " = " << 
                     vecDeviation[0]);
    }
    
    //---- Return value ----
    return vecDeviation;
  }
  
  template vector<float> deviation<icl8u>(const Img<icl8u>*, int);
  template vector<float> deviation<icl32f>(const Img<icl32f>*, int);

  //--------------------------------------------------------------------------
  vector<float> deviation(const ImgBase *poImg, int iChannel)
  {
    FUNCTION_LOG("");
    
    vector<float> vecVar;
    
    switch(poImg->getDepth())
    {
      case depth8u:
        vecVar = deviation(poImg->asImg<icl8u>(), iChannel);
        break;
        
      case depth32f:
        vecVar = deviation(poImg->asImg<icl32f>(), iChannel);
        break;
    }
    
    // Return
    return vecVar;
  }

// }}}

// }}}

} //namespace icl
