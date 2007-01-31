/*
  VQ.cpp

  Written by: Michael Götting (2007)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "VQ.h"
#include "Mathematics.h"
#include "StackTimer.h"

using namespace std;
using namespace icl;

namespace icl {
  
  // {{{ Constructor/ Destructor

  template <typename T, template<typename> class U>
  VQ<T,U>::VQ(ImgBase *poSrc, bool deepCopyData) {
    FUNCTION_LOG("");
    m_poData = new U<T>(poSrc, deepCopyData);
    m_vecRefDataPtr = m_poData->getDataPtr();
    m_uiVQDim = m_poData->getDim();
    m_uiSrcDim = poSrc->getDim();
    m_bClusterIsInitialized = false;
    m_uiMaxTrainSteps=3000;
  }

  template <typename T, template<typename> class U>
  VQ<T,U>::~VQ() {
    FUNCTION_LOG("");

  }

// }}}
  
  // {{{ cluster functions 
    template <typename T, template<typename> class U>
    void VQ<T,U>::createCluster(unsigned int uiCenter) {
    FUNCTION_LOG("");
    
    // store variables
    m_uiCenter = uiCenter;
    
    // allocate memory for cluster
    m_vecCluster.resize(m_uiCenter);
    for (unsigned int i=0;i<m_uiCenter;i++) {
      m_vecCluster[i].resize(m_uiVQDim);
    }
  }
  
  template <typename T, template<typename> class U>
  void VQ<T,U>::resizeCluster(unsigned int uiCenter) {
    FUNCTION_LOG("");
    
    m_vecCluster.clear();
    createCluster(uiCenter);
  }
  
  template <typename T, template<typename> class U>
  void VQ<T,U>::clearCluster() {
    FUNCTION_LOG("");
    
    for (unsigned int i=0;i<m_uiCenter;i++) {
      fill(m_vecCluster[i].begin(),m_vecCluster[i].end(),0);
    }
  }

  // }}}

  // {{{ cluster initilization methods 
  template <typename T, template<typename> class U>
  void VQ<T,U>::initClusterFromSrc(vqinitmode eMode,
                                   unsigned int uiStart) {
    BENCHMARK_THIS_FUNCTION
    FUNCTION_LOG("");
    
    unsigned int uiRndPos;
    
    switch(eMode) {
      case initRnd:
        for (unsigned int i=0;i<m_uiCenter;i++) {
          initVector (m_vecCluster[i], icl::random, 255.0);
        }
        break;
        
      case initRndFromData:
        for(unsigned int i=0;i<m_uiCenter;i++) {
          uiRndPos = random(m_uiSrcDim);
          for(unsigned int j=0;j<m_uiVQDim;j++) {
            m_vecCluster[i][j] = m_vecRefDataPtr[j][uiRndPos];
          }
        }
        break;
        
      case initSeqFromData:
        for (unsigned int i=0;m_uiCenter;i++) {
          for(unsigned int j=0;j<m_uiVQDim;j++,uiStart++) {
            m_vecCluster[i][j] = m_vecRefDataPtr[j][uiStart];
          }
        }
        break;
    }
    
    // Initilization finished sucessfully
    m_bClusterIsInitialized = true;
  }

  // }}}

  // {{{ cluster algorithms 
  template <typename T, template<typename> class U>
  void VQ<T,U>::vq(unsigned int uiDataIdx) {
    BENCHMARK_THIS_FUNCTION;
    FUNCTION_LOG("");
    
    // check pre start options
    ICLASSERT(m_bClusterIsInitialized);
    
    // variable initilization
    unsigned int uiMinDistIdx; // The min distance codeword to the source data
    float fErrBuf=0; // The minimal distance 
    
    // calc the euklidian distance to each codeword
    uiMinDistIdx = nn(uiDataIdx, fErrBuf);
    
    // update the codebook
    for (unsigned int i=0;i<m_uiVQDim;i++) {
      m_vecCluster[uiMinDistIdx][i] += 0.1 * 
        (m_vecRefDataPtr[i][uiDataIdx] -m_vecCluster[uiMinDistIdx][i]) ;
    }
  }
  
  template <typename T, template<typename> class U>
  void VQ<T,U>::lbg() {
    // {{{ open

    BENCHMARK_THIS_FUNCTION;
    FUNCTION_LOG("");
    
    // check pre start options
    ICLASSERT(m_bClusterIsInitialized);
    
    // variable initilization
    unsigned int uiMinDistIdx; // The min distance codeword
    float fErrBuf=0;
    vector<unsigned int> vecClustCnt(m_uiCenter); // cluster size counters
    vector<vector<float> > vecAccu(m_uiCenter); // data accumulators
    for (unsigned int i=0;i<m_uiCenter;i++) {
      vecAccu[i].resize(m_uiVQDim);
    }
    
    // start algorithm
    for (unsigned int uiStep=0;uiStep<m_uiMaxTrainSteps;uiStep++) {
      LOOP_LOG("TrainStep = " << uiStep);

      // clean akkumulators and element counter
      for (unsigned int i=0;i<vecAccu.size();i++) {
        fill(vecAccu[i].begin(),vecAccu[i].end(),0);
      }
      fill(vecClustCnt.begin(),vecClustCnt.end(),0);
      
      // compute voronoi cells
      for (unsigned int uiIdx=0;uiIdx<m_uiSrcDim;uiIdx++) {
        // calc the euklidian distance to each codeword
        uiMinDistIdx = nn(uiIdx, fErrBuf);
        //fQuantErr += fErrBuf;
        //cout << "Quantization error = " << fQuantErr << endl;
        
        for (unsigned int i=0;i<m_uiVQDim;i++) {
          vecAccu[uiMinDistIdx][i] += m_vecRefDataPtr[i][uiIdx];
        }
        vecClustCnt[uiMinDistIdx]++;
        
        // ToDo: test if minimum quantization error has already been reached
      }

      // update the codebook
      for (unsigned int j=0;j<m_uiCenter;j++) {
        for (unsigned int i=0;i<m_uiVQDim;i++) {
          m_vecCluster[j][i] = (1/(float)vecClustCnt[j]) * vecAccu[j][i];
        }
      }
    }
  }

// }}}

  // }}}

  // {{{ classification algorithms 
  template <typename T, template<typename> class U>
  unsigned int VQ<T,U>::nn(unsigned int uiDataIdx, float &fMinDist) {
    // {{{ open

    BENCHMARK_THIS_FUNCTION;
    FUNCTION_LOG("");
    
    // variable deklaration
    unsigned int uiMinDistIdx=0;
    float tmp=0;
    fMinDist=numeric_limits<float>::max();
    
    // calc the euklidian distance to each codeword
    LOOP_LOG("Center -> Distance");
    for (unsigned int i=0;i<m_uiCenter;i++) {
        for (unsigned int j=0;j<m_uiVQDim;j++) {
          tmp += ((m_vecRefDataPtr[j][uiDataIdx] - m_vecCluster[i][j]) * 
                  (m_vecRefDataPtr[j][uiDataIdx] - m_vecCluster[i][j]));
        }
        tmp = sqrt(tmp);
        LOOP_LOG(i<<" -> "<<tmp);
        if (tmp < fMinDist) {
          uiMinDistIdx = i;
          fMinDist = tmp;
        }
        tmp = 0;
    }
    LOOP_LOG("MinDistIndex = "<<m_uiMinDistIdx<<"  /  Distance"<<fMinDist);
    return uiMinDistIdx;
  }

// }}}

  // }}}

  // {{{ helper functions 
  template <typename T, template<typename> class U>
  void VQ<T,U>::printCluster() {
    BENCHMARK_THIS_FUNCTION;
    FUNCTION_LOG("");

    // calc center with minimal distance
    printf("Cluster data\n");
    printf("------------\n");
    for (unsigned int i=0;i<m_uiCenter;i++) {
      for (unsigned int j=0;j<m_uiVQDim;j++) {
        printf("%4.15f  ",(icl64f) m_vecCluster[i][j]);
      }
      printf("\n");
    }
    printf("\n\n");
  }

  // }}}

  template class VQ<icl8u, Interleaved>;
  template class VQ<icl16s, Interleaved>;
  template class VQ<icl32f, Interleaved>;

}
