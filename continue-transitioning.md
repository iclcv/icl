# Image Migration — Continuation Guide

## Current State (Session 15 — MotionSensitiveTemporalSmoothing: ALL UnaryOp filters migrated)

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

### Fully Native Image UnaryOp Filters (27 done — ALL COMPLETE)

These override `apply(const Image&, Image&)` directly, no `applyImgBase` bridge.

**With BackendDispatch (6)** — separate _Simd.cpp/_Ipp.cpp backend files, `if constexpr`
optype templates, `dispatchEnum<>` for runtime→compile-time dispatch:
1. **ThresholdOp** — 3 selectors (ltVal, gtVal, ltgtVal), SSE2 8u/32f, IPP 8u/16s/32f
2. **UnaryCompareOp** — 2 selectors (compare, compareEqTol), SSE2 8u, IPP 8u/16s/32f
3. **UnaryArithmeticalOp** — 2 selectors (withVal, noVal), SSE2 32f, IPP stub

**Without BackendDispatch (17)** — inline dispatch structs:
4. **DitheringOp** — fixed depth (8u), uses `prepare()` + `convertTo()` + `as8u()`
5. **MirrorOp** — same depth, uses `prepare()` + `visitWith()` for typed dispatch
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
19. **MedianOp** — `MedianImpl<T>` dispatch struct, 4 algorithms
20. **ConvolutionOp** — mixed-depth (8u→16s default), keeps IPP fixed-kernel dispatch chain
21. **MorphologicalOp** — 8u/32f only, 11 optypes
22. **LocalThresholdOp** — 4 algorithms, internal ImgBase* ROI buffering kept
23. **WarpOp** — `WarpImpl<T>` dispatch struct, IPP 8u/32f, OpenCL 8u+LIN via stateful backend
24. **BilateralFilterOp** — brute-force bilateral, C++ all depths, OpenCL 8u/32f via stateful backend
25. **FFTOp** — thin subclass of BaseFFTOp (forward), delegates to FFTUtils (IPP/MKL/C++)
26. **IFFTOp** — thin subclass of BaseFFTOp (inverse), join + PAD_REMOVE support
27. **MotionSensitiveTemporalSmoothing** — temporal filter with motion detection, ring buffer
    per channel, template C++ implementation, 8u/32f only, no ROI support, motion image output

### Fully Native Image BinaryOp Filters (3 done)

These override `apply(const Image&, const Image&, Image&)` with BackendDispatch:
1. **BinaryArithmeticalOp** — 1 selector (apply), add/sub/mul/div/absSub
2. **BinaryCompareOp** — 2 selectors (compare, compareEqTol), output always 8u
3. **BinaryLogicalOp** — 1 selector (apply), and/or/xor, integer depths only

All use `visitROILinesPerChannel2With`, `if constexpr` optype templates,
`dispatchEnum<>`. BinaryOp base class has Image-based `prepare()` + `check()`.

### BaseFFTOp — Common Base for FFTOp/IFFTOp — DONE

FFTOp and IFFTOp merged into a common `BaseFFTOp` base class. Unified enums
(ResultMode with all 8 modes, SizeAdaptionMode with PAD_REMOVE added at end).
FFTOp and IFFTOp are now thin header-only subclasses with backward-compatible
constructors and setter aliases. Old FFTOp.cpp/IFFTOp.cpp deleted.

**Known issue**: FFTUtils C++ fallback has thread-safety problems (global static
buffers). FFT tests are flaky under parallel execution. Passes 100% with `-j 1`.

### UnaryOp Filters with applyImgBase Bridge — NONE REMAINING

All 27 UnaryOp filters now override `apply(const Image&, Image&)` directly.
The `applyImgBase` bridge is no longer used by any UnaryOp subclass.

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

328 tests total (tests/ directory, single icl-tests executable):
- `test-utils.cpp` — Size, Point, Rect, Range, string, random
- `test-math.cpp` — FixedMatrix, DynMatrix
- `test-core.cpp` — Image, Img<T> (including initializer list + equality)
- `test-filter.cpp` — 163 filter tests covering all 27 migrated UnaryOp filters + 3 BinaryOp filters

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

- **FFTUtils thread safety** — **IMPORTANT**: the C++ FFT/IFFT fallback in
  `ICLMath/FFTUtils.cpp` is NOT thread-safe. Multiple FFTOp/IFFTOp instances
  running concurrently produce corrupt results. Likely cause: global/static
  DynMatrix buffers in the FFT implementation. Fix options:
  (a) Add a mutex around `fft2D_cpp`/`ifft2D_cpp` (simplest, small perf cost)
  (b) Make buffers thread-local (better for multi-threaded apps)
  (c) Pass buffers through from BaseFFTOp (avoids global state entirely)
  This affects the test suite — FFT tests are flaky under parallel execution.
- **bpp()** — still used in ~30 places. Once all filters use Image natively,
  bpp() call sites can be replaced with Image-based apply, then bpp() removed.
- **applyParallel()** — free function replacing deprecated applyMT
- **MSTS OpenCL backend** — MotionSensitiveTemporalSmoothing currently C++ only.
  The OpenCL path was removed during migration (tightly coupled to TemporalSmoothingCL).
  Could be re-added as a stateful backend, but the temporal ring buffer on GPU
  makes the dispatch pattern awkward. Low priority unless Kinect perf is needed.
- **BinaryOp filters** — same migration as UnaryOp, needs visitROILines2With
- **KuwaharaOp** — dropped from BilateralFilterOp (different algorithm). Could be
  its own NeighborhoodOp if there's demand.
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
### Backend Dispatch Framework (ICLCore/BackendDispatch.h) — DONE

Moved from ICLFilter/FilterDispatch to ICLCore/BackendDispatch (`icl::core` namespace).
Validated with 6 UnaryOp + 3 BinaryOp filters, old implementations replaced.

Key types:
```
Backend              — enum: Cpp, Simd, Ipp, OpenCL
BackendSelector<Sig> — typed dispatch table for one sub-op
Dispatching          — mixin base class, owns BackendSelectors
ApplicabilityFn      — std::function<bool(const Image&)>
applicableTo<Ts...>  — predefined depth-check (e.g. applicableTo<icl8u, icl32f>)
```

Usage pattern:
```cpp
// Header: sub-op signature typedef
using Sig = void(const Image&, Image&, double, int);

// Constructor: register C++ fallback + auto-load backends from registry
initDispatching("MyOp");
auto& sel = addSelector<Sig>("subOp");
sel.add(Backend::Cpp, cpp_fn);

// apply(): dispatch to best backend
getSelector<Sig>("subOp").resolve(src)->apply(src, dst, val, ot);

// Backend _Simd.cpp: self-registration with applicability check
static const int _r = registerBackend<Op::Sig>(
    "MyOp.subOp", Backend::Simd, simd_fn,
    applicableTo<icl8u, icl32f>, "SSE2 description");
```

Utilities:
- `dispatchEnum<V1, V2, ...>(runtime, f)` — runtime→compile-time enum dispatch (ICLUtils/EnumDispatch.h)
- `applicableTo<icl8u, icl32f>(src)` — depth predicate template (BackendDispatch.h)
- `forceAll(Backend::Cpp)` / `forEachCombination()` — testing cross-validation

### Stateful Backends (`registerStatefulBackend`) — DONE

For backends that need per-instance state (OpenCL buffers, precomputed LUTs, temp
buffers), use `registerStatefulBackend<Sig>()`. It takes a **factory** that returns a
callable capturing its own state. The factory is called once per filter instance
during `loadFromRegistry()`.

```cpp
// In _OpenCL.cpp backend file:
struct CLMyState {
    CLProgram program;
    CLKernel kernel;
    // ...
    void apply(const Image& src, Image& dst, ...) { /* uses this->program etc. */ }
};

static const int _reg = registerStatefulBackend<MyOp::MySig>(
    "MyOp.apply", Backend::OpenCL,
    []() {  // factory — called per filter instance
        auto state = std::make_shared<CLMyState>();
        return [state](const Image& src, Image& dst, ...) {
            state->apply(src, dst, ...);
        };
    },
    applicableTo<icl8u, icl32f>, "OpenCL description");
```

If the factory throws (e.g. no OpenCL device), the backend is skipped silently.

**Validated with**: WarpOp (OpenCL warp, 8u), BilateralFilterOp (OpenCL bilateral + C++ LUT cache).

WarpOp previously used ad-hoc `#ifdef ICL_HAVE_OPENCL` + raw `CLWarp*` member +
`m_tryUseOpenCL` flag. Now uses `Dispatching` mixin with self-registering stateful
OpenCL backend. `setTryUseOpenCL()` removed — use `forceAll(Backend::Cpp)` to
disable OpenCL, or let the cascade auto-select the best available backend.
