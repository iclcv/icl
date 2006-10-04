#ifndef VQ2D_H
#define VQ2D_H

#include <stdlib.h>
#include "VQVectorSet.h"
#include "VQClusterInfo.h"

namespace icl{
  class VQ2D{
    public:
    VQ2D(float *data=0, int dim=0, bool deepCopyData = false);
    
    /// data will be copied once
    void setData(float *data, int dim, bool deepCopy = false);
    
    /// retruns just the center information
    /** @param centers count of prototypes to use 
        @param max. count of steps to perform 
        @param mmqe mininum mean qauntisation error 
        @param qe quantisation error */
    const VQVectorSet &run(int centers, int steps, float mmqe, float &qe);
    
    /// retruns all available information of the blobs
    /** @param centers count of prototypes to use 
        @param max. count of steps to perform 
        @param mmqe mininum mean qauntisation error 
        @param qe quantisation error */
    const VQClusterInfo &run2(int centers, int steps, float mmqe,float &qe);
    
    protected:
    VQVectorSet *m_poCenters;
    VQClusterInfo *m_poClusterInfo;
    VQVectorSet *m_poData;
  };
}

#endif
