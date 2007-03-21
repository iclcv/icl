#include "iclPWCGrabEngine.h"
#include <iclUnicapDevice.h>
using namespace std;

namespace icl{
  PWCGrabEngine::PWCGrabEngine(UnicapDevice *unicapDev) : 
    UnicapGrabEngine(unicapDev){
    string deviceName = unicapDev->getDevice();
    int idev = 
    deviceName == "/dev/video0" ? 0 :
    deviceName == "/dev/video1" ? 1 :
    deviceName == "/dev/video2" ? 2 :
    deviceName == "/dev/video3" ? 3 : -1;
    if(idev == -1) ERROR_LOG("could not found device association for: \""<<deviceName<<"\"!");
    
    m_poPWCGrabber  = new PWCGrabber(Size(640,480),30,idev);
  }
  
  void PWCGrabEngine::getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth,ImgBase **ppoDst){
    ensureCompatible(ppoDst,desiredDepth,desiredParams);
    m_poPWCGrabber->grab(*ppoDst);
  }  
}
