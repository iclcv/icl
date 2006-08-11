#include "ICLFilter.h"

namespace icl{

  void ICLFilter::morphROI(ICLBase *poImage, int iHorz, int iVert)
  {
    FUNCTION_LOG("MorphRoi(ICLBase*," << iHorz << "," << iVert << ")");
    
    int a,b;

    poImage->getROISize(a,b);
    poImage->setROISize(a+2*iHorz,b+2*iVert);

    poImage->getROIOffset(a,b);
    poImage->setROIOffset(a-iHorz,b-iVert);
    

  }
}
