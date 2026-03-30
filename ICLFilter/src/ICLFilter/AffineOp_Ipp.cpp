#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Img.h>
#include <ICLFilter/AffineOp.h>

// ippiWarpAffine API changed in modern IPP (oneAPI 2022+).
// TODO: update to ippiWarpAffineNearest/ippiWarpAffineLinear + init spec.
#if 0 // was: ICL_HAVE_IPP — ippiWarpAffine_*_C1R removed

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  void ipp_affine(const Image &src, Image &dst, const double* fwd, scalemode interp) {
    // Reinterpret the 6 contiguous doubles as a double[2][3] for IPP
    double m[2][3];
    std::memcpy(m, fwd, sizeof(m));

    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u>) {
        for(int c = 0; c < s.getChannels(); c++) {
          ippiWarpAffine_8u_C1R(s.getData(c),
                                s.getSize(), s.getLineStep(),
                                Rect(Point::null, s.getSize()),
                                d.getData(c),
                                d.getLineStep(), d.getROI(),
                                m, interp);
        }
      } else if constexpr (std::is_same_v<T, icl32f>) {
        for(int c = 0; c < s.getChannels(); c++) {
          ippiWarpAffine_32f_C1R(s.getData(c),
                                 s.getSize(), s.getLineStep(),
                                 Rect(Point::null, s.getSize()),
                                 d.getData(c),
                                 d.getLineStep(), d.getROI(),
                                 m, interp);
        }
      }
    });
  }

  using AOp = icl::filter::AffineOp;

  static int _reg = [] {
    using Op = AOp::Op;
    auto& proto = AOp::prototype();
    proto.addBackend<AOp::AffineSig>(Op::apply, Backend::Ipp, ipp_affine,
      applicableTo<icl8u, icl32f>, "IPP affine warp (8u/32f)");
    return 0;
  }();

} // anonymous namespace

#endif // ICL_HAVE_IPP
