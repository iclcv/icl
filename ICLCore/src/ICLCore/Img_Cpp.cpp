#include <ICLCore/ImgOps.h>
#include <ICLCore/Img.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLMath/MathFunctions.h>
#include <algorithm>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

// ================================================================
// Mirror helpers (moved from Img.cpp — only used by in-place mirror
// and flippedCopyChannelROI)
// ================================================================

namespace {

  static inline int getPointerOffset(int x, int y, int iLineLen) {
    return (x + y * iLineLen);
  }

  static bool getMirrorPointerOffsets(axis eAxis, bool bInplace,
                                      const Point& oSrcOffset, const int iSrcLineLen,
                                      const Point& oDstOffset, const int iDstLineLen, const Size& oSize,
                                      int& s, int& d, int& e, int& eLine, int& iLineWarpS, int& iLineWarpD) {
    int iRows = 0, iCols = 0;
    switch (eAxis) {
      case axisHorz:
        iRows = bInplace ? oSize.height / 2 : oSize.height;
        iCols = oSize.width;
        iLineWarpS = iSrcLineLen - iCols;
        iLineWarpD = -(iDstLineLen + iCols);
        s = getPointerOffset(oSrcOffset.x, oSrcOffset.y, iSrcLineLen);
        d = getPointerOffset(oDstOffset.x, oDstOffset.y + oSize.height - 1, iDstLineLen);
        e = s + iRows * iSrcLineLen;
        break;
      case axisVert:
        iRows = oSize.height;
        iCols = bInplace ? oSize.width / 2 : oSize.width;
        iLineWarpS = iSrcLineLen - iCols;
        iLineWarpD = iDstLineLen + iCols;
        s = getPointerOffset(oSrcOffset.x, oSrcOffset.y, iSrcLineLen);
        d = getPointerOffset(oDstOffset.x + oSize.width - 1, oDstOffset.y, iDstLineLen);
        e = s + iRows * iSrcLineLen;
        break;
      case axisBoth:
        iRows = bInplace ? oSize.height / 2 : oSize.height;
        iCols = oSize.width;
        iLineWarpS = iSrcLineLen - iCols;
        iLineWarpD = iCols - iDstLineLen;
        s = getPointerOffset(oSrcOffset.x, oSrcOffset.y, iSrcLineLen);
        d = getPointerOffset(oDstOffset.x + oSize.width - 1, oDstOffset.y + oSize.height - 1, iDstLineLen);
        e = s + iRows * iSrcLineLen;

        if (bInplace && (oSize.height % 2)) {
          iRows++;
          e += oSize.width / 2;
        }
        break;
    }
    eLine = s + iCols;
    return ((iRows != 0) && (iCols != 0));
  }

  template<typename Type>
  static inline bool getMirrorPointers(axis eAxis, bool bInplace,
                                       const Type* const srcBegin, const Point& oSrcOffset, const int iSrcLineLen,
                                       Type* const dstBegin, const Point& oDstOffset, const int iDstLineLen, const Size& oSize,
                                       const Type*& s, Type*& d, const Type*& e, const Type*& eLine, int& iLineWarpS, int& iLineWarpD) {
    int deltaSrc, deltaDst, deltaEnd, deltaLineEnd;
    if (!getMirrorPointerOffsets(eAxis, bInplace, oSrcOffset, iSrcLineLen, oDstOffset, iDstLineLen, oSize,
                                 deltaSrc, deltaDst, deltaEnd, deltaLineEnd, iLineWarpS, iLineWarpD))
      return false;
    s = srcBegin + deltaSrc;
    d = dstBegin + deltaDst;
    e = srcBegin + deltaEnd;
    eLine = srcBegin + deltaLineEnd;
    return true;
  }

  template<typename Type>
  static inline bool getMirrorPointers(axis eAxis, bool bInplace,
                                       Type* const srcBegin, const Point& oSrcOffset, const int iSrcLineLen,
                                       Type* const dstBegin, const Point& oDstOffset, const int iDstLineLen, const Size& oSize,
                                       Type*& s, Type*& d, const Type*& e, const Type*& eLine, int& iLineWarpS, int& iLineWarpD) {
    int deltaSrc, deltaDst, deltaEnd, deltaLineEnd;
    if (!getMirrorPointerOffsets(eAxis, bInplace, oSrcOffset, iSrcLineLen, oDstOffset, iDstLineLen, oSize,
                                 deltaSrc, deltaDst, deltaEnd, deltaLineEnd, iLineWarpS, iLineWarpD))
      return false;
    s = srcBegin + deltaSrc;
    d = dstBegin + deltaDst;
    e = srcBegin + deltaEnd;
    eLine = srcBegin + deltaLineEnd;
    return true;
  }

  template<class iterator>
  std::pair<iterator, iterator> get_min_and_max_element_util(iterator begin, iterator end) {
    std::pair<iterator, iterator> mm;
    if (begin == end) return mm;
    mm.first = begin;
    mm.second = begin;
    for (++begin; begin != end; ++begin) {
      if (*begin < *mm.first) mm.first = begin;
      if (*begin > *mm.second) mm.second = begin;
    }
    return mm;
  }

} // anonymous namespace


// ================================================================
// Img<T>::mirror(axis, int, Point, Size) — per-channel in-place swap
// Moved from Img.cpp so helpers stay in one TU.
// ================================================================

namespace icl { namespace core {

  template<class Type> void
  Img<Type>::mirror(axis eAxis, int iChannel,
                    const Point& oOffset, const Size& oSize) {
    FUNCTION_LOG("");

    static const int aiDstStep[] = {1, -1, -1};
    int iLineWarpS, iLineWarpD;
    const Type *e = 0, *eLine = 0;
    Type *s = 0, *d = 0;

    if (!getMirrorPointers(eAxis, true,
                           getData(iChannel), oOffset, getWidth(),
                           getData(iChannel), oOffset, getWidth(), oSize,
                           s, d, e, eLine, iLineWarpS, iLineWarpD)) return;

    int dir = aiDstStep[eAxis];
    do {
      std::swap(*s, *d);
      ++s; d += dir;
      if (s == eLine) {
        eLine += getWidth();
        s += iLineWarpS;
        d += iLineWarpD;
      }
    } while (s != e);
  }

#define ICL_INSTANTIATE_DEPTH(D) \
  template ICLCore_API void Img<icl##D>::mirror(axis, int, const Point&, const Size&);
  ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH

}} // namespace icl::core


// ================================================================
// Backend registrations
// ================================================================

namespace {

  // ---- mirror ----
  void cpp_mirror(ImgBase& img, axis a, bool roiOnly) {
    const Point& offset = roiOnly ? img.getROIOffset() : Point::null;
    const Size& size = roiOnly ? img.getROISize() : img.getSize();
    for(int c = 0; c < img.getChannels(); ++c) {
      switch(img.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
        case depth##D: img.asImg<icl##D>()->mirror(a, c, offset, size); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: break;
      }
    }
  }

  // ---- clearChannelROI ----
  void cpp_clearChannelROI(ImgBase& img, int ch, icl64f val,
                           const Point& offs, const Size& size) {
    switch(img.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
      case depth##D: { \
        auto* im = img.asImg<icl##D>(); \
        icl##D typedVal = static_cast<icl##D>(val); \
        ImgIterator<icl##D> it(im->getData(ch), im->getSize().width, Rect(offs, size)); \
        ImgIterator<icl##D> itEnd = ImgIterator<icl##D>::create_end_roi_iterator( \
          im->getData(ch), im->getWidth(), Rect(offs, size)); \
        std::fill(it, itEnd, typedVal); \
        break; \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: break;
    }
  }

  // ---- lut ----
  void cpp_lut(ImgBase& srcBase, const void* lutPtr, ImgBase& dstBase, int bits) {
    const int shift = 8 - bits;
    switch(srcBase.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
      case depth##D: { \
        auto& src = *srcBase.asImg<icl##D>(); \
        auto& dst = *dstBase.asImg<icl##D>(); \
        const icl##D* lut = static_cast<const icl##D*>(lutPtr); \
        for(int c = src.getChannels()-1; c >= 0; --c) { \
          auto itSrc = src.beginROI(c); \
          auto itSrcEnd = src.endROI(c); \
          auto itDst = dst.beginROI(c); \
          for(; itSrc != itSrcEnd; ++itSrc, ++itDst) { \
            *itDst = lut[(static_cast<int>(*itSrc)) >> shift]; \
          } \
        } \
        break; \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: break;
    }
  }

  // ---- getMax ----
  icl64f cpp_getMax(ImgBase& img, int ch, Point* coords) {
    switch(img.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
      case depth##D: { \
        auto& im = *img.asImg<icl##D>(); \
        if(im.hasFullROI()) { \
          auto it = std::max_element(im.begin(ch), im.end(ch)); \
          if(coords) *coords = im.getLocation(it, ch); \
          return static_cast<icl64f>(*it); \
        } else { \
          auto it = std::max_element(im.beginROI(ch), im.endROI(ch)); \
          if(coords) *coords = im.getLocation(&*it, ch); \
          return static_cast<icl64f>(*it); \
        } \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: return 0;
    }
  }

  // ---- getMin ----
  icl64f cpp_getMin(ImgBase& img, int ch, Point* coords) {
    switch(img.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
      case depth##D: { \
        auto& im = *img.asImg<icl##D>(); \
        if(im.hasFullROI()) { \
          auto it = std::min_element(im.begin(ch), im.end(ch)); \
          if(coords) *coords = im.getLocation(it, ch); \
          return static_cast<icl64f>(*it); \
        } else { \
          auto it = std::min_element(im.beginROI(ch), im.endROI(ch)); \
          if(coords) *coords = im.getLocation(&*it, ch); \
          return static_cast<icl64f>(*it); \
        } \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: return 0;
    }
  }

  // ---- getMinMax ----
  void cpp_getMinMax(ImgBase& img, int ch,
                     icl64f* minVal, icl64f* maxVal,
                     Point* minCoords, Point* maxCoords) {
    switch(img.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
      case depth##D: { \
        auto& im = *img.asImg<icl##D>(); \
        if(im.hasFullROI()) { \
          auto its = get_min_and_max_element_util(im.begin(ch), im.end(ch)); \
          if(minCoords) { \
            *minCoords = im.getLocation(its.first, ch); \
            *maxCoords = im.getLocation(its.second, ch); \
          } \
          *minVal = static_cast<icl64f>(*its.first); \
          *maxVal = static_cast<icl64f>(*its.second); \
        } else { \
          auto its = get_min_and_max_element_util(im.beginROI(ch), im.endROI(ch)); \
          if(minCoords) { \
            *minCoords = im.getLocation(&*its.first, ch); \
            *maxCoords = im.getLocation(&*its.second, ch); \
          } \
          *minVal = static_cast<icl64f>(*its.first); \
          *maxVal = static_cast<icl64f>(*its.second); \
        } \
        break; \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: *minVal = *maxVal = 0; break;
    }
  }

  // ---- normalize ----
  void cpp_normalize(ImgBase& img, int iChannel,
                     icl64f srcMin, icl64f srcMax,
                     icl64f dstMin, icl64f dstMax) {
    icl64f srcLen = srcMax - srcMin;
    icl64f fScale = (dstMax - dstMin) / srcLen;
    icl64f fShift = (srcMax * dstMin - srcMin * dstMax) / srcLen;
    int startC = iChannel < 0 ? 0 : iChannel;
    int endC = iChannel < 0 ? img.getChannels() : iChannel + 1;
    switch(img.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
      case depth##D: { \
        auto& im = *img.asImg<icl##D>(); \
        for(int c = startC; c < endC; ++c) { \
          auto e = im.endROI(c); \
          for(auto p = im.beginROI(c); p != e; ++p) { \
            *p = clipped_cast<icl64f, icl##D>( \
              icl::utils::clip(fShift + static_cast<icl64f>(*p) * fScale, dstMin, dstMax)); \
          } \
        } \
        break; \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: break;
    }
  }

  // ---- flippedCopyChannelROI ----
  void cpp_flippedCopy(axis eAxis,
                       ImgBase& srcBase, int srcC, const Point& srcOffs, const Size& srcSize,
                       ImgBase& dstBase, int dstC, const Point& dstOffs, const Size& dstSize) {
    static const int aiDstStep[] = {1, -1, -1};

    switch(srcBase.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
      case depth##D: { \
        using T = icl##D; \
        auto* src = srcBase.asImg<T>(); \
        auto* dst = dstBase.asImg<T>(); \
        int iLineWarpS, iLineWarpD; \
        const T *s = 0, *e = 0, *eLine = 0; \
        T *d = 0; \
        if (!getMirrorPointers(eAxis, false, \
                               src->getData(srcC), srcOffs, src->getWidth(), \
                               dst->getData(dstC), dstOffs, dst->getWidth(), srcSize, \
                               s, d, e, eLine, iLineWarpS, iLineWarpD)) return; \
        if (eAxis == axisHorz) { \
          int iSrcStep = src->getSize().width, iDstStep = dst->getSize().width; \
          for (; s != e; s += iSrcStep, d -= iDstStep) \
            icl::core::copy<T>(s, s + srcSize.width, d); \
          return; \
        } \
        int dir = aiDstStep[eAxis]; \
        do { \
          *d = *s; \
          ++s; d += dir; \
          if (s == eLine) { \
            eLine += src->getSize().width; \
            s += iLineWarpS; \
            d += iLineWarpD; \
          } \
        } while (s != e); \
        break; \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: break;
    }
  }


  // ---- Scaled copy (C++ fallback — NN, bilinear, region average) ----

  template<class T>
  void cpp_scaledCopyChannel(const Img<T> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                              Img<T> *dst, int dstC, const Point &dstOffs, const Size &dstSize,
                              scalemode eScaleMode) {
    float fSX = float(srcSize.width) / float(dstSize.width);
    float fSY = float(srcSize.height) / float(dstSize.height);

    switch(eScaleMode) {
      case interpolateNN: {
        const T *d = src->getData(srcC);
        const unsigned int w = src->getWidth();
        ImgIterator<T> itDst(dst->getData(dstC),dst->getSize().width,Rect(dstOffs,dstSize));
        const ImgIterator<T> itDstEnd = ImgIterator<T>::create_end_roi_iterator(dst->getData(dstC),dst->getWidth(),Rect(dstOffs,dstSize));
        int xD = 0, yD = 0;
        float yS = srcOffs.y + fSY * yD;
        for(; itDst != itDstEnd; ++itDst) {
          *itDst = clipped_cast<float, T>(*(d + int(srcOffs.x + fSX * xD) + int(yS) * w));
          if(++xD == dstSize.width) { yS = srcOffs.y + fSY * ++yD; xD = 0; }
        }
        return;
      }
      case interpolateLIN: {
        fSX = dstSize.width  > 1 ? (float(srcSize.width)  - 1) / float(dstSize.width  - 1) : 0.0f;
        fSY = dstSize.height > 1 ? (float(srcSize.height) - 1) / float(dstSize.height - 1) : 0.0f;
        const T *d = src->getData(srcC);
        const unsigned int w = src->getWidth();
        const int maxX = srcOffs.x + srcSize.width  - 1;
        const int maxY = srcOffs.y + srcSize.height - 1;
        ImgIterator<T> itDst(dst->getData(dstC),dst->getSize().width,Rect(dstOffs,dstSize));
        const ImgIterator<T> itDstEnd = ImgIterator<T>::create_end_roi_iterator(dst->getData(dstC),dst->getWidth(),Rect(dstOffs,dstSize));
        int xD = 0, yD = 0;
        float yS = srcOffs.y + fSY * yD;
        for(; itDst != itDstEnd; ++itDst) {
          float xS = srcOffs.x + fSX * xD;
          int x0 = int(xS);
          int y0 = int(yS);
          int x1 = std::min(x0 + 1, maxX);
          int y1 = std::min(y0 + 1, maxY);
          float fX0 = xS - x0, fX1 = 1.0f - fX0;
          float fY0 = yS - y0, fY1 = 1.0f - fY0;
          float a = d[x0 + y0 * w];
          float b = d[x1 + y0 * w];
          float c = d[x0 + y1 * w];
          float dd = d[x1 + y1 * w];
          *itDst = clipped_cast<float, T>(fX1 * (fY1*a + fY0*c) + fX0 * (fY1*b + fY0*dd));
          if(++xD == dstSize.width) { yS = srcOffs.y + fSY * ++yD; xD = 0; }
        }
        return;
      }
      case interpolateRA: {
        float ratio = 1.0f / (fSX * fSY);
        const T *d = src->getData(srcC);
        const unsigned int w = src->getWidth();
        std::vector<unsigned int> xBegin(dstSize.width), xEnd(dstSize.width);
        std::vector<unsigned int> yBegin(dstSize.height), yEnd(dstSize.height);
        std::vector<float> xBMul(dstSize.width), xEMul(dstSize.width);
        std::vector<float> yBMul(dstSize.height), yEMul(dstSize.height);
        for(int i = 0; i < dstSize.width; ++i) {
          float b = srcOffs.x + i*fSX;
          xBegin[i] = b; xBMul[i] = 1.0f - (b - xBegin[i]);
          xEnd[i] = ceilf((b+fSX)-1); xEMul[i] = 1.0f - (xEnd[i] - (b+fSX-1));
        }
        for(int i = 0; i < dstSize.height; ++i) {
          float e = srcOffs.y + i*fSY;
          yBegin[i] = e; yBMul[i] = 1.0f - (e - yBegin[i]);
          yEnd[i] = ceilf((e+fSY)-1); yEMul[i] = 1.0f - (yEnd[i] - (e+fSY-1));
        }
        ImgIterator<T> itDst(dst->getData(dstC),dst->getSize().width,Rect(dstOffs,dstSize));
        const ImgIterator<T> itDstEnd = ImgIterator<T>::create_end_roi_iterator(dst->getData(dstC),dst->getWidth(),Rect(dstOffs,dstSize));
        int xD = 0, yD = 0;
        for(; itDst != itDstEnd; ++itDst) {
          float sum = 0.0f;
          unsigned int xB = xBegin[xD], xE = xEnd[xD];
          unsigned int yB = yBegin[yD], yE = yEnd[yD];
          sum += (*(d + xB + yB*w)) * xBMul[xD];
          for(unsigned int x = xB+1; x < xE; ++x) sum += *(d + x + yB*w);
          sum = (sum + (*(d + xE + yB*w)) * xEMul[xD]) * yBMul[yD];
          for(unsigned int y = yB+1; y < yE; ++y) {
            sum += (*(d + xB + y*w)) * xBMul[xD];
            for(unsigned int x = xB+1; x < xE; ++x) sum += *(d + x + y*w);
            sum += (*(d + xE + y*w)) * xEMul[xD];
          }
          float psum = (*(d + xB + yE*w)) * xBMul[xD];
          for(unsigned int x = xB+1; x < xE; ++x) psum += *(d + x + yE*w);
          sum += (psum + (*(d + xE + yE*w)) * xEMul[xD]) * yEMul[yD];
          *itDst = clipped_cast<float, T>(sum * ratio + 0.5f);
          if(++xD == dstSize.width) { ++yD; xD = 0; }
        }
        return;
      }
      default: break;
    }
  }

  void cpp_scaledCopy(const ImgBase& src, int srcC,
                      const Point& srcOffs, const Size& srcSize,
                      ImgBase& dst, int dstC,
                      const Point& dstOffs, const Size& dstSize,
                      scalemode mode) {
    switch(src.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
      case depth##D: \
        cpp_scaledCopyChannel(src.asImg<icl##D>(), srcC, srcOffs, srcSize, \
                              dst.asImg<icl##D>(), dstC, dstOffs, dstSize, mode); \
        break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: break;
    }
  }

  // ---- Direct registration into ImgOps singleton (no global registry) ----

  static int _reg = [] {
    using Op = ImgOps::Op;
    auto cpp = ImgOps::instance().backends(Backend::Cpp);
    cpp.add<ImgOps::MirrorSig>(Op::mirror, cpp_mirror, "C++ swap-based mirror");
    cpp.add<ImgOps::ClearChannelROISig>(Op::clearChannelROI, cpp_clearChannelROI, "C++ std::fill clear");
    cpp.add<ImgOps::LutSig>(Op::lut, cpp_lut, "C++ per-pixel LUT");
    cpp.add<ImgOps::GetMaxSig>(Op::getMax, cpp_getMax, "C++ std::max_element");
    cpp.add<ImgOps::GetMinSig>(Op::getMin, cpp_getMin, "C++ std::min_element");
    cpp.add<ImgOps::GetMinMaxSig>(Op::getMinMax, cpp_getMinMax, "C++ min/max element scan");
    cpp.add<ImgOps::NormalizeSig>(Op::normalize, cpp_normalize, "C++ scale+shift normalize");
    cpp.add<ImgOps::FlippedCopySig>(Op::flippedCopy, cpp_flippedCopy, "C++ pointer-walk flipped copy");
    cpp.add<ImgOps::ChannelMeanSig>(Op::channelMean, [](ImgBase& img, int channel, bool roiOnly) -> icl64f {
      switch(img.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
        case depth##D: { \
          auto& im = *img.asImg<icl##D>(); \
          if(roiOnly && !im.hasFullROI()) \
            return math::mean(im.beginROI(channel), im.endROI(channel)); \
          else \
            return math::mean(im.begin(channel), im.end(channel)); \
        }
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: return 0;
      }
    }, "C++ math::mean iterator");
    cpp.add<ImgOps::ScaledCopySig>(Op::scaledCopy, cpp_scaledCopy, "C++ NN/bilinear/region-average");
    return 0;
  }();

} // anonymous namespace
