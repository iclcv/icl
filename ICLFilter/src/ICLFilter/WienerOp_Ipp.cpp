#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Img.h>
#include <ICLFilter/WienerOp.h>
#include <vector>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  template<class T> struct WienerImpl {
    static void apply(const Img<T> &, Img<T> &, const Size &, const Point &,
                      const Point &, std::vector<icl8u> &, icl32f) {
      throw ICLException("WienerOp: unsupported depth");
    }
  };

  template<> struct WienerImpl<icl8u> {
    static void apply(const Img8u &src, Img8u &dst, const Size &maskSize,
                      const Point &anchor, const Point &roiOffset,
                      std::vector<icl8u> &buf, icl32f noise) {
      int bufSize;
      ippiFilterWienerGetBufferSize(dst.getROISize(), maskSize, 1, &bufSize);
      buf.reserve(bufSize);
      for(int c = src.getChannels()-1; c >= 0; --c) {
        ippiFilterWiener_8u_C1R(src.getROIData(c, roiOffset), src.getLineStep(),
                                dst.getROIData(c), dst.getLineStep(),
                                dst.getROISize(), maskSize, anchor,
                                &noise, buf.data());
      }
    }
  };

  template<> struct WienerImpl<icl16s> {
    static void apply(const Img16s &src, Img16s &dst, const Size &maskSize,
                      const Point &anchor, const Point &roiOffset,
                      std::vector<icl8u> &buf, icl32f noise) {
      int bufSize;
      ippiFilterWienerGetBufferSize(dst.getROISize(), maskSize, 1, &bufSize);
      buf.reserve(bufSize);
      for(int c = src.getChannels()-1; c >= 0; --c) {
        ippiFilterWiener_16s_C1R(src.getROIData(c, roiOffset), src.getLineStep(),
                                  dst.getROIData(c), dst.getLineStep(),
                                  dst.getROISize(), maskSize, anchor,
                                  &noise, buf.data());
      }
    }
  };

  template<> struct WienerImpl<icl32f> {
    static void apply(const Img32f &src, Img32f &dst, const Size &maskSize,
                      const Point &anchor, const Point &roiOffset,
                      std::vector<icl8u> &buf, icl32f noise) {
      int bufSize;
      ippiFilterWienerGetBufferSize(dst.getROISize(), maskSize, 1, &bufSize);
      buf.reserve(bufSize);
      for(int c = src.getChannels()-1; c >= 0; --c) {
        ippiFilterWiener_32f_C1R(src.getROIData(c, roiOffset), src.getLineStep(),
                                  dst.getROIData(c), dst.getLineStep(),
                                  dst.getROISize(), maskSize, anchor,
                                  &noise, buf.data());
      }
    }
  };

  using WOp = icl::filter::WienerOp;

  // Stateful backend: captures a reusable working buffer per instance
  static const int _r1 = ImageBackendDispatching::registerStatefulBackend<WOp::WienerSig>(
    "WienerOp.apply", Backend::Ipp,
    []() {
      auto buf = std::make_shared<std::vector<icl8u>>();
      return [buf](const Image &src, Image &dst, const Size &maskSize,
                   const Point &anchor, const Point &roiOffset, icl32f noise) {
        src.visit([&](const auto &s) {
          using T = typename std::remove_reference_t<decltype(s)>::type;
          auto &d = dst.as<T>();
          WienerImpl<T>::apply(s, d, maskSize, anchor, roiOffset, *buf, noise);
        });
      };
    },
    applicableTo<icl8u, icl16s, icl32f>, "IPP Wiener filter (8u/16s/32f)");

} // anonymous namespace
