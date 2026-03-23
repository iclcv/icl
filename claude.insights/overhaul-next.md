# Next Steps — Post-Overhaul Work Items

## Completed
- ~~NULL → nullptr~~
- ~~typedef → using~~
- ~~Uncopyable → = delete~~
- ~~std::iterator<> base class~~
- ~~SmartPtr std::shared_ptr backend~~
- ~~register keyword removal~~
- ~~Qt4/5 legacy guards~~
- ~~Eigen 5 support~~
- ~~PCL re-enable~~
- ~~PugiXML update~~
- ~~Warning cleanup~~
- ~~Function.h → std::function~~ (removed entirely)
- ~~C-style casts~~ (~1,250 replaced with static_cast/reinterpret_cast/const_cast)
- ~~Random.h modernize~~ (drand48/srand48 → thread_local std::mt19937)
- ~~malloc/free cleanup~~ (CannyOp, DynMatrix, CLSurfLib → std::vector)
- ~~Precompiled headers~~ (39% CPU time reduction)

## Ready to Do

### Quick wins (1-2 hrs each)
| Task | Description |
|------|-------------|
| **sse2neon** | Drop in single header from https://github.com/DLTcollab/sse2neon, re-enable SIMD on ARM. Test with MedianOp, ConvolutionOp |
| **OpenCL C++ bindings** | Bundle `cl2.hpp` from Khronos, re-enable `-DBUILD_WITH_OPENCL=ON` on macOS |

### Medium effort (3-4 hrs each)
| Task | Description |
|------|-------------|
| **Mutex → std::mutex** | Replace pthread wrapper. `Mutex` → `std::mutex`, `Mutex::Locker` → `std::lock_guard<std::mutex>`. ~100 use sites. |
| **Thread → std::thread** | Replace pthread-based Thread class. Uses `ShallowCopyable` pattern, virtual `run()`. ~20 use sites. |
| **SmartPtr/SmartArray → std::shared_ptr** | 97 files use SmartPtr, 8 use SmartArray. SmartArray deeply woven into Img channel storage with ownership control (`bOwn` flag). Needs careful refactor especially for non-owning wraps in Img.cpp and LowLevelPlotWidget.cpp. |

### Larger efforts (half day+)
| Task | Description |
|------|-------------|
| **FFmpeg 7+ rewrite** | `LibAVVideoWriter.cpp` needs full rewrite for modern AVCodecContext API |
| **ImageMagick 7 rewrite** | `FileGrabber/WriterPluginImageMagick` — PixelPacket API removed, use Magick::Pixels |
| **Apple Accelerate** | Investigate vImage/vDSP as IPP replacement for convolution, color conversion, FFT on macOS |

### Code quality (ongoing)
| Task | Description |
|------|-------------|
| **ShallowCopyable removal** | Replace with `std::shared_ptr<Impl>` pattern. Used by Thread, File, Semaphore. Defer until after Mutex/Thread modernization. |
| **DynMatrix RAII** | Replace raw `new/delete[]` with `std::unique_ptr<T[]>` or `std::vector<T>` |
| **Channel const-correctness** | Remove mutable abuse in Channel.h, proper const/non-const design |
| **Any.h rethink** | Currently inherits std::string. Consider `std::any` or cleaner wrapper |

## Currently Disabled Optional Features
To re-enable, add these cmake flags:
```bash
-DBUILD_WITH_EIGEN3=ON     # ✅ works with Eigen 5.0.1
-DBUILD_WITH_PCL=ON        # ✅ works with PCL 1.15
-DBUILD_WITH_OPENCL=ON     # ❌ needs C++ bindings on macOS
-DBUILD_WITH_IMAGEMAGICK=ON # ❌ needs API v7 rewrite
-DBUILD_WITH_LIBAV=ON      # ❌ needs FFmpeg 7+ rewrite
```

## Recommended Build Command (all working features)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_QT=ON \
  -DBUILD_WITH_OPENCV=ON \
  -DBUILD_WITH_EIGEN3=ON \
  -DBUILD_WITH_PCL=ON \
  -DBUILD_WITH_BULLET=ON \
  -DBUILD_APPS=ON \
  -DBUILD_DEMOS=ON \
  -DBUILD_EXAMPLES=ON
```
