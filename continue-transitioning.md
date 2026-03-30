# Image Migration — Continuation Guide

## Current State (Session 18 — Docker IPP Green, Backend Dispatch for Img Utilities)

### Session 18 Summary

**Docker IPP build — now green:**
- Fixed `ContourDetector.cpp` missing `#include <cstring>`
- Fixed `CornerDetectorCSS.cpp` removed `ippsConv_32f` (deprecated, use C++ fallback)
- Fixed `CV.cpp` removed `ippiCrossCorrValid_Norm` / `ippiSqrDistanceValid_Norm` (deprecated)
- Fixed `TemplateTracker.h` missing `#include <ICLUtils/Point32f.h>`
- Fixed `DataSegment.h` missing `#include <cstring>`
- Fixed `ICLMarkers/CMakeLists.txt` spurious Qt PCH headers
- Fixed `FiducialDetectorPluginART.cpp` dead `Quick.h` include
- Fixed `FiducialDetectorPluginICL1.cpp` guarded `Quick.h`, added proper includes
- Fixed `ProximityOp.cpp` — provided stub implementations (was entirely inside `#if 0`)
- All modules compile and tests pass on Linux/IPP (Docker) and macOS

**Incremental Docker builds:**
- `build-and-test.sh` now uses `rsync` (preserves timestamps) instead of `cp -a`
- Use named Docker volume for persistent build cache
- Dockerfile adds `rsync` package

**ICLFilter IPP migration — complete:**
- Extracted `WarpOp` inline IPP (`ippiRemap_8u/32f_C1R`) to `WarpOp_Ipp.cpp` as `Backend::Ipp`
- Removed `#ifdef ICL_HAVE_IPP` from NeighborhoodOp.cpp (anchor workaround now always-on)
- Removed `#ifdef ICL_HAVE_IPP` from LocalThresholdOp.cpp (C++ path is both faster and higher quality)
- Removed `#ifdef ICL_HAVE_IPP` from UnaryOp.cpp (Canny creator — works without IPP now)
- Removed redundant `#ifdef ICL_HAVE_IPP` guards from all `_Ipp.cpp` files (CMake already excludes them)
- ICLFilter now has zero active `#ifdef ICL_HAVE_IPP` in non-backend files

**ImgOps singleton + ImgBaseBackendDispatching (new this session):**

Established the pattern for migrating `Img<T>` utility functions to dispatched backends.
First operation migrated: **mirror**.

Key design decisions:
- **ImgBaseBackendDispatching** (`BackendDispatching<ImgBase*>`) — new dispatch context for
  `Img<T>` methods, so they can dispatch via `this` without constructing an `Image` wrapper
- **ImgOps singleton** — inherits from `ImgBaseBackendDispatching`, owns `BackendSelector`s
  for Img utility operations (mirror, and later: min/max, lut, normalize, etc.)
- **ALL implementations in separate files** — `Img_Cpp.cpp` has the C++ fallback,
  `Img_Ipp.cpp` has the IPP backend. The `Img<T>` method itself is dispatch-only.
  This ensures dispatch is always used regardless of call path (`Image::mirror()` or
  `Img<T>::mirror()` directly).
- **`applicableToBase<Ts...>()`** — applicability helper for `ImgBase*` context (checks depth)
- **`resolveOrThrow()`** — safe dispatch that throws `std::logic_error` with selector name
  instead of returning nullptr

**Mirror migration details:**

```
Call chain:
  Image::mirror(axis)
    → ImgBase::mirror(axis, bool)  [virtual]
      → Img<T>::mirror(axis, bool)  [dispatch-only, calls resolveOrThrow]
        → ImgOps::instance().getSelector<MirrorSig>("mirror").resolveOrThrow(this)
          → Backend::Ipp: Img_Ipp.cpp — ippiMirror_8u/16u/32s_C1IR (4 depths)
          → Backend::Cpp: Img_Cpp.cpp — calls Img<T>::mirror(axis, ch, offset, size)

The per-channel Img<T>::mirror(axis, int, Point, Size) is the raw C++ swap
implementation. It never dispatches — backends call it directly.
```

Files created:
- `ICLCore/src/ICLCore/ImgOps.h` — singleton header, dispatch signatures
- `ICLCore/src/ICLCore/ImgOps.cpp` — singleton impl, creates selectors
- `ICLCore/src/ICLCore/Img_Cpp.cpp` — C++ backend (mirror)
- `ICLCore/src/ICLCore/Img_Ipp.cpp` — IPP backend (mirror)
- `ICLCore/src/ICLCore/ImageBackendDispatching.h` — added `ImgBaseBackendDispatching` + `applicableToBase<>`

Changes:
- `ICLCore/CMakeLists.txt` — added `_Ipp.cpp` exclusion pattern (same as ICLFilter)
- `ICLCore/src/ICLCore/Img.cpp` — `Img<T>::mirror(axis, bool)` is now dispatch-only
- `ICLCore/src/ICLCore/Img.h` — made per-channel mirror/normalize public (backends need access)
- `ICLUtils/src/ICLUtils/BackendDispatching.h` — added `resolveOrThrow()`, `#include <stdexcept>`

### Important Rules (Learned This Session)

1. **Never delete IPP/MKL code** — extract to `_Ipp.cpp`/`_Mkl.cpp` backend files.
   IPP specializations have real performance value (BLAS, image ops, etc.).

2. **No `#ifdef ICL_HAVE_IPP` in `_Ipp.cpp` files** — CMake excludes them via
   `list(FILTER SOURCES EXCLUDE REGEX "_Ipp\\.cpp$")` when `!IPP_FOUND`.

3. **All implementations in backend files** — both `_Cpp.cpp` AND `_Ipp.cpp`.
   The main code is dispatch-only. This ensures dispatch works regardless of call path.

4. **MKL follows the same pattern** — `_Mkl.cpp` files, `Backend::Mkl` enum (to be added).

### Previous Session Summary (Session 17)

**BackendDispatching refactoring:**
- Nested `BackendSelectorBase`, `BackendSelector<Sig>`, `ApplicabilityFn` inside `BackendDispatching<Context>`
- `ImageBackendDispatching` is now just `using = BackendDispatching<Image>` (removed `Dispatching` alias)
- All 15 filter headers updated from `core::Dispatching` to `core::ImageBackendDispatching`
- `dispatchEnum` applied to BinaryOp SIMD backends to eliminate inner-loop branching

**Cross-validation tests (20 new, 349 total):**
- Added `crossValidateBackends()` template helper (forces C++ ref, iterates all backend combos)
- All 15 BackendDispatch filters now have cross-validation tests
- Tests cover per-depth validation for all applicable depths

**Benchmarks (25 filter benchmarks):**
- All benchmarks use 1000x1000 (1M pixels) baseline
- Backend parameter: `-p backend=cpp/simd/ipp/auto` for direct comparison

### IPP APIs — What's Active vs Disabled

**ACTIVE (compiles with modern oneAPI IPP 2022+):**

| Backend File | IPP Functions | Filter |
|---|---|---|
| ThresholdOp_Ipp.cpp | `ippiThreshold_LTVal/GTVal_*` | ThresholdOp |
| UnaryCompareOp_Ipp.cpp | `ippiCompareC_*` | UnaryCompareOp |
| UnaryLogicalOp_Ipp.cpp | `ippiAndC/OrC/XorC_*` | UnaryLogicalOp |
| WienerOp_Ipp.cpp | `ippiFilterWiener_*` | WienerOp |
| WarpOp_Ipp.cpp | `ippiRemap_*` | WarpOp |
| Img_Ipp.cpp | `ippiMirror_*` | Img::mirror (new) |
| Img.cpp (inline) | `ippiLUTPalette_*`, `ippiMin/Max*`, `ippiMulC/AddC_*` | Img utilities (TODO) |
| CoreFunctions.cpp (inline) | `ippiMean_*` | channel mean (TODO) |
| DynMatrixUtils.cpp (inline) | `ippsMean_*`, `ippsStdDev_*`, `ippsMeanStdDev_*` | matrix stats (TODO) |
| DynMatrix.h (inline) | `ippsDiv_*`, `ippsMulC_*`, `ippsNorm_*`, `ippsNormDiff_*` | matrix ops (TODO) |
| MathFunctions.h (inline) | `ippsMean_*` | math mean (TODO) |

**DISABLED (deprecated/removed APIs — TODO re-add via BackendDispatch):**

| Location | Deprecated API | Modern Replacement | Priority |
|---|---|---|---|
| `ConvolutionOp_Ipp.cpp` | `ippiFilterSobelHoriz/Vert/Laplace/Gauss_*` | `ippiFilterSobelBorder_*`, `ippiFilterGaussBorder_*` | HIGH — 34 specializations |
| `MorphologicalOp_Ipp.cpp` | `ippiMorphologyInitAlloc_*`, `ippiDilate/Erode_*_C1R` | `ippiDilate/Erode_*_C1R_L` + spec buffers | HIGH |
| `AffineOp_Ipp.cpp` | `ippiWarpAffine_*_C1R` | `ippiWarpAffineNearest/Linear_*` + spec | MEDIUM |
| `MedianOp_Ipp.cpp` | `ippiFilterMedian_*_C1R` | `ippiFilterMedianBorder_*_C1R` | MEDIUM |
| `LUTOp_Ipp.cpp` | `ippiReduceBits_8u_C1R` | Modern `ippiReduceBits` (added noise param) | LOW |
| `CannyOp.cpp` (inline) | `ippiCanny_32f8u/16s8u_C1R` | Modern `ippiCanny` with border spec | MEDIUM |
| `ProximityOp.cpp` | `ippiSqrDistance/CrossCorr Full/Same/Valid_Norm_*` | `ippiSqrDistanceNorm_*` | LOW |
| `Img.cpp` (inline) | `ippiResizeSqrPixel_*` | `ippiResizeLinear/Nearest_*` | MEDIUM |
| `CoreFunctions.cpp` (inline) | `ippiHistogramEven_*` | `ippiHistogram_*` (new API) | LOW |
| `FFTUtils.cpp` (inline) | `ippiFFTInitAlloc_*` | `ippiFFTInit_*` + manual buffers | MEDIUM (or use MKL) |
| `DynMatrix.h/.cpp` (inline) | `ippmMul_mm/Invert/Det/Eigen_*` | MKL BLAS/LAPACK | MEDIUM (ippm module dropped entirely) |
| `DynMatrixUtils.cpp` (inline) | `ippmAdd/Sub/Mul_mm/tm/tt_*` | MKL BLAS | MEDIUM |

### Backend Dispatch Framework

```
BackendDispatching<Context>           — ICLUtils (template, header-only)
  ::BackendSelectorBase               — abstract per-selector base
  ::BackendSelector<Sig>              — typed dispatch table
    .resolve(ctx) → ImplBase*         — returns nullptr if no match
    .resolveOrThrow(ctx) → ImplBase*  — throws logic_error if no match
  ::ApplicabilityFn                   — std::function<bool(const Context&)>

Two context types:
  ImageBackendDispatching             — BackendDispatching<Image>
    Used by: filter operations (UnaryOp, BinaryOp subclasses)
    Applicability: applicableTo<Ts...>(const Image&)

  ImgBaseBackendDispatching           — BackendDispatching<ImgBase*>
    Used by: ImgOps singleton for Img<T> utility methods
    Applicability: applicableToBase<Ts...>(ImgBase* const&)

ImgOps singleton                      — ICLCore
  Inherits ImgBaseBackendDispatching
  Owns selectors for: mirror (more to come)
  Backends registered from: Img_Cpp.cpp, Img_Ipp.cpp

Backend enum: Cpp, Simd, Ipp, OpenCL  — ICLUtils
Priority: OpenCL > Ipp > Simd > Cpp

Self-registration from _Cpp.cpp, _Ipp.cpp, _Simd.cpp, _OpenCL.cpp, _Mkl.cpp files
  registerBackend<Sig>(key, backend, fn, applicability, desc)
  registerStatefulBackend<Sig>(key, backend, factory, applicability, desc)
CMake: _Ipp.cpp excluded when !IPP_FOUND, _OpenCL.cpp when !OPENCL_FOUND
       _Cpp.cpp always built
```

### Remaining Inline `#ifdef ICL_HAVE_IPP` Blocks to Migrate

**ICLCore — via ImgOps singleton (pattern established, mirror done):**
- `Img.h` — `ippiSet_*` (clearChannelROI specializations, 4 depths)
- `Img.cpp` — lut, min/max, normalize, flippedCopyChannelROI (5 remaining blocks)
- `CoreFunctions.cpp` — channel_mean specializations (4 depths)
- `ImgBorder.cpp` — border replication (8u, 32f)
- `CCFunctions.cpp` — planarToInterleaved/interleavedToPlanar macros
- `BayerConverter.h/.cpp` — Bayer pattern conversion
- `Types.h` — conditional enum definitions (compile-time, may stay)

Note: once `flippedCopyChannelROI` is migrated, the `getMirrorPointers` helper
functions can move from `Img.cpp` to `Img_Cpp.cpp`.

**ICLMath — needs own dispatch singleton (similar pattern):**
- `DynMatrix.h` — `ippsNormDiff_L2_*`, `ippsDiv_*`, `ippsMulC_*`, `ippsNorm_*`
- `MathFunctions.h` — `ippsMean_*`
- `DynMatrixUtils.cpp` — mean/stddev/meanstddev (3 blocks), unary math functions (large block)

**ICLIO — needs own dispatch or extend ImgOps:**
- `DC.cpp` — `ippiRGBToGray_8u_C3C1R`
- `ColorFormatDecoder.cpp` — `ippiYUVToRGB_8u_C3R`
- `PylonColorConverter.cpp/.h` — YUV conversion classes

### Docker Build Commands

```bash
# First run (full build with persistent volume):
docker build --platform linux/amd64 -t icl-ipp packaging/docker/noble-ipp
docker run --platform linux/amd64 --rm -e JOBS=16 -e BUILD_DIR=/build-cache \
  -v $(pwd):/src:ro -v icl-ipp-build:/build-cache \
  icl-ipp bash /src/packaging/docker/noble-ipp/build-and-test.sh

# Subsequent runs (incremental — only recompiles changed files):
# Same command — volume "icl-ipp-build" persists CMake state + object files
```

### Key Files

```
ICLUtils/src/ICLUtils/BackendDispatching.h     — framework template (resolveOrThrow added)
ICLUtils/src/ICLUtils/EnumDispatch.h           — dispatchEnum utility
ICLCore/src/ICLCore/ImageBackendDispatching.h  — Image + ImgBase* typedefs
ICLCore/src/ICLCore/ImgOps.h                   — singleton header, dispatch signatures
ICLCore/src/ICLCore/ImgOps.cpp                 — singleton impl, creates selectors
ICLCore/src/ICLCore/Img_Cpp.cpp                — C++ backends (mirror)
ICLCore/src/ICLCore/Img_Ipp.cpp                — IPP backends (mirror)
tests/test-filter.cpp                          — 349 tests
benchmarks/bench-filter.cpp                    — 25 filter benchmarks
packaging/docker/noble-ipp/                    — Docker IPP build
.github/workflows/ci.yaml                     — CI with IPP job
```

### Next Steps

1. **Continue ICLCore ImgOps migration** — add more ops to ImgOps singleton:
   lut, min/max, normalize, clearChannelROI, flippedCopyChannelROI.
   Each gets a `MirrorSig`-style typedef, a selector in `ImgOps()`, and
   implementations in `Img_Cpp.cpp` + `Img_Ipp.cpp`.

2. **Migrate ICLCore non-Img IPP blocks** — CoreFunctions (channel_mean),
   ImgBorder (border replication), CCFunctions (planar↔interleaved),
   BayerConverter. These may need their own dispatch singletons or extend ImgOps.

3. **Migrate ICLMath IPP blocks** — DynMatrix, DynMatrixUtils, MathFunctions.
   Needs a `MathOps` singleton with `BackendDispatching<DynMatrixBase*>` or similar.
   Same pattern applies to MKL (`_Mkl.cpp`).

4. **Migrate ICLIO IPP blocks** — DC.cpp, ColorFormatDecoder.cpp, PylonColorConverter.

5. **Update disabled IPP backends** to modern oneAPI APIs
   (ConvolutionOp, MorphologicalOp, AffineOp — highest priority)

6. **Add `Backend::Mkl`** — enum value, `_Mkl.cpp` files for FFT and matrix ops

7. **Expand benchmarks on Linux** — IPP vs C++ vs SIMD comparison
