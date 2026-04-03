# ICL Filter Implementation Overview

## UnaryOp Filters (27)

| # | Filter | BD | Selectors | Backends | Notes |
|---|--------|----|-----------|----------|-------|
| 1 | ThresholdOp | Yes | 3 (ltVal, gtVal, ltgtVal) | Cpp, Simd(8u/32f), Ipp(8u/16s/32f) | `dispatchEnum<>` templates |
| 2 | UnaryCompareOp | Yes | 2 (compare, compareEqTol) | Cpp, Simd(8u), Ipp(8u/16s/32f) | Output depth differs from src |
| 3 | UnaryArithmeticalOp | Yes | 2 (withVal, noVal) | Cpp, Simd(32f), Ipp(stub) | `dispatchEnum<>` |
| 4 | UnaryLogicalOp | Yes | 2 (withVal, noVal) | Cpp, Simd(all int), Ipp | Integer depths only |
| 5 | AffineOp | Yes | 1 (apply) | Cpp, Ipp(8u/32f) | Inverse matrix C++ fallback |
| 6 | WienerOp | Yes | 1 (apply) | Ipp(8u/16s/32f) | IPP-only, stateful backend |
| 7 | LUTOp | Yes | 1 (reduceBits) | Cpp, Ipp(8u) | Always 8u, static method kept |
| 8 | MedianOp | Yes | 2 (fixed, generic) | Cpp, Simd(8u/16s/32f), Ipp(8u/16s) | Sorting networks shared |
| 9 | ConvolutionOp | Yes | 1 (apply) | Cpp, Ipp(8u/16s/32f) | 34 IPP specializations |
| 10 | MorphologicalOp | Yes | 1 (apply) | Cpp, Ipp(8u/32f) | Stateful, maskVersion() |
| 11 | WarpOp | Yes | 1 (apply) | Cpp, Ipp(8u/32f), OpenCL(8u) | Stateful OpenCL |
| 12 | BilateralFilterOp | Yes | 1 (apply) | Cpp, OpenCL(8u/32f) | Stateful OpenCL + LUT cache |
| 13 | DitheringOp | No | — | Cpp | Fixed 8u depth |
| 14 | MirrorOp | No | — | Cpp | `visitWith()` typed dispatch |
| 15 | WeightChannelsOp | No | — | Cpp | Per-channel multiply |
| 16 | WeightedSumOp | No | — | Cpp | Output 32f/64f |
| 17 | ColorDistanceOp | No | — | Cpp | `visitROILinesNWith<3,1>` |
| 18 | LUTOp3Channel | No | — | Cpp | Template class |
| 19 | IntegralImgOp | No | — | Cpp | Sequential algorithm |
| 20 | GaborOp | No | — | Cpp | Composition (wraps ConvolutionOp) |
| 21 | ColorSegmentationOp | No | — | Cpp | depth8u only |
| 22 | ChamferOp | No | — | Cpp | Multi-pass distance propagation |
| 23 | CannyOp | No | — | Cpp | Composition (Sobel->Canny) |
| 24 | LocalThresholdOp | No | — | Cpp | 4 algorithms |
| 25 | FFTOp | No | — | Cpp | Delegates to FFTUtils |
| 26 | IFFTOp | No | — | Cpp | Delegates to FFTUtils |
| 27 | MSTS | No | — | Cpp | Temporal filter, ring buffer |

## BinaryOp Filters (4)

| # | Filter | BD | Selectors | Backends | Notes |
|---|--------|----|-----------|----------|-------|
| 1 | BinaryArithmeticalOp | Yes | 1 (apply) | Cpp, Simd(32f) | add/sub/mul/div/absSub |
| 2 | BinaryCompareOp | Yes | 2 (compare, eqTol) | Cpp, Simd(8u/32f) | Output always 8u |
| 3 | BinaryLogicalOp | Yes | 1 (apply) | Cpp, Simd(all int) | and/or/xor |
| 4 | ProximityOp | No | — | Ipp | IPP-only, no C++ fallback |

## Legend

- **BD** = BackendDispatch (`ImageBackendDispatching` mixin + self-registering backend files)
- 15 filters with BackendDispatch (12 UnaryOp + 3 BinaryOp)
- 16 filters without (no IPP/#ifdef to extract)
- Backend files: `_Simd.cpp` (always built), `_Ipp.cpp` (excluded when !IPP_FOUND), `_OpenCL.cpp` (excluded when !OPENCL_FOUND)

---

## Optimization Priority List

### P0 — Correctness / Stability

| # | Item | Scope | Effort | Notes |
|---|------|-------|--------|-------|
| 1 | IPP cross-validation | Testing | Medium | Build in Linux container with IPP, run both IPP + C++ backends on same inputs via `forceAll()`, compare outputs. Validates all 15 BackendDispatch filters. |
| 2 | Remaining `#ifdef ICL_HAVE_IPP` in filter .cpp | ICLFilter | Small | 12 occurrences in 8 files (LocalThresholdOp, ProximityOp, UnaryOp×3, ImageRectification, CannyOp, WarpOp, NeighborhoodOp×3, IntegralImgOp-deactivated). Some are bug workarounds, others could be extracted or removed. |

### P1 — Image API Completion

| # | Item | Scope | Effort | Notes |
|---|------|-------|--------|-------|
| 3 | applyParallel() | ICLFilter | Medium | Free function replacing deprecated `applyMT`. Currently only on NeighborhoodOp. Should use `std::async` with Image-based apply. |
| 4 | Converter::convert() | ICLCore | Small | Static method replacing constructor-that-does-work pattern. |
| 5 | Quick.h rework | ICLQt | Medium | Uses `ImgQ` (Img<icl32f>) throughout. Rework to use Image value type. 28 ImgIterator usages in Quick.cpp. |
| 6 | ImgBase* in filter headers | ICLFilter | Large | 60 occurrences across 16 headers. BinaryOp.h (13), UnaryOpPipe.h (7), OpROIHandler.h (5), ImageSplitter.h (4). Migrate to Image where possible. |
| 7 | ProximityOp C++ fallback | ICLFilter | Medium | Currently IPP-only. Add C++ fallback so it builds without IPP. Currently excluded from non-IPP builds entirely. |

### P2 — Performance / SIMD

| # | Item | Scope | Effort | Notes |
|---|------|-------|--------|-------|
| 8 | Apple Accelerate as IPP replacement | ICLFilter | Large | macOS-native SIMD/DSP framework. Could provide accelerated convolution, FFT, morphology on Apple Silicon without IPP. |
| 9 | More SIMD backends | ICLFilter | Medium | Many C++ filters could benefit from SSE2/NEON: MirrorOp, WeightChannelsOp, DitheringOp, IntegralImgOp. Low-hanging fruit since sse2neon already works. |
| 10 | MSTS OpenCL backend | ICLFilter | Medium | Removed during migration. Temporal ring buffer on GPU is awkward for dispatch. Low priority unless Kinect perf needed. |
| 11 | ConvolutionOp SIMD | ICLFilter | Medium | No _Simd.cpp yet. Generic 3x3 convolution is a good SIMD candidate (8u/32f). |

### P3 — External Dependencies

| # | Item | Scope | Effort | Notes |
|---|------|-------|--------|-------|
| 12 | ImageMagick 7 rewrite | ICLIO | Large | PixelPacket API removed in IM7. Need full rewrite of IM grabber/output plugins. |
| 13 | FFmpeg 6+/7+ rewrite | ICLIO | Large | LibAVVideoWriter uses deprecated APIs. Needs rewrite for modern avcodec/avformat. |
| 14 | Qt6 multimedia grabbers | ICLQt | Medium | QVideoSink rewrite needed. Currently disabled. |
| 15 | Qt4/Qt5 legacy removal | ICLQt | Medium | Remove all compat code; Qt6 is sole target. |
| 16 | OpenCL macOS C++ bindings | ICLUtils | Medium | Need C++ wrapper work for Metal-backed OpenCL on macOS. Deferred. |

### P4 — Code Quality / Cleanup

| # | Item | Scope | Effort | Notes |
|---|------|-------|--------|-------|
| 17 | ImgIterator deprecation | All | Large | 201 occurrences in 13 files. Replace with line-based visitors (Visitors.h) where possible. CCFunctions.cpp has 82, Quick.cpp has 28. |
| 18 | C++17 source fixes | All | Medium | CMake done, but source code may still use pre-C++17 patterns that could be modernized. |
| 19 | Eigen 5 compatibility | ICLMath | Small | Currently pinned to 3.4.x due to Apple Clang bug in Eigen 5.0.1. Re-test periodically. |
| 20 | KuwaharaOp | ICLFilter | Small | Dropped from BilateralFilterOp. Could be its own NeighborhoodOp if needed. |
| 21 | MirrorOp clipToROI=false test | Testing | Small | Affine ops write full image in non-clip mode — needs separate ROI test. |
