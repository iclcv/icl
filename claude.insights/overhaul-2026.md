# ICL Overhaul 2026 — Build System & Platform Modernization

## What was done

### CMake Modernization
- **cmake_minimum_required** bumped from 2.8.12 to 3.16
- **C++ standard** switched from C++11 to C++17
- Root CMakeLists.txt rewritten: ~1,256 → ~650 lines (removed ~600 lines dead code)
- New `icl_add_module()` function in ICLHelperMacros.cmake replaces ~1,700 lines of boilerplate across all 10 module CMakeLists.txt files
- All modules now use `file(GLOB)` for source discovery instead of explicit file lists
- Examples/demos/apps are built after all modules (deferred processing) so all include dirs are available
- GTest integration switched from manual download to `FetchContent`
- SSE SIMD detection restricted to x86 only (was falsely triggering on ARM)
- `NO_DEFAULT_PATH` removed from `ICL_FIND_PACKAGE` so Homebrew paths are found
- `/opt/homebrew` added to `CMAKE_PREFIX_PATH` on macOS
- macOS `.tbd` library stubs handled correctly (don't override `CMAKE_FIND_LIBRARY_SUFFIXES`)

### Qt5 → Qt6 Migration
- `QGLWidget` → `QOpenGLWidget` (Widget.h, Application.h, GLPaintEngine.h, GUI.cpp)
- `QGLContext` → `QOpenGLContext` (GLFragmentShader.cpp, GLImg.cpp)
- `QGLPixelBuffer` → `QOpenGLFramebufferObject` + `QOffscreenSurface` (Scene.cpp)
- `QWheelEvent::delta()/orientation()` → `angleDelta()` (Widget.cpp, Quick.cpp)
- `QMouseEvent::x()/y()` → `position().x()/y()` (Widget.cpp)
- `QDesktopWidget` → `QScreen` (Widget.cpp)
- `QMutex::Recursive` → `QRecursiveMutex` (6 files)
- `Qt::MidButton` → `Qt::MiddleButton` (3 files)
- `QLayout::setMargin()` → `setContentsMargins()` (4 files)
- `grabFrameBuffer()` → `grabFramebuffer()` (Widget.cpp)
- `findChildren<EmptyWidget*>` → `findChildren<QWidget*>` (Q_OBJECT requirement)
- Qt moc: `QT5_WRAP_CPP` → `qt_wrap_cpp`
- Qt multimedia grabbers disabled (need QVideoSink rewrite)

### C++17 Compatibility Fixes
- `std::random_shuffle` → `std::shuffle` with `std::mt19937` (Random.h, CoplanarPointPoseEstimator.cpp)
- `std::unary_function` / `std::binary_function` base classes removed (Function.h)
- `std::bind2nd` → lambdas (DynMatrix.h, FixedMatrix.h, Img.cpp, ConvolutionKernel.cpp, DemoGrabber.cpp)
- `std::mem_fun_ref` → lambda (RunLengthEncoder.cpp)
- `register` keyword: suppressed with `-Wno-register` (full removal deferred)
- `parse<T>` template: added `if constexpr` + `is_stream_extractable` trait to avoid instantiation errors with non-streamable types (StringUtils.h)
- Explicit `Any::as<std::string>()` call to avoid ambiguous Label conversion (GUI.cpp)

### macOS / Apple Silicon Fixes
- OpenGL includes: `<gl.h>` → `<OpenGL/gl.h>` on macOS (CompatMacros.h)
- OpenCL includes: `<CL/cl.h>` → `<OpenCL/opencl.h>` on macOS (CLIncludes.h)
- IPP/MKL restricted to Linux only (no Apple Silicon support)
- SSE SIMD disabled on ARM (NEON is the ARM equivalent)
- FFmpeg: `avresample` → `swresample` (removed in modern FFmpeg)

### Dependency Cleanup
Removed (dead/discontinued):
- RSB/RST/RSC (Bielefeld robotics middleware, unmaintained)
- OpenNI (discontinued)
- MesaSR (SwissRanger cameras, niche hardware)
- Xine (old video library, superseded by FFmpeg)
- XIAPI (Ximea cameras, specialized)
- LIBIRIMAGER/LIBUDEV (Optris IR cameras, specialized)

### Documentation
- README.md fully rewritten with current dependency tables and build instructions
- CLAUDE.md added for development guidance
- claude.insights/icl-thesis.md added with Chapter 2 thesis insights

## What still needs fixing (deferred)

### Dependency-specific
- **Eigen 5.0.1** — macro bug with Apple Clang. Pin to Eigen 3.4.x or wait for fix.
- **OpenCL on macOS** — C++ bindings (cl2.hpp) not available via framework. Bundle Khronos headers or use sse2neon approach.
- **ImageMagick 7** — `PixelPacket` API removed. FileGrabber/WriterPluginImageMagick need rewrite.
- **FFmpeg 7+** — LibAVVideoWriter needs rewrite for modern `AVCodecContext` API changes.
- **PCL** — pulls in Eigen, blocked by Eigen 5 bug.

### Qt6-specific
- **Qt multimedia grabbers** — `QAbstractVideoSurface` → `QVideoSink` rewrite needed for QtVideoGrabber and QtCameraGrabber.

### Performance / Platform
- **Apple Accelerate** — investigate vImage/vDSP as IPP replacement on macOS.
- **ARM NEON** — investigate sse2neon header for translating SSE intrinsics to NEON on Apple Silicon.
- **`register` keyword** — currently suppressed with `-Wno-register`, should be removed from source.

## Build Status

All 10 libraries compile and link on macOS ARM (Apple Silicon M3):
```
libICLUtils.10.0.dylib    ✓
libICLMath.10.0.dylib     ✓
libICLCore.10.0.dylib     ✓
libICLFilter.10.0.dylib   ✓
libICLIO.10.0.dylib       ✓
libICLCV.10.0.dylib       ✓
libICLQt.10.0.dylib       ✓
libICLGeom.10.0.dylib     ✓
libICLMarkers.10.0.dylib  ✓
libICLPhysics.10.0.dylib  ✓
```

### Currently disabled optional features
- Eigen3 (Eigen 5 bug)
- OpenCL (no C++ bindings on macOS)
- ImageMagick (API v7 incompatibility)
- FFmpeg/libav (API v7+ incompatibility)
- PCL (depends on Eigen)
- Qt multimedia grabbers (QVideoSink rewrite needed)
