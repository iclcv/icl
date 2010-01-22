#ifndef VQVECTOR_SET_H
#define VQVECTOR_SET_H

#include <ICLUtils/Macros.h>
#include <cstring>
using std::memcpy;

namespace icl{
  
  /// Utility class for 2D Vector quantisation \ingroup G_UTILS
  class VQVectorSet{
    public:
    /// Create a new vector set with given data,and data dimension 
    /** @param data data pointer
        @param dim number of data elements data.size = 2*dim
        @param deepCopy if set to true, the given data is copied deeply
    **/
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
    /// Empty Constructor
    VQVectorSet():
      m_pfData(0),m_iDim(0),m_bDeleteDataFlag(0){}
    
    /// Default constructor (data is allocated internally)
    VQVectorSet(int dim):
      m_pfData(new float[2*dim]),m_iDim(dim),m_bDeleteDataFlag(true){}

    /// Destructor
    ~VQVectorSet(){
      if(m_bDeleteDataFlag) delete [] m_pfData;
    }
    
    /// data access operator
    float *operator[](int index) const {return m_pfData+2*index;}
    
    /// returns the internal data pointer
    float *data() const {return m_pfData;}
    
    /// returns the data element count
    int dim() const { return m_iDim;}
    
    /// resizes this vectorset to given dim (data content is lost)
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
    float *m_pfData; /**!< xyxyx.. data array */
    int m_iDim;     /**!< count of data points */    
    bool m_bDeleteDataFlag;/**!< flag to indicate wheather data must be deleted in the destructor */
  };
}

#endif

