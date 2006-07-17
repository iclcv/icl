#include "ICLFilter.h"

namespace icl{

  void ICLFilter::morphROI(ICLBase *poImage, int iHorz, int iVert)
  {
    DEBUG_LOG4("MorphRoi*,int,int)");
    
    int a,b;
    poImage->getROIOffset(a,b);
    poImage->setROIOffset(a-iHorz,b-iVert);
    
    poImage->getROISize(a,b);
    poImage->setROISize(a+2*iHorz,b+2*iVert);
  }
}
