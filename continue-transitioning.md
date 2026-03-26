# Image Migration — Continuation Guide

## Current State (Session 8)

### Architecture

```
Public API:  Image (value type, shared_ptr<ImgBase>)
                |
Internal:    ImgBase → Img<T>  (implementation detail)
```

### SIMD Status

SSE2 intrinsics are transparently mapped to ARM NEON via `sse2neon` (3rdparty).
`ICL_HAVE_SSE2` is effectively "have SIMD" on both x86 and Apple Silicon.
IPP is x86-only (absent on ARM) — filters fall back to C++ on Apple Silicon.
No runtime dispatcher needed; compile-time `#ifdef` is the right granularity.

### Line-Based ROI Visitors (NEW)

`Visitors.h` and `VisitorsN.h` in ICLCore replace ImgIterator per-pixel loops
with tight line-pointer inner loops. All visitors have a contiguous fast-path:
when `roiWidth == imageWidth`, the callback gets `w * h` elements in one call.

```
Visitors.h  (lightweight, no extra headers beyond Img.h):
  visitROILines(img, ch, f)                        — f(T*, width)
  visitROILinesWith(src, sCh, dst, dCh, f)         — f(const S*, D*, width)
  visitROILinesPerChannel(img, f)                   — f(T*, ch, width)
  visitROILinesPerChannelWith(src, dst, f)          — f(const S*, D*, ch, width)

VisitorsN.h (includes Visitors.h + <array> + <utility>):
  visitROILinesN<N>(img, f)                         — f(const T* ch0, ..., width)
  visitROILinesNWith<N,M>(src, dst, f)              — f(const S* s0, ..., D* d0, ..., width)
```

**All migrated filters now use these** instead of ImgIterator. This fixed pre-existing
ROI bugs in SSE2/NEON specializations (ThresholdOp, UnaryArithmeticalOp) that were
treating ROI data as contiguous when it wasn't.

Image.h `visitChannel`/`visitChannels`/`visitChannelWith`/`visitChannelsWith` were
removed (zero usage, ROI-broken). Image.h now points to Visitors.h in a comment.

**TODO for BinaryOp**: will need a `visitROILines2With(src1, src2, dst, f)` variant
for two-source + one-dst patterns. Add to Visitors.h when BinaryOp migration starts.

### UnaryOp Virtual Interface

```cpp
class UnaryOp {
    // Pure virtual — all filters implement this
    virtual void apply(const Image &src, Image &dst) = 0;

    // Legacy wrapper (final, cannot be overridden) — delegates to Image apply
    virtual void apply(const ImgBase *src, ImgBase **dst) final;

    // Single-arg convenience — returns ref to internal Image buffer
    const Image& apply(const Image &src);

    // Image-based prepare() overloads
    bool prepare(Image &dst, depth d, const Size &s, format fmt, int ch, const Rect &roi, Time t);
    bool prepare(Image &dst, const Image &src);
    bool prepare(Image &dst, const Image &src, depth d);
};
```

### BinaryOp Virtual Interface

Same pattern as UnaryOp but with two inputs:
```cpp
class BinaryOp {
    virtual void apply(const Image &src1, const Image &src2, Image &dst);  // primary (has default)
    virtual void apply(const ImgBase *a, const ImgBase *b, ImgBase **dst); // legacy (has default)
};
```
Note: BinaryOp's Image apply has a DEFAULT implementation (not pure virtual) that
delegates to the ImgBase version. BinaryOp filters have NOT been migrated yet — they
still override the ImgBase version.

TODO: Make BinaryOp::apply(Image) pure virtual + final on ImgBase version, same as UnaryOp.

### NeighborhoodOp Image-based prepare (NEW)

NeighborhoodOp now has Image-based `prepare()` methods that compute the shrunk ROI
(accounting for mask size), then delegate to `UnaryOp::prepare(Image, ...)`.
This is required for migrating any NeighborhoodOp subclass (WienerOp, MedianOp,
ConvolutionOp, MorphologicalOp, LocalThresholdOp, etc.).

```cpp
bool NeighborhoodOp::prepare(Image &dst, const Image &src);
bool NeighborhoodOp::prepare(Image &dst, const Image &src, depth d);
```

### Fully Native Image Filters (17 done)

These override `apply(const Image&, Image&)` directly, no `applyImgBase` bridge:

1. **DitheringOp** — fixed depth (8u), uses `prepare()` + `convertTo()` + `as8u()`
2. **MirrorOp** — same depth, uses `prepare()` + `visitWith()` for typed dispatch
3. **ThresholdOp** — same depth, dispatch structs, all paths use `visitROILinesPerChannelWith`
4. **UnaryArithmeticalOp** — same depth, `LoopFunc` structs, all paths use `visitROILinesPerChannelWith`
5. **UnaryCompareOp** — output always 8u, `CmpImpl` struct, uses `visitROILinesPerChannelWith`
6. **WeightChannelsOp** — same depth, per-channel multiply via `visitROILinesPerChannelWith`
7. **WeightedSumOp** — output 32f/64f, 1 channel, `visit()` + typed dst
8. **ColorDistanceOp** — output 8u/32f/64f, `visitROILinesNWith<3,1>`, all math in double
9. **UnaryLogicalOp** — integer depths only, `if constexpr` guard, `visitROILinesPerChannelWith`
10. **LUTOp** — always 8u, converts non-8u input via buffer, IPP `reduceBits` untouched
11. **LUTOp3Channel** — template class, `visit()` for src depth, output typed by class param
12. **IntegralImgOp** — output 32s/32f/64f, full-image sequential algorithm, IPP disabled
13. **WienerOp** — IPP-only (8u/16s/32f), `WienerImpl` dispatch struct, NeighborhoodOp::prepare
14. **GaborOp** — composition filter (wraps ConvolutionOp), `m_vecResults` now `vector<Image>`
15. **ColorSegmentationOp** — depth8u only, `visitROILinesNWith<3,1>`, compile-time shift templates
16. **ChamferOp** — output depth32s, `visitROILinesWith` for init, multi-pass distance propagation
17. **AffineOp** — `AffineImpl` dispatch struct, IPP 8u/32f, C++ fallback with inverse matrix

### Filters with applyImgBase Bridge (11 remaining)

**With IPP acceleration (5):**
CannyOp, ConvolutionOp, LocalThresholdOp, MedianOp,
MorphologicalOp, WarpOp

**Pure C++ (4):**
BilateralFilterOp,
FFTOp, IFFTOp, MotionSensitiveTemporalSmoothing

### BinaryOp Filters (not started)

BinaryArithmeticalOp, BinaryCompareOp, BinaryLogicalOp — still override
the ImgBase version. All three have IPP specializations.
Need same treatment as UnaryOp filters.

## Migration Pattern — Dispatch Struct Approach

The established pattern uses **template structs with specializations** to replace
the `#ifdef` macro jungle. This was validated on ThresholdOp (most complex case)
and UnaryCompareOp (different output depth).

### Structure of a migrated filter .cpp:

```cpp
#include <ICLCore/Visitors.h>  // or VisitorsN.h for multi-channel

// 1. C++ fallback — uses visitROILinesPerChannelWith
template<class T> struct OpImpl {
  static void apply(const Img<T> &src, Img<T> &dst, ...) {
    visitROILinesPerChannelWith(src, dst, [...](const T *s, T *d, int, int w) {
      for(int i = 0; i < w; ++i) d[i] = /* operation */;
    });
  }
};

// 2. SSE2/NEON overrides — same visitor, SIMD inner loop
#ifdef ICL_HAVE_SSE2
template<> struct OpImpl<icl32f> {
  static void apply(const Img32f &src, Img32f &dst, ...) {
    visitROILinesPerChannelWith(src, dst, [...](const icl32f *s, icl32f *d, int, int w) {
      int i = 0;
      for(; i <= w-4; i += 4) { /* _mm_xxx_ps */ }
      for(; i < w; ++i) { /* scalar tail */ }
    });
  }
};
#endif

// 3. IPP overrides — use getROIData/getLineStep/getROISize (IPP handles stride)
#if defined(ICL_HAVE_IPP)
template<> struct OpImpl<icl8u> { ... };
#elif defined(ICL_HAVE_SSE2)
template<> struct OpImpl<icl8u> { ... };
#endif

// 4. apply() — zero #ifdefs
void Filter::apply(const Image &src, Image &dst) {
    if(!prepare(dst, src)) return;
    src.visitWith(dst, [this](const auto &s, auto &d) {
        using T = typename std::remove_reference_t<decltype(s)>::type;
        OpImpl<T>::apply(s, d, ...);
    });
}
```

### Key conventions:

- **Visitors for inner loops** — all C++ fallbacks and SSE2 specializations use
  `visitROILinesPerChannelWith` (or `visitROILinesNWith` for multi-channel).
  This ensures correct ROI handling and enables the contiguous fast-path.
- **References everywhere** — `visitWith`/`visit` give references, pass them through.
- **No macros for depth dispatch** — `visitWith`/`visit` replaces all
  `ICL_INSTANTIATE_DEPTH` macro rounds.
- **IPP is the exception** — IPP functions accept stride+size natively, so they
  use `getROIData()`/`getLineStep()`/`getROISize()` directly (no visitor needed).

### For different output depths (e.g. UnaryCompareOp → always 8u):

```cpp
void Filter::apply(const Image &src, Image &dst) {
    if(!prepare(dst, src, depth8u)) return;
    Img8u &d = dst.as8u();
    src.visit([&](const auto &s) {
        using T = typename std::remove_reference_t<decltype(s)>::type;
        OpImpl<T>::apply(s, d, ...);
    });
}
```

Use `visit()` (single-image) instead of `visitWith()` since src/dst have different depths.

### For multi-channel → single-channel (e.g. ColorDistanceOp):

```cpp
src.visit([&](const auto &s) {
    visitROILinesNWith<3,1>(s, dst_typed, [&](const S *r, const S *g, const S *b,
                                              D *d, int w) {
        for(int i = 0; i < w; ++i) { ... }
    });
});
```

## Key Image Methods for Filter Migration

```cpp
// Prepare (replaces ensureCompatible + OpROIHandler)
prepare(dst, src)                    // match src params
prepare(dst, src, depth)             // match src but different depth
prepare(dst, d, size, fmt, ch, roi, time)  // explicit

// Conversion (replaces ImgBase::convert)
src.convertTo(dst)                   // convert into dst (reuses buffer)
src.convertROITo(dst)                // ROI-only conversion

// Typed access
dst.as8u() / dst.as32f() / ...      // Img<T>& for pixel work
dst.visit([](auto &img){...})       // generic dispatch
src.visitWith(dst, [](auto &s, auto &d){...})  // two-image dispatch

// Metadata
src.getSize(), getChannels(), getDepth(), getFormat(), getROI(), ...
src.getImageRect()                   // Rect(0,0,w,h)
```

## Other TODOs

- **bpp()** — still used in ~30 places. Once all filters use Image natively,
  bpp() call sites can be replaced with Image-based apply, then bpp() removed.
- **applyParallel()** — free function replacing deprecated applyMT
- **BinaryOp filters** — same migration as UnaryOp, needs visitROILines2With
- **Converter::convert()** — static method replacing constructor-that-does-work
- **Quick.h** — consider reworking to use Image instead of ImgQ (Img<icl32f>)
- **Custom SSE2/NEON specializations** — add SIMD fast paths to filters that currently
  only have IPP acceleration or pure C++ fallback. The dispatch struct pattern makes
  this easy: just add a `#elif defined(ICL_HAVE_SSE2)` specialization block.
  Priority candidates (high pixel throughput, simple inner loops):
  - UnaryLogicalOp (bitwise ops — natural SIMD fit)
  - UnaryCompareOp (compare+mask — similar to ThresholdOp SSE2 pattern)
  - BinaryArithmeticalOp, BinaryLogicalOp, BinaryCompareOp (same patterns, two inputs)
  - ConvolutionOp (fixed-kernel convolutions, e.g. 3x3 Sobel)
  - MedianOp (already has SSE2 — extend to more depths/sizes)
  - MorphologicalOp (erode/dilate with flat structuring elements)
