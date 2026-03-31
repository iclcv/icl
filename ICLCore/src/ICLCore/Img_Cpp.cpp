#include <ICLCore/ImgOps.h>
#include <ICLCore/Img.h>
#include <ICLUtils/ClippedCast.h>
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
    return 0;
  }();

} // anonymous namespace
