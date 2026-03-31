#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Img.h>
#include <ICLCore/Visitors.h>
#include <ICLFilter/UnaryLogicalOp.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // --- IPP wrappers ---

  template <typename T, IppStatus (IPP_DECL *func)(const T*, int, T, T*, int, IppiSize)>
  inline void ipp_call_with_val(const Img<T> &src, Img<T> &dst, T val) {
    for(int c = src.getChannels()-1; c >= 0; --c) {
      func(src.getROIData(c), src.getLineStep(), val,
           dst.getROIData(c), dst.getLineStep(), dst.getROISize());
    }
  }

  template <typename T, IppStatus (IPP_DECL *func)(const T*, int, T*, int, IppiSize)>
  inline void ipp_call_no_val(const Img<T> &src, Img<T> &dst) {
    for(int c = src.getChannels()-1; c >= 0; --c) {
      func(src.getROIData(c), src.getLineStep(),
           dst.getROIData(c), dst.getLineStep(), dst.getROISize());
    }
  }

  // --- Backend functions ---

  void ipp_withval(const Image &src, Image &dst, icl32s val, int optype) {
    src.visitWith(dst, [val, optype](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u>) {
        icl8u v = clipped_cast<icl32s, icl8u>(val);
        switch(optype) {
          case filter::UnaryLogicalOp::andOp: ipp_call_with_val<icl8u, ippiAndC_8u_C1R>(s, d, v); break;
          case filter::UnaryLogicalOp::orOp:  ipp_call_with_val<icl8u, ippiOrC_8u_C1R>(s, d, v); break;
          case filter::UnaryLogicalOp::xorOp: ipp_call_with_val<icl8u, ippiXorC_8u_C1R>(s, d, v); break;
          default: break;
        }
      } else if constexpr (std::is_same_v<T, icl32s>) {
        icl32s v = val;
        switch(optype) {
          case filter::UnaryLogicalOp::andOp: ipp_call_with_val<icl32s, ippiAndC_32s_C1R>(s, d, v); break;
          case filter::UnaryLogicalOp::orOp:  ipp_call_with_val<icl32s, ippiOrC_32s_C1R>(s, d, v); break;
          case filter::UnaryLogicalOp::xorOp: ipp_call_with_val<icl32s, ippiXorC_32s_C1R>(s, d, v); break;
          default: break;
        }
      }
    });
  }

  void ipp_noval(const Image &src, Image &dst) {
    src.visitWith(dst, [](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u>) {
        ipp_call_no_val<icl8u, ippiNot_8u_C1R>(s, d);
      }
    });
  }

  // --- Direct registration into prototype ---
  using ULOp = icl::filter::UnaryLogicalOp;
  using Op = ULOp::Op;

  static int _reg = [] {
    auto ipp = ULOp::prototype().backends(Backend::Ipp);
    ipp.add<ULOp::WithValSig>(Op::withVal, ipp_withval, applicableTo<icl8u, icl32s>, "IPP logical and/or/xor (8u/32s)");
    ipp.add<ULOp::NoValSig>(Op::noVal, ipp_noval, applicableTo<icl8u>, "IPP logical not (8u)");
    return 0;
  }();

} // anonymous namespace
