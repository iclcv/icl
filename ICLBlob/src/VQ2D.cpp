#include "VQ2D.h"
#include <math.h>

namespace icl{
  
  inline int rnd(int max){
    // {{{ open

    double d = drand48()*(max+1);
    if(d==max+1) d-=0.1;
    return (int)floor(d);
  }

  // }}}
  inline float distance(float *a, float *b){
    // {{{ open
    return sqrt( pow(a[0]-b[0],2)+pow(a[1]-b[1],2) );
  }
  // }}}
  inline int nn(float *data, int dim, float* point, float &dist){
    // {{{ open
    int bestIndex = 0;
    dist = distance(data,point);
    float currDist = -1;
    for(int i=1 ; i<dim ; ++i){
      currDist = distance(data+2*i,point); 
      if( currDist < dist){
        bestIndex = i;
        dist = currDist;
      }
    }
    return bestIndex;
  } 
  // }}}
  inline void copy_elem(float *src, int isrc, float *dst, int idst){
    // {{{ open

    dst[2*idst] = src[2*isrc];
    dst[2*idst+1] = src[2*isrc+1];
  }

  // }}}

  VQ2D::VQ2D(float *data, int dim, bool deepCopyData){
    // {{{ open

    m_poCenters = 0;
    m_poClusterInfo = 0;
    if(dim && data){
      m_poData = new VQVectorSet(data,dim, deepCopyData);
    }else{
      m_poData = new VQVectorSet();
    }
    m_poCenters = new VQVectorSet();
    m_poClusterInfo = new VQClusterInfo();
  }

  // }}}
  
  void VQ2D::setData(float *data, int dim, bool deepCopyData){
    // {{{ open
    ICLASSERT_RETURN( data );
    ICLASSERT_RETURN( dim );

    delete m_poData;
    m_poData = new VQVectorSet(data,dim,deepCopyData); 
  }

  // }}}

  const VQVectorSet &VQ2D::run(int k, int maxSteps, float mmqe, float &qe){
    // {{{ open

    // temporary variables
    int n = m_poData->dim();
    float *data = m_poData->data();
    m_poCenters->resize(k);
    float *centers = m_poCenters->data();
    
    // check if all parameters are valid!
    ICLASSERT_RETURN_VAL( k>=0 ,*m_poCenters);
    ICLASSERT_RETURN_VAL( n>=0 ,*m_poCenters);
        
    /**************************************************
    ** Random initialisation of the center vectors ****      >> TODO other strategy or
    ***************************************************        give them from anywhere else */
    for(int p=0 ; p<k ; ++p){
      copy_elem(data,rnd(n-1),centers,p);
    } 
    
    std::vector<float> vecx(k),vecy(k); // mean x/y accumulators
    std::vector<int> vecn(k); // cluster size counters

    for(int step = 0 ; step<maxSteps ; ++step){
      float errBuf(0);
      
      // clean mean-akkumulators and elem counter
      std::fill(vecx.begin(),vecx.end(),0);
      std::fill(vecy.begin(),vecy.end(),0);
      std::fill(vecn.begin(),vecn.end(),0);

      /*************************************************
      ** 1.Step calculating voronoi cells **************
      *************************************************/
      for(int i = 0 ; i<n; ++i){
        int iNN = nn(data,n,centers+2*i,errBuf);
        qe += errBuf;
        vecx[iNN] += data[2*i];
        vecy[iNN] += data[2*i+1];
        vecn[iNN]++;
      }
      
      // test if minimum quantisation error has already been reached
      if(qe/n <= mmqe) break;
      
      /*************************************************
      ** 2.Step updating centers to voronoi means ******
      *************************************************/
      for(int i = 0 ; i<k ; ++i){     
        if(!vecn[i]){
          centers[2*i]=centers[2*i+1]=-1;
        }
        else{
          centers[2*i]  = vecx[i]/vecn[i];
          centers[2*i+1]= vecy[i]/vecn[i];
        }     
      }
    }
    qe/=n;
    return *m_poCenters;
}


  // }}}

  const VQClusterInfo &VQ2D::run2(int centers, int steps, float mmqe,float &qe){
    // {{{ open
    return *m_poClusterInfo;
  }

  // }}}

}
