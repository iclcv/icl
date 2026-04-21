#include <icl/filter/BinaryArithmeticalOp.h>
#include <icl/core/Visitors.h>
#include <icl/utils/EnumDispatch.h>
#include <cmath>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using BOp = filter::BinaryArithmeticalOp;
  using Op = BOp::Op;

  template<BOp::optype OT, class T>
  void arithOp(const Img<T> &s1, const Img<T> &s2, Img<T> &dst) {
    visitROILinesPerChannel2With(s1, s2, dst, [](const T *a, const T *b, T *d, int, int w) {
      for(int i = 0; i < w; ++i) {
        if constexpr (OT == BOp::addOp)    d[i] = a[i] + b[i];
        else if constexpr (OT == BOp::subOp)    d[i] = a[i] - b[i];
        else if constexpr (OT == BOp::mulOp)    d[i] = a[i] * b[i];
        else if constexpr (OT == BOp::divOp)    d[i] = a[i] / b[i];
        else if constexpr (OT == BOp::absSubOp) d[i] = std::abs(a[i] - b[i]);
      }
    });
  }

  template<BOp::optype OT>
  void apply_typed(const Image &s1, const Image &s2, Image &dst) {
    s1.visitWith(dst, [&](const auto &a, auto &d) {
      using T = typename std::remove_reference_t<decltype(a)>::type;
      arithOp<OT>(a, s2.as<T>(), d);
    });
  }

  void cpp_apply(const Image &s1, const Image &s2, Image &dst, int ot) {
    dispatchEnum<BOp::addOp, BOp::subOp, BOp::mulOp, BOp::divOp, BOp::absSubOp>(ot, [&](auto tag) {
      apply_typed<decltype(tag)::value>(s1, s2, dst);
    });
  }

  // Direct registration into the class prototype
  static int _reg = [] {
    auto cpp = BOp::prototype().backends(Backend::Cpp);
    cpp.add<BOp::Sig>(Op::apply, cpp_apply, "C++ binary arithmetic");
    return 0;
  }();

} // anonymous namespace
