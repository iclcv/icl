# Next Steps — Post-Overhaul Work Items

## Completed in Overhaul (can be removed from lists)
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

## Ready to Do (next session)

### Quick wins (1-2 hrs each)
| Task | Description |
|------|-------------|
| **C-style casts** | Replace `(type)expr` with `static_cast<>` etc. throughout |
| **sse2neon** | Drop in single header from https://github.com/DLTcollab/sse2neon, re-enable SIMD on ARM. Test with MedianOp, ConvolutionOp |
| **OpenCL C++ bindings** | Bundle `cl2.hpp` from Khronos, re-enable `-DBUILD_WITH_OPENCL=ON` on macOS |

### Medium effort (3-4 hrs each)
| Task | Description |
|------|-------------|
| **Function.h → std::function** | Replace 623-line custom class. ~50 call sites using `utils::Function<>` and `utils::function()` factory. Also `FunctionImpl`, `GlobalFunctionImpl`, `MemberFunctionImpl` etc. |
| **Mutex → std::mutex** | Replace pthread wrapper. `Mutex` → `std::mutex`, `Mutex::Locker` → `std::lock_guard<std::mutex>`. ~100 use sites. |
| **Thread → std::thread** | Replace pthread-based Thread class. Uses `ShallowCopyable` pattern, virtual `run()`. ~20 use sites. |
| **SmartPtr → std::shared_ptr** | Make `SmartPtr<T>` a direct alias for `std::shared_ptr<T>`. ~300 use sites. `SmartArray<T>` → `std::shared_ptr<T[]>` (C++17). |

### Larger efforts (half day+)
| Task | Description |
|------|-------------|
| **FFmpeg 7+ rewrite** | `LibAVVideoWriter.cpp` needs full rewrite for modern AVCodecContext API, const-correctness changes |
| **ImageMagick 7 rewrite** | `FileGrabber/WriterPluginImageMagick` — PixelPacket API removed, use Magick::Pixels |
| **Apple Accelerate** | Investigate vImage/vDSP as IPP replacement for convolution, color conversion, FFT on macOS |

### Code quality (ongoing)
| Task | Description |
|------|-------------|
| **ShallowCopyable removal** | Replace with `std::shared_ptr<Impl>` pattern. Used by Thread, File, Semaphore |
| **DynMatrix RAII** | Replace raw `new/delete[]` with `std::unique_ptr<T[]>` or `std::vector<T>` |
| **Channel const-correctness** | Remove mutable abuse in Channel.h, proper const/non-const design |
| **Any.h rethink** | Currently inherits std::string. Consider `std::any` or cleaner wrapper |
| **Random.h modernize** | Replace `drand48`/`srand48` with `std::mt19937` + distributions |

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
