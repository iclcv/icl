# Apple Accelerate Opportunities for ICL

Reference for backend implementation. ICL uses planar image storage
(each channel contiguous), which maps well to vImage's `Planar8`/`PlanarF` types.

## Already Implemented

| Operation | Backend file | vImage/vDSP function | Types |
|---|---|---|---|
| Convolution | `ConvolutionOp_Accelerate.cpp` | `vImageConvolve_PlanarF` | 32f |
| Morphology | `MorphologicalOp_Accelerate.cpp` | `vImageDilate/Erode_Planar8/PlanarF` | 8u, 32f |
| Affine warp | `AffineOp_Accelerate.cpp` | `vImageAffineWarp_Planar8/PlanarF` | 8u, 32f |
| FFT | `FFTOps_Accelerate.cpp` | `vDSP_DFT_Execute` | float, double |
| BLAS gemm | `BlasOps_Accelerate.cpp` | `cblas_sgemm/dgemm` | float, double |
| LAPACK | `LapackOps_Accelerate.cpp` | `sgesdd_/dgesdd_` etc. | float, double |

## Tier 1 — High Impact, Straightforward

### Image Scaling (scaledCopyChannelROI)
- **ICL file:** `ICLCore/src/ICLCore/Img.cpp` lines 1050-1240
- **Current:** Plain C++ NN/bilinear/rect-avg, no backend dispatch, IPP TODO
- **Accelerate:** `vImageScale_Planar8`, `vImageScale_PlanarF` (Lanczos resampling)
- **Impact:** 2-4x speedup, heavily used in image pipelines

### Unary Arithmetic (add/sub/mul/div/sqr/sqrt/abs with scalar)
- **ICL file:** `ICLFilter/src/ICLFilter/UnaryArithmeticalOp_Cpp.cpp` + `_Simd.cpp`
- **Current:** SSE2 for 32f (4 pixels/iter), C++ scalar fallback for other depths
- **Accelerate:** `vDSP_vsadd`, `vDSP_vsmul`, `vDSP_vsdiv`, `vDSP_vsqrt`, `vDSP_vabs`
- **Has backend dispatch:** YES
- **Impact:** 2-3x for 32f, replaces SSE2 with native ARM-optimized

### Binary Arithmetic (pixel-wise add/sub/mul/div/absSub)
- **ICL file:** `ICLFilter/src/ICLFilter/BinaryArithmeticalOp_Cpp.cpp` + `_Simd.cpp`
- **Current:** SSE2 for 32f, C++ scalar fallback
- **Accelerate:** `vDSP_vadd`, `vDSP_vsub`, `vDSP_vmul`, `vDSP_vdiv`
- **Has backend dispatch:** YES
- **Impact:** 2-3x

### Threshold / Clamp
- **ICL file:** `ICLFilter/src/ICLFilter/ThresholdOp_Cpp.cpp` + `_Simd.cpp`
- **Current:** SSE2 for some depths, C++ scalar
- **Accelerate:** `vDSP_vclip` (clamp to [lo,hi]), `vDSP_vthr` (threshold)
- **Has backend dispatch:** YES
- **Impact:** 2-3x

### Statistical Operations (mean, variance, min/max)
- **ICL file:** `ICLCore/src/ICLCore/CoreFunctions.cpp`
- **Current:** Plain C++ iteration, no backend dispatch
- **Accelerate:** `vDSP_meanv`, `vDSP_sve` (sum), `vDSP_maxv`/`vDSP_minv`, `vDSP_measqv` (mean of squares for variance)
- **Impact:** 3-5x for large images

## Tier 2 — Good Gains, Moderate Effort

### Color Space Conversion
- **ICL file:** `ICLCore/src/ICLCore/CCFunctions.cpp` (massive, ~82K tokens)
- **Current:** Fixed-point arithmetic + CCLUT lookup tables
- **Accelerate:** vImage `vImageConvert_*` for YCbCr/YpCbCr conversions;
  `vImageMatrixMultiply_*` for linear color transforms (RGB↔XYZ)
- **Impact:** 2-3x, but complex integration (many color spaces)

### Pixel Type Conversion (depth conversion)
- **ICL file:** `ICLCore/src/ICLCore/PixelOps.cpp`
- **Current:** SSE2 specializations for 8u↔32f, 16s↔32f, etc.
- **Accelerate:** `vDSP_vflt8/vflt16/vflt32` (int→float), `vDSP_vfix8/vfix16` (float→int);
  vImage `vImageConvert_Planar8toPlanarF` / `vImageConvert_PlanarFtoPlanar8`
- **Impact:** 1.5-2x (already SSE2, but native ARM would be cleaner)

### Image Comparison Operations
- **ICL file:** `ICLFilter/src/ICLFilter/UnaryCompareOp_Cpp.cpp`, `BinaryCompareOp_Cpp.cpp`
- **Current:** Plain C++ scalar, SIMD variant
- **Has backend dispatch:** YES
- **Accelerate:** No direct vector compare in vDSP; could use vDSP_vthr + arithmetic
- **Impact:** 2x

### Mirror/Flip/Rotate90
- **ICL file:** `ICLFilter/src/ICLFilter/MirrorOp.cpp`
- **Current:** Plain C++ memcpy/reverse
- **Accelerate:** `vImageHorizontalReflect_*`, `vImageVerticalReflect_*`,
  `vImageRotate90_*` (all in vImage)
- **Has backend dispatch:** NO (would need to add)
- **Impact:** 2-3x for large images

### Histogram
- **ICL file:** `ICLCore/src/ICLCore/CoreFunctions.cpp`
- **Current:** Plain C++ iteration, no backend dispatch
- **Accelerate:** `vImageHistogramCalculation_Planar8/PlanarF`,
  `vImageEqualization_Planar8` (equalization)
- **Impact:** 2-3x

## Tier 3 — Niche / Lower Priority

### Integral Image
- `ICLFilter/src/ICLFilter/IntegralImgOp.cpp` — C++ loop-unrolled already fast

### Median Filter
- `ICLFilter/src/ICLFilter/MedianOp_Simd.cpp` — already has SIMD backend

### Logical Operations (AND/OR/XOR/NOT)
- Already fast with bitwise ops; minimal Accelerate benefit

### Wiener Filter
- `ICLFilter/src/ICLFilter/WienerOp_Cpp.cpp` — currently throws (IPP-only)
- Would need vImage convolution + statistics; complex

### Warp (arbitrary map)
- `ICLFilter/src/ICLFilter/WarpOp_Cpp.cpp` — no vImage equivalent for general warp

## New Capabilities (not in ICL today)

| Capability | Accelerate API | Notes |
|---|---|---|
| Perspective warp | `vImagePerspectiveWarp_*` | 4-point homography mapping |
| Histogram equalization | `vImageEqualization_Planar8` | Contrast enhancement |
| Histogram specification | `vImageHistogramSpecification_*` | Match target histogram |
| Planar↔interleaved convert | `vImageConvert_PlanarToChunky/ChunkyToPlanar` | Critical for external lib interop |
| vForce transcendentals | `vvsinf`, `vvcosf`, `vvexpf`, `vvlogf` | Vectorized sin/cos/exp/log on arrays |
| Biquad (IIR) filtering | `vDSP_biquad` | Temporal smoothing for tracking |
