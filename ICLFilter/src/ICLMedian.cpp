#include "ICLMedian.h"

namespace icl{
  
  ICLMedian::ICLMedian(int iWidth, int iHeight):
    iWidth(iWidth),iHeight(iHeight){
  }
  
  ICLMedian::~ICLMedian(){}
  
  void ICLMedian::apply(ICLBase *poSrc, ICLBase *poDst){
#ifdef WITH_IPP_OPTIMIZATION
    
    
#else
    
    
#endif
  }
  
  
}
