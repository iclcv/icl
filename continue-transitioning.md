# Image Migration — Continuation Guide

## Current State (Session 12 — WarpOp + Backend Dispatch Framework)

### Architecture

```
Public API:  Image (value type, shared_ptr<ImgBase>)
                |
Internal:    ImgBase → Img<T>  (implementation detail)
```

### Img<T> Initializer List Constructors

```cpp
Img32f img = {{1, 2, 3},
              {4, 5, 6}};           // 3x2, 1 channel

Img8u img = {{{1, 2}, {3, 4}},     // channel 0: 2x2
             {{5, 6}, {7, 8}}};    // channel 1: 2x2

Image img = Img32f{{1, 2, 3}};     // implicit conversion to Image
```

Single-channel delegates to multi-channel internally. All rows must be same length.
**Ambiguity**: `{{{x}}, {{y}}}` is ambiguous when each row has 1 element — use 2+ columns
for multi-channel, or construct differently for the 1-pixel-wide case.

### Img<T> and Image Equality Operators

```cpp
Img8u a = {{1,2},{3,4}};
Img8u b = {{1,2},{3,4}};
a == b;  // true — element-wise, compares ROI data
```

- Integer types: exact comparison
- `icl32f`: tolerance `4 * FLT_EPSILON` (~4.8e-7)
- `icl64f`: tolerance `4 * DBL_EPSILON` (~8.9e-16)
- `Image::operator==` delegates through `visit()` to typed `Img<T>::operator==`

### SIMD Status

SSE2 intrinsics are transparently mapped to ARM NEON via `sse2neon` (3rdparty).
`ICL_HAVE_SSE2` is effectively "have SIMD" on both x86 and Apple Silicon.
IPP is x86-only (absent on ARM) — filters fall back to C++ on Apple Silicon.
No runtime dispatcher needed; compile-time `#ifdef` is the right granularity.

### Line-Based ROI Visitors

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

**TODO for BinaryOp**: will need a `visitROILines2With(src1, src2, dst, f)` variant
for two-source + one-dst patterns. Add to Visitors.h when BinaryOp migration starts.

### Per-Pixel Convenience Visitors (Img<T> members)

```cpp
// Auto-detected lambda signature via if constexpr + is_invocable:
img.visitPixels([](T &val) { ... });                    // value only
img.visitPixels([](int x, int y, T &val) { ... });     // coordinates + value
img.visitPixels([](int x, int y, int c, T &val) { ... }); // full

// roiOnly parameter (default true):
img.visitPixels(f);          // iterates ROI only
img.visitPixels(f, false);   // iterates full image
```

Both mutable and const overloads. Uses `getData(c)` + direct indexing (not operator()).
Not for hot paths — uses per-pixel indexing. Ideal for tests, initialization, non-critical code.

### Img<T>::from() Static Factory

```cpp
auto img = Img8u::from(10, 10, 1, [](int x, int y, int c) -> icl8u {
    return x + y * 10;
});
```

Replaces the common construct + fill-loop pattern. Generator called as `f(x, y, channel)`.

### Generic ROI Test Helper

```cpp
testROIHandling(UnaryOp &op, const Image &src, const Rect &roi);
```

Tests both clipToROI=true (correct size, content matches no-clip mode) and
clipToROI=false (full-size output, ROI content matches, sentinel survives outside ROI).
Pre-allocates dst at output depth for depth-changing filters (e.g. UnaryCompareOp).
Works for both UnaryOp and NeighborhoodOp (mask border shrinking tested automatically).
Note: affine ops (MirrorOp) write full image in non-clip mode — test those separately.

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

### NeighborhoodOp Image-based prepare

NeighborhoodOp has Image-based `prepare()` methods that compute the shrunk ROI
(accounting for mask size), then delegate to `UnaryOp::prepare(Image, ...)`.
Required for migrating any NeighborhoodOp subclass (WienerOp, MedianOp,
ConvolutionOp, MorphologicalOp, LocalThresholdOp, etc.).

```cpp
bool NeighborhoodOp::prepare(Image &dst, const Image &src);
bool NeighborhoodOp::prepare(Image &dst, const Image &src, depth d);
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

### Fully Native Image Filters (23 done)

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
18. **CannyOp** — composition (Sobel→Canny), shared `applyCannyCore`, fixed non-clipToROI stride bug
19. **MedianOp** — `MedianImpl<T>` dispatch struct, 4 algorithms: SIMD sorting networks (3x3/5x5),
    Huang histogram O(n) for 8u/16s arbitrary masks, generic sort for other types. Fixed pre-existing
    bug in column-oriented `sse_median3x3` driver (didn't re-sort newly loaded row). IPP for 8u/16s.
20. **ConvolutionOp** — mixed-depth (8u→16s default), keeps existing dispatch chain for IPP fixed-
    kernel specializations (Sobel, Laplace, Gauss). Rewrote generic C++ fallback to remove ImgIterator.
21. **MorphologicalOp** — 8u/32f only, 11 optypes. C++ fallback `morph_cpp` rewritten with raw
    pointers. Compound ops (open/close/tophat/blackhat/gradient) now use Image-based apply internally.
    Fixed pre-existing bug: constructors called setMask before initializing m_eType, causing
    uninitialized-read that randomly set mask to Size(1,1) under threading.
22. **LocalThresholdOp** — 4 algorithms (regionMean, tiledNN, tiledLIN, global). Internal ROI
    buffering uses ImgBase* (kept for now, internal detail). Fixed pre-existing bug: algorithm
    constructor missing "invert output" property, also fixed typo "gobal"→"global" in menu.
23. **WarpOp** — `WarpImpl<T>` dispatch struct, IPP 8u/32f via `ippiRemap`, OpenCL 8u+LIN
    (full-ROI only, falls back otherwise). Added ROI support: source ROI defines which output
    pixels to compute, full source is always the lookup domain. Warp offset handles clip/non-clip
    coordinate mapping. Fixed pre-existing bug: `interpolate_pixel_lin` had no -1 sentinel check
    (OOB read when warp map contained out-of-bounds coordinates).

### Filters with applyImgBase Bridge (5 remaining)

**Pure C++ (4):**
BilateralFilterOp, FFTOp, IFFTOp, MotionSensitiveTemporalSmoothing

**Difficulty estimates:**
- BilateralFilterOp (hard) — PIMPL, OpenCL/CPU dual path
- FFTOp (hard) — DynMatrix FFT, 5 size-adaptation modes
- IFFTOp (hard) — same as FFTOp but reverse
- MotionSensitiveTemporalSmoothing (hardest) — stateful, OpenCL, temporal buffers

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
template<> struct OpImpl<icl32f> { ... };
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

### For composition filters (e.g. CannyOp, GaborOp):

These delegate to other ops rather than doing pixel work directly.
Use `m_innerOp->apply(src, result)` with Image-based apply on the inner ops.
Store intermediate results as `Image` members (not `ImgBase*`).
For legacy secondary apply overloads (e.g. CannyOp's 3-arg apply), use a member
`Image m_legacyResult` to keep the result alive past the call.

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

## Test Infrastructure

257 tests total (tests/ directory, single icl-tests executable):
- `test-utils.cpp` — Size, Point, Rect, Range, string, random
- `test-math.cpp` — FixedMatrix, DynMatrix
- `test-core.cpp` — Image, Img<T> (including initializer list + equality)
- `test-filter.cpp` — 127 filter tests covering 23 migrated filters + NewThresholdOp

Test patterns — prefer these concise forms:
```cpp
// Initializer list for small known images
ICL_TEST_TRUE((op.apply(Img8u{{1,2,3}}) == Img8u{{3,2,1}}));

// Img::from for computed images
auto src = Img8u::from(20, 20, 1, [](int x, int y, int) -> icl8u { return x + y * 20; });

// visitPixels for result checking
bool ok = true;
dst.as8u().visitPixels([&](const icl8u &v) { if(v != 0 && v != 255) ok = false; });
ICL_TEST_TRUE(ok);

// Generic ROI test — covers clipToROI, non-clip, sentinel check
testROIHandling(op, src, Rect(2, 2, 6, 6));

// clear(-1, val) for uniform fill
Img8u src(Size(7,7), 1); src.clear(-1, 42);
```

**Guideline**: use `Img::from`, `visitPixels`, and `clear(-1, val)` wherever possible
to avoid manual construct+loop and per-pixel read patterns. These are zero-cost
abstractions (fully inlined by the compiler) and significantly reduce test boilerplate.
The same patterns should be used in production code where performance is not critical.

## Bugs Fixed During Migration

- **UnaryOp::prepare clipToROI** — source ROI offset was incorrectly applied to the
  smaller clipped destination, causing "Invalid Img-Parameter: roi". Fixed to use
  `Rect(Point::null, src.getROISize())` for destination when clipping.
- **CannyOp non-clipToROI stride** — pre-existing bug: C++ fallback used derivative
  width as stride for dst writes and followEdge, but dst can be larger than derivatives
  when clipToROI=false. Fixed by separating mag indexing (derivative stride) from dst
  indexing (dst stride via getROIData + getWidth).
- **MirrorOp missing `using UnaryOp::apply`** — single-arg apply was hidden.
- **MedianOp sse_median3x3 sorting bug** — column-oriented driver didn't re-sort newly
  loaded rows in the inner loop, producing wrong medians for unsorted neighborhoods.
  Fixed by switching 8u 3x3 to sse_for + subMedian3x3 (correct, still SIMD-accelerated).
- **flippedCopyChannelROI infinite loop** — `getMirrorPointerOffsets` computed end sentinel
  `e` relative to buffer start, not relative to source offset. When `srcOffset.x > 0`,
  `s != e` never matched → infinite loop (SIGBUS). Fixed by computing `e` and `eLine`
  relative to `s`. Affected MirrorOp with ROI at non-zero x offset.
- **MorphologicalOp uninitialized m_eType** — all constructors called `setMask()` before
  initializing `m_eType`. Since `setMask` checks `m_eType >= 6`, uninitialized stack memory
  could randomly set NeighborhoodOp mask to Size(1,1) → zero border shrink. Flaky under
  multi-threading. Fixed by setting `m_eType` before `setMask` in all constructors.
- **LocalThresholdOp missing "invert output" property** — algorithm constructor didn't add
  the property, causing exception when queried. Also fixed typo "gobal"→"global" in menu.
- **WarpOp interpolate_pixel_lin OOB read** — bilinear interpolation function had no check
  for the -1 sentinel set by `prepare_warp_table_inplace` for out-of-bounds coordinates.
  NN mode checked `if(x < 0) return 0` but LIN did not, causing undefined behavior when
  warp map contained OOB entries. Fixed by adding the same sentinel check to LIN.

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
- **IPP cross-validation** — compile ICL in a Linux container with IPP, run both
  IPP and C++ paths on same inputs, compare outputs for all migrated filters.
- **MirrorOp clipToROI=false** — affine ops write full image in non-clip mode, so
  the generic ROI sentinel test doesn't apply. Test MirrorOp ROI handling separately.
- **Use visitor patterns** — prefer `Img<T>::from`, `visitPixels`, line-based visitors
  (Visitors.h) wherever manual pixel loops exist. These are fully inlined — zero cost.
  Apply to production code (non-hot paths) and all test code.
- **Backend Dispatch Framework — IN PROGRESS** — `FilterDispatch.h/cpp` implements
  a cascaded backend dispatch mechanism. Proof-of-concept: `NewThresholdOp` with
  C++ fallback + SSE2 + IPP backends in separate files. Next steps:
  - Port `NewUnaryCompareOp` and `NewUnaryArithmeticalOp` to validate further
  - Consider moving `FilterDispatch.h` to ICLUtils (it's not filter-specific)
  - Once validated, replace old filters and drop the "New" prefix

### Backend Dispatch Framework (FilterDispatch.h)

Key types and naming:
```
Backend              — enum: Cpp, Simd, Ipp, OpenCL
BackendSelector<Sig> — typed dispatch table for one sub-op, templated on function signature
BackendSelectorBase  — non-templated base for introspection
Dispatching          — mixin base class, owns BackendSelectors via PIMPL storage
ApplicabilityFn      — std::function<bool(const Image&)> for depth/ROI/etc. checks
```

Usage pattern in a filter .cpp:
```cpp
// Constructor:
initDispatching("MyFilterOp");
auto& sub = addSelector<Sig>("subOpName");  // creates + loads from registry
sub.add(Backend::Cpp, cpp_fn);              // C++ fallback (auto-description)

// apply():
auto& sub = getSelector<Sig>("subOpName");
sub.resolve(src)->apply(src, dst, ...);     // cascaded backend selection

// Backend .cpp self-registration (no #include of filter header):
static const int _r = registerBackend<Sig>(
    "MyFilterOp.subOpName", Backend::Simd, simd_fn,
    [](const Image& s) { return s.getDepth() == depth8u; },
    "SSE2 description");
```

Testing: `forceAll(Backend::Cpp)`, `forEachCombination()` for exhaustive cross-validation.

### NewThresholdOp (proof-of-concept)

Files: `NewThresholdOp.h` (clean header, no dispatch internals), `.cpp` (C++ fallback + apply),
`_Simd.cpp` (SSE2 for 8u/32f), `_Ipp.cpp` (IPP for 8u/16s/32f, conditional).
3 sub-ops: ltVal, gtVal, ltgtVal. 9 tests including cross-validation of all backend combos.
