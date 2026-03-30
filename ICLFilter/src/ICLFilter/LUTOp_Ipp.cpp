#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Img.h>
#include <ICLFilter/LUTOp.h>

// ippiReduceBits signature changed in modern IPP (oneAPI 2022+).
// TODO: update to modern ippiReduceBits API (added noise parameter).
#if 0 // was: ICL_HAVE_IPP — ippiReduceBits_8u_C1R signature changed

using namespace icl;
using namespace icl::core;

namespace {

  void ipp_reduceBits(const Img8u &src, Img8u &dst, icl8u n) {
    for(int c = src.getChannels() - 1; c >= 0; --c) {
      ippiReduceBits_8u_C1R(src.getROIData(c), src.getLineStep(),
                             dst.getROIData(c), dst.getLineStep(),
                             src.getROISize(), 0, ippDitherNone, n);
    }
  }

  using LOp = icl::filter::LUTOp;

  static int _reg = [] {
    using Op = LOp::Op;
    auto& proto = LOp::prototype();
    proto.addBackend<LOp::ReduceBitsSig>(Op::reduceBits, Backend::Ipp, ipp_reduceBits,
      applicableTo<icl8u>, "IPP reduceBits (8u)");
    return 0;
  }();

} // anonymous namespace

#endif // ICL_HAVE_IPP
