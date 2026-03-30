#include <ICLFilter/UnaryLogicalOp.h>
#include <ICLCore/Visitors.h>
#include <type_traits>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using ULOp = filter::UnaryLogicalOp;
  using Op = ULOp::Op;

  void cpp_withval(const Image &src, Image &dst, icl32s val, int optype) {
    src.visitWith(dst, [val, optype](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_integral_v<T>) {
        T v = clipped_cast<icl32s,T>(val);
        switch(optype) {
          case ULOp::andOp:
            visitROILinesPerChannelWith(s, d, [v](const T *sp, T *dp, int, int w) {
              for(int i = 0; i < w; ++i) dp[i] = sp[i] & v;
            }); break;
          case ULOp::orOp:
            visitROILinesPerChannelWith(s, d, [v](const T *sp, T *dp, int, int w) {
              for(int i = 0; i < w; ++i) dp[i] = sp[i] | v;
            }); break;
          case ULOp::xorOp:
            visitROILinesPerChannelWith(s, d, [v](const T *sp, T *dp, int, int w) {
              for(int i = 0; i < w; ++i) dp[i] = sp[i] ^ v;
            }); break;
          default: break;
        }
      }
    });
  }

  void cpp_noval(const Image &src, Image &dst) {
    src.visitWith(dst, [](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_integral_v<T>) {
        visitROILinesPerChannelWith(s, d, [](const T *sp, T *dp, int, int w) {
          for(int i = 0; i < w; ++i) dp[i] = ~sp[i];
        });
      }
    });
  }

  // Direct registration into the class prototype
  static int _reg = [] {
    auto& proto = ULOp::prototype();
    proto.addBackend<ULOp::WithValSig>(Op::withVal, Backend::Cpp, cpp_withval, "C++ logical and/or/xor");
    proto.addBackend<ULOp::NoValSig>(Op::noVal, Backend::Cpp, cpp_noval, "C++ logical not");
    return 0;
  }();

} // anon namespace
