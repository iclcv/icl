# Image Migration — Continuation Guide

## Current State (Session 24 — LapackOps expansion, API cleanup, DynMatrixBase split)

### Session 24 Summary

**New LapackOps (3 new ops, 2 gap-fills):**
- `geqrf` + `orgqr` (QR factorization via Householder reflectors) — all 4 backends
- `gelsd` (SVD least-squares solve) — all 4 backends
- `getrf`/`getri` added to MKL and Eigen backends (were missing)

**Accelerate backend row-major fix:**
- getrf, getri, geqrf, orgqr, gelsd now transpose explicitly to/from column-major
  before calling LAPACK. Ensures packed output (L/U, Householder reflectors)
  matches C++ backend convention. gesdd/syev keep dimension-swap trick.

**Consumer wiring:**
- `decompose_QR()` → geqrf + orgqr (was hand-written Gram-Schmidt)
- `decompose_LU()` → getrf + unpack (was hand-written partial pivoting)
- `solve()` → gelsd directly (was string-based method dispatch with "lu"/"svd"/"qr"/"inv")
- `pinv()` → always reduced SVD + BLAS gemm (was bool useSVD with QR fallback)
- `mult()` → BLAS gemm for float/double (was hand-written inner_product loop)
- `matrix_mult_t()` → gemm transpose flags for float/double (no temp copies)

**API simplification:**
- Removed `big_matrix_pinv()` — merged into `pinv()`
- Removed `big_matrix_mult_t()` — merged into `matrix_mult_t()`
- Removed `pinv(bool useSVD)` parameter — always SVD
- Removed `solve(b, string method)` parameter — always gelsd
- Removed `solve_upper_triangular()` / `solve_lower_triangular()` — dead code
- Removed `PolynomialRegression::apply()` useSVD parameter
- Removed dead `#if 0` block in DynMatrixUtils.h (old svd_cpp_64f)
- Removed dead EigenICLConverter (zero callers)

**DynMatrixBase<T> refactor — compilation time reduction:**
- New `DynMatrixBase<T>` (header-only, ~290 lines): storage, element access,
  flat iterators, properties, operator<</>>
- `DynMatrix<T>` inherits DynMatrixBase, adds col/row iterators, DynMatrixColumn,
  all arithmetic and linalg — declarations only in header (~200 lines)
- All method bodies in `DynMatrix.cpp` (~700 lines), whole-class instantiation
  for float/double: `template class ICLMath_API DynMatrix<float/double>;`
- `DynVector.h` declarations only, `DynVector.cpp` with whole-class instantiation
- DynMatrix.h includes only `<iterator>` beyond ICL headers (was `<numeric>`,
  `<functional>`, `<vector>`, `<cmath>`, `<algorithm>`)
- Concatenation operators (operator,/%) moved out of line

**License header cleanup:**
- Replaced 29-line decorative headers with 3-line SPDX across 1038 files (~27,600 lines removed)
- Format: `SPDX-License-Identifier: LGPL-3.0-or-later` / `ICL - Image Component Library (URL)` / `Copyright (original authors)`
- New top-level LICENSE file with project info and EXC 277 acknowledgment

**Tests: 364/364 pass (15 new).** Build clean, zero warnings on macOS.

### LapackOps Summary (8 operations, 4 backends)

| Op | Signature | C++ | Accelerate | MKL | Eigen |
|---|---|---|---|---|---|
| gesdd | SVD divide-and-conquer | ✓ | ✓ | ✓ | ✓ |
| syev | Symmetric eigenvalue | ✓ | ✓ | ✓ | ✓ |
| getrf | LU factorization | ✓ | ✓ | ✓ | ✓ |
| getri | LU inverse | ✓ | ✓ | ✓ | ✓ |
| geqrf | QR factorization | ✓ | ✓ | ✓ | ✓ |
| orgqr | Form Q from reflectors | ✓ | ✓ | ✓ | ✓ |
| gelsd | SVD least-squares solve | ✓ | ✓ | ✓ | ✓ |
| gemm | Matrix multiply (BlasOps) | ✓ | ✓ | ✓ | — |

### DynMatrix File Layout

```
DynMatrixBase.h   — storage, element access, streaming (header-only, any type)
DynMatrix.h       — col/row iterators, arithmetic/linalg declarations (~200 lines)
DynMatrix.cpp     — all method bodies + whole-class instantiation float/double
DynVector.h       — DynColVector/DynRowVector declarations
DynVector.cpp     — all method bodies + whole-class instantiation float/double
```

### Next Steps

- **Investigate legacy test stubs** — per-module test executables compile to 0 tests
- **NeighborhoodOp.cpp** — 2 `#ifdef ICL_HAVE_IPP` workaround blocks (anchor bug)
- **Re-enable IPP backends** on Linux — update to modern oneAPI APIs
- **Consider DynMatrixBase for non-float/double users** — GraphCutter (bool),
  masks (unsigned char) could use DynMatrixBase directly instead of DynMatrix

## Previous State (Session 23 — Full Backend Dispatch Architecture)

### Session 23 Summary

**FFTOps_Cpp.cpp segfault fixed:**
- Root cause: bug in `fft2D_cpp` — `buf(src.rows(),src.cols())` was element access
  (operator()) on a null pointer, not resize. Fix: one-liner → `buf.setBounds(...)`.
- FFTOps_Cpp.cpp keeps non-owning wrappers for src/dst (zero-copy, no overhead).

**FFTUtils.cpp fully wired to FFTOps — 21 MKL blocks eliminated:**
- Generic `fft2D`/`ifft2D` templates dispatch through FFTOps with type conversion.
- FFTDispatching.h/.cpp deleted. All explicit MKL specializations removed.

**SVD deduplicated — svd_dyn dispatches through LapackOps:**
- Single generic `svd_dyn<T>` template, no float/double specializations.
- Handles LAPACK Vt → ICL V convention via post-dispatch transpose.

**GLImg.cpp — last active IPP block removed.**

**Dead code cleanup — ~1,600 lines removed total:**
- `#if 0` blocks in FFTUtils.cpp, DynMatrixUtils.cpp, DynMatrix.cpp deleted.
- PThreadFix.h stripped to empty header.
- `jacobi_iterate_vtk` + `find_eigenvectors` removed from DynMatrix.cpp (moved to
  LapackOps_Cpp.cpp as `cpp_syev`).
- `get_minor_matrix` removed (inv() no longer uses cofactor expansion).

**Apple Accelerate backend:**
- CMake auto-detects on macOS via `find_library(Accelerate)`.
- `BlasOps_Accelerate.cpp`: cblas_sgemm/dgemm.
- `FFTOps_Accelerate.cpp`: vDSP_DFT row-column decomposition (arbitrary sizes).
- `LapackOps_Accelerate.cpp`: sgesdd/dgesdd, ssyev/dsyev, sgetrf/dgetrf, sgetri/dgetri.

**LapackOps dispatcher — clean BLAS/LAPACK separation:**
- `LapackOps.h`: `enum LapackOp { gesdd, syev, getrf, getri }`.
- `LapackOps.cpp`: constructor, instance(), toString().
- gesdd moved from BlasOps to LapackOps (all backends updated, all consumers rewired).
- BlasOps now contains only GEMM.

**LapackOps backends:**
- `LapackOps_Cpp.cpp`: gesdd (Golub-Kahan), syev (Jacobi), getrf (LU), getri (LU inverse).
- `LapackOps_Accelerate.cpp`: gesdd, syev, getrf, getri via Apple LAPACK.
- `LapackOps_Mkl.cpp`: gesdd, syev via MKL LAPACK.
- `LapackOps_Eigen.cpp`: gesdd (JacobiSVD), syev (SelfAdjointEigenSolver) via Eigen3.

**Backend enum expanded:**
- Added `Backend::Eigen = 3` between OpenBlas and Ipp.
- Priority order: Cpp(0) < Simd(1) < OpenBlas(2) < Eigen(3) < Ipp(4) < FFTW(5)
  < Accelerate(6) < Mkl(7) < OpenCL(8).

**Consumer wiring:**
- `svd_dyn()` → `LapackOps<T>::gesdd`
- `big_matrix_pinv()` → `LapackOps<T>::gesdd` + `BlasOps<T>::gemm`
- `DynMatrix::eigen()` → `LapackOps<T>::syev` (resolveOrThrow, C++ fallback always available)
- `DynMatrix::inv()` → `LapackOps<T>::getrf` + `getri` (O(n³) LU, replaces O(n!) cofactor)
- `DynMatrix::det()` for n>4 → `LapackOps<T>::getrf` (product of U diagonal, replaces O(n!) cofactor)

**C++ fallback convention enforced:**
- All C++ backend registrations now in `_Cpp.cpp` files.
- MathOps C++ backends moved from DynMatrixUtils.cpp → `MathOps_Cpp.cpp`.
- ImgOps channelMean moved from CoreFunctions.cpp → `Img_Cpp.cpp`.
- Only exception: ImgBorder.cpp (depends on file-local `_copy_border`, documented).

**Design decisions:**
- BlasOps for GEMM only; element-wise ops stay inlined (bandwidth-bound).
- FixedMatrix (4x4): keep inlined, IPP ippm dropped, compiler auto-vectorization sufficient.
- det() keeps special cases for 1x1–4x4 (zero overhead); n>4 uses LU.

**Tests: 349/349 pass.** Build clean on macOS.

### Next Steps

- **Add more LapackOps** — geqrf (QR factorization) to accelerate decompose_QR(), solve()
- **Investigate legacy test stubs** — Per-module test executables compile to 0 tests
- **NeighborhoodOp.cpp** — Two `#ifdef ICL_HAVE_IPP` workaround blocks (anchor bug)

## Previous State (Session 22 — BlasOps/FFTOps + BackendDispatching Refactor + BLAS Consumer Wiring)

### Session 22 Summary (continued)

**New BLAS/FFT abstraction layer + BackendDispatching refactor.**

**BackendDispatching refactored: array → sorted vector:**
- `std::array<ImplPtr, NUM_BACKENDS>` replaced with sorted `std::vector<Entry>`
- Backends sorted by priority descending (highest first); only registered backends occupy space
- `NUM_BACKENDS` constant removed entirely — enum can grow freely
- New `resolve()` / `resolveOrThrow()` no-arg overloads for dummy-context singletons
- New `Entry` struct: `{ Backend backend; shared_ptr<ImplBase> impl; }`
- `setImpl()` private helper maintains sorted order on insertion

**Backend enum expanded (values encode priority):**
```
Cpp=0, Simd=1, OpenBlas=2, Ipp=3, FFTW=4, Accelerate=5, Mkl=6, OpenCL=7
```

**BlasOps\<T\> — BLAS/LAPACK abstraction (new):**
- `BlasOps.h` — singleton template, `enum class BlasOp { gemm, gesdd }`
- `BlasOps.cpp` — constructor (addSelector), instance(), toString()
- `BlasOps_Cpp.cpp` — C++ fallback: naive GEMM + SVD via existing `svd_dyn`
- `BlasOps_Mkl.cpp` — MKL backend: `cblas_sgemm/dgemm` + `sgesdd/dgesdd`
- Raw pointer interface (no DynMatrix dependency in signatures)
- Consumer code will use `resolveOrThrow()` — C++ fallback always available

**FFTOps\<T\> — FFT abstraction (new, replaces FFTDispatching):**
- `FFTOps.h` — singleton template, `enum class FFTOp { r2c, c2c, inv_c2c }`
- `FFTOps.cpp` — constructor, instance(), toString()
- `FFTOps_Cpp.cpp` — C++ fallback via existing `fft2D_cpp` / `ifft2D_cpp`
- `FFTOps_Mkl.cpp` — MKL DFTI backend (forward/inverse, real/complex)
- Raw pointer interface; DynMatrix wrapping stays in consumers
- `FFTOps<float>` + `FFTOps<double>` — separate singletons per precision

**FFTDispatching transitional rename:**
- `FFTOp` enum → `LegacyFFTOp` to avoid conflict with FFTOps
- FFTDispatching struct kept temporarily until FFTUtils.cpp is rewired
- Will be deleted once FFTUtils.cpp uses FFTOps directly

**Convention: C++ fallbacks in `_Cpp.cpp` files:**
- All backend implementations (including C++ fallback) go in `_<Backend>.cpp` files
- The `.cpp` file (e.g., `BlasOps.cpp`) only contains constructor, instance(), toString()
- Pattern: `BlasOps_Cpp.cpp`, `BlasOps_Mkl.cpp`, `FFTOps_Cpp.cpp`, `FFTOps_Mkl.cpp`

**CMake: new exclusion patterns in ICLMath/CMakeLists.txt:**
- `_FFTW.cpp` excluded when `!FFTW_FOUND`
- `_Accelerate.cpp` excluded when `!ACCELERATE_FOUND`
- `_OpenBlas.cpp` excluded when `!OPENBLAS_FOUND`

**BLAS consumers wired (DynMatrix + DynMatrixUtils):**
- `DynMatrix.cpp`: `big_matrix_pinv` now uses `BlasOps<T>::gesdd` + `gemm` via
  `resolveOrThrow()`. No fallback logic — C++ backend always available.
- `DynMatrixUtils.cpp`: `big_matrix_mult_t` now uses `BlasOps<T>::gemm` via
  `resolveOrThrow()`. MKL explicit specializations removed.
- All `#ifdef ICL_HAVE_MKL` removed from DynMatrix.h, DynMatrix.cpp, DynMatrixUtils.cpp (6 blocks).
- `BlasOps_Cpp.cpp` GEMM optimized: compile-time dispatch of transA/transB/alpha/beta
  via template specialization + `if constexpr` (eliminates inner-loop conditionals).
  Epsilon-based `isZero()`/`isOne()` for float-safe alpha/beta comparison.
- `BlasOps_Cpp.cpp` SVD: full Golub-Kahan bidiagonalization (`svd_bidiag`) moved here
  from DynMatrixUtils.cpp. C++ GESDD backend always works (no more stub returning -1).

**FFT consumer wiring attempted but reverted (segfault):**
- Rewrote `fft2D<T1,T2>` and `ifft2D<T1,T2>` generic templates to use FFTOps dispatch.
  All 19 explicit specializations with `#ifdef ICL_HAVE_MKL` eliminated (replaced by
  template instantiations). The generic template handles type conversion + dispatch.
- **Segfault in multi-threaded tests.** Root cause: `FFTOps_Cpp.cpp` wraps raw pointers
  in non-owning `DynMatrix(cols, rows, ptr, false)` and passes to `fft2D_cpp`. The FFT
  row-column decomposition writes to `dst` with transposed dimensions — the non-owning
  wrapper's buffer may be too small or have wrong stride. Fix: use owned DynMatrix
  intermediates in the C++ backend, copy result back to raw pointer at the end.
- **Changes reverted** to keep branch green. FFTUtils.cpp still has 21 MKL blocks.

**Remaining `ICL_HAVE_MKL` references:**
- FFTUtils.cpp: 21 blocks (2 wrapper blocks + 19 specialization blocks, all still active)
- Camera.cpp: 1 (dead comment)
- PThreadFix.h: 1 (dead, inside `#if 0`)
- ICLConfig.h.in / doxyfile.in: build system definitions (must stay)

**Tests: 349/349 pass.** Build clean on macOS.

### Next Steps

- **Fix FFTOps_Cpp.cpp segfault** — Use owned DynMatrix in C++ FFT backend instead of
  non-owning wrappers. The issue is `fft2D_cpp`'s row-column decomposition writes `dst`
  with transposed buffer layout. Either: (a) allocate an owned DynMatrix, call fft2D_cpp,
  copy result to raw pointer; or (b) rewrite the C++ FFT backend to work directly on
  raw pointers without going through fft2D_cpp.
- **Wire FFTUtils.cpp to FFTOps** — The generic template rewrite is ready (tested
  single-threaded), just needs the C++ backend fix above. Once fixed, 21 MKL blocks
  disappear and FFTDispatching.h/.cpp can be deleted.
- **Deduplicate SVD** — `svd_internal` now lives in both `BlasOps_Cpp.cpp` and
  `DynMatrixUtils.cpp`. Wire `svd_dyn()` to dispatch through `BlasOps<T>::gesdd`,
  then remove the old copy from DynMatrixUtils.cpp.
- **Camera.cpp** — Remove dead MKL comment (trivial)
- **GLImg.cpp** — Last active `#ifdef ICL_HAVE_IPP` block (min/max, could use ImgOps)
- **Add Accelerate backend** — `BlasOps_Accelerate.cpp` + `FFTOps_Accelerate.cpp`
  for macOS (cblas via Accelerate framework, vDSP FFT)
- **Consider LapackOps** — Separate dispatch for LAPACK operations (eigenvalue
  decomposition, Cholesky, LU, etc.) beyond just GESDD
- **FixedMatrix BLAS** — For small matrices (commonly 4x4), BlasOps dispatch overhead
  is too high. Keep inlined implementations. IPP's `ippm` module (small matrix ops) was
  dropped from modern IPP. Best choice for small matrices: compiler auto-vectorization
  of the inlined code, or hand-written SIMD for the hot 4x4 case.

## Previous State (Session 21 — A2 + B + C/IPP Complete)

### Session 21 Summary

**A2 complete, B complete, C/IPP complete for all modules.** All active
`#ifdef ICL_HAVE_IPP` operational code across ICLUtils/ICLCore/ICLMath/
ICLFilter/ICLIO has been migrated to dispatch or removed with TODOs.

**A2 — all three phases:**

**Phase 1 — Global string registry removed:**
- Deleted `detail::RegistryEntry`, `detail::globalRegistry()`, `detail::addToRegistry()`
- Deleted `registerBackend()`, `registerStatefulBackend()` static methods
- Deleted `loadFromRegistry()` private method
- Deleted `BackendDispatching.cpp` (only contained the registry impl)
- Removed `m_selectorByName` map and `m_prefix` string member
- Removed `initDispatching()` — all 15 filters, ImgOps, FFTDispatching updated
- Removed string-keyed `addSelector(string)`, `getSelector(string)`, `selectorByName(string)`
- Enum-keyed `addSelector(K)` is now standalone (no delegation to string version)
- Added `selector(K)` returning `BackendSelectorBase*` (replaces `selectorByName` for tests)
- Fixed `ThresholdOp_Simd.cpp` — switched from global registry to prototype direct registration

**Phase 2 — Stateful backend cloning (factory pattern):**
- `ImplBase` now has `std::function<shared_ptr<ImplBase>()> cloneFn` (null for stateless)
- `BackendSelector::clone()`: calls `cloneFn()` for stateful backends, shares `shared_ptr` for stateless
- `BackendSelector::addStateful(b, factory, applicability, desc)` — factory called per clone
- `BackendDispatching::addStatefulBackend<Sig>(key, b, factory, app, desc)` — convenience
- Migrated 4 stateful backends:
  - `WienerOp_Ipp.cpp` — IPP scratch buffer now per-instance
  - `WarpOp_OpenCL.cpp` — CLWarpState (GPU buffers/kernels) now per-instance
  - `BilateralFilterOp_OpenCL.cpp` — CLBilateralState now per-instance
  - `MorphologicalOp_Ipp.cpp` — MorphIppState (IPP state objects) now per-instance

**Phase 3 — AffineOp test bug fixed:**
- `AffineOp_Cpp.cpp`: bilinear interpolation bounds check now correctly validates
  the full 2x2 neighborhood (`x2 < width-1, y2 < height-1`)
- Edge pixels outside the safe bilinear zone fall back to nearest-neighbor
- **349/349 tests pass both single-threaded AND multi-threaded** (SIGTRAP eliminated)

**FFTDispatching migrated to enum pattern:**
- Added `enum class FFTOp : int { fwd32f, inv32f, fwd32fc }` with `toString()`
- Selector setup moved to `FFTDispatching()` constructor
- `FFTUtils.cpp` now uses `getSelector<Sig>(FFTOp::xxx)` (enum-keyed O(1))

**Test changes:**
- All `selectorByName("name")` calls replaced with `selector(FilterOp::Op::name)`

**B: ICLCore IPP blocks — all active blocks migrated:**
- `CCFunctions.cpp`: `planarToInterleaved`/`interleavedToPlanar` IPP specializations
  extracted to `Img_Ipp.cpp` as ImgOps selectors. Generic template now dispatches
  through ImgOps for same-type cases (S==D), falling back to `_Generic` if no backend.
  The `#ifdef` block + conditional instantiations removed. Two new `ImgOps::Op` values added.
- `BayerConverter.h/.cpp`: Removed dead IPP code. `nnInterpolationIpp()` was defined but
  never called (the `apply()` method always calls `nnInterpolation()` instead).
  `m_IppBayerPattern` member and constructor switch statements removed.
- Only `Types.h` retains compile-time `ICL_HAVE_IPP` guards (enum definitions, stays).

**BackendProxy for terser registration:**
- Added `BackendProxy` struct + `backends(Backend b)` method on `BackendDispatching`
- All ~65 `addBackend`/`addStatefulBackend` call sites across _Cpp/_Ipp/_Simd/_OpenCL files
  migrated to use the proxy: `auto cpp = proto.backends(Backend::Cpp); cpp.add<Sig>(...);`
- Removed `addBackend`/`addStatefulBackend` as public API — proxy calls getSelector().add() directly

**C/ICLMath — MathOps dispatch framework + all IPP removed:**
- Created `MathOps<T>` template singletons (`MathOps<float>`, `MathOps<double>`)
  using `BackendDispatching<int>` (dummy context, no applicability checks)
- `enum class MathOp` with 11 selectors: mean, var, meanvar, min, max, minmax,
  unaryInplace, unaryCopy, unaryConstInplace, unaryConstCopy, binaryCopy
- Sub-operation enums: `UnaryMathFunc` (12 ops), `UnaryConstFunc` (5 ops), `BinaryMathFunc` (6 ops)
- `DynMatrixUtils.cpp`: entire `#ifdef ICL_HAVE_IPP` block (lines 80-548) replaced with
  MathOps dispatch + C++ backend registration
- `DynMatrix.h`: removed IPP specializations (sqrDistanceTo, distanceTo, elementwise_div,
  mult-by-scalar, norm), added TODO
- `MathFunctions.h`: removed IPP mean specializations, added TODO
- `FixedMatrix.h`: removed unused `#include <ipp.h>`
- `CMakeLists.txt`: added `_Ipp.cpp`/`_Mkl.cpp` exclusion patterns

**C/ICLIO — all IPP removed:**
- `DC.cpp`: removed `ippiRGBToGray_8u_C3C1R`, always use weighted-sum loop
- `ColorFormatDecoder.cpp`: removed `ippiYUVToRGB_8u_C3R` + `ippiCbYCr422ToRGB_8u_C2C3R`
- `PylonColorConverter.h/.cpp`: removed `Yuv422ToRgb8Icl` and `Yuv422YUYVToRgb8Icl`
  IPP-only classes, always use PylonColorToRgb fallback
- TODOs added at every removal site

### Remaining `ICL_HAVE_IPP` References

Only these remain in the codebase:

| Category | Files | Status |
|---|---|---|
| Type definitions | BasicTypes.h, Size.h, Point.h, Rect.h, Types.h | Compile-time aliases, must stay |
| `#if 0` dead code | 5 *_Ipp.cpp files, FFTUtils (2), CannyOp, ImageRectification, CoreFunctions.cpp (histogramEven), DynMatrixUtils.cpp (ippm matrix ops) | Disabled, contains code for future re-enablement |
| IPP bug workaround | NeighborhoodOp.cpp (2 blocks) | Minor, stays |
| Qt GPU upload | GLImg.cpp (1 block) | ICLQt, deferred |
| Comments | CCFunctions.cpp (2 `/* */` blocks), DynMatrix.cpp (`#if 1 // was:`), DynMatrixUtils.cpp (SVD comment), IntegralImgOp.cpp (`ICL_HAVE_IPP_DEACTIVATED_...`) | Dead |
| CMake / config | CMakeLists.txt, ICLConfig.h.in, doxyfile.in | Build system definitions |
| Dead (nested `#if 0`) | PThreadFix.h | Inside outer `#if 0`, unreachable |

### Future IPP Optimization Opportunities (TODOs in code)

| Location | IPP Function | What It Accelerates |
|---|---|---|
| BayerConverter.cpp | `ippiCFAToRGB_8u_C1C3R` | Bayer → RGB nearest-neighbor |
| DynMatrix.h | `ippsNormDiff_L2`, `ippsDiv`, `ippsMulC`, `ippsNorm_L1/L2` | Matrix distance, element-wise div, scalar mult, norms |
| MathFunctions.h | `ippsMean_32f/64f` | Scalar mean over float/double arrays |
| DynMatrixUtils.cpp | All math ops already dispatched | Re-enable via `DynMatrixUtils_Ipp.cpp` |
| DC.cpp | `ippiRGBToGray_8u_C3C1R` | RGB → grayscale conversion |
| ColorFormatDecoder.cpp | `ippiYUVToRGB_8u_C3R`, `ippiCbYCr422ToRGB_8u_C2C3R` | YUV → RGB color conversion |
| PylonColorConverter.cpp | `ippiCbYCr422ToRGB_8u_C2C3R`, `ippiYCbCr422ToRGB_8u_C2C3R` | Pylon YUV422 → RGB |
| ConvolutionOp_Ipp.cpp | `ippiFilterSobelBorder_*`, `ippiFilterGaussBorder_*` | 34 Sobel/Gauss/Laplace specializations |
| MorphologicalOp_Ipp.cpp | `ippiDilate/Erode_*_C1R_L` + spec | Modern morphology with border |
| AffineOp_Ipp.cpp | `ippiWarpAffineNearest/Linear_*` + spec | Affine warp |
| MedianOp_Ipp.cpp | `ippiFilterMedianBorder_*_C1R` | Median filter with border |
| LUTOp_Ipp.cpp | `ippiReduceBits` (new signature) | Bit reduction |
| CannyOp.cpp | Modern `ippiCanny` with border spec | Canny edge detection |

### Next Steps

- **Wire consumers to BlasOps/FFTOps** — Replace 27 `#ifdef ICL_HAVE_MKL` blocks
  in DynMatrix.cpp, DynMatrixUtils.cpp, FFTUtils.cpp with BlasOps/FFTOps dispatch.
  Remove MKL includes, delete FFTDispatching.h/.cpp and LegacyFFTOp.
- **Add Accelerate backend** — `BlasOps_Accelerate.cpp` + `FFTOps_Accelerate.cpp`
  for macOS (cblas via Accelerate framework, vDSP FFT)
- **Re-enable disabled IPP backends** — update to modern oneAPI APIs (see table above)
- **GLImg.cpp** — 1 remaining `ICL_HAVE_IPP` block in ICLQt
- **NeighborhoodOp.cpp** — 2 IPP bug workaround blocks (minor)
- **Expand benchmarks on Linux** — IPP vs C++ vs SIMD vs MKL comparison

### Key Files (Updated)

```
ICLUtils/src/ICLUtils/BackendDispatching.h     — framework (sorted vector storage)
ICLMath/src/ICLMath/BlasOps.h                  — BLAS/LAPACK dispatch singleton
ICLMath/src/ICLMath/BlasOps.cpp                — constructor, instance, toString
ICLMath/src/ICLMath/BlasOps_Cpp.cpp            — C++ fallback (naive GEMM + svd_dyn SVD)
ICLMath/src/ICLMath/BlasOps_Mkl.cpp            — MKL backend (cblas_xgemm + xgesdd)
ICLMath/src/ICLMath/FFTOps.h                   — FFT dispatch singleton
ICLMath/src/ICLMath/FFTOps.cpp                 — constructor, instance, toString
ICLMath/src/ICLMath/FFTOps_Cpp.cpp             — C++ fallback (row-column FFT)
ICLMath/src/ICLMath/FFTOps_Mkl.cpp             — MKL DFTI backend
ICLMath/src/ICLMath/FFTDispatching.h           — TRANSITIONAL (LegacyFFTOp, delete after wiring)
ICLMath/src/ICLMath/FFTDispatching.cpp         — TRANSITIONAL (delete after wiring)
```

## Previous State (Session 20 — All Filters Migrated to Prototype+Clone)

### Session 20 Summary

**All 15 filters now use the prototype+clone pattern.** The global string
registry is no longer needed for filter dispatch.

Migrated the remaining 14 filters (ThresholdOp was already done in session 19):

| Filter | Selectors | Backend Files | Notes |
|---|---|---|---|
| WienerOp | apply | _Cpp, _Ipp | Now always built (removed IPP-only exclusion from CMakeLists) |
| LUTOp | reduceBits | _Cpp, _Ipp | Two constructors, both clone from prototype |
| AffineOp | apply | _Cpp, _Ipp | C++ uses FixedMatrix for inverse transform |
| ConvolutionOp | apply | _Cpp, _Ipp | Large dispatch chain moved to _Cpp.cpp |
| MorphologicalOp | apply | _Cpp, _Ipp | Buffer accessors added for _Cpp.cpp access |
| BilateralFilterOp | apply | _Cpp, _OpenCL | C++ bilateral filter (all depths) |
| BinaryLogicalOp | apply | _Cpp, _Simd | dispatchEnum pattern preserved |
| BinaryArithmeticalOp | apply | _Cpp, _Simd | dispatchEnum pattern preserved |
| BinaryCompareOp | compare, compareEqTol | _Cpp, _Simd | 2 selectors |
| WarpOp | warp | _Cpp, _Ipp, _OpenCL | 3 backend files |
| MedianOp | fixed, generic | _Cpp, _Ipp, _Simd | Sorting networks + Huang median in _Cpp |
| UnaryArithmeticalOp | withVal, noVal | _Cpp, _Ipp, _Simd | 2 selectors |
| UnaryLogicalOp | withVal, noVal | _Cpp, _Ipp, _Simd | 2 selectors |
| UnaryCompareOp | compare, compareEqTol | _Cpp, _Ipp, _Simd | 2 selectors |

**Pattern for each migrated filter:**
1. **Header** — `enum class Op : int { ... }`, `static prototype()`,
   `toString(Op)` declaration with `ICLFilter_API`
2. **_Cpp.cpp** — C++ backend implementations, registers via
   `prototype().addBackend<Sig>(Op::x, Backend::Cpp, fn, desc)`
3. **_Ipp/_Simd/_OpenCL.cpp** — same registration pattern into prototype
4. **.cpp** — constructor is `ImageBackendDispatching(prototype())`,
   `apply()` uses `getSelector<Sig>(Op::x)` (enum-indexed, O(1))
5. `toString(Op)` free function for ADL (used by `addSelector(K)`)

**CMake change:** WienerOp is no longer excluded when `!IPP_FOUND`. Its C++
backend throws an exception (IPP required), but the class is always available.

**MorphologicalOp special handling:** The C++ backend accesses private
composite-operation buffers. Added public buffer accessors
(`openingBuffer()`, `gradientBuffer1()`, `gradientBuffer2()`) so the free
function in `_Cpp.cpp` can use them via the `MorphologicalOp&` parameter.

**14 new _Cpp.cpp files created:**
```
ICLFilter/src/ICLFilter/WienerOp_Cpp.cpp
ICLFilter/src/ICLFilter/LUTOp_Cpp.cpp
ICLFilter/src/ICLFilter/AffineOp_Cpp.cpp
ICLFilter/src/ICLFilter/ConvolutionOp_Cpp.cpp
ICLFilter/src/ICLFilter/MorphologicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BilateralFilterOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryLogicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryArithmeticalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryCompareOp_Cpp.cpp
ICLFilter/src/ICLFilter/WarpOp_Cpp.cpp
ICLFilter/src/ICLFilter/MedianOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryArithmeticalOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryLogicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryCompareOp_Cpp.cpp
```

**Test results:** 349/349 pass (single-threaded). Multi-threaded test crash
is a **pre-existing bug** in the AffineOp test: heap-buffer-overflow in
`Img<icl8u>::subPixelLIN()` when bilinear-interpolating a 2×2 image
(ASAN confirmed at `AffineOp_Cpp.cpp:45`, `test-filter.cpp:745`).

### Known Issue: Stateful Backend Sharing

With the prototype+clone pattern, `ImplBase` objects are shared across all
instances via `shared_ptr`. This is correct for stateless backends (free
functions), but **stateful backends share mutable state** across instances:

| Backend | Shared State | Impact |
|---|---|---|
| WienerOp_Ipp.cpp | IPP scratch buffer (`vector<icl8u>`) | Concurrent calls corrupt buffer |
| WarpOp_OpenCL.cpp | `CLWarpState` (GPU buffers/kernels) | Concurrent calls corrupt GPU state |
| BilateralFilterOp_OpenCL.cpp | `CLBilateralState` | Same |
| MorphologicalOp_Ipp.cpp | `MorphIppState` (IPP state objects) | Same |

**Fix needed:** Add a factory/creator pattern to `ImplBase` so that stateful
backends can create fresh state per clone. The `BackendSelector::clone()`
should call `impl->cloneImpl()` instead of copying the `shared_ptr`:

```
Option A: virtual cloneImpl() on ImplBase
  - Stateless impls: return shared_from_this() (share as before)
  - Stateful impls: call factory, return new Impl with fresh state

Option B: Store optional factory lambda on ImplBase
  - add() takes optional Factory parameter
  - clone() calls factory if present, else shares shared_ptr
  - Backends register: proto.addStatefulBackend<Sig>(key, backend,
      factory, applicability, desc)
  - The factory lambda creates fresh state each time
```

**Important:** If the state contains `Image` or `Img<T>` members, the factory
must `deepCopy()` them — ICL images use shallow copy by default.

## Previous State (Session 19 — Full ImgOps Dispatch Migration)

### Session 19 Summary

**Img.cpp + Img.h now have zero `#ifdef ICL_HAVE_IPP` blocks.**

Migrated all 7 remaining IPP-guarded operations in Img.cpp/Img.h to the
ImgOps BackendDispatch framework (Img_Cpp.cpp / Img_Ipp.cpp):

| Operation | IPP Functions | Depths |
|---|---|---|
| clearChannelROI | `ippiSet_*_C1R` | 8u, 16s, 32s, 32f |
| lut | `ippiLUTPalette_8u_C1R` | 8u |
| getMax | `ippiMax/MaxIndx_*_C1R` | 8u, 16s, 32f |
| getMin | `ippiMin/MinIndx_*_C1R` | 8u, 16s, 32f |
| getMinMax | `ippiMinMax/MinMaxIndx_*_C1R` | 8u, 32f |
| normalize | `ippiMulC/AddC_32f_C1IR` | 32f |
| flippedCopyChannelROI | `ippiMirror_*_C1R` | 8u, 32f |

**Key design details:**
- **Type-erased dispatch signatures** — operations that return typed values
  (getMax, getMin) use `icl64f` return through dispatch, cast back at call site
- **const methods** — `getMax`/`getMin`/`getMinMax`/`lut` use `const_cast` to
  pass `const ImgBase*` through `ImgBase*` dispatch context (safe: resolve
  only checks depth, backends don't modify source)
- **clearChannelROI** — dispatch at `Img<T>::clear()` level, not in the header
  template itself (avoids circular include: Img.h→ImgOps.h→Image.h→Img.h).
  Direct callers of `clearChannelROI<T>()` get the C++ path.
- **Mirror helpers moved to Img_Cpp.cpp** — `getPointerOffset`,
  `getMirrorPointerOffsets`, `getMirrorPointers`, plus the per-channel
  `Img<T>::mirror(axis, int, Point, Size)` definition (with explicit
  instantiations). These were only used by mirror and flippedCopy.
- **scaledCopyChannelROI** `#if 0` dead code cleaned up — just uses
  `ICL_INSTANTIATE_ALL_DEPTHS` now (IPP APIs deprecated, TODO for future)
- **`getStartIndex`/`getEndIndex`** are protected on `Img<T>` — backends
  inline the logic: `startC = ch < 0 ? 0 : ch`

**New dispatch signatures in ImgOps:**
```
ClearChannelROISig  = void(ImgBase&, int ch, icl64f val, const Point& offs, const Size& size)
LutSig             = void(ImgBase& src, const void* lut, ImgBase& dst, int bits)
GetMaxSig          = icl64f(ImgBase&, int ch, Point* coords)
GetMinSig          = icl64f(ImgBase&, int ch, Point* coords)
GetMinMaxSig       = void(ImgBase&, int ch, icl64f* minVal, icl64f* maxVal, Point* minCoords, Point* maxCoords)
NormalizeSig       = void(ImgBase&, int ch, icl64f srcMin, icl64f srcMax, icl64f dstMin, icl64f dstMax)
FlippedCopySig     = void(axis, ImgBase& src, int srcC, const Point& srcOffs, const Size& srcSize,
                          ImgBase& dst, int dstC, const Point& dstOffs, const Size& dstSize)
```

**BackendDispatching rewrite (also this session):**
- `map<Backend,ImplPtr>` → `array<shared_ptr<ImplBase>, 4>` (fixed array, shared for cloning)
- Removed `backendPriority[]` — reverse iteration over enum values
- `BackendSelector` un-nested from `BackendDispatching` (standalone template)
- `resolve`/`resolveOrThrow`/`get` now const
- Added `virtual clone()` + clone constructor for per-instance dispatch
- Enum-keyed `addSelector(K)` with index assertion + ADL `toString(K)`
- Enum-keyed `getSelector<Sig>(K)` — O(1) vector index
- ImgOps backends register directly into singleton (no global registry)
- ThresholdOp migrated as proof of concept for prototype+clone pattern:
  all backends in `_Cpp.cpp` / `_Ipp.cpp`, constructor just clones prototype
- Removed dead code (`callWith`, `qualifiedName`, `backendPriority[]`)
- CoreFunctions `channel_mean` + ImgBorder `replicateBorder` migrated to ImgOps

**CMake note:** `FILE(GLOB)` is evaluated at configure time. After adding new
`_Cpp.cpp` / `_Ipp.cpp` files, re-run `cmake ..` to pick them up.

### Previous Session Summary (Session 18)

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
BackendDispatching<Context>           — ICLUtils (header-only, no .cpp)
  BackendSelectorBase<Context>        — abstract per-selector base
  BackendSelector<Context, Sig>       — typed dispatch table
    .add(b, f, applicability, desc)   — register stateless backend
    .addStateful(b, factory, app, d)  — register stateful backend (factory per clone)
    .resolve(ctx) → ImplBase*         — returns nullptr if no match
    .resolveOrThrow(ctx) → ImplBase*  — throws logic_error if no match
    .clone()                          — stateful: calls cloneFn(); stateless: shares shared_ptr
  ApplicabilityFn<Context>            — std::function<bool(const Context&)>
  ImplBase::cloneFn                   — optional factory for stateful backends

API on BackendDispatching<Context>:
  addSelector<Sig>(K key)             — enum-keyed only (no string overloads)
  getSelector<Sig>(K key)             — O(1) vector index
  selector(K key)                     — returns BackendSelectorBase* (introspection/tests)
  addBackend<Sig>(K, b, f, app, desc) — convenience for getSelector().add()
  addStatefulBackend<Sig>(K, b, factory, app, desc) — convenience for getSelector().addStateful()

Two context types:
  ImageBackendDispatching             — BackendDispatching<Image>
  ImgBaseBackendDispatching           — BackendDispatching<ImgBase*>

ImgOps singleton                      — ICLCore (enum class Op, 10 selectors)
FFTDispatching singleton              — ICLMath (enum class FFTOp, 3 selectors)

Filter prototype+clone pattern        — all 15 ICLFilter ops
  Static prototype() holds selectors + ImplBase objects
  Constructor clones: ImageBackendDispatching(prototype())
  Stateful backends get fresh state per instance via factory cloneFn
  _Cpp.cpp / _Ipp.cpp / _Simd.cpp / _OpenCL.cpp register into prototype()

Backend enum: Cpp, Simd, Ipp, OpenCL  — ICLUtils
Priority: OpenCL > Ipp > Simd > Cpp

CMake: _Ipp.cpp excluded when !IPP_FOUND, _OpenCL.cpp when !OPENCL_FOUND
       _Cpp.cpp always built
```

### Remaining Inline `#ifdef ICL_HAVE_IPP` Blocks to Migrate

**ICLCore — Img.cpp and Img.h are DONE (zero `#ifdef ICL_HAVE_IPP`).**

Remaining ICLCore files:
- `CoreFunctions.cpp` — channel_mean specializations (4 depths)
- `ImgBorder.cpp` — border replication (8u, 32f)
- `CCFunctions.cpp` — planarToInterleaved/interleavedToPlanar macros
- `BayerConverter.h/.cpp` — Bayer pattern conversion
- `Types.h` — conditional enum definitions (compile-time, may stay)

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
ICLUtils/src/ICLUtils/BackendDispatching.h     — framework template (header-only, no .cpp)
ICLUtils/src/ICLUtils/EnumDispatch.h           — dispatchEnum utility
ICLCore/src/ICLCore/ImageBackendDispatching.h  — Image + ImgBase* typedefs
ICLCore/src/ICLCore/ImgOps.h                   — singleton header, dispatch signatures
ICLCore/src/ICLCore/ImgOps.cpp                 — singleton impl, creates selectors
ICLCore/src/ICLCore/Img_Cpp.cpp                — C++ backends (8 ops + mirror helpers)
ICLCore/src/ICLCore/Img_Ipp.cpp                — IPP backends (8 ops)
ICLMath/src/ICLMath/FFTDispatching.h           — FFTOp enum, FFTDispatching singleton
ICLMath/src/ICLMath/FFTDispatching.cpp         — FFT C++ backends
ICLFilter/src/ICLFilter/*_Cpp.cpp              — 15 C++ backend files (one per filter)
ICLFilter/src/ICLFilter/*_Ipp.cpp              — IPP backends (excluded when !IPP_FOUND)
ICLFilter/src/ICLFilter/*_Simd.cpp             — SIMD backends (always built)
ICLFilter/src/ICLFilter/*_OpenCL.cpp           — OpenCL backends (excluded when !OPENCL_FOUND)
tests/test-filter.cpp                          — 349 tests
benchmarks/bench-filter.cpp                    — 25 filter benchmarks
packaging/docker/noble-ipp/                    — Docker IPP build
.github/workflows/ci.yaml                     — CI with IPP job
```

### Next Steps

#### A. ~~Migrate all 14 remaining filters to prototype+clone pattern~~ **DONE** (Session 20)

All 15 filters now use prototype+clone. See Session 20 summary above.

#### A2. ~~Remove global string registry + add stateful backend cloning~~ **DONE** (Session 21)

See Session 21 summary above. All three phases complete.

#### B. ~~Remaining ICLCore IPP blocks~~ **DONE** (Session 21)

- ~~CoreFunctions.cpp — channel_mean~~ **DONE** (Session 19)
- ~~ImgBorder.cpp — border replication~~ **DONE** (Session 19)
- ~~CCFunctions.cpp — planarToInterleaved/interleavedToPlanar~~ **DONE** (Session 21, added to ImgOps)
- ~~BayerConverter.h/.cpp~~ **DONE** (Session 21, removed dead IPP code — `nnInterpolationIpp` was never called)
- Types.h — enum value definitions (compile-time, stays as-is)

#### C. Other modules

- ~~**ICLMath IPP**~~ **DONE** (Session 21) — MathOps<T> singletons, all `ICL_HAVE_IPP` removed
  from headers + DynMatrixUtils.cpp. CMake `_Ipp.cpp`/`_Mkl.cpp` exclusion added.
- **ICLMath MKL** — 27+ `#ifdef ICL_HAVE_MKL` blocks remain (DynMatrix, DynMatrixUtils,
  FFTUtils). Need `_Mkl.cpp` files + `Backend::Mkl` enum value. Deferred.
- ~~**ICLIO**~~ **DONE** (Session 21) — DC.cpp, ColorFormatDecoder.cpp, PylonColorConverter
  IPP guards removed, C++ fallbacks always used, TODOs added
- **Update disabled IPP backends** to modern oneAPI APIs
- **Expand benchmarks on Linux** — IPP vs C++ vs SIMD comparison
