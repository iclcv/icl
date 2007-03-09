#include <iclMacros.h>
#ifndef VQVECTOR_SET_H
#define VQVECTOR_SET_H


namespace icl{
  
  class VQVectorSet{
    public:
    VQVectorSet(float *data, int dim, bool deepCopyData):m_iDim(dim){
      if(deepCopyData){
        m_pfData = new float[2*dim];
        memcpy(m_pfData,data,2*dim*sizeof(float));
        m_bDeleteDataFlag = true;
      }else{
        m_pfData = data;
        m_bDeleteDataFlag = false;
      }
    }
    VQVectorSet():
      m_pfData(0),m_iDim(0),m_bDeleteDataFlag(0){}
    VQVectorSet(int dim):
      m_pfData(new float[2*dim]),m_iDim(dim),m_bDeleteDataFlag(true){}
    ~VQVectorSet(){
      if(m_bDeleteDataFlag) delete [] m_pfData;
    }
    float *operator[](int index) const {return m_pfData+2*index;}
    
    float *data() const {return m_pfData;}
    int dim() const { return m_iDim;}
    
    void resize(int dim){
      ICLASSERT_RETURN( dim );
      if(dim !=m_iDim || !m_bDeleteDataFlag){
        if(m_bDeleteDataFlag && m_pfData) delete [] m_pfData;
        m_pfData = new float[2*dim];
        m_iDim = dim;
        m_bDeleteDataFlag = true;
      }
    }
    private:
    float *m_pfData; /**< xyxyx.. data array */
    int m_iDim;     /**< count of data points */    
    bool m_bDeleteDataFlag;
  };
}

#endif

