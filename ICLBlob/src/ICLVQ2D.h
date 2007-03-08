#include <stdlib.h>
#include <ICLVQVectorSet.h>
#include <ICLVQClusterInfo.h>
#include <ICLMathematics.h>
#include <ICLTime.h>
#ifndef VQ2D_H
#define VQ2D_H


namespace icl{
  class VQ2D{
    public:
    VQ2D(float *data=0, int dim=0, bool deepCopyData = false);
    
    /// data will be copied once
    void setData(float *data, int dim, bool deepCopy = false);
    
    /// retruns just the center information
    /** @param centers count of prototypes to use 
        @param steps count of steps to perform 
        @param mmqe mininum mean qauntisation error 
        @param qe quantisation error */
    const VQVectorSet &run(int centers, int steps, float mmqe, float &qe);
    
    /// calculates advanced features like local pca
    const VQClusterInfo &features();

    const VQVectorSet &centers();
    
    protected:
    VQVectorSet *m_poCenters;
    VQClusterInfo *m_poClusterInfo;
    VQVectorSet *m_poData;
  };
}

#endif
