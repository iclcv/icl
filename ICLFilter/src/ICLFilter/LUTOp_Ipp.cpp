#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Img.h>
#include <ICLFilter/LUTOp.h>

#ifdef ICL_HAVE_IPP

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

  static const int _r = ImageBackendDispatching::registerBackend<LOp::ReduceBitsSig>(
    "LUTOp.reduceBits", Backend::Ipp, ipp_reduceBits,
    applicableTo<icl8u>, "IPP reduceBits (8u)");

} // anonymous namespace

#endif // ICL_HAVE_IPP
