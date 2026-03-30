#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Img.h>
#include <ICLFilter/WarpOp.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  void ipp_warp(const Image& src, Image& dst, const Channel32f* warpMap,
                Point warpOffset, scalemode mode) {
    const int wmW = warpMap[0].getWidth();
    const Rect dstROI = dst.getROI();
    const Size roiSize = dst.getROISize();

    auto doWarp = [&](auto ippFunc, const auto& s, auto& d) {
      for(int c = 0; c < s.getChannels(); ++c) {
        const icl32f *wmX = warpMap[0].begin()
          + (dstROI.y + warpOffset.y) * wmW + (dstROI.x + warpOffset.x);
        const icl32f *wmY = warpMap[1].begin()
          + (dstROI.y + warpOffset.y) * wmW + (dstROI.x + warpOffset.x);

        IppStatus st = ippFunc(
          s.begin(c), s.getSize(), s.getLineStep(), s.getImageRect(),
          wmX, sizeof(icl32f) * wmW,
          wmY, sizeof(icl32f) * wmW,
          d.getROIData(c), d.getLineStep(),
          roiSize, static_cast<int>(mode));

        if(st != ippStsNoErr) {
          ERROR_LOG("IPP-Error:" << ippGetStatusString(st));
          return;
        }
      }
    };

    switch(src.getDepth()) {
      case depth8u:
        doWarp(ippiRemap_8u_C1R, src.as<icl8u>(), dst.as<icl8u>());
        break;
      case depth32f:
        doWarp(ippiRemap_32f_C1R, src.as<icl32f>(), dst.as<icl32f>());
        break;
      default:
        break; // not applicable — filtered by applicability
    }
  }

  using WOp = icl::filter::WarpOp;

  static const int _r1 = ImageBackendDispatching::registerBackend<WOp::WarpSig>(
    "WarpOp.warp", Backend::Ipp,
    ipp_warp,
    applicableTo<icl8u, icl32f>, "IPP ippiRemap (8u/32f)");

} // anonymous namespace
