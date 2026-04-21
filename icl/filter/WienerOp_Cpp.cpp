#include <icl/filter/WienerOp.h>
#include <icl/core/Image.h>
#include <icl/core/Img.h>
#include <icl/utils/ClippedCast.h>
#include <algorithm>
#include <cmath>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using WOp = filter::WienerOp;

  /// Adaptive Wiener filter — classic per-pixel formulation.
  ///
  /// For each destination pixel, compute the local mean μ and variance σ²
  /// over a maskSize×maskSize window of src. Then:
  ///     dst = μ + max(0, σ² − noise) / max(σ², noise) · (src − μ)
  ///
  /// Where local variance is dominated by noise (σ² ≤ noise), the pixel is
  /// replaced by the local mean (smooth flat regions). Where real signal
  /// dominates (σ² ≫ noise), the output stays near src (preserves edges).
  ///
  /// Optimization: use integral images for both values and squared values —
  /// O(W·H) independent of mask size. IntegralImgOp itself is used only for
  /// the values-only case; since Wiener needs both I and IQ, it's cheaper to
  /// build them in a single sweep than to compose two IntegralImgOp calls
  /// (which would require a scratch squared-src buffer).
  ///
  /// NeighborhoodOp shrinks dst.ROI by the mask size, so the src window is
  /// guaranteed to fit — no border-replicate handling needed.

  /// Build sum and sum-of-squares integral images for one channel of `src`.
  /// `I` and `IQ` are padded by one row/col of zeros on top/left so that the
  /// sum over rect [x1,x2)×[y1,y2) is I(x2,y2)−I(x1,y2)−I(x2,y1)+I(x1,y1).
  template<class T>
  static void build_integrals(const Img<T> &src, int channel,
                               std::vector<double> &I, std::vector<double> &IQ,
                               int IW, int IH) {
    const int W = src.getWidth();
    const T *sd = src.getData(channel);
    // Row 0 and column 0 stay zero (pre-cleared by caller).
    for(int y = 1; y < IH; ++y){
      double rowSum = 0, rowSqSum = 0;
      const T *srow    = sd + (y - 1) * W;
      double *Irow     = I.data()  + y       * IW;
      double *IQrow    = IQ.data() + y       * IW;
      const double *Ip = I.data()  + (y - 1) * IW;
      const double *Qp = IQ.data() + (y - 1) * IW;
      for(int x = 1; x < IW; ++x){
        const double v = double(srow[x - 1]);
        rowSum   += v;
        rowSqSum += v * v;
        Irow[x]   = Ip[x] + rowSum;
        IQrow[x]  = Qp[x] + rowSqSum;
      }
    }
  }

  template<class T>
  static void wiener_channel(const Img<T> &src, Img<T> &dst,
                              const std::vector<double> &I,
                              const std::vector<double> &IQ, int IW,
                              const Size &maskSize, const Point &anchor,
                              const Point &roiOffset, icl32f noise, int c) {
    const Size dstSize = dst.getROISize();
    const int kw = maskSize.width;
    const int kh = maskSize.height;
    const int ax = anchor.x;
    const int ay = anchor.y;
    const int srcStride = src.getWidth();
    const double invK   = 1.0 / double(kw * kh);
    const double noiseD = double(noise);

    const T *sd = src.getData(c);
    T *dd = dst.getROIData(c);
    const int dstLineStep = dst.getLineStep() / int(sizeof(T));

    for(int y = 0; y < dstSize.height; ++y){
      T *drow = dd + y * dstLineStep;
      for(int x = 0; x < dstSize.width; ++x){
        // Window in src coords: [x1,x2)×[y1,y2), center = (sx,sy).
        const int sx = roiOffset.x + x;
        const int sy = roiOffset.y + y;
        const int x1 = sx - ax;
        const int y1 = sy - ay;
        const int x2 = x1 + kw;
        const int y2 = y1 + kh;
        // 4-corner integral lookup. Indices are into the padded grid, which
        // is why the +1 offsets on x1/y1 aren't needed here.
        const double sum   = I[y2 * IW + x2]  - I[y1 * IW + x2]
                           - I[y2 * IW + x1]  + I[y1 * IW + x1];
        const double sqsum = IQ[y2 * IW + x2] - IQ[y1 * IW + x2]
                           - IQ[y2 * IW + x1] + IQ[y1 * IW + x1];

        const double mean = sum * invK;
        const double var  = std::max(0.0, sqsum * invK - mean * mean);
        const double srcV = double(sd[sy * srcStride + sx]);
        const double adj  = std::max(0.0, var - noiseD) /
                            std::max(var, noiseD);
        drow[x] = clipped_cast<double, T>(mean + adj * (srcV - mean));
      }
    }
  }

  static void cpp_wiener(const Image &src, Image &dst, const Size &maskSize,
                         const Point &anchor, const Point &roiOffset,
                         icl32f noise) {
    src.visitWith(dst, [&](const auto &s, auto &d){
      using T = typename std::remove_reference_t<decltype(s)>::type;
      const int IW = s.getWidth()  + 1;
      const int IH = s.getHeight() + 1;
      // One allocation per apply, reused across channels.
      std::vector<double> I(IW * IH, 0.0);
      std::vector<double> IQ(IW * IH, 0.0);
      for(int c = s.getChannels() - 1; c >= 0; --c){
        build_integrals<T>(s, c, I, IQ, IW, IH);
        wiener_channel<T>(s, d, I, IQ, IW, maskSize, anchor, roiOffset, noise, c);
      }
    });
  }

  static int _reg = [] {
    using Op = WOp::Op;
    auto cpp = WOp::prototype().backends(Backend::Cpp);
    // Match the IPP backend's depth coverage (8u/16s/32f). Other depths
    // go through visitWith but wiener_channel itself is template-any-depth,
    // so the dispatch filter is applicableTo<icl8u, icl16s, icl32f>.
    cpp.add<WOp::WienerSig>(Op::apply, cpp_wiener,
                             applicableTo<icl8u, icl16s, icl32f>,
                             "C++ adaptive Wiener filter (8u/16s/32f)");
    return 0;
  }();

} // anonymous namespace
