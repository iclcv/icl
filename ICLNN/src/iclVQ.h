#include "iclImg.h"
#include "iclInterleaved.h"
#include <limits>
 /*
  VQ.h

  Written by: Michael Götting and Christof Elbrechter (2007)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

#ifndef VQ_H
#define VQ_H


namespace icl {

enum vqinitmode {
  initRnd = 0,
  initRndFromData,
  initSeqFromData
};

template <typename T, template<typename> class U>
class VQ : public Img<T> {
 public:
  // Constructor/ Destructor
  VQ(ImgBase *pSrc, float fLearnRate=0.01, bool deepCopyData=false);
  virtual ~VQ();
  
  // operator
  T* operator[](int i) { return m_vecRefDataPtr[i]; }
  
  // Variable deklaration
  U<T> *m_poData; /// The abstract information orientation

  // Variable deklaration for the reference data
  std::vector<T*> m_vecRefDataPtr; /// The first element of each data set
  unsigned int m_uiSrcDim; /// The dimension of the src data (w*h)
  
  // Variable deklaration for the VQ cluster
  std::vector<std::vector<icl64f> > m_vecCluster; /// The VQ cluster data
  unsigned int m_uiVQDim; /// The cluster vector dimension
  unsigned int m_uiCenter; /// The number of VQ centers
  unsigned int m_uiMaxTrainSteps; /// The maximum training steps
  float m_fLearnRate; /// The learning rate of the VQ
  bool m_bClusterIsInitialized; 
  
  // Set functions
  void setLearnRate(float fLearnRate) { m_fLearnRate = fLearnRate; }

  // Get functions
  float getLearnRate() { return m_fLearnRate; }
  
  // cluster functions
  void createCluster(unsigned int uiCenter);
  void resizeCluster(unsigned int uiCenter);
  void clearCluster();

  // cluster initialization methods
  void initClusterFromSrc(vqinitmode eMode, unsigned int uiStart=0);
  
  // cluster algorithms
  void vq(unsigned int uiDataIdx);
  void lbg();

  // distance algorithms
  unsigned int nn(unsigned int uiDataIdx, float &fMinDist);
  ImgBase* wta(ImgBase **dstImg=0);
  
  // helper functions
  void printCluster();
  
}; // class VQ
 

} // namespace

#endif
