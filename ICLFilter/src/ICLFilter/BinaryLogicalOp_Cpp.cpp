#include <ICLFilter/BinaryLogicalOp.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/EnumDispatch.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using BLOp = filter::BinaryLogicalOp;

  template<BLOp::optype OT, class T>
  void logicalOp(const Img<T> &s1, const Img<T> &s2, Img<T> &dst) {
    visitROILinesPerChannel2With(s1, s2, dst, [](const T *a, const T *b, T *d, int, int w) {
      for(int i = 0; i < w; ++i) {
        if constexpr (OT == BLOp::andOp) d[i] = a[i] & b[i];
        else if constexpr (OT == BLOp::orOp)  d[i] = a[i] | b[i];
        else if constexpr (OT == BLOp::xorOp) d[i] = a[i] ^ b[i];
      }
    });
  }

  template<BLOp::optype OT>
  void apply_typed(const Image &s1, const Image &s2, Image &dst) {
    s1.visitWith(dst, [&](const auto &a, auto &d) {
      using T = typename std::remove_reference_t<decltype(a)>::type;
      if constexpr (std::is_integral_v<T>) {
        logicalOp<OT>(a, s2.as<T>(), d);
      }
    });
  }

  void cpp_apply(const Image &s1, const Image &s2, Image &dst, int ot) {
    dispatchEnum<BLOp::andOp, BLOp::orOp, BLOp::xorOp>(ot, [&](auto tag) {
      apply_typed<decltype(tag)::value>(s1, s2, dst);
    });
  }

  // Direct registration into the class prototype
  static int _reg = [] {
    using Op = BLOp::Op;
    auto& proto = BLOp::prototype();
    proto.addBackend<BLOp::Sig>(Op::apply, Backend::Cpp, cpp_apply, "C++ binary logical");
    return 0;
  }();

} // anonymous namespace
