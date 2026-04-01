# Apple Accelerate Equivalents for IPP Image Operations

Reference for future backend implementation. ICL uses planar image storage
(each channel contiguous), which maps well to vImage's `Planar8`/`PlanarF` types.

## Summary

| Operation | Accelerate? | vImage function(s) | Types | Notes |
|---|---|---|---|---|
| Affine warp | YES | `vImageAffineWarp_Planar8`, `vImageAffineWarp_PlanarF` | 8u, 32f | Lanczos3/5 resampling; kvImageEdgeExtend/BackgroundColorFill |
| Convolution | YES | `vImageConvolve_PlanarF`, `vImageConvolve_Planar8` | 8u, 32f | Separable variants available (row/column); border modes supported |
| LUT / bit reduction | PARTIAL | `vImageConvert_*_dithered` | 8u/16u/32f→8u | Dithering (ordered/none) only, not general LUT mapping |
| Median filter | NO | — | — | No vImage equivalent; keep C++ impl |
| Morphology (erode/dilate) | PARTIAL | `vImageErode_Planar8/PlanarF`, `vImageDilate_Planar8/PlanarF` | 8u, 32f | Basic erode/dilate only; no open/close/gradient/tophat/blackhat |
| Canny edge detection | NO | — | — | No vImage equivalent; Core Image has CIEdges but different API |
| Integral image | NO | — | — | C++ loop-unrolled version already faster than IPP was |
| Histogram | YES | `vImageHistogramCalculation_Planar8`, `vImageHistogramCalculation_PlanarF` | 8u, 32f | Fixed 256-bin for 8u; equalization also available |

## Detailed Notes

### Affine Warp (AffineOp)
- IPP (old): `ippiWarpAffine_8u_C1R`, `ippiWarpAffine_32f_C1R`
- IPP (modern): `ippiWarpAffineNearest` / `ippiWarpAffineLinear` + spec init
- Accelerate: `vImageAffineWarp_Planar8`, `vImageAffineWarp_PlanarF`
- Uses `vImage_AffineTransform` struct (3x2 matrix)
- Edge modes: `kvImageEdgeExtend`, `kvImageBackgroundColorFill`
- Resampling: Lanczos3, Lanczos5

### Convolution (ConvolutionOp)
- IPP (old): `ippiFilterSobelHoriz/Vert_*`, `ippiFilterLaplace_*`, `ippiFilterGauss_*`, `ippiFilter_*` (34 specializations)
- IPP (modern): `ippiFilterSobelBorder_*`, `ippiFilterGaussBorder_*`, etc.
- Accelerate: `vImageConvolve_PlanarF` (arbitrary kernel), `vImageConvolve_Planar8`
- Separable: row/column variants for performance
- Good fit for ICL's generic convolution path; fixed kernels (Sobel, Gauss) use same API

### LUT / Bit Reduction (LUTOp)
- IPP (old): `ippiReduceBits_8u_C1R` (signature changed, added noise param)
- Accelerate: `vImageConvert_ARGBFFFFtoARGB8888_dithered` and similar
- Limited: dithering modes (none, ordered) but not general LUT table mapping
- ICL's LUTOp does arbitrary lookup tables, not just bit reduction

### Median Filter (MedianOp)
- IPP (old): `ippiFilterMedianCross_*`, `ippiFilterMedian_*`
- IPP (modern): `ippiFilterMedianBorder_*` with border type + buffer management
- Accelerate: **none** — no median filter in vImage
- Keep C++ implementation as primary backend

### Morphology (MorphologicalOp)
- IPP (old): `ippiMorphologyInitAlloc`, `ippiDilate/Erode_*`, `ippiMorphOpenBorder_*`, etc.
- IPP (modern): spec-based init with `ippiMorphInit`
- Accelerate: `vImageErode_Planar8`, `vImageErode_PlanarF`, `vImageDilate_Planar8`, `vImageDilate_PlanarF`
- Custom structuring elements supported
- **Gap**: no composite operations (open = erode+dilate, close = dilate+erode, etc.)
  - Could compose from erode/dilate but needs two-pass with temp buffer
  - Gradient, tophat, blackhat would need manual composition

### Canny Edge Detection (CannyOp)
- IPP (old): `ippiCanny_32f8u_C1R`, `ippiCanny_16s8u_C1R`
- IPP (modern): spec-based `ippiCanny` with `IppiCannyBorderSpec`
- Accelerate: **none** — no edge detection in vImage
- ICL has full C++ implementation with SSE optimizations (non-max suppression + hysteresis)

### Integral Image (IntegralImgOp)
- IPP: `ippiIntegral_8u32s_C1R`, `ippiIntegral_8u32f_C1R`
- Accelerate: **none**
- ICL's C++ loop-unrolled implementation was already faster than IPP — permanently dead

### Histogram (CoreFunctions)
- IPP (old): `ippiHistogramEven_8u_C1R`, `ippiHistogramEven_16s_C1R`
- IPP (modern): `ippiHistogram` (different semantics)
- Accelerate: `vImageHistogramCalculation_Planar8` (256 bins), `vImageHistogramCalculation_PlanarF`
- Also: `vImageEqualization_Planar8` for histogram equalization

## Priority for Accelerate Backend Implementation

1. **Convolution** — highest impact, most frequently used filter
2. **Morphology** (erode/dilate) — good fit for basic ops, compose the rest
3. **Affine warp** — clean API match
4. **Histogram** — straightforward mapping
5. **LUT** — limited benefit (partial match only)
