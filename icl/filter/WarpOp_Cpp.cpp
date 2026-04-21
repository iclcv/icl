#include <icl/filter/WarpOp.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using WOp = filter::WarpOp;

  template<class T>
  inline T interpolate_pixel_nn(float x, float y, const Channel<T> &src){
    if(x < 0) return T(0);
    return src(round(x),round(y));
  }

  template<class T>
  inline T interpolate_pixel_lin(float x, float y, const Channel<T> &src){
    if(x < 0) return T(0);
    float fX0 = x - floor(x), fX1 = 1.0f - fX0;
    float fY0 = y - floor(y), fY1 = 1.0f - fY0;
    int xll = static_cast<int>(x);
    int yll = static_cast<int>(y);

    const T* pLL = &src(xll,yll);
    float a = *pLL;        //  a b
    float b = *(++pLL);    //  c d
    pLL += src.getWidth();
    float d = *pLL;
    float c = *(--pLL);

    return fX1 * (fY1*a + fY0*c) + fX0 * (fY1*b + fY0*d);
  }

  // C++ fallback: iterates dst ROI, indexes warp map via warpOffset
  template<class T>
  struct WarpImpl {
    static void apply(const Img<T> &src, Img<T> &dst,
                      const Channel32f warpMap[2],
                      const Point &warpOffset,
                      scalemode mode) {
      const int wmW = warpMap[0].getWidth();
      const Rect dstROI = dst.getROI();
      auto interpolator = (mode == interpolateNN)
        ? interpolate_pixel_nn<T>
        : interpolate_pixel_lin<T>;

      for(int c = 0; c < src.getChannels(); ++c) {
        const Channel<T> s = src[c];
        Channel<T> d = dst[c];

        for(int y = dstROI.y; y < dstROI.y + dstROI.height; ++y) {
          const int wmy = y + warpOffset.y;
          const icl32f *wmXRow = warpMap[0].begin() + wmy * wmW + dstROI.x + warpOffset.x;
          const icl32f *wmYRow = warpMap[1].begin() + wmy * wmW + dstROI.x + warpOffset.x;
          T *dstRow = &d(dstROI.x, y);

          for(int i = 0; i < dstROI.width; ++i) {
            dstRow[i] = interpolator(wmXRow[i], wmYRow[i], s);
          }
        }
      }
    }
  };

  void cpp_warp(const Image& src, Image& dst, const Channel32f* cwm,
                Point warpOffset, scalemode mode) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      WarpImpl<T>::apply(s, d, cwm, warpOffset, mode);
    });
  }

  // Direct registration into the class prototype
  static int _reg = [] {
    using Op = WOp::Op;
    auto cpp = WOp::prototype().backends(Backend::Cpp);
    cpp.add<WOp::WarpSig>(Op::warp, cpp_warp, "C++ warp");
    return 0;
  }();

} // anon namespace
