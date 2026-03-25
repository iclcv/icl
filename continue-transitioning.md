# Image Migration — Continuation Guide

## Current State (Session 6)

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

### Fully Native Image Filters (5 done)

These override `apply(const Image&, Image&)` directly, no `applyImgBase` bridge:

1. **DitheringOp** — fixed depth (8u), uses `prepare()` + `convertTo()` + `as8u()`
2. **MirrorOp** — same depth, uses `prepare()` + `visitWith()` for typed dispatch
3. **ThresholdOp** — same depth, uses `prepare()` + `visitWith()`, dispatch structs
4. **UnaryArithmeticalOp** — same depth, uses `visitWith()` + `LoopFunc` template structs
5. **UnaryCompareOp** — different output depth (always 8u), uses `visit()` + `as8u()` + dispatch struct

### Filters with applyImgBase Bridge (23 remaining)

These have `apply(const Image&, Image&) override` that delegates to `applyImgBase()`:

```cpp
void Filter::apply(const Image &src, Image &dst) {
    // TODO: use Image natively!
    ImgBase *dstPtr = dst.isNull() ? nullptr : dst.ptr();
    applyImgBase(src.ptr(), &dstPtr);
    if(dstPtr) dst = Image(*dstPtr);
}
```

**With IPP acceleration (11):**
AffineOp, CannyOp, ConvolutionOp, LUTOp, LocalThresholdOp, MedianOp,
MorphologicalOp, UnaryLogicalOp, WarpOp, WienerOp
(+ IntegralImgOp has IPP code but disabled — slower than C++)

**Pure C++ (12):**
BilateralFilterOp, ChamferOp, ColorDistanceOp, ColorSegmentationOp,
FFTOp, GaborOp, IFFTOp, LUTOp3Channel, MotionSensitiveTemporalSmoothing,
WeightChannelsOp, WeightedSumOp

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
// 1. C++ fallback functors / loops (always compiled)
template <typename T, class Op>
inline void fallbackLoop(const Img<T> &src, Img<T> &dst, ...) { ... }

// 2. SSE2/NEON SIMD helpers (file-local, inside #ifdef ICL_HAVE_SSE2)
#ifdef ICL_HAVE_SSE2
inline void sse_op_32f(const Img32f &src, Img32f &dst, ...) { ... }
inline void sse_op_8u(const Img8u &src, Img8u &dst, ...) { ... }
#endif

// 3. IPP helpers (file-local, inside #ifdef ICL_HAVE_IPP)
#ifdef ICL_HAVE_IPP
template <typename T, IppStatus (IPP_DECL *F)(...)>
inline void ippCall(const Img<T> &src, Img<T> &dst, ...) { ... }
#endif

// 4. Dispatch struct — primary template = C++ fallback
template<class T> struct OpImpl {
  static void apply(const Img<T> &src, Img<T> &dst, ...) {
    fallbackLoop(src, dst, ...);
  }
};

// 5. Single #if/#elif/#endif block for accelerated specializations
#if defined(ICL_HAVE_IPP)
template<> struct OpImpl<icl8u>  { static void apply(...) { ippCall<...>(...); } };
template<> struct OpImpl<icl16s> { static void apply(...) { ippCall<...>(...); } };
template<> struct OpImpl<icl32f> { static void apply(...) { ippCall<...>(...); } };
#elif defined(ICL_HAVE_SSE2)
template<> struct OpImpl<icl8u>  { static void apply(...) { sse_op_8u(...); } };
template<> struct OpImpl<icl32f> { static void apply(...) { sse_op_32f(...); } };
#endif

// 6. apply() — zero #ifdefs
void Filter::apply(const Image &src, Image &dst) {
    if(!prepare(dst, src)) return;
    src.visitWith(dst, [this](const auto &s, auto &d) {
        using T = typename std::remove_reference_t<decltype(s)>::type;
        OpImpl<T>::apply(s, d, ...);
    });
}
```

### Key conventions:

- **References everywhere** — `visitWith`/`visit` give references, pass them through
  to helpers and dispatch structs. No pointer-taking (`&s, &d`) needed.
- **No macros for depth dispatch** — `visitWith`/`visit` replaces all
  `ICL_INSTANTIATE_DEPTH` / `ICL_INSTANTIATE_ALL_DEPTHS` macro rounds.
- **No forward declarations** — primary template defined first, specializations after.
- **Non-val ops fold into val-variant calls** — e.g. `ThreshLTVal<T>::apply(s, d, t, t)`
  for the non-val `lt` operation, avoiding separate dispatch paths.

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
- **BinaryOp filters** — same migration as UnaryOp
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
