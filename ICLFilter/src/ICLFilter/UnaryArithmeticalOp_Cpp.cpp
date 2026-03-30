#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLUtils/EnumDispatch.h>
#include <cmath>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using UAOp = filter::UnaryArithmeticalOp;

  template<class T>
  inline T absVal(T t) { return std::abs(t); }
  template<> inline icl8u absVal(icl8u t) { return t; }
  template<> inline icl32f absVal(icl32f t) { return std::fabs(t); }
  template<> inline icl64f absVal(icl64f t) { return std::fabs(t); }

  template<UAOp::optype OT, class T>
  void arithWithVal(const Img<T> &src, Img<T> &dst, T val) {
    visitROILinesPerChannelWith(src, dst, [val](const T *s, T *d, int, int w) {
      for(int i = 0; i < w; ++i) {
        if constexpr (OT == UAOp::addOp) d[i] = clipped_cast<icl64f,T>(static_cast<icl64f>(s[i]) + static_cast<icl64f>(val));
        else if constexpr (OT == UAOp::subOp) d[i] = clipped_cast<icl64f,T>(static_cast<icl64f>(s[i]) - static_cast<icl64f>(val));
        else if constexpr (OT == UAOp::mulOp) d[i] = clipped_cast<icl64f,T>(static_cast<icl64f>(s[i]) * static_cast<icl64f>(val));
        else if constexpr (OT == UAOp::divOp) d[i] = clipped_cast<icl64f,T>(static_cast<icl64f>(s[i]) / static_cast<icl64f>(val));
      }
    });
  }

  template<UAOp::optype OT, class T>
  void arithNoVal(const Img<T> &src, Img<T> &dst) {
    visitROILinesPerChannelWith(src, dst, [](const T *s, T *d, int, int w) {
      for(int i = 0; i < w; ++i) {
        if constexpr (OT == UAOp::sqrOp)  d[i] = s[i] * s[i];
        else if constexpr (OT == UAOp::sqrtOp) d[i] = clipped_cast<double,T>(std::sqrt(static_cast<double>(s[i])));
        else if constexpr (OT == UAOp::lnOp)   d[i] = clipped_cast<double,T>(std::log(static_cast<double>(s[i])));
        else if constexpr (OT == UAOp::expOp)   d[i] = clipped_cast<double,T>(std::exp(static_cast<double>(s[i])));
        else if constexpr (OT == UAOp::absOp)   d[i] = absVal(s[i]);
      }
    });
  }

  template<UAOp::optype OT>
  void apply_with_val(const Image &src, Image &dst, double value) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      arithWithVal<OT>(s, d, clipped_cast<icl64f,T>(value));
    });
  }

  template<UAOp::optype OT>
  void apply_no_val(const Image &src, Image &dst) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      arithNoVal<OT>(s, d);
    });
  }

  void cpp_arith_with_val(const Image &src, Image &dst, double value, int optype) {
    dispatchEnum<UAOp::addOp, UAOp::subOp, UAOp::mulOp, UAOp::divOp>(optype, [&](auto tag) {
      apply_with_val<decltype(tag)::value>(src, dst, value);
    });
  }

  void cpp_arith_no_val(const Image &src, Image &dst, int optype) {
    dispatchEnum<UAOp::sqrOp, UAOp::sqrtOp, UAOp::lnOp, UAOp::expOp, UAOp::absOp>(optype, [&](auto tag) {
      apply_no_val<decltype(tag)::value>(src, dst);
    });
  }

  // Direct registration into the class prototype
  static int _reg = [] {
    using Op = UAOp::Op;
    auto& proto = UAOp::prototype();
    proto.addBackend<UAOp::ArithValSig>(Op::withVal, Backend::Cpp, cpp_arith_with_val, "C++ arithmetic with-val");
    proto.addBackend<UAOp::ArithNoValSig>(Op::noVal, Backend::Cpp, cpp_arith_no_val, "C++ arithmetic no-val");
    return 0;
  }();

} // anon namespace
