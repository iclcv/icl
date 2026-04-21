#include <icl/filter/WienerOp.h>
#include <icl/core/Image.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using WOp = filter::WienerOp;

  void cpp_wiener(const Image &, Image &, const Size &, const Point &,
                  const Point &, icl32f) {
    throw ICLException("WienerOp: requires IPP (no C++ fallback available)");
  }

  static int _reg = [] {
    using Op = WOp::Op;
    auto cpp = WOp::prototype().backends(Backend::Cpp);
    cpp.add<WOp::WienerSig>(Op::apply, cpp_wiener,
                             "C++ Wiener (throws — IPP required)");
    return 0;
  }();

} // anonymous namespace
