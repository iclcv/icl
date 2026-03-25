# Image Migration — Continuation Guide

## Current State (Session 5)

### Architecture

```
Public API:  Image (value type, shared_ptr<ImgBase>)
                |
Internal:    ImgBase → Img<T>  (implementation detail)
```

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

### Fully Native Image Filters (4 done)

These override `apply(const Image&, Image&)` directly, no `applyImgBase` bridge:

1. **DitheringOp** — fixed depth (8u), uses `prepare()` + `convertTo()` + `as8u()`
2. **MirrorOp** — same depth, uses `prepare()` + `visitWith()` for typed dispatch
3. **ThresholdOp** — same depth, uses `prepare()` + `visitWith()`, typed statics are file-local
4. **UnaryArithmeticalOp** — same depth, uses `visitWith()` + `LoopFunc` template structs

### Filters with applyImgBase Bridge (24 remaining)

These have `apply(const Image&, Image&) override` that delegates to `applyImgBase()`:

```cpp
void Filter::apply(const Image &src, Image &dst) {
    // TODO: use Image natively!
    ImgBase *dstPtr = dst.isNull() ? nullptr : dst.ptr();
    applyImgBase(src.ptr(), &dstPtr);
    if(dstPtr) dst = Image(*dstPtr);
}
```

Remaining: AffineOp, BilateralFilterOp, CannyOp, ChamferOp, ColorDistanceOp,
ColorSegmentationOp, ConvolutionOp, FFTOp, GaborOp, IFFTOp, IntegralImgOp,
LUTOp, LUTOp3Channel, LocalThresholdOp, MedianOp, MorphologicalOp,
MotionSensitiveTemporalSmoothing, UnaryCompareOp, UnaryLogicalOp,
WarpOp, WeightChannelsOp, WeightedSumOp, WienerOp

### BinaryOp Filters (not started)

BinaryArithmeticalOp, BinaryCompareOp, BinaryLogicalOp — still override
the ImgBase version. Need same treatment as UnaryOp filters.

## Migration Pattern

### For each filter:

**1. In the .cpp:**
- Rename `applyImgBase(const ImgBase*, ImgBase**)` to file-local static or remove
- Rewrite `apply(const Image&, Image&)` natively:

```cpp
void Filter::apply(const Image &src, Image &dst) {
    if(!prepare(dst, src)) return;          // or prepare(dst, depth, size, ...)
    src.visitWith(dst, [this](const auto &s, auto &d) {
        using T = typename std::remove_reference_t<decltype(s)>::type;
        // call typed implementation
    });
}
```

**2. In the .h:**
- Remove `void applyImgBase(const core::ImgBase *, core::ImgBase **);`
- Remove any typed dispatch declarations that were only used internally
- Keep `using UnaryOp::apply;` if present

**3. Typed implementations:**
- Keep IPP/SSE2/fallback `LoopFunc` or similar typed structs as file-local
- Move any class-member typed methods to file-local static functions
- The visitor calls them directly — no dispatch switch needed

### Common patterns in the existing filters:

**Same depth (most filters):** `prepare(dst, src)` + `visitWith`
**Different output depth:** `prepare(dst, outputDepth, src.getSize(), ...)` + `convertTo`
**Method pointer table:** Replace with `visitWith` (eliminates m_aMethods array)
**ICL_INSTANTIATE_ALL_DEPTHS macro switch:** Replace with `visitWith`

### Extracting pixel type in visitor:
```cpp
src.visitWith(dst, [](const auto &s, auto &d) {
    using T = typename std::remove_reference_t<decltype(s)>::type;
    // T is icl8u, icl16s, icl32s, icl32f, or icl64f
});
```
(`Img<T>::type` was added to Img.h for this purpose)

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
- **Runtime dispatcher for IPP/SSE/fallback** — could simplify the #ifdef maze
