#include <ICLFilter/BinaryCompareOp.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLUtils/EnumDispatch.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using BCOp = filter::BinaryCompareOp;

  template<BCOp::optype OT, class T>
  void cmpTyped(const Img<T> &s1, const Img<T> &s2, Img8u &dst) {
    visitROILinesPerChannel2With(s1, s2, dst,
      [](const T *a, const T *b, icl8u *d, int, int w) {
        for(int i = 0; i < w; ++i) {
          if constexpr (OT == BCOp::lt)   d[i] = a[i] < b[i] ? 255 : 0;
          else if constexpr (OT == BCOp::lteq) d[i] = a[i] <= b[i] ? 255 : 0;
          else if constexpr (OT == BCOp::eq)   d[i] = a[i] == b[i] ? 255 : 0;
          else if constexpr (OT == BCOp::gteq) d[i] = a[i] >= b[i] ? 255 : 0;
          else if constexpr (OT == BCOp::gt)   d[i] = a[i] > b[i] ? 255 : 0;
        }
      });
  }

  template<BCOp::optype OT>
  void compare_typed(const Image &s1, const Image &s2, Img8u &d) {
    s1.visit([&](const auto &a) {
      using T = typename std::remove_reference_t<decltype(a)>::type;
      cmpTyped<OT>(a, s2.as<T>(), d);
    });
  }

  void cpp_compare(const Image &s1, const Image &s2, Image &dst, int ot) {
    Img8u &d = dst.as8u();
    dispatchEnum<BCOp::lt, BCOp::lteq, BCOp::eq, BCOp::gteq, BCOp::gt>(ot, [&](auto tag) {
      compare_typed<decltype(tag)::value>(s1, s2, d);
    });
  }

  void cpp_compare_eqt(const Image &s1, const Image &s2, Image &dst, double tolerance) {
    Img8u &d = dst.as8u();
    s1.visit([&](const auto &a) {
      using T = typename std::remove_reference_t<decltype(a)>::type;
      T tol = clipped_cast<double,T>(tolerance);
      visitROILinesPerChannel2With(a, s2.as<T>(), d,
        [tol](const T *ap, const T *bp, icl8u *dp, int, int w) {
          for(int i = 0; i < w; ++i)
            dp[i] = std::abs(ap[i] - bp[i]) <= tol ? 255 : 0;
        });
    });
  }

  // Direct registration into the class prototype
  static int _reg = [] {
    using Op = BCOp::Op;
    auto cpp = BCOp::prototype().backends(Backend::Cpp);
    cpp.add<BCOp::CmpSig>(Op::compare, cpp_compare, "C++ compare");
    cpp.add<BCOp::CmpEqtSig>(Op::compareEqTol, cpp_compare_eqt, "C++ compareEqTol");
    return 0;
  }();

} // anon namespace
