# Next Steps — Post-Overhaul Work Items

## Completed (Phase 1 — initial overhaul)
- NULL → nullptr, typedef → using, Uncopyable → = delete
- std::iterator<> base class removal, register keyword removal
- Qt4/5 legacy guards, Qt5→Qt6 migration
- Eigen 5 support, PCL re-enable, PugiXML update
- Warning cleanup (1,209 → 0), CMake modernization (3.16+, C++17)

## Completed (Phase 2 — this session)
- **Function.h → std::function** (removed entirely, 87+ files)
- **C-style casts** (~1,250 replaced with static_cast/reinterpret_cast/const_cast, 248 files)
- **Random.h** modernized (drand48/srand48 → thread_local std::mt19937, thread-safe)
- **malloc/free** → std::vector (CannyOp, DynMatrix, CLSurfLib)
- **Precompiled headers** (39% CPU time reduction, per-module PCH)
- **SmartPtr** → std::shared_ptr directly (deleted SmartPtr.h, 108 files)
- **SmartArray** rewritten as standalone wrapper (no SmartPtrBase dependency)
- **sse2neon** — SIMD re-enabled on ARM via SSE→NEON translation
- **SIMD optimizations** — SSE2 for icl8u/icl32f thresholds (71x speedup), arithmetic ops, find_first_not
- **Benchmark infrastructure** — BenchmarkRegistry + icl-benchmarks binary (14 benchmarks)
- **Mutex** → std::mutex/std::recursive_mutex (deleted Mutex.h/cpp, 133 files)
- **Header optimization** — major refactoring to reduce compilation times:
  - Split CoreFunctions.h into PixelOps.h (copy/convert) + Types.h (getDepth, enums)
  - ImgBase.h no longer includes CoreFunctions.h (was pulling in 2,600+ lines of SSE code)
  - Moved 15 Img<Type> template methods to Img.cpp (explicit instantiation)
  - Moved 11 ImgBase methods to ImgBase.cpp
  - Forward-declared in SceneObject.h (ViewRay, GLFragmentShader) and Grabber.h (ProgArg, ImageUndistortion)
  - Moved SSE2 convert specializations to PixelOps.cpp
  - Removed IPP ippsCopy (memcpy is equally fast on modern systems)
  - Moved Point/Rect non-trivial methods to .cpp files
  - Removed CoreFunctions.h from 10 headers that didn't need it

## Ready to Do

### Quick wins
| Task | Description |
|------|-------------|
| **OpenCL C++ bindings** | Bundle `cl2.hpp` from Khronos, re-enable `-DBUILD_WITH_OPENCL=ON` on macOS |
| **Write tests** | SIMD correctness tests (threshold, arithmetic, find_first_not), Img tests (copy, convert, ROI), DynMatrix tests exist but test harness needs fixing |

### Medium effort
| Task | Description |
|------|-------------|
| **Thread → std::thread** | Replace pthread-based Thread class. Uses ShallowCopyable, virtual `run()`. ~20 use sites. |
| **More SIMD** | Convolution kernels (3x3, 5x5), morphological ops (erode/dilate 3x3), icl8u arithmetic |
| **FixedMatrix 4x4 SIMD** | 4x4 matrix multiply fits perfectly in SSE registers — hot path in 3D transforms |

### Larger efforts
| Task | Description |
|------|-------------|
| **FFmpeg 7+ rewrite** | `LibAVVideoWriter.cpp` needs full rewrite for modern AVCodecContext API |
| **ImageMagick 7 rewrite** | `FileGrabber/WriterPluginImageMagick` — PixelPacket API removed |
| **Apple Accelerate** | vImage/vDSP as IPP replacement for convolution, color conversion, FFT on macOS |

### Code quality
| Task | Description |
|------|-------------|
| **ShallowCopyable removal** | Replace with std::shared_ptr<Impl>. Used by Thread, File, Semaphore. |
| **DynMatrix RAII** | Replace raw new/delete[] with unique_ptr/vector |
| **Channel const-correctness** | Remove mutable abuse in Channel.h |
| **Any.h rethink** | Currently inherits std::string. Consider std::any |

## Architecture Notes

### Header dependency chain (optimized)
```
Types.h          ← enums (depth, format), type aliases, getDepth<T>()
PixelOps.h       ← copy<T>(), convert<S,D>() declarations only
ImgParams.h      ← Size, format, channels, ROI
ImgBase.h        ← includes PixelOps.h (NOT CoreFunctions.h)
Img.h            ← includes ImgBase.h
CoreFunctions.h  ← imgNew, ensureCompatible, mean/variance (only where needed)
```

### SIMD Architecture
```
SSETypes.h       ← type wrappers (icl128, icl256, etc.)
SSEUtils.h       ← sse_for() loop helpers
sse2neon.h       ← ARM NEON backend (3rdparty/)
PixelOps.cpp     ← SSE2 convert specializations (13 type pairs)
ThresholdOp.cpp  ← SSE2 threshold (icl8u + icl32f)
UnaryArithmeticalOp.cpp ← SSE2 arithmetic (icl32f)
RegionDetectorTools.h   ← SSE2 find_first_not (icl8u)
```

### Benchmark Tool
```bash
icl-benchmarks -l                    # list all 14+ benchmarks
icl-benchmarks -f "filter.*"         # glob filter
icl-benchmarks -p width=3840,height=2160  # parameter override
icl-benchmarks -n 50 -w 2.0 -csv    # 50 iterations, 2s warmup, CSV output
```

## Currently Disabled Optional Features
```bash
-DBUILD_WITH_EIGEN3=ON     # ✅ works with Eigen 5.0.1
-DBUILD_WITH_PCL=ON        # ✅ works with PCL 1.15
-DBUILD_WITH_OPENCL=ON     # ❌ needs C++ bindings on macOS
-DBUILD_WITH_IMAGEMAGICK=ON # ❌ needs API v7 rewrite
-DBUILD_WITH_LIBAV=ON      # ❌ needs FFmpeg 7+ rewrite
```

## Recommended Build Command
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_NATIVE_BUILD_OPTION=ON \
  -DBUILD_WITH_QT=ON \
  -DBUILD_WITH_OPENCV=ON \
  -DBUILD_WITH_EIGEN3=ON \
  -DBUILD_WITH_PCL=ON \
  -DBUILD_WITH_BULLET=ON \
  -DBUILD_APPS=ON \
  -DBUILD_DEMOS=ON \
  -DBUILD_EXAMPLES=ON
```
