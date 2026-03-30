#include <ICLFilter/AffineOp.h>
#include <ICLCore/Image.h>
#include <ICLCore/Img.h>
#include <ICLMath/FixedMatrix.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace {

  using AOp = filter::AffineOp;

  // C++ fallback: inverse-map destination pixels to source via inverse matrix
  void cpp_affine(const Image &src, Image &dst, const double* fwd, scalemode interp) {
    // Compute inverse of the 2x3 forward transform (extended to 3x3)
    FixedMatrix<double,3,3> M(fwd[0], fwd[1], fwd[2],
                               fwd[3], fwd[4], fwd[5],
                               0, 0, 1);
    M = M.inv();
    double inv[3][3];
    for(int i = 0; i < 3; ++i)
      for(int j = 0; j < 3; ++j)
        inv[i][j] = M(i,j);

    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      Rect dr = d.getROI();
      int sx = dr.x, sy = dr.y, ex = dr.right(), ey = dr.bottom();
      Rect r = s.getROI();

      for(int ch = 0; ch < s.getChannels(); ch++) {
        const Channel<T> srcCh = s[ch];
        Channel<T> dstCh = d[ch];

        if(interp == interpolateLIN){
          for(int x = sx; x < ex; ++x){
            for(int y = sy; y < ey; ++y){
              float x2 = inv[0][0]*x + inv[1][0]*y + inv[2][0];
              float y2 = inv[0][1]*x + inv[1][1]*y + inv[2][1];
              int x3 = round(x2);
              int y3 = round(y2);
              if(r.contains(x3,y3)){
                dstCh(x,y) = s.subPixelLIN(x2,y2,ch);
              }else{
                dstCh(x,y) = 0;
              }
            }
          }
        }else{
          for(int x = sx; x < ex; ++x){
            for(int y = sy; y < ey; ++y){
              float x2 = inv[0][0]*x + inv[1][0]*y + inv[2][0];
              float y2 = inv[0][1]*x + inv[1][1]*y + inv[2][1];
              int x3 = round(x2);
              int y3 = round(y2);
              if(r.contains(x3,y3)){
                dstCh(x,y) = srcCh(x3,y3);
              }else{
                dstCh(x,y) = 0;
              }
            }
          }
        }
      }
    });
  }

  static int _reg = [] {
    using Op = AOp::Op;
    auto& proto = AOp::prototype();
    proto.addBackend<AOp::AffineSig>(Op::apply, Backend::Cpp, cpp_affine, "C++ affine warp");
    return 0;
  }();

} // anonymous namespace
