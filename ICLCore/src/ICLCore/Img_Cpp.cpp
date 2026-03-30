#include <ICLCore/ImgOps.h>
#include <ICLCore/Img.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // Calls the per-channel Img<T>::mirror(axis, channel, offset, size) directly.
  // This is the raw C++ swap-based implementation — no dispatch recursion.
  void cpp_mirror(ImgBase& img, axis a, bool roiOnly) {
    const Point& offset = roiOnly ? img.getROIOffset() : Point::null;
    const Size& size = roiOnly ? img.getROISize() : img.getSize();
    for(int c = 0; c < img.getChannels(); ++c) {
      switch(img.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
        case depth##D: img.asImg<icl##D>()->mirror(a, c, offset, size); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: break;
      }
    }
  }

  static const int _r1 = ImgBaseBackendDispatching::registerBackend<ImgOps::MirrorSig>(
    "Img.mirror", Backend::Cpp, cpp_mirror,
    nullptr, "C++ swap-based mirror");

} // anonymous namespace
