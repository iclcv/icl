#include <ICLFilter/UnaryCompareOp.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLUtils/EnumDispatch.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using UCO = filter::UnaryCompareOp;

  template<UCO::optype OT, class T>
  void cmpTyped(const Img<T> &src, Img8u &dst, T value) {
    visitROILinesPerChannelWith(src, dst, [value](const T *s, icl8u *d, int, int w) {
      for(int i = 0; i < w; ++i) {
        if constexpr (OT == UCO::lt)   d[i] = s[i] < value ? 255 : 0;
        else if constexpr (OT == UCO::lteq) d[i] = s[i] <= value ? 255 : 0;
        else if constexpr (OT == UCO::eq)   d[i] = s[i] == value ? 255 : 0;
        else if constexpr (OT == UCO::gteq) d[i] = s[i] >= value ? 255 : 0;
        else if constexpr (OT == UCO::gt)   d[i] = s[i] > value ? 255 : 0;
      }
    });
  }

  template<UCO::optype OT>
  void compare_typed(const Image &src, Img8u &d, double value) {
    src.visit([&](const auto &s) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      cmpTyped<OT>(s, d, clipped_cast<double,T>(value));
    });
  }

  void cpp_compare(const Image &src, Image &dst, double value, int optype) {
    Img8u &d = dst.as8u();
    dispatchEnum<UCO::lt, UCO::lteq, UCO::eq, UCO::gteq, UCO::gt>(optype, [&](auto tag) {
      compare_typed<decltype(tag)::value>(src, d, value);
    });
  }

  void cpp_compare_eqt(const Image &src, Image &dst, double value, double tolerance) {
    Img8u &d = dst.as8u();
    src.visit([&](const auto &s) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      T v = clipped_cast<double,T>(value);
      T t = clipped_cast<double,T>(tolerance);
      visitROILinesPerChannelWith(s, d, [v, t](const T *sp, icl8u *dp, int, int w) {
        for(int i = 0; i < w; ++i)
          dp[i] = std::abs(sp[i] - v) <= t ? 255 : 0;
      });
    });
  }

  // Direct registration into the class prototype
  static int _reg = [] {
    using Op = UCO::Op;
    auto& proto = UCO::prototype();
    proto.addBackend<UCO::CmpSig>(Op::compare, Backend::Cpp, cpp_compare, "C++ compare");
    proto.addBackend<UCO::CmpEqtSig>(Op::compareEqTol, Backend::Cpp, cpp_compare_eqt, "C++ compareEqTol");
    return 0;
  }();

} // anon namespace
