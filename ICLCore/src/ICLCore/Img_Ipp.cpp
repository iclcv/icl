#include <ICLCore/ImgOps.h>
#include <ICLCore/Img.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  void ipp_mirror(ImgBase& img, axis a, bool roiOnly) {
    const int lineStep = img.getLineStep();
    const Point& offset = roiOnly ? img.getROIOffset() : Point::null;
    const Size& size = roiOnly ? img.getROISize() : img.getSize();

    for(int c = 0; c < img.getChannels(); ++c) {
      switch(img.getDepth()) {
        case depth8u:
          ippiMirror_8u_C1IR(img.asImg<icl8u>()->getROIData(c, offset),
                             lineStep, size, static_cast<IppiAxis>(a));
          break;
        case depth16s:
          ippiMirror_16u_C1IR(reinterpret_cast<Ipp16u*>(img.asImg<icl16s>()->getROIData(c, offset)),
                              lineStep, size, static_cast<IppiAxis>(a));
          break;
        case depth32s:
          ippiMirror_32s_C1IR(img.asImg<icl32s>()->getROIData(c, offset),
                              lineStep, size, static_cast<IppiAxis>(a));
          break;
        case depth32f:
          ippiMirror_32s_C1IR(reinterpret_cast<Ipp32s*>(img.asImg<icl32f>()->getROIData(c, offset)),
                              lineStep, size, static_cast<IppiAxis>(a));
          break;
        default: break;
      }
    }
  }

  static const int _r1 = ImgBaseBackendDispatching::registerBackend<ImgOps::MirrorSig>(
    "Img.mirror", Backend::Ipp, ipp_mirror,
    applicableToBase<icl8u, icl16s, icl32s, icl32f>,
    "IPP ippiMirror (8u/16s/32s/32f)");

} // anonymous namespace
