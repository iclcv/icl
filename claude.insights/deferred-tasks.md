# Deferred Tasks from 2026 Overhaul

Items identified during the initial overhaul that were postponed for dedicated follow-up work.

## Code Modernization

### Replace SmartPtr/SmartArray with std::shared_ptr
`SmartPtrBase` is now just a thin wrapper around `std::shared_ptr` (legacy ref-counting removed). The next step is to replace all ~300+ use sites of `SmartPtr<T>` / `SmartArray<T>` with `std::shared_ptr<T>` / `std::shared_ptr<T[]>` directly, then remove `SmartPtrBase.h`, `SmartPtr.h`, and `SmartArray.h`.
- **Approach:** `using SmartPtr = std::shared_ptr` as transitional typedef, then grep and replace all usage
- `SmartArray<T>` → `std::shared_ptr<T[]>` (C++17 supports shared_ptr for arrays)

### Remove C-style casts
Replace all C-style casts `(type)expr` with proper C++ casts (`static_cast<>`, `reinterpret_cast<>`, `const_cast<>`). Found in FourCC.h, SSETypes.h, Time.h, and scattered through .cpp files.
- **Approach:** `grep -rn '(icl8u)\|(icl32f)\|(int)\|(float)\|(double)\|(char\*)' across src/`

## Dependency Rewrites

### Eigen 5.0.1
Homebrew ships Eigen 5.0.1 which has a macro incompatibility with Apple Clang in `GeneralBlockPanelKernel.h`. ICL's EigenICLConverter.cpp triggers it. PCL is also blocked since it pulls in Eigen headers.
- **Fix:** `brew install eigen@3` to pin to 3.4.x, or wait for Eigen 5 patch
- **Re-enable:** `-DBUILD_WITH_EIGEN3=ON -DBUILD_WITH_PCL=ON`

### ImageMagick 7
`Magick::PixelPacket` API removed in ImageMagick 7. Affected files:
- `ICLIO/src/ICLIO/FileGrabberPluginImageMagick.cpp`
- `ICLIO/src/ICLIO/FileWriterPluginImageMagick.cpp`
- **Fix:** Rewrite using `Magick::Pixels` view class and Quantum-based pixel access
- **Re-enable:** `-DBUILD_WITH_IMAGEMAGICK=ON`

### FFmpeg 7+
`ICLIO/src/ICLIO/LibAVVideoWriter.cpp` written against old FFmpeg API. Modern FFmpeg has breaking changes: `AVCodecContext` struct changes, const-correctness (`AVCodec*` → `const AVCodec*`), `avresample` removed (header include already fixed to `swresample`).
- **Fix:** Rewrite LibAVVideoWriter.cpp against FFmpeg 7+ API
- **Re-enable:** `-DBUILD_WITH_LIBAV=ON`

### OpenCL on macOS
macOS provides `<OpenCL/opencl.h>` (C API only). ICL's CL code uses C++ bindings (`cl::Device`, `cl::Platform`, etc.) from `<CL/cl.hpp>` / `<CL/cl2.hpp>` which are not shipped with macOS.
- **Fix:** Bundle `cl2.hpp` from [Khronos OpenCL-CLHPP](https://github.com/KhronosGroup/OpenCL-CLHPP) (single header, MIT licensed)
- **Note:** Apple deprecated OpenCL in favor of Metal
- **Re-enable:** `-DBUILD_WITH_OPENCL=ON`

## Qt6 Migration

### Qt Multimedia Grabbers
`QAbstractVideoSurface` removed in Qt6, replaced by `QVideoSink`. Currently disabled with `#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)`. Affected files:
- `ICLQt/src/ICLQt/ICLVideoSurface.h/.cpp`
- `ICLQt/src/ICLQt/QtVideoGrabber.h/.cpp`
- `ICLQt/src/ICLQt/QtCameraGrabber.h/.cpp`
- **Fix:** Rewrite using `QVideoSink` + `QMediaCaptureSession`
- **Workaround:** OpenCV camera backend (`-i cvcam 0`) works

## Performance / Platform

### Apple Accelerate Framework
IPP removed for macOS (Intel Macs too old). Apple's Accelerate framework is the native alternative:
- **vImage** — image processing (convolution, morphology, color conversion, scaling)
- **vDSP** — signal processing, FFT
- **BLAS/LAPACK** — linear algebra (could replace MKL too)
- **Approach:** Grep for `ICL_HAVE_IPP`, identify hot paths, add Accelerate backends

### ARM NEON via sse2neon
SSE SIMD code disabled on ARM. [sse2neon](https://github.com/DLTcollab/sse2neon) is a header-only library that translates SSE intrinsics to NEON (used by Chromium, FFmpeg, etc.).
- **Approach:** Add `sse2neon.h` to `3rdparty/`, include in `SSETypes.h`/`SSEUtils.h` when on ARM, re-enable SIMD code paths
- **Test with:** MedianOp and ConvolutionOp benchmarks

### `register` Keyword Removal
Currently suppressed with `-Wno-register`. The `register` storage class specifier is removed in C++17 and used in performance-critical files (BayerConverter, CCLUT, CCFunctions, MathFunctions, Img).
- **Fix:** Remove all `register` keywords from source (they have no effect on modern compilers)

## Code Modernization

### C++17 Source Code Pass
CMake is set to C++17 but source code doesn't use C++17 features yet. Opportunities:
- `std::optional` for nullable return types
- `std::string_view` for read-only string parameters
- `if constexpr` for template dispatch (already used in `parse<T>`)
- Structured bindings
- `std::filesystem` instead of custom File utilities
- Replace remaining `boost` usage if any
