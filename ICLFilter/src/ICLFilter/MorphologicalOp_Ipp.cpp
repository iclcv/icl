#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLFilter/MorphologicalOp.h>

// ippiMorphologyInitAlloc/ippiDilate/ippiErode_*_C1R removed from modern IPP.
// Replaced by ippiDilate_*_C1R_L / ippiMorphInit_* (new spec-based API).
// TODO: update to modern morphology API with spec buffers.
#if 0 // was: ICL_HAVE_IPP — old morphology APIs removed

using namespace icl;
using namespace icl::utils;
using namespace icl::core;
using MOp = filter::MorphologicalOp;

namespace {

  // ================================================================
  // IPP call patterns (4 patterns as member function templates)
  // ================================================================

  // Pattern 1: standard morphological ops with mask
  template<typename T, IppStatus (IPP_DECL *ippiFunc)(const T*, int, T*, int, IppiSize, const Ipp8u*, IppiSize, IppiPoint)>
  IppStatus morphCall(const Img<T> &src, Img<T> &dst, const Point &roiOffset,
                       const icl8u *mask, const Size &maskSize, const Point &anchor) {
    for(int c = 0; c < src.getChannels(); c++) {
      IppStatus s = ippiFunc(src.getROIData(c, roiOffset), src.getLineStep(),
                              dst.getROIData(c), dst.getLineStep(),
                              dst.getROISize(), mask, maskSize, anchor);
      if(s != ippStsNoErr) return s;
    }
    return ippStsNoErr;
  }

  // Pattern 2: optimized 3x3 ops
  template<typename T, IppStatus (IPP_DECL *ippiFunc)(const T*, int, T*, int, IppiSize)>
  IppStatus morphCall3x3(const Img<T> &src, Img<T> &dst, const Point &roiOffset) {
    for(int c = 0; c < src.getChannels(); c++) {
      IppStatus s = ippiFunc(src.getROIData(c, roiOffset), src.getLineStep(),
                              dst.getROIData(c), dst.getLineStep(),
                              dst.getROISize());
      if(s != ippStsNoErr) return s;
    }
    return ippStsNoErr;
  }

  // Pattern 3: border replicate ops with IppiMorphState
  template<typename T, IppStatus (IPP_DECL *ippiFunc)(const T*, int, T*, int, IppiSize, _IppiBorderType, IppiMorphState*)>
  IppStatus morphBorderReplicateCall(const Img<T> &src, Img<T> &dst, const Point &roiOffset,
                                      IppiMorphState *state) {
    for(int c = 0; c < src.getChannels(); c++) {
      IppStatus s = ippiFunc(src.getROIData(c, roiOffset), src.getLineStep(),
                              dst.getROIData(c), dst.getLineStep(),
                              dst.getROISize(), ippBorderRepl, state);
      if(s != ippStsNoErr) return s;
    }
    return ippStsNoErr;
  }

  // Pattern 4: advanced border ops with IppiMorphAdvState
  template<typename T, IppStatus (IPP_DECL *ippiFunc)(const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
  IppStatus morphBorderCall(const Img<T> &src, Img<T> &dst, const Point &roiOffset,
                             IppiMorphAdvState *advState) {
    for(int c = 0; c < src.getChannels(); c++) {
      IppStatus s = ippiFunc(src.getROIData(c, roiOffset), src.getLineStep(),
                              dst.getROIData(c), dst.getLineStep(),
                              dst.getROISize(), ippBorderRepl, advState);
      if(s != ippStsNoErr) return s;
    }
    return ippStsNoErr;
  }

  // ================================================================
  // Stateful IPP backend — manages IPP state objects per instance
  // ================================================================

  struct MorphIppState {
    IppiMorphState *pState8u = nullptr;
    IppiMorphState *pState32f = nullptr;
    IppiMorphAdvState *pAdvState8u = nullptr;
    IppiMorphAdvState *pAdvState32f = nullptr;
    unsigned cachedVersion = 0;

    ~MorphIppState() { freeAll(); }

    void freeAll() {
      if(pState8u)    { ippiMorphologyFree(pState8u);  pState8u = nullptr; }
      if(pState32f)   { ippiMorphologyFree(pState32f); pState32f = nullptr; }
      if(pAdvState8u) { ippiMorphAdvFree(pAdvState8u); pAdvState8u = nullptr; }
      if(pAdvState32f){ ippiMorphAdvFree(pAdvState32f);pAdvState32f = nullptr; }
    }

    void invalidateIfNeeded(unsigned currentVersion) {
      if(cachedVersion != currentVersion) {
        freeAll();
        cachedVersion = currentVersion;
      }
    }

    void ensureState8u(const Size &roiSize, const icl8u *mask, const Size &maskSize, const Point &anchor) {
      if(!pState8u) {
        ippiMorphologyInitAlloc_8u_C1R(roiSize.width, mask, maskSize, anchor, &pState8u);
      }
    }
    void ensureState32f(const Size &roiSize, const icl8u *mask, const Size &maskSize, const Point &anchor) {
      if(!pState32f) {
        ippiMorphologyInitAlloc_32f_C1R(roiSize.width, mask, maskSize, anchor, &pState32f);
      }
    }
    void ensureAdvState8u(const Size &roiSize, const icl8u *mask, const Size &maskSize, const Point &anchor) {
      if(!pAdvState8u) {
        ippiMorphAdvInitAlloc_8u_C1R(&pAdvState8u, roiSize, mask, maskSize, anchor);
      }
    }
    void ensureAdvState32f(const Size &roiSize, const icl8u *mask, const Size &maskSize, const Point &anchor) {
      if(!pAdvState32f) {
        ippiMorphAdvInitAlloc_32f_C1R(&pAdvState32f, roiSize, mask, maskSize, anchor);
      }
    }

    // ================================================================
    // Main dispatch — depth × optype
    // ================================================================

    void apply(const Image &src, Image &dst, MOp &op) {
      invalidateIfNeeded(op.maskVersion());

      const icl8u *mask = op.getMask();
      const Size maskSize = op.getMaskSize();
      const Point anchor = op.getAnchor();
      const Point roiOffset = op.getROIOffset();
      const auto ot = op.getOptype();

      IppStatus s = ippStsNoErr;
      switch(src.getDepth()) {
        case depth8u: {
          const Img8u &ps = src.as8u();
          Img8u &pd = dst.as8u();
          switch(ot) {
            case MOp::dilate:    s = morphCall<icl8u, ippiDilate_8u_C1R>(ps, pd, roiOffset, mask, maskSize, anchor); break;
            case MOp::erode:     s = morphCall<icl8u, ippiErode_8u_C1R>(ps, pd, roiOffset, mask, maskSize, anchor); break;
            case MOp::dilate3x3: s = morphCall3x3<icl8u, ippiDilate3x3_8u_C1R>(ps, pd, roiOffset); break;
            case MOp::erode3x3:  s = morphCall3x3<icl8u, ippiErode3x3_8u_C1R>(ps, pd, roiOffset); break;
            case MOp::dilateBorderReplicate:
              ensureState8u(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderReplicateCall<icl8u, ippiDilateBorderReplicate_8u_C1R>(ps, pd, roiOffset, pState8u); break;
            case MOp::erodeBorderReplicate:
              ensureState8u(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderReplicateCall<icl8u, ippiErodeBorderReplicate_8u_C1R>(ps, pd, roiOffset, pState8u); break;
            case MOp::openBorder:
              ensureAdvState8u(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderCall<icl8u, ippiMorphOpenBorder_8u_C1R>(ps, pd, roiOffset, pAdvState8u); break;
            case MOp::closeBorder:
              ensureAdvState8u(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderCall<icl8u, ippiMorphCloseBorder_8u_C1R>(ps, pd, roiOffset, pAdvState8u); break;
            case MOp::tophatBorder:
              ensureAdvState8u(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderCall<icl8u, ippiMorphTophatBorder_8u_C1R>(ps, pd, roiOffset, pAdvState8u); break;
            case MOp::blackhatBorder:
              ensureAdvState8u(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderCall<icl8u, ippiMorphBlackhatBorder_8u_C1R>(ps, pd, roiOffset, pAdvState8u); break;
            case MOp::gradientBorder:
              ensureAdvState8u(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderCall<icl8u, ippiMorphGradientBorder_8u_C1R>(ps, pd, roiOffset, pAdvState8u); break;
          }
          break;
        }
        case depth32f: {
          const Img32f &ps = src.as32f();
          Img32f &pd = dst.as32f();
          switch(ot) {
            case MOp::dilate:    s = morphCall<icl32f, ippiDilate_32f_C1R>(ps, pd, roiOffset, mask, maskSize, anchor); break;
            case MOp::erode:     s = morphCall<icl32f, ippiErode_32f_C1R>(ps, pd, roiOffset, mask, maskSize, anchor); break;
            case MOp::dilate3x3: s = morphCall3x3<icl32f, ippiDilate3x3_32f_C1R>(ps, pd, roiOffset); break;
            case MOp::erode3x3:  s = morphCall3x3<icl32f, ippiErode3x3_32f_C1R>(ps, pd, roiOffset); break;
            case MOp::dilateBorderReplicate:
              ensureState32f(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderReplicateCall<icl32f, ippiDilateBorderReplicate_32f_C1R>(ps, pd, roiOffset, pState32f); break;
            case MOp::erodeBorderReplicate:
              ensureState32f(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderReplicateCall<icl32f, ippiErodeBorderReplicate_32f_C1R>(ps, pd, roiOffset, pState32f); break;
            case MOp::openBorder:
              ensureAdvState32f(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderCall<icl32f, ippiMorphOpenBorder_32f_C1R>(ps, pd, roiOffset, pAdvState32f); break;
            case MOp::closeBorder:
              ensureAdvState32f(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderCall<icl32f, ippiMorphCloseBorder_32f_C1R>(ps, pd, roiOffset, pAdvState32f); break;
            case MOp::tophatBorder:
              ensureAdvState32f(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderCall<icl32f, ippiMorphTophatBorder_32f_C1R>(ps, pd, roiOffset, pAdvState32f); break;
            case MOp::blackhatBorder:
              ensureAdvState32f(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderCall<icl32f, ippiMorphBlackhatBorder_32f_C1R>(ps, pd, roiOffset, pAdvState32f); break;
            case MOp::gradientBorder:
              ensureAdvState32f(ps.getROISize(), mask, maskSize, anchor);
              s = morphBorderCall<icl32f, ippiMorphGradientBorder_32f_C1R>(ps, pd, roiOffset, pAdvState32f); break;
          }
          break;
        }
        default: ICL_INVALID_DEPTH; break;
      }
      if(s != ippStsNoErr) {
        ERROR_LOG("IPP-Error: \"" << ippGetStatusString(s) << "\"");
      }
    }
  };

  // ================================================================
  // Registration as stateful backend
  // ================================================================

  static const int _r = ImageBackendDispatching::registerStatefulBackend<MOp::MorphSig>(
    "MorphologicalOp.apply", Backend::Ipp,
    []() {
      auto state = std::make_shared<MorphIppState>();
      return [state](const Image &src, Image &dst, MOp &op) {
        state->apply(src, dst, op);
      };
    },
    applicableTo<icl8u, icl32f>, "IPP morphological ops (8u/32f)");

} // anonymous namespace

#endif // ICL_HAVE_IPP
