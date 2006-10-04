#ifndef VQCLUSTERINFO_H
#define VQCLUSTERINFO_H

#include <vector>

namespace icl{


  class VQClusterInfo{
    typedef std::vector<float> fvec;  
    
    public:
    
    VQClusterInfo(int dim=0): m_iDim(0), m_pfCenters(0),
      m_pfPCAInfos(0), m_pfErrors(0), m_pvecElems(0){
      if(dim){
        resize(dim);
      }
    }
    
    ~VQClusterInfo(){
      resize(0);
    }
    
    void resize(int dim){
      if(m_iDim == dim) return;
      m_iDim = dim;
      if(m_pfCenters){
        delete m_pfCenters;
        delete m_pfPCAInfos;
        delete m_pfErrors;
        delete m_pvecElems;
      }
      if(dim){
        m_pfCenters = new float[dim];
        m_pfPCAInfos = new float[dim*3];
        m_pfErrors = new float[dim];
        m_pvecElems = new fvec[dim];
      }else{
        m_pfCenters = 0;
        m_pfPCAInfos = 0;
        m_pfErrors = 0;
        m_pvecElems = 0;
      }
    }

    void addElem(int prototypeIndex, float x, float y){
      m_pvecElems[prototypeIndex].push_back(x);
      m_pvecElems[prototypeIndex].push_back(y);
    }

    void clearElems(){
      for(int i=0;i<m_iDim;i++){
        m_pvecElems[i].clear();
      }
    }

    int dim() const {
      return m_iDim; 
    }
    float *center(int index=0) const {
      return m_pfCenters+2*index;
    }
    float *pcainfo(int index=0) const {
      return m_pfPCAInfos+3*index;
    }
    float *error(int index=0) const {
      return m_pfErrors+index;
    }
    int size(int index) const {
      return (int)m_pvecElems[index].size();
    }
    float *elem(int prototypeIndex, int elemIndex=0) const {
      return &((m_pvecElems[prototypeIndex])[2*elemIndex]);
    }
  
    private:
  
    int m_iDim;
    float *m_pfCenters;
    float *m_pfPCAInfos;
    float *m_pfErrors;
    fvec *m_pvecElems;
  };
}

#endif
