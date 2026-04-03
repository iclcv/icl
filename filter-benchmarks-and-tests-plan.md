# Plan: Comprehensive Filter Tests, Benchmarks, Docker IPP Build

## Context

All 15 BackendDispatch filters are implemented with C++, SIMD, IPP, and OpenCL backends. But test coverage is uneven — only 4 filters have cross-backend validation, benchmarks cover only 3 filters with no backend comparison, and there's no IPP build to validate against. IPP is x86-only (no Apple Silicon), so Docker/Linux is required. IPP results serve as ground truth; all other backends must match.

On Linux with GPU + IPP, we get up to 4-way validation: C++ vs SIMD vs IPP vs OpenCL (for WarpOp, BilateralFilterOp). The `forEachCombination` machinery handles this automatically.

## Phase 1: Test Helper — `crossValidateBackends()`

**File:** `tests/test-filter.cpp` (add near existing `testROIHandling()`)

One small template function that encapsulates the repetitive 15-line cross-validation pattern:

```cpp
template<class Op, class ApplyFn>
void crossValidateBackends(Op &op, const Image &ctx, ApplyFn &&applyFn) {
  op.forceAll(Backend::Cpp);
  Image ref = applyFn();
  op.unforceAll();

  auto combos = op.allBackendCombinations(ctx);
  int nCombos = 0;
  bool allMatch = true;
  op.forEachCombination(combos, [&](const std::vector<Backend>&) {
    Image dst = applyFn();
    dst.visit([&](const auto &d) {
      using T = typename std::remove_reference_t<decltype(d)>::type;
      if(!(d == ref.as<T>())) allMatch = false;
    });
    nCombos++;
  });
  ICL_TEST_TRUE(allMatch);
  ICL_TEST_TRUE(nCombos >= 1);
}
```

Usage for UnaryOp: `crossValidateBackends(op, src, [&]{ return op.apply(src); })`
Usage for BinaryOp: `crossValidateBackends(op, src1, [&]{ return op.apply(src1, src2); })`

That's the only new util. `testROIHandling()` already exists for ROI tests.

## Phase 2: Comprehensive Tests for All 15 BackendDispatch Filters

**File:** `tests/test-filter.cpp`

For each filter, ensure all 5 test layers exist. Add what's missing:

### Test layers per filter

1. **Smoke** — runs, output has correct size/depth/channels/format
2. **Pixel correctness** — small known inputs, hand-verified expected outputs
3. **ROI handling** — `testROIHandling(op, src, roi)` (clipToROI + non-clip)
4. **Edge cases** — multi-channel, all applicable depths, single-pixel, large image
5. **Cross-backend** — `crossValidateBackends()` at one depth + per-depth loop

### What exists vs what's needed

| Filter | Has 1-2 | Has 3 | Has 4 | Has 5 | New tests needed |
|--------|---------|-------|-------|-------|------------------|
| ThresholdOp | Yes (13) | Yes | Yes | Yes (2) | — |
| UnaryCompareOp | Yes (14) | Yes | Yes | Yes (2) | — |
| UnaryArithmeticalOp | Yes (19) | Yes | Yes | Yes (2) | — |
| UnaryLogicalOp | Yes (5) | No | Partial | No | ROI + cross-validate (3-4) |
| BinaryArithmeticalOp | Yes (8) | No | No | No | ROI + cross-validate (3-4) |
| BinaryCompareOp | Yes (5) | No | No | No | ROI + cross-validate (3-4) |
| BinaryLogicalOp | Yes (5) | No | No | No | ROI + cross-validate (3-4) |
| MedianOp | Yes (17) | Yes | Partial | No | cross-validate (2) |
| ConvolutionOp | Yes (6) | Yes | Partial | No | cross-validate (2) |
| MorphologicalOp | Yes (1) | No | No | No | smoke + pixel + ROI + cross-validate (5-6) |
| AffineOp | Yes (5) | No | No | No | cross-validate (2) |
| LUTOp | Yes (4) | No | Partial | No | cross-validate (2) |
| WienerOp | — | — | — | — | skip (IPP-only, no C++ ref) |
| WarpOp | Yes (11) | Yes | Yes | No | cross-validate (2) |
| BilateralFilterOp | Yes (13) | Yes | Yes | Yes (3) | — |

**Estimated: ~30 new test registrations** across 11 filters.

### Per-depth cross-validation specifics

- Integer-only ops (UnaryLogicalOp, BinaryLogicalOp): test `{depth8u, depth16s, depth32s}`
- All-depth ops (threshold, arithmetic, compare, median, etc.): test `{depth8u, depth16s, depth32s, depth32f, depth64f}`
- 8u-only ops (LUTOp reduceBits): test at depth8u with varying quantization levels
- AffineOp: use identity transform + NN interpolation for exact match
- WarpOp: use identity warp map + NN interpolation for exact match

### BinaryOp ROI testing

`testROIHandling()` is UnaryOp-specific. For BinaryOps, write inline ROI tests: set ROI on both sources, verify output size/content matches. Simpler than a generic helper since there are only 3 BinaryOp filters.

## Phase 3: Expanded Benchmarks with Backend Parameter

**File:** `benchmarks/bench-filter.cpp`

### 3a. Add backend parameter to all benchmarks

Every benchmark gets `BenchParamDef::Str("backend", "auto")`. Inside the lambda:
```cpp
std::string be = p.getStr("backend");
if(be == "cpp") op.forceAll(Backend::Cpp);
else if(be == "simd") op.forceAll(Backend::Simd);
```

Retrofit existing 8 benchmarks + switch from old `op.apply(&src, &dst)` to Image-based API.

**Standard image size: 1000x1000** (1M pixels — clean baseline, easy to reason about).

### 3b. New benchmarks (13 new)

**BinaryOps (6):**
| Benchmark key | Op | Depth | SIMD? |
|---|---|---|---|
| `filter.binary_arith.add_32f` | addOp | 32f | Yes |
| `filter.binary_arith.mul_32f` | mulOp | 32f | Yes |
| `filter.binary_arith.add_8u` | addOp | 8u | No (baseline) |
| `filter.binary_compare.gt_32f` | gt | 32f | Yes |
| `filter.binary_compare.gt_8u` | gt | 8u | Yes |
| `filter.binary_logical.and_8u` | andOp | 8u | Yes |

**Additional UnaryOps (5):**
| Benchmark key | Op | Depth | SIMD? |
|---|---|---|---|
| `filter.unary_compare.gt_8u` | gt | 8u | Yes |
| `filter.unary_logical.and_8u` | andOp | 8u | Yes |
| `filter.median.3x3_8u` | 3x3 | 8u | Yes |
| `filter.median.3x3_32f` | 3x3 | 32f | Yes |
| `filter.morphological.dilate3x3_8u` | dilate3x3 | 8u | C++ (IPP on Linux) |

**More NeighborhoodOps (2):**
| Benchmark key | Op | Depth |
|---|---|---|
| `filter.convolution.gauss5x5_32f` | gauss5x5 | 32f |
| `filter.convolution.gauss3x3_8u` | gauss3x3 | 8u |

**Usage:** `./icl-benchmarks -f "filter.*" -p backend=cpp` vs `-p backend=simd`

Additional includes: BinaryArithmeticalOp, BinaryCompareOp, BinaryLogicalOp, UnaryCompareOp, UnaryLogicalOp, MedianOp, MorphologicalOp.

## Phase 4: CMake IPP Path Fix for oneAPI

**File:** `CMakeLists.txt` (lines 270-290)

Add `/opt/intel/oneapi/ipp/latest` to PATHS, `"include"` to HEADER_PATH_SUFFIXES, `"lib"` to LIB_PATH_SUFFIXES. Tiny change, unblocks Docker build.

## Phase 5: Docker + CI for IPP Cross-Validation

### 5a. Dockerfile
**New file:** `packaging/docker/noble-ipp/Dockerfile`
- Base: `ubuntu:24.04`
- Deps: build-essential, cmake, clang, libjpeg-dev, libpng-dev
- Intel oneAPI IPP via APT: `intel-oneapi-ipp-devel`
- Env: IPPROOT, LD_LIBRARY_PATH, CMAKE_PREFIX_PATH

### 5b. Build/test script
**New file:** `packaging/docker/noble-ipp/build-and-test.sh`
- `source /opt/intel/oneapi/setvars.sh`
- cmake with `-DBUILD_WITH_IPP=ON -DBUILD_TESTS=ON` (no Qt/OpenGL)
- build + `ctest --output-on-failure`

### 5c. CI job
**File:** `.github/workflows/ci.yaml` — add `ipp-validation` job
- Build Docker image, mount source read-only, run build-and-test.sh
- Path filter: only trigger on ICLFilter/**, ICLMath/**, ICLUtils/** changes

**Docker on M3 Mac:** Use `--platform linux/amd64` for x86 emulation via QEMU/Rosetta. Slow but functional for build + test. IPP works fine (just x86 math). OpenCL does NOT work in Docker on macOS (no GPU passthrough). So Docker gives 3-way: C++ vs SIMD vs IPP. OpenCL validation (WarpOp, BilateralFilterOp) would need a real Linux box with GPU — not a blocker, `forEachCombination` simply skips unavailable backends.

## Progress

- [x] **Phase 4** — CMake IPP path fix for oneAPI (`6a11b758`)
- [x] **Phase 1** — `crossValidateBackends()` helper (`6a11b758`)
- [x] **Phase 2** — 20 new cross-validation tests, 349 total tests all passing (`6a11b758`)
- [x] **Phase 3** — 25 filter benchmarks with backend param (`891309e1`, `b11a5556`)
- [x] **Phase 5a/b** — Dockerfile + build script (`230d4d34`, `b11a5556`)
- [x] **Phase 5c** — CI job (`230d4d34`)
- [ ] **Phase 5 validation** — Docker IPP build running (x86 emulated on M3)

## Benchmark Results (Release, M3 Mac, 1000x1000)

### SIMD vs C++ highlights
| Benchmark | C++ | SIMD | Winner | Speedup |
|-----------|-----|------|--------|---------|
| threshold.ltgt_32f | 483 us | 158 us | SIMD | 3.1x |
| arithmetic.mul_32f | 618 us | 215 us | SIMD | 2.9x |
| median.3x3_32f | 1132 us | 606 us | SIMD | 1.9x |
| arithmetic.sqrt_32f | 346 us | 217 us | SIMD | 1.6x |
| arithmetic.sqr_32f | 155 us | 217 us | C++ | 0.7x |
| arithmetic.abs_32f | 152 us | 217 us | C++ | 0.7x |

Note: On M3 Mac, SIMD goes through sse2neon. Compiler auto-vectorizes simple C++ ops better.

### OpenCL vs C++ highlights
| Benchmark | OpenCL | C++ | Winner | Speedup |
|-----------|--------|-----|--------|---------|
| bilateral.mono_8u | 8678 us | 45091 us | OpenCL | 5.2x |
| bilateral.mono_32f | 9272 us | 41793 us | OpenCL | 4.5x |
| warp.nn_8u | 13671 us | 5732 us | C++ | 2.4x |
| warp.lin_8u | 14314 us | 6185 us | C++ | 2.3x |

OpenCL wins big for compute-heavy bilateral filter; loses for memory-bound warp (transfer overhead).
