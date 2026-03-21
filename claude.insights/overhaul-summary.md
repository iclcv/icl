# ICL Overhaul 2026 — Complete Summary

## Version: 26.0.0 (year-based versioning)

## What Was Done (25 commits on `overhaul` branch)

### Build System
- CMake minimum: 2.8.12 → 3.16
- C++ standard: C++11 → C++17
- New `icl_add_module()` function replaces ~1,700 lines of boilerplate
- All modules use `file(GLOB)` for source discovery
- Examples/demos/apps built after all modules (deferred processing)
- Output: `build/bin/` and `build/lib/` (unified)
- GTest via FetchContent (was manual download)
- SIMD detection restricted to x86 only
- 32-bit support dropped (64-bit enforced)
- Homebrew paths auto-detected on macOS
- `.tbd` library stubs handled on macOS

### Qt5 → Qt6 (complete)
- QGLWidget → QOpenGLWidget
- QGLContext → QOpenGLContext
- QGLPixelBuffer → QOpenGLFramebufferObject + QOffscreenSurface
- QAbstractVideoSurface → QVideoSink (full multimedia rewrite)
- QDesktopWidget → QScreen
- QMutex::Recursive → QRecursiveMutex
- Qt::MidButton → Qt::MiddleButton
- setMargin → setContentsMargins
- grabFrameBuffer → grabFramebuffer
- enterEvent(QEvent*) → enterEvent(QEnterEvent*)
- event->x()/y() → position().x()/y() throughout
- Camera permission plugin linked statically for macOS
- All Qt4/5 version guards removed

### macOS Apple Silicon (ARM) Support
- OpenGL includes fixed (OpenGL/gl.h, OpenGL/glu.h)
- OpenCL includes fixed (OpenCL/opencl.h)
- Retina/HiDPI viewport scaling (2D widget + 3D scene)
- SSE disabled on ARM (NEON planned)
- IPP/MKL restricted to Linux x86
- Info.plist embedding attempted, settled on static permission plugins
- Homebrew prefix added to CMAKE_PREFIX_PATH

### C++17 Source Modernization
- NULL → nullptr (115 replacements, 45 files)
- typedef → using (263 replacements, 75 files)
- Uncopyable base → = delete (63 classes, 56 files)
- std::iterator<> base → explicit type aliases (3 classes)
- SmartPtrBase: std::shared_ptr backend enabled, legacy removed
- std::bind2nd → lambdas (all instances)
- std::unary_function/binary_function → removed
- std::random_shuffle → std::shuffle
- std::mem_fun_ref → lambdas
- register keyword removed (93 instances, 11 files)
- parse<T> with if constexpr + is_stream_extractable trait

### Warning Cleanup (1,209 → 0 ICL-owned)
- delete vs delete[] mismatches fixed (memory bugs)
- Infinite recursion in RigidObject and Thread fixed (real bugs)
- sprintf → snprintf throughout
- GL_SILENCE_DEPRECATION for macOS OpenGL
- Unused variables/functions cleaned
- Hidden virtual function warnings fixed
- Non-virtual destructors added
- Self-assignment, bit-field, null deref bugs fixed
- #warning → // TODO comments

### Dependencies
- Removed: RSB/RST/RSC, OpenNI, MesaSR, Xine, XIAPI, LIBIRIMAGER/LIBUDEV
- Updated: PugiXML 1.6 → 1.15
- Fixed: Eigen 5 support (leaked C4 macro bug in FixedMatrix.h)
- Fixed: PCL 1.15 (boost::shared_ptr → std::shared_ptr in demo)
- Fixed: FFmpeg avresample → swresample (header include)
- Fixed: OpenCV BGR→RGB in camera grabber
- Fixed: ICLFindPackage NO_DEFAULT_PATH removed (finds Homebrew packages)

### Build Status
All 10 libraries + 43 apps + 64 demos + 17 examples build warning-free:
- ICLUtils, ICLMath, ICLCore, ICLFilter, ICLIO, ICLCV
- ICLQt, ICLGeom, ICLMarkers, ICLPhysics

Tested features: Qt6, OpenCV, Eigen 5, PCL 1.15, Bullet3

## Files Added
- `CLAUDE.md` — development guidance
- `claude.insights/icl-thesis.md` — Chapter 2 thesis insights
- `claude.insights/overhaul-2026.md` — detailed change log
- `claude.insights/deferred-tasks.md` — postponed work items
- `claude.insights/overhaul-next.md` — C++17 modernization plan
- `cmake/Info.plist.in` — macOS permission plist template
- `ICLUtils/src/ICLUtils/pugiconfig.hpp` — PugiXML 1.15 config
