# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ICL (Image Component Library) is a C++ computer vision framework (v10.0.2) developed at the Neuroinformatics Group, University of Bielefeld, since 2006. Despite the name "library", ICL is a full framework: it provides not just CV functions but also a control strategy for multi-threaded GUI applications, build tools, and project scaffolding. Uses C++11, licensed under LGPL v3. Core requirements are libjpeg-dev and libpng-dev; all other dependencies are optional.

### Design Principles

- **Self-containment**: programmers write applications *in* ICL, minimizing need for external libraries. External dependencies are fully wrapped behind ICL's own interfaces.
- **Simple C++ interface**: short, readable code with minimal boilerplate. PIMPL pattern used extensively to hide implementation complexity and reduce compilation times. Templates only used when absolutely necessary (pixel access).
- **Optimal performance**: Intel IPP acceleration with C++ fallbacks. SIMD, OpenMP, and OpenCL acceleration paths available. Internal image buffers avoid runtime allocations.
- **Extensible plugin interfaces**: string-based configuration for non-time-critical paths (e.g., grabber selection via `-i` argument).

## Build Commands

```bash
# Configure (out-of-source build required)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Configure with tests (requires CMake 3.10+, downloads GTest automatically)
cmake .. -DBUILD_TESTS=ON

# Build only tests
cmake --build build --target tests

# Run tests
cd build && ctest

# Build with examples/demos/apps
cmake .. -DBUILD_EXAMPLES=ON -DBUILD_DEMOS=ON -DBUILD_APPS=ON

# Build documentation
cmake --build build --target pages
```

Key CMake options: `ENABLE_NATIVE_BUILD_OPTION`, `ENABLE_OPENMP_BUILD_OPTION`, `BUILD_REDIST=DEB|WIX`.

When `BUILD_TESTS=ON` and no build type is specified, CMake defaults to Debug; otherwise it defaults to Release.

## Pre-commit

```bash
pre-commit install        # set up hooks
pre-commit run -a         # run all checks
```

Checks: large files, case conflicts, merge conflicts, symlinks, YAML, trailing whitespace, line endings.

## Module Dependency Hierarchy

Modules are built in strict dependency order (each depends on all above it):

```
ICLUtils        ← foundation: threading, config, XML, file I/O
ICLMath         ← matrices, vectors, FFT, optimization
ICLCore         ← image representation (Img/ImgBase), color spaces
ICLFilter       ← convolution, morphology, thresholds
ICLIO           ← image grabbers, file I/O, camera backends
ICLCV           ← region detection, corners, tracking, template matching
ICLQt           ← GUI/visualization (conditional: requires Qt5)
ICLGeom         ← 3D geometry, point clouds, pose estimation
ICLMarkers      ← fiducial/marker detection
ICLPhysics      ← physics simulation (conditional: requires Bullet3 + Qt5)
```

ICLQt and ICLPhysics are conditionally built based on dependency availability.

## Key Architectural Concepts

### Image (Primary Image Type)

`core::Image` is the primary image type for application code. It wraps `shared_ptr<ImgBase>` providing value semantics, automatic memory management, and type-safe depth dispatch via the visitor pattern. Included automatically via `Common.h`.

```cpp
// Typical usage:
Image img = grabber.grabImage();
Image result = filter.apply(img);
gui["display"] = result;

// Or as a one-liner:
gui["display"] = filter.apply(grabber.grabImage());

// Type-safe pixel access via visit():
img.visit([](auto &typed) { typed(0, 0, 0) = 42; });

// Channel-level visitors:
img.visitChannels([](auto *data, int ch, int dim) { ... });
src.visitChannelsWith(dst, [](auto *s, auto *d, int ch, int dim) { ... });
```

Copy semantics are shallow (shared_ptr). Use `deepCopy()` for independence, `detach()` to make channel data independent. Use `as<T>()` for direct `Img<T>&` access, `ptr()` for legacy `ImgBase*` interop.

### Image Class Hierarchy (Internal)

`core::ImgBase` (abstract) manages all non-pixel image parameters: size, channel count, ROI, format, and a runtime *depth* parameter indicating the actual pixel type. The template class `core::Img<T>` derives from it for concrete pixel types: `icl8u`, `icl16s`, `icl32s`, `icl32f`, `icl64f`. Channels are stored in **planar** layout (each channel is a single contiguous data segment, not interleaved). New code should use `Image` instead of `ImgBase*` directly.

### Filter Architecture

Filters are split into `UnaryOp` (one input image) and `BinaryOp` (two input images) base classes. `UnaryOp` has a deep inheritance hierarchy (see `ICLFilter/src/ICLFilter/`): `BaseAffineOp`, `NeighborhoodOp`, etc. The `apply()` method returns `Image` for convenience, or takes `Image &dst` for buffer reuse. The virtual dispatch mechanism uses the 2-arg `apply(const ImgBase*, ImgBase**)` internally.

### Grabber Framework (ICLIO)

`GenericGrabber` provides a plugin-based backend system for image acquisition. Backend and device are selected at runtime via string-based `-input` / `-i` program argument (e.g., `-i file image.png`, `-i dc 0`, `-i create lena`). Use `grabImage()` to get an `Image` value. Properties are set inline: `-i dc 0@gain=500`. The same plugin architecture exists for image output.

### Application Pattern (ICLQt)

`ICLApp` ties together program argument parsing, an `init()` callback, and a `run()` callback that loops automatically. GUI is built declaratively with stream operators:
```cpp
gui << Display().handle("img") << CamCfg() << Show();
```

GUI components: `Display` (image view), `Canvas` (2D drawing), `Canvas3D` (3D drawing), `Slider`, `Button`, `Combo`, `CheckBox`, `Label`, etc.

### Visualization Stack (ICLQt + ICLGeom)

Three-layer OpenGL-based stack: `ICLWidget` (hardware-accelerated image display) → `ICLDrawWidget` (2D annotation in image coordinates) → `ICLDrawWidget3D` (3D overlays). The `Scene` class (geom module) provides a scene graph with object trees and camera management that links directly into `ICLDrawWidget3D`'s rendering loop.

### Marker Detection (ICLMarkers)

`FiducialDetector` with plugin backends for different marker types (ARToolKit, BCH, etc.). Returns `Fiducial` instances with deferred feature computation — marker boundary, 6D pose, etc. are only computed when the getter is called, then cached.

### Physics Module (ICLPhysics)

Wraps the Bullet physics engine. Object classes derive from `geom::SceneObject` for seamless visualization. `PhysicsScene` multiply-inherits from `Scene` and `PhysicsWorld`. Note: ICL's geom module uses **millimeters**, Bullet uses **centimeters** — an internal scaling factor bridges this.

## Module Structure Convention

Every module follows the same layout:

```
ModuleName/
  CMakeLists.txt
  src/ModuleName/    ← headers (.h) and source files (.cpp)
  examples/          ← example programs (BUILD_EXAMPLE macro)
  demos/             ← demo applications (BUILD_DEMO macro)
  apps/              ← end-user tools (BUILD_APP macro, select modules only)
  test/test-*.cpp    ← GTest files (one per module, discovered via CONFIGURE_GTEST)
```

## Test Structure

Each module has a single test file at `ModuleName/test/test-*.cpp` using Google Test. The `CONFIGURE_GTEST` macro in `cmake/Modules/ICLHelperMacros.cmake` auto-discovers `test/test-*.cpp` files and links against `gtest_main` + the module library. Test executables are named `tests_<modulename>` (lowercase).

## CMake Build System Internals

- `cmake/Modules/ICLHelperMacros.cmake` — defines `BUILD_EXAMPLE`, `BUILD_DEMO`, `BUILD_APP`, `CONFIGURE_GTEST` macros
- `cmake/Modules/ICLFindPackage.cmake` — custom dependency detection
- `cmake/Modules/CheckArchitecture.cmake` — 32/64-bit detection
- Each module's CMakeLists.txt assembles sources via `FILE(GLOB ...)` and registers examples/demos/apps using the helper macros
- Optional dependencies use `BUILD_WITH_*` cache variables set by ICLFindPackage

## CI

GitHub Actions (`.github/workflows/ci.yaml`): Linux (Ubuntu), with ccache. Format checks via `.github/workflows/format.yaml` running pre-commit.
