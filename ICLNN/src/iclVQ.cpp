/*
  VQ.cpp

  Written by: Michael Götting (2007)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "iclVQ.h"
#include "iclStackTimer.h"
#include "iclMathematics.h"
#include <iclCC.h>

using namespace std;
using namespace icl;

namespace icl {
  
  // {{{ Constructor/ Destructor

  template <typename T, template<typename> class U>
  VQ<T,U>::VQ(const ImgBase *poSrc, float fLearnRate) :
    m_fLearnRate(fLearnRate) {
    FUNCTION_LOG("");
    m_poData = new U<T>(poSrc);
    m_vecRefDataPtr = m_poData->getDataPtr();
    m_uiVQDim = m_poData->getDim();
    m_uiSrcDim = poSrc->getDim();
    m_bClusterIsInitialized = false;
    m_uiMaxTrainSteps = 3000;
  }

  template <typename T, template<typename> class U>
  VQ<T,U>::VQ(unsigned int uiVQDim, float fLearnRate) :
    m_fLearnRate(fLearnRate), m_uiVQDim(uiVQDim) {
    FUNCTION_LOG("");
    m_poData = new U<T>();
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

    // Prepare cluster info
    m_vecClusterInfo.resize(m_uiCenter);
  }
  
  template <typename T, template<typename> class U>
  void VQ<T,U>::resizeCluster(unsigned int uiCenter) {
    FUNCTION_LOG("");
    
    m_vecCluster.clear();
    m_vecClusterInfo.clear();    
    createCluster(uiCenter);
  }
  
  template <typename T, template<typename> class U>
  void VQ<T,U>::clearCluster() {
    FUNCTION_LOG("");
    
    for (unsigned int i=0;i<m_uiCenter;i++) {
      fill(m_vecCluster[i].begin(),m_vecCluster[i].end(),0);
    }
    
    m_vecClusterInfo.clear(); 
    m_vecClusterInfo.resize(m_uiCenter);
  }

  // }}}

  // {{{ cluster initilization methods 
  template <typename T, template<typename> class U>
  void VQ<T,U>::initClusterFromSrc(const ImgBase *poSrc,
                                   vqinitmode eMode,
                                   unsigned int uiStart) {
    FUNCTION_LOG("");

    // Variable initialisation
    static MathematicsRandomSeedInitializer initSeed;
    unsigned int uiRndPos;
    m_poData->setData(poSrc);
    m_vecRefDataPtr = m_poData->getDataPtr();
    m_uiSrcDim = poSrc->getDim();
    
    // Select initialisation mode
    switch(eMode) {
      case initRnd:
        for (unsigned int i=0;i<m_uiCenter;i++) {
          initVector (m_vecCluster[i], icl::random, 255.0);
        }
        break;
        
      case initRndFromData:
        for(unsigned int i=0;i<m_uiCenter;i++) {
          uiRndPos = random(m_uiSrcDim);
          cout << uiRndPos << endl;
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
  void VQ<T,U>::vq(const ImgBase *poSrc, unsigned int uiDataIdx) {
    // {{{ open

    FUNCTION_LOG("");
  
    // check pre start options
    ICLASSERT(m_bClusterIsInitialized);
    
    // Initialise data object
    m_poData->setData(poSrc);
    m_vecRefDataPtr = m_poData->getDataPtr();

    // variable initilization
    unsigned int uiMinDistIdx; // The min distance codeword to the source data
    float fErrBuf=0; // The minimal distance 
    
    // calc the euklidian distance to each codeword
    uiMinDistIdx = nn(uiDataIdx, fErrBuf);
    
    // update the codebook
    for (unsigned int i=0;i<m_uiVQDim;i++) {
      m_vecCluster[uiMinDistIdx][i] += m_fLearnRate * 
        (m_vecRefDataPtr[i][uiDataIdx] - m_vecCluster[uiMinDistIdx][i]) ;
    }
  }

// }}}

  template <typename T, template<typename> class U>
  void VQ<T,U>::lbg() {
    // {{{ open
    FUNCTION_LOG("");
    
    // check pre start options
    ICLASSERT(m_bClusterIsInitialized);
    
    // variable initilization
    unsigned int uiMinDistIdx; // The min distance codeword
    float fErrBuf=0;
    vector<unsigned int> vecClustCnt(m_uiCenter); //cluster size counters
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

  template <typename T, template<typename> class U>
  ImgBase* VQ<T,U>::wta(bool bInking, ImgBase **dstImg) {
    // {{{ open
    FUNCTION_LOG("");

    // variable deklaration
    ImgBase *oTmpImg;
    icl8u *dataPtr=0;
    float fMinDist = 0;
    int iPos, iWinnerCV = 0, iColorIdx=1;
    unsigned int uiSrcWidth = m_poData->m_poData->getWidth();
    unsigned int uiSrcHeight = m_poData->m_poData->getHeight();
    clearClusterInfo();
    
    // Ink WTA map
    if (bInking) { iColorIdx = 255 / (m_uiCenter-1); }
    
    // deklare dstImg
    if (!dstImg) {
      oTmpImg = imgNew(depth8u, m_poData->m_poData->getSize(), formatGray);
    } else {
      ensureCompatible(dstImg, depth8u, m_poData->m_poData->getSize(), 
                       formatGray);
      oTmpImg = *dstImg;
    }

    dataPtr = (icl8u*) oTmpImg->getDataPtr(0);

    // Compute WTA
    for(unsigned int y=0;y<uiSrcHeight;y++) {
      for(unsigned int x=0;x<uiSrcWidth;x++) {
        iPos = x+(y*uiSrcWidth);
        iWinnerCV = nn(iPos,fMinDist);
        dataPtr[iPos] = iWinnerCV * iColorIdx;
        
        m_vecClusterInfo[iWinnerCV].uiClSize++;
        m_vecClusterInfo[iWinnerCV].fCentroX += x;
        m_vecClusterInfo[iWinnerCV].fCentroY += y;
      }
    }

    // Compute Cluster info
    for (unsigned int i=0;i<m_uiCenter;i++) {
      if (m_vecClusterInfo[i].uiClSize == 0) {
        m_vecClusterInfo[i].fCentroX = 0;
        m_vecClusterInfo[i].fCentroY = 0;
      } else {
        m_vecClusterInfo[i].fCentroX /= m_vecClusterInfo[i].uiClSize;
        m_vecClusterInfo[i].fCentroY /= m_vecClusterInfo[i].uiClSize;
      }
    }
    
    // return image
    return oTmpImg;
  }
  
// }}}

  // }}}

  // {{{ helper functions 
  template <typename T, template<typename> class U>
  void VQ<T,U>::printCluster() {
    // {{{ open

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

  template <typename T, template<typename> class U>
  float VQ<T,U>::discriminantAnalysis(const ImgBase *clusterImg) {
    // {{{ open
    
    FUNCTION_LOG("");
    
    // variable deklaration
    ImgBase *grayImg = imgNew(depth8u, m_poData->m_poData->getSize(),
                              formatGray);
    vector<_ClProp> vecClProp(m_uiVQDim);
    vector<float> fMeanImg;
    vector<int> vecSelChannel;
    float fIntraClassVar=0, fInterClassVar=0;
    float fTmp;
    icl8u *clusterPtr = (icl8u*) clusterImg->getDataPtr(0);
    ImgBase *rgbImg2=0;
    
    // Compute gray src image
    vecSelChannel.push_back(0);
    vecSelChannel.push_back(1);
    vecSelChannel.push_back(2);
    const ImgBase *rgbImg = 
      m_poData->getSrcImg()->selectChannels(vecSelChannel);
    rgbImg2 = const_cast<ImgBase*>(rgbImg);
    rgbImg2->setFormat(formatRGB);
    rgbImg2->normalizeImg(Range<icl64f>(0,255));
    cc(rgbImg2, grayImg);
    icl8u *grayPtr = (icl8u*) grayImg->getDataPtr(0);
    
    // Compute cluster properties
    for (int i=0;i<clusterImg->getDim();i++) {
      vecClProp[clusterPtr[i]].pixSum += grayPtr[i];
      vecClProp[clusterPtr[i]].uiClSize++;  
    }
    
    for (unsigned int i=0;i<m_uiVQDim;i++) {
      vecClProp[i].fMean = vecClProp[i].pixSum / vecClProp[i].uiClSize;
    }

    fMeanImg = mean(grayImg);

    // Compute Intra class varianz
    for (int i=0;i<clusterImg->getDim();i++) {
      fTmp = grayPtr[i] - vecClProp[clusterPtr[i]].fMean;
      vecClProp[clusterPtr[i]].fIntraVar += fTmp * fTmp; 
    }
    
    for (unsigned int i=0;i<m_uiVQDim;i++) {
      vecClProp[i].fIntraVar = (vecClProp[i].fIntraVar / vecClProp[i].uiClSize);
      fIntraClassVar += vecClProp[i].fIntraVar;
    }
    
    fIntraClassVar = fIntraClassVar / m_uiVQDim;
    
    // Comuter Inter class varianz
    for (unsigned int i=0;i<m_uiVQDim;i++) {
      vecClProp[i].fInterVar = ((vecClProp[i].fMean-fMeanImg[0]) * 
                                (vecClProp[i].fMean-fMeanImg[0]));
      fInterClassVar += vecClProp[i].fInterVar;
    }
    
    fInterClassVar = fInterClassVar / m_uiVQDim;
    return (fInterClassVar / fIntraClassVar);
  }

// }}}
  
  template <typename T, template<typename> class U>
  void VQ<T,U>::clearClusterInfo() {
    // {{{ open

    for (unsigned int i=0;i<m_uiCenter;i++) {
      m_vecClusterInfo[i].uiClSize = 0;
      m_vecClusterInfo[i].fMean = 0;
      m_vecClusterInfo[i].fIntraVar = 0;
      m_vecClusterInfo[i].fInterVar = 0;
      m_vecClusterInfo[i].fCentroX = 0;
      m_vecClusterInfo[i].fCentroY = 0;
      m_vecClusterInfo[i].pixSum = 0;
    }
  }

// }}}
  
  // }}}

  template class VQ<icl8u, Interleaved>;
  template class VQ<icl16s, Interleaved>;
  template class VQ<icl32f, Interleaved>;

}
