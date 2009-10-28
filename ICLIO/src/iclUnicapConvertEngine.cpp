#include <iclUnicapConvertEngine.h>
#include <iclUnicapFormat.h>
#include <iclUnicapDevice.h>
#include <iclCoreFunctions.h>

namespace icl{

  void UnicapConvertEngine::cvtNative(const icl8u *rawData, ImgBase **ppoDst){
    depth d = depth8u; // maybe we can try to estimate depth from m_poDevice
    UnicapFormat f = m_poDevice->getCurrentUnicapFormat();
    ImgParams p(f.getSize(),formatRGB); // here we use RGB as default
    ensureCompatible(ppoDst,d,p);
    cvt(rawData,p,d,ppoDst);
  }

}
