#include <ICLFilter/ThresholdOp.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using TOp = filter::ThresholdOp;

  void cpp_ltval(const Image &src, Image &dst, double threshold, double value) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      T t = clipped_cast<double,T>(threshold);
      T v = clipped_cast<double,T>(value);
      visitROILinesPerChannelWith(s, d, [t, v](const T *sp, T *dp, int, int w) {
        for(int i = 0; i < w; ++i) dp[i] = sp[i] < t ? v : sp[i];
      });
    });
  }

  void cpp_gtval(const Image &src, Image &dst, double threshold, double value) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      T t = clipped_cast<double,T>(threshold);
      T v = clipped_cast<double,T>(value);
      visitROILinesPerChannelWith(s, d, [t, v](const T *sp, T *dp, int, int w) {
        for(int i = 0; i < w; ++i) dp[i] = sp[i] > t ? v : sp[i];
      });
    });
  }

  void cpp_ltgtval(const Image &src, Image &dst,
                   double tLo, double vLo, double tHi, double vHi) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      T lo = clipped_cast<double,T>(tLo);
      T vl = clipped_cast<double,T>(vLo);
      T hi = clipped_cast<double,T>(tHi);
      T vh = clipped_cast<double,T>(vHi);
      visitROILinesPerChannelWith(s, d, [lo, vl, hi, vh](const T *sp, T *dp, int, int w) {
        for(int i = 0; i < w; ++i) {
          T val = sp[i];
          dp[i] = val < lo ? vl : (val > hi ? vh : val);
        }
      });
    });
  }

  // Direct registration into the class prototype
  static int _reg = [] {
    using Op = TOp::Op;
    auto& proto = TOp::prototype();
    proto.getSelector<TOp::ThreshSig>(Op::ltVal).add(Backend::Cpp, cpp_ltval, "C++ ltVal");
    proto.getSelector<TOp::ThreshSig>(Op::gtVal).add(Backend::Cpp, cpp_gtval, "C++ gtVal");
    proto.getSelector<TOp::ThreshDualSig>(Op::ltgtVal).add(Backend::Cpp, cpp_ltgtval, "C++ ltgtVal");
    return 0;
  }();

} // anon namespace
