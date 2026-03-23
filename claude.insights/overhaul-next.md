# Next Steps — Post-Overhaul Work Items

## Completed (Phase 1 — initial overhaul)
- NULL → nullptr, typedef → using, Uncopyable → = delete
- std::iterator<> base class removal, register keyword removal
- Qt4/5 legacy guards, Qt5→Qt6 migration
- Eigen 5 support, PCL re-enable, PugiXML update
- Warning cleanup (1,209 → 0), CMake modernization (3.16+, C++17)

## Completed (Phase 2)
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
- **Header optimization** — CoreFunctions.h split, moved impls to .cpp, forward declarations

## Completed (Phase 3)
- **Thread → std::thread** (removed pthread, cooperative stop via std::atomic<bool>)
  - Fixed 5 subclasses: DCGrabberThread, PylonGrabberThread, KinectGrabber, ExecThread, llm-2D
- **ShallowCopyable removal** — all 6 classes converted to std::shared_ptr<Impl>, files deleted
- **SmartPtrBase.h deleted** — last user (ConfigFile) converted to std::shared_ptr
- **MultiThreader + Semaphore deleted** — replaced by std::async/std::future in UnaryOp/NeighborhoodOp
- **UnaryOpWork.h deleted** — no longer needed
- **C++17 → C++20** — fixed parse<const char*>, removed all `using namespace std;` (53 files)
- **C++20 idioms** — 17× map::contains(), 20× string::starts_with(), 1× string::ends_with()
- **ImageMagick 7 fixed** — PixelPacket → image.write() export, IntegerPixel → LongPixel, enabled by default
- **Dark theme infrastructure** — DefaultStyle.h (Fusion + QPalette + QSS), opt-in via ICL_THEME env
  - Fixed CompabilityLabel hardcoded black text → palette-aware
  - Known issue: GroupBox title text on macOS needs more work for default-on
- **Folding markers removed** — 1,722 lines of `// {{{ open` / `// }}}` stripped from 67 files
- **Qt style demo** — work/qt-style-demo/ with all Qt controls + loadable .qss

## Completed (Phase 4)
- **Debug build fixes** — missing std:: qualifiers (10+ files), SSE2 instantiation warning, GLEW/Qt warning suppression
- **HiDPI text rendering** — GLPaintEngine renders text textures at devicePixelRatio resolution
- **Test framework** — custom registry (Test.h), parallel runner, 84 tests (utils + FixedMatrix + DynMatrix)
  - Replaces GTest dependency entirely, `BUILD_TESTS=ON`, integrated with ctest
- **SSE2 FixedMatrix** — 4×4 float matrix multiply + 4×4×4×1 vector transform (replaces IPP dependency)
- **FixedMatrix::length()** — L2 norm uses sum-of-squares+sqrt (was calling pow() per element)
- **`using std::` removal** — all 18 temporary `using std::X` declarations replaced with explicit `std::`
- **OpenCL → C API** — rewrote all 6 CL wrapper classes from cl:: C++ bindings to C API
  - Works on macOS natively (no bundled headers needed), M3 unified memory
  - Also converted PlanarRansacEstimator.cpp (only non-wrapper cl:: user)
- **FFmpeg 7+/8+ rewrite** — LibAVVideoWriter rewritten for modern send/receive API
  - Removed 6 deprecated APIs (avcodec_encode_video2, AVPicture, av_init_packet, etc.)
  - Tested with FFmpeg 8.1, enabled by default
- **Mandelbrot demo** — interactive OpenCL GPU fractal explorer with CPU/GPU toggle, rect zoom

## Ready to Do

### Quick wins
| Task | Description |
|------|-------------|
| **More tests** | Img (copy, convert, ROI, channel), SIMD correctness (threshold, arithmetic) |

### Medium effort
| Task | Description |
|------|-------------|
| **More SIMD** | Convolution kernels (3x3, 5x5), morphological ops (erode/dilate 3x3), icl8u arithmetic |
| **Channel const-correctness** | Remove mutable abuse in Channel.h |
| **Any.h rethink** | Currently inherits std::string. Consider std::any or std::variant |

### Larger efforts
| Task | Description |
|------|-------------|
| **Apple Accelerate** | vImage/vDSP as IPP replacement for convolution, color conversion, FFT on macOS |

### Code quality
| Task | Description |
|------|-------------|
| **DynMatrix** | RAII deferred — dual-ownership mode (owning + non-owning views) makes simple replacement risky |

### Not available on Apple Clang
| Feature | Status |
|---------|--------|
| **std::jthread** | Apple Clang 15 doesn't support it. Thread stays with std::thread + atomic<bool> |

## Architecture Notes

### Deleted Infrastructure (no longer exists)
- `Function.h` / `Function.cpp` — replaced by std::function
- `SmartPtr.h` / `SmartPtr.cpp` — replaced by std::shared_ptr
- `Mutex.h` / `Mutex.cpp` — replaced by std::mutex
- `Random.cpp` — Random.h is now header-only
- `ShallowCopyable.h` — replaced by std::shared_ptr<Impl> in each class
- `SmartPtrBase.h` — replaced by std::shared_ptr
- `MultiThreader.h` / `MultiThreader.cpp` — replaced by std::async
- `Semaphore.h` / `Semaphore.cpp` — only user was MultiThreader
- `UnaryOpWork.h` — only user was MultiThreader-based applyMT

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
-DBUILD_WITH_IMAGEMAGICK=ON # ✅ works with ImageMagick 7.1 (fixed this session)
-DBUILD_WITH_OPENCL=ON     # ❌ needs C++ bindings on macOS
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
  -DBUILD_WITH_IMAGEMAGICK=ON \
  -DBUILD_APPS=ON \
  -DBUILD_DEMOS=ON \
  -DBUILD_EXAMPLES=ON
```
