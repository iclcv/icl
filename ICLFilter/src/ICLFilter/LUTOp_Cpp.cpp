#include <ICLFilter/LUTOp.h>
#include <ICLCore/Img.h>
#include <vector>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using LOp = filter::LUTOp;

  void cpp_reduceBits(const Img8u &src, Img8u &dst, icl8u n) {
    std::vector<icl8u> lut(256), lv(n);
    float range = 255.0f / (n - 1);
    for(int i = 0; i < n; i++) {
      lv[i] = round(iclMin(i * range, 255.f));
    }
    for(int i = 0; i < 256; i++) {
      float rel = i / 256.f;
      lut[i] = lv[static_cast<int>(round(rel * (n - 1)))];
    }
    LOp::simple(&src, &dst, lut);
  }

  static int _reg = [] {
    using Op = LOp::Op;
    auto cpp = LOp::prototype().backends(Backend::Cpp);
    cpp.add<LOp::ReduceBitsSig>(Op::reduceBits, cpp_reduceBits, "C++ reduceBits");
    return 0;
  }();

} // anonymous namespace
