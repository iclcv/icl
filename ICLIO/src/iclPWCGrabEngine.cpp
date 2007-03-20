#include "iclPWCGrabEngine.h"

namespace icl{

  void PWCGrabEngine::getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth,ImgBase **ppoDst){
    ensureCompatible(ppoDst,desiredDepth,desiredParams);
    m_poPWCGrabber->grab(*ppoDst);
  }  
}
