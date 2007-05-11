#ifndef VQ_H
#define VQ_H

/*
  VQ.h

  Written by: Michael Götting and Christof Elbrechter (2007)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

#include "iclImg.h"
#include "iclInterleaved.h"
#include <limits>

namespace icl {

enum vqinitmode {
  initRnd = 0,
  initRndFromData,
  initSeqFromData
};

struct _ClProp {
  unsigned int uiClSize;
  float fMean;
  float fIntraVar;
  float fInterVar;
  float fCentroX;
  float fCentroY;
  float pixSum;
};
 
template <typename T, template<typename> class U>
class VQ : public Img<T> {
 public:
  // Constructor/ Destructor
  VQ(const ImgBase *pSrc, float fLearnRate=0.01);
  VQ(unsigned int uiVQDim, float fLearnRate=0.01);
  virtual ~VQ();
  
  // operator
  T* operator[](int i) { return m_vecRefDataPtr[i]; }
  
  // Variable deklaration
  U<T> *m_poData; /// The abstract information orientation
  
  // Variable deklaration for the reference data
  std::vector<T*> m_vecRefDataPtr; ///> The first element of each data set
  unsigned int m_uiSrcDim; ///> The dimension of the src data (w*h)
  
  // Variable deklaration for the VQ cluster
  float m_fLearnRate; ///> The learning rate of the VQ
  std::vector<std::vector<icl64f> > m_vecCluster; ///> The VQ cluster data
  std::vector<_ClProp> m_vecClusterInfo;
  unsigned int m_uiVQDim; ///> The cluster vector dimension
  unsigned int m_uiCenter; ///> The number of VQ centers
  unsigned int m_uiMaxTrainSteps; ///> The maximum training steps
  bool m_bClusterIsInitialized, m_bDeepCopyData; 

  // Set functions
  void setLearnRate(float fLearnRate) { m_fLearnRate = fLearnRate; }

  // Get functions
  float getLearnRate() { return m_fLearnRate; }
  std::vector<_ClProp> getClusterInfo() { return m_vecClusterInfo; }
  
  // cluster functions
  void createCluster(unsigned int uiCenter);
  void resizeCluster(unsigned int uiCenter);
  void clearCluster();

  // cluster initialization methods
  void initClusterFromSrc(const ImgBase *poSrc,
                          vqinitmode eMode, 
                          unsigned int uiStart=0);
  
  // cluster algorithms
  void vq(const ImgBase *poSrc, unsigned int uiDataIdx);
  void lbg();

  // distance algorithms
  unsigned int nn(unsigned int uiDataIdx, float &fMinDist);
  ImgBase* wta(bool bInking=false, ImgBase **dstImg=0);
  
  // helper functions
  void printCluster();
  float discriminantAnalysis(const ImgBase* clusterImg);
  void clearClusterInfo();
  
}; // class VQ
 

} // namespace

#endif
