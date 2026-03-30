# Image Migration — Continuation Guide

## Current State (Session 18 — Docker IPP Green, ICLFilter IPP Migration)

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
- Use named Docker volume for persistent build cache: `-v icl-ipp-build:/build-cache -e BUILD_DIR=/build-cache`
- Dockerfile adds `rsync` package

**ICLFilter IPP migration — complete:**
- Extracted `WarpOp` inline IPP (`ippiRemap_8u/32f_C1R`) to `WarpOp_Ipp.cpp` as `Backend::Ipp`
- Removed `#ifdef ICL_HAVE_IPP` from NeighborhoodOp.cpp (anchor workaround now always-on)
- Removed `#ifdef ICL_HAVE_IPP` from LocalThresholdOp.cpp (C++ path is both faster and higher quality)
- Removed `#ifdef ICL_HAVE_IPP` from UnaryOp.cpp (Canny creator — works without IPP now)
- Removed redundant `#ifdef ICL_HAVE_IPP` guards from all `_Ipp.cpp` files (CMake already excludes them)
- ICLFilter now has zero active `#ifdef ICL_HAVE_IPP` in non-backend files

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
- Covers: Threshold, UnaryArithmetical, UnaryCompare, UnaryLogical, Convolution, Median, Morphological, BinaryArithmetical, BinaryCompare, BinaryLogical, WarpOp, BilateralFilterOp
- Image-based API (replaced old ImgBase* calls)

**Docker IPP build (in progress):**
- Dockerfile: Ubuntu 24.04 + Intel oneAPI IPP (`packaging/docker/noble-ipp/`)
- Build script copies source writable, configures with IPP, builds Release
- **BUILD STATUS: Almost green** — last run had namespace fixes for `_Ipp.cpp` files, awaiting confirmation

### Docker IPP Build — Remaining Work

The Docker build (`docker run --platform linux/amd64 ...`) was iteratively fixed. The last commit (`658291b2`) includes all fixes. **Run this to verify:**

```bash
cd /Users/celbrech/projects/icl
docker build --platform linux/amd64 -t icl-ipp packaging/docker/noble-ipp
docker run --platform linux/amd64 --rm -e JOBS=16 -v $(pwd):/src:ro icl-ipp \
  bash /src/packaging/docker/noble-ipp/build-and-test.sh
```

If there are still errors, they'll be namespace issues in remaining `_Ipp.cpp` files (add `using namespace icl;`).

### IPP APIs — What's Active vs Disabled

**ACTIVE (compiles with modern oneAPI IPP 2022+):**

| Backend File | IPP Functions | Filter |
|---|---|---|
| ThresholdOp_Ipp.cpp | `ippiThreshold_LTVal/GTVal_*` | ThresholdOp |
| UnaryCompareOp_Ipp.cpp | `ippiCompareC_*` | UnaryCompareOp |
| UnaryLogicalOp_Ipp.cpp | `ippiAndC/OrC/XorC_*` | UnaryLogicalOp |
| WienerOp_Ipp.cpp | `ippiFilterWiener_*` | WienerOp |
| WarpOp.cpp (inline) | `ippiRemap_*` | WarpOp |
| CoreFunctions.cpp (inline) | `ippiMean_*` | channel mean |
| Img.cpp (inline) | `ippiLUTPalette_*`, `ippiMirror_*`, `ippiMin/Max*`, `ippiMulC/AddC_*` | Img utilities |
| DynMatrixUtils.cpp (inline) | `ippsMean_*`, `ippsStdDev_*`, `ippsMeanStdDev_*` | matrix stats |
| DynMatrix.h (inline) | `ippsDiv_*`, `ippsMulC_*`, `ippsNorm_*`, `ippsNormDiff_*` | matrix ops |
| MathFunctions.h (inline) | `ippsMean_*` | math mean |
| ImageRectification.cpp | `ippiWarpPerspective_*` — **DISABLED** (API changed) | ImageRectification |

**DISABLED (deprecated/removed APIs — TODO re-add via BackendDispatch):**

| Location | Deprecated API | Modern Replacement | Priority |
|---|---|---|---|
| `ConvolutionOp_Ipp.cpp` | `ippiFilterSobelHoriz/Vert/Laplace/Gauss_*` | `ippiFilterSobelBorder_*`, `ippiFilterGaussBorder_*` | HIGH — 34 specializations |
| `MorphologicalOp_Ipp.cpp` | `ippiMorphologyInitAlloc_*`, `ippiDilate/Erode_*_C1R` | `ippiDilate/Erode_*_C1R_L` + spec buffers | HIGH |
| `AffineOp_Ipp.cpp` | `ippiWarpAffine_*_C1R` | `ippiWarpAffineNearest/Linear_*` + spec | MEDIUM |
| `MedianOp_Ipp.cpp` | `ippiFilterMedian_*_C1R` | `ippiFilterMedianBorder_*_C1R` | MEDIUM |
| `LUTOp_Ipp.cpp` | `ippiReduceBits_8u_C1R` | Modern `ippiReduceBits` (added noise param) | LOW |
| `CannyOp.cpp` (inline) | `ippiCanny_32f8u/16s8u_C1R` | Modern `ippiCanny` with border spec | MEDIUM |
| `ImageRectification.cpp` (inline) | `ippiWarpPerspectiveBack_*_C1R` | Modern `ippiWarpPerspective*` + spec | LOW |
| `ProximityOp.cpp` (inline) | `ippiSqrDistance/CrossCorr Full/Same/Valid_Norm_*` | `ippiSqrDistanceNorm_*` | LOW |
| `Img.cpp` (inline) | `ippiResizeSqrPixel_*` | `ippiResizeLinear/Nearest_*` | MEDIUM |
| `CoreFunctions.cpp` (inline) | `ippiHistogramEven_*` | `ippiHistogram_*` (new API) | LOW |
| `FFTUtils.cpp` (inline) | `ippiFFTInitAlloc_*` | `ippiFFTInit_*` + manual buffers | MEDIUM (or use MKL) |
| `DynMatrix.h/.cpp` (inline) | `ippmMul_mm/Invert/Det/Eigen_*` | MKL BLAS/LAPACK | MEDIUM (ippm module dropped entirely) |
| `DynMatrixUtils.cpp` (inline) | `ippmAdd/Sub/Mul_mm/tm/tt_*` | MKL BLAS | MEDIUM |

**Goal:** Zero `#ifdef ICL_HAVE_IPP` in source files. All IPP code moves to self-registering `_Ipp.cpp` backend files via `BackendDispatching`. CMake conditionally includes `_Ipp.cpp` when IPP is found.

### Inline IPP blocks to migrate to BackendDispatch

These are `#ifdef ICL_HAVE_IPP` blocks still inside regular `.cpp` files (not `_Ipp.cpp` backends). Each needs to be extracted into a self-registering backend file:

- `ICLCore/Img.cpp` — mirror, min/max, LUT palette, arithmetic, resize (6 blocks)
- `ICLCore/CoreFunctions.cpp` — mean, histogram (2 blocks)
- `ICLMath/DynMatrix.h` — norm, div, mulC (1 block with multiple ops)
- `ICLMath/DynMatrixUtils.cpp` — mean, stddev, meanstddev (3 blocks)
- `ICLMath/MathFunctions.h` — mean (1 block)
- `ICLFilter/WarpOp.cpp` — ippiRemap (1 block)
- `ICLFilter/CannyOp.cpp` — ippiCanny (1 block)
- `ICLFilter/NeighborhoodOp.cpp` — IPP anchor workaround (3 blocks)
- `ICLCV/CV.cpp` — cross-correlation (1 block)
- `ICLCV/CornerDetectorCSS.cpp` — convolution (1 block)
- `ICLIO/ColorFormatDecoder.cpp` — YUV conversion (1 block)
- `ICLIO/DC.cpp` — RGB to gray (1 block)
- `ICLIO/PylonColorConverter.cpp` — color conversion (1 block)

### Benchmark Results (Release, M3 Mac, 1000x1000)

**SIMD vs C++:**
| Benchmark | C++ (us) | SIMD (us) | Speedup |
|-----------|----------|-----------|---------|
| threshold.ltgt_32f | 483 | 158 | 3.1x SIMD |
| arithmetic.mul_32f | 618 | 215 | 2.9x SIMD |
| median.3x3_32f | 1132 | 606 | 1.9x SIMD |
| arithmetic.sqrt_32f | 346 | 217 | 1.6x SIMD |
| arithmetic.sqr_32f | 155 | 217 | 0.7x (C++ wins) |
| arithmetic.abs_32f | 152 | 217 | 0.7x (C++ wins) |

Note: On M3 Mac, SIMD goes through sse2neon. Compiler auto-vectorizes simple C++ ops better.

**OpenCL vs C++:**
| Benchmark | OpenCL (us) | C++ (us) | Speedup |
|-----------|-------------|----------|---------|
| bilateral.mono_8u | 8,678 | 45,091 | 5.2x OpenCL |
| bilateral.mono_32f | 9,272 | 41,793 | 4.5x OpenCL |
| warp.nn_8u | 13,671 | 5,732 | 0.4x (C++ wins) |
| warp.lin_8u | 14,314 | 6,185 | 0.4x (C++ wins) |

### FFT Thread Safety — RESOLVED

Confirmed not an issue. `fft()` and `ifft_()` are pure recursive with `new[]` per call. `fft2D_cpp`/`ifft2D_cpp` take `buf`/`dst` by reference. No global/static mutable buffers. Tests pass reliably at `-j 8`.

### Architecture

```
Public API:  Image (value type, shared_ptr<ImgBase>)
                |
Internal:    ImgBase → Img<T>  (implementation detail)
```

### Backend Dispatch Framework

```
BackendDispatching<Context>           — ICLUtils (template, header-only)
  ::BackendSelectorBase               — abstract per-selector base
  ::BackendSelector<Sig>              — typed dispatch table
  ::ApplicabilityFn                   — std::function<bool(const Context&)>

ImageBackendDispatching               — ICLCore (typedef to BackendDispatching<Image>)
FFTDispatching                        — ICLMath (BackendDispatching<FFTContext>)

Backend enum: Cpp, Simd, Ipp, OpenCL  — ICLUtils
Priority: OpenCL > Ipp > Simd > Cpp

Self-registration: _Ipp.cpp, _Simd.cpp, _OpenCL.cpp files
  registerBackend<Sig>(key, backend, fn, applicability, desc)
  registerStatefulBackend<Sig>(key, backend, factory, applicability, desc)
CMake: _Ipp.cpp excluded when !IPP_FOUND, _OpenCL.cpp when !OPENCL_FOUND
```

### Test Infrastructure

- 349 tests total, all passing (macOS)
- `crossValidateBackends()` helper for backend comparison
- `testROIHandling()` helper for ROI testing
- Benchmark framework: `BenchmarkRegistrar` with backend parameter

### Key Files

```
ICLUtils/src/ICLUtils/BackendDispatching.h   — framework template
ICLUtils/src/ICLUtils/EnumDispatch.h         — dispatchEnum utility
ICLCore/src/ICLCore/ImageBackendDispatching.h — Image typedef
tests/test-filter.cpp                        — 349 tests
benchmarks/bench-filter.cpp                  — 25 filter benchmarks
packaging/docker/noble-ipp/                  — Docker IPP build
.github/workflows/ci.yaml                   — CI with IPP job
filter-benchmarks-and-tests-plan.md          — detailed plan with progress
```

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

### Remaining Inline `#ifdef ICL_HAVE_IPP` Blocks (non-filter)

**ICLCore (highest volume):**
- `Img.h` — `ippiSet_*` (clearChannelROI specializations, 3 depths)
- `Img.cpp` — lut, mirror, min/max, normalize, flippedCopy (6 blocks)
- `CoreFunctions.cpp` — channel_mean specializations (4 depths)
- `ImgBorder.cpp` — border replication (8u, 32f)
- `CCFunctions.cpp` — planarToInterleaved/interleavedToPlanar macros
- `BayerConverter.h/.cpp` — Bayer pattern conversion
- `Types.h` — conditional enum definitions

**ICLMath:**
- `DynMatrix.h` — `ippsNormDiff_L2_*` (distance)
- `MathFunctions.h` — `ippsMean_*`
- `DynMatrixUtils.cpp` — mean/stddev/meanstddev, unary math functions

**ICLIO:**
- `DC.cpp` — `ippiRGBToGray_8u_C3C1R`
- `ColorFormatDecoder.cpp` — `ippiYUVToRGB_8u_C3R`
- `PylonColorConverter.cpp/.h` — YUV conversion classes

### Next Steps

1. **Migrate ICLCore inline IPP blocks** — extract to `_Ipp.cpp` files (need `BackendDispatching` for utility functions or simpler function-pointer dispatch)
2. **Migrate ICLMath inline IPP blocks** — matrix/mean functions
3. **Migrate ICLIO inline IPP blocks** — color conversion
4. **Update disabled IPP backends** to modern oneAPI APIs (ConvolutionOp, MorphologicalOp, AffineOp — highest priority)
5. **Add MKL as a backend** — `Backend::Mkl`, `_Mkl.cpp` files for FFT and matrix ops
6. **Expand benchmarks on Linux** — IPP vs C++ vs SIMD comparison
