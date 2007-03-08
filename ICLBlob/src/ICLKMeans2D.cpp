#include "ICLKMeans2D.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "ICLMacros.h"

namespace icl{

  // {{{ typedefs
  typedef KMeans2D::vectorset vectorset;
  typedef KMeans2D::intvec intvec;
  typedef KMeans2D::distfunc distfunc;
  // }}}

  // {{{ utility functions

  // remove the n trailing elements of x
  inline void popFromVectorSet(vectorset &x, int n){
    // {{{ open
    int c = (int)x.size();
    for(int i=c-1-n;i<c;++i){
      delete [] x[i];
    }
    x.resize(c-n);
  }
  // }}}
  
  // adds n new elements to a vectorset
  inline void pushToVectorSet(vectorset &x, int n){
    // {{{ open
    for(int i=0;i<n;i++){
      float *pf = new float[2];
      pf[0]=pf[1]=0;
      x.push_back(pf);
    }
  }
  // }}}
  
  // creates a uniform distributed integer value in range [0,max]
  inline int rnd(int max){
    // {{{ open

    double d = random()*(max+1);
    if(d==max+1) d-=0.1;
    return (int)floor(d);
  }

  // }}}
  
  // deep copy of float* from src to dst vectorset
  inline void copyElem(const vectorset &src, int srcIndex, vectorset &dst, int dstIndex){
    // {{{ open

    (dst[dstIndex])[0] = (src[srcIndex])[0];
    (dst[dstIndex])[1] = (src[srcIndex])[1];
  }

  // }}}
  
  // calculates the nearest neighbor index in a given vectorset src to a point p
  inline int nn(const vectorset &src, float* p, distfunc f, float &dist){
    // {{{ open

    int bestIndex = 0;
    dist = f(src[0],p);
    float currDist = -1;
    for( unsigned int i=1 ; i<src.size() ; ++i){
      currDist = f(src[i],p); 
      if( currDist < dist){
        bestIndex = i;
        dist = currDist;
      }
    }
    return bestIndex;
  }

  // }}}

  // }}}
  
  // default distance function (euclidian norm) (STATIC int KMeans2D)
  float KMeans2D::euclNorm(float *a, float *b){
    // {{{ open
    return sqrt( pow(a[0]-b[0],2)+pow(a[1]-b[1],2) );
  }
  // }}}
  
  KMeans2D::KMeans2D(int k, distfunc f):m_fError(-1),m_funcDist(f){
    // {{{ open

    randomSeed();
    setK(k);
  }

  // }}}
  
  void KMeans2D::add(float x,float y){
    // {{{ open

    float *pf = new float[2];
    pf[0]=x;
    pf[1]=y;
    m_vecData.push_back(pf);
  }

  // }}}
  
  void KMeans2D::run(int maxSteps, float minQuantisationError, const vectorset &prototypes){
    // {{{ open

    // temporary variables
    int k = getK();
    int n = m_vecData.size();
    vectorset &D = m_vecData;
    vectorset &C = m_vecCBV;
    
    // check if all parameters are valid!
    ICLASSERT_RETURN( k>=0 );
    ICLASSERT_RETURN( n>=0 );
    
    if(prototypes.size() == 0){
      // initialize (search k random prototype vectors from the data)
      for(int p=0 ; p<k ; ++p){
        int index = rnd(n-1);
        copyElem(D,index,C,p);
      } 
    }else if( (int)prototypes.size() == k){
      // use the given prototypes 
      popFromVectorSet(m_vecCBV,m_vecCBV.size());
      m_vecCBV = prototypes;
    }else{
      ERROR_LOG("noting to do with vectorset of size " << prototypes.size() << " k is " << k);
    }
    
    float fQE; // qauantisation error
    int iNN;   // index of neares prototype
    float *pfx = new float[k];  // mean x accumulators
    float *pfy = new float[k];  // mean y accumulators
    int  *pin = new int[k]; // counter of cluster elements

    for(int step = 0 ; step<maxSteps ; ++step){
      m_fError = 0;
      
      // clean mean-akkumulators and elem counter
      for(int i=0;i<k;i++){
        pfx[i]=pfy[i]=pin[i]=0;
      }
      
      // calculating new means for all clusters
      for(int i = 0 ; i<n; ++i){
        iNN = nn(C,D[i],m_funcDist,fQE);
        m_fError += fQE;
        pfx[iNN] += (D[i])[0];
        pfy[iNN] += (D[i])[1];
        pin[iNN]++;
      }
      
      // test if minimum quantisation error has already been reached
      if(m_fError/n <= minQuantisationError){
        delete [] pfx;    
        delete [] pfy;    
        delete [] pin;
        m_fError/=n;
        return;
      }
      
      // move all prototypes to their voronoi-mean
      for(int i = 0 ; i<k ; ++i){
        if(!pin[i]){
          (C[i])[0] = -1;
          (C[i])[1] = -1;
        }else{
          (C[i])[0] = pfx[i]/pin[i];
          (C[i])[1] = pfy[i]/pin[i];
        }
      }       
    }
    m_fError/=n;
    delete [] pfx;
    delete [] pfy; 
    delete [] pin;
  }
  // }}}
  
  void KMeans2D::setK(int k){
    // {{{ open
    int c = getK();
    if(c > k) popFromVectorSet(m_vecCBV,c-k);
    else if (k > c) pushToVectorSet(m_vecCBV,k-c);
  }
  // }}}

  void KMeans2D::clear(){
    // {{{ open
    popFromVectorSet(m_vecData,(int)m_vecData.size());
    m_fError = -1;
  }
  // }}}
  
  // {{{ getter functions

  int KMeans2D::getK() const{
    // {{{ open
    return (int)m_vecCBV.size();
  }

  // }}}
  float KMeans2D::getError() const{
    // {{{ open
    return m_fError;
  }
  // }}}
  const vectorset &KMeans2D::getPrototypes() const{
    // {{{ open
    return m_vecCBV;
  }
  // }}}

  // }}}

}
