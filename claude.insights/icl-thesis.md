# ICL Thesis Insights

Notes extracted from Chapter 2 ("A New Image Processing Library") of Christof Elbrechter's PhD thesis
"Towards Anthropomorphic Robotic Paper Manipulation" (2020, Universitat Bielefeld).

## Origin and Motivation

- ICL development began in early 2006 at the Neuroinformatics Group, Bielefeld University
- Goal: establish a new standard for computer vision applications internally, particularly for robotics research
- At the time, OpenCV was at version 0.8 with a C interface; ICL was built directly in C++ on top of Intel IPP
- Released as open-source in 2011, license changed from GPL to LGPL
- Name "ICL" reflects the internal structure of the image class: construction of images from different shared channels — image *components*

## Three Core Requirements

1. **Ease of use**: intuitive API, easy installation, platform independence, open-source license, readable produced code
2. **Speed**: efficient implementations hiding complexity; interfaces that allow custom optimization when needed
3. **Function volume**: hierarchical, indexed structure so functions are easy to find; intelligent use of parametrization, templating, and inheritance for a slim interface

## ICL as a Framework (not just a Library)

A *software library* provides reusable types/functions/classes. A *software framework* additionally provides tools for connecting, controlling, and maintaining software components. ICL is technically a framework:
- Provides a control strategy for multi-threaded GUI applications (dedicated GUI thread + worker threads)
- Includes build tools (`icl-make`, `icl-create-project`, `icl-create-cmake-project`)
- Manages application lifecycle via `ICLApp` with init/run callbacks
- However, the name "ICL" was kept since it was already established in the community

## Design Principles (Section 2.3.1)

### Self-Containment
- Programmers write applications *in* ICL, not just *using* it
- External libraries should only be needed for very specialized tasks
- Reduces data-type conversions between library interfaces and associated overhead
- Consistent naming conventions throughout improve source code readability

### Optimal Performance
- Wraps Intel IPP functions with C++ fallback implementations
- SIMD and OpenMP acceleration paths available
- GPGPU (OpenCL) acceleration for some components
- All classes use internal image buffers to avoid runtime memory allocations
- Image class provides direct access to channel data (each channel packed contiguously)
- Images can be shallowly *wrapped around* existing data segments to avoid copies

### Extensible and Convenient Interfaces
- Image acquisition uses a plugin interface class (`GenericGrabber`) for many image sources
- String-based interfaces used in non-time-critical functions for maximum generality
- Command-line parameter selection of image source backends at runtime

### Simple C++ Interface
- Designed for short, readable, and intuitive code
- Minimizes boilerplate, even for interactive applications
- PIMPL (private implementation) pattern used extensively to:
  - Hide implementation complexity
  - Reduce compilation times when linking against ICL
  - Completely wrap external dependencies for consistent interfaces
- Templates only used when completely necessary (pixel access)
- Shallow inheritance trees
- No compulsory library dependencies

## Image Class Hierarchy (Section 2.4.1)

The central design decision in ICL's image representation:

```
core::ImgBase (abstract)
  ├── core::Img<icl8u>    (8-bit unsigned int)
  ├── core::Img<icl16s>   (16-bit signed int)
  ├── core::Img<icl32s>   (32-bit signed int)
  ├── core::Img<icl32f>   (32-bit float)
  └── core::Img<icl64f>   (64-bit float)
```

- `ImgBase` manages all non-pixel parameters: size, channel count, ROI, format
- Holds a runtime `depth` parameter indicating the actual pixel type
- Allows generic, template-free code via `ImgBase` pointers
- Downcasting to `Img<T>` only needed for direct pixel access
- **Planar channel storage**: each channel is a contiguous data segment (not interleaved RGBRGB)
- Avoids the problems of other approaches:
  - Pure template (`Image<T>`): forces all generic code to also be templated, complicates interfaces
  - Runtime-only (OpenCV `Mat`): no compile-time type safety for pixel access
  - ICL combines both: type-safe pixel access via templates, generic operations via `ImgBase`

## Filter Architecture (Section 2.4.1)

- Filters split into `UnaryOp` (one input) and `BinaryOp` (two inputs)
- `BinaryOp` covers: pixel-wise comparison, arithmetic, logical combination, proximity measurement
- `UnaryOp` inheritance hierarchy:
  - `BaseAffineOp` → `AffineOp`, `MirrorOp`, `RotateOp`, `ScaleOp`, `TranslateOp`
  - `NeighborhoodOp` → `ConvolutionOp`, `MedianOp`, `MorphologicalOp`, `WienerOp`
  - `FFTOp`, `ThresholdOp`, etc.
- All share generic `apply()` methods — uniform interface for stacking/chaining
- Explicit hierarchy helps beginners classify and discover available operations

## Grabber Framework (Section 2.4.2)

- `GenericGrabber` provides a slim, plugin-based interface for image acquisition
- Backend and device selected at runtime via `-input` / `-i` program argument
- Examples:
  - `-i create lena` — synthetic test image
  - `-i dc 0` — first FireWire camera
  - `-i file 'images/*.jpeg'` — all JPEG files in directory
  - `-i list all` — enumerate supported devices
  - `-i dc 0@gain=500` — set device property at init
  - `-i dc 0@info` — query device properties
- `CamCfg` GUI component auto-creates control interfaces for all initialized grabbers
- Similar plugin framework exists for image output (file, video, network, shared memory)

## Visualization Stack (Section 2.4.3)

Three-layer OpenGL-based architecture:

1. **ICLWidget** (bottom): hardware-accelerated image display as OpenGL textures. Auto-scaling with aspect ratio preservation, brightness/contrast adjustment, seamless cross-thread updates. Two menus: on-screen (blue buttons at top) and external (detailed tabs). Mouse events report image-space coordinates accounting for current scaling/zoom.

2. **ICLDrawWidget** (middle): state-machine-like 2D annotation in image pixel coordinates. Drawing primitives (lines, polygons, text) are aligned with the background image and rendered via OpenGL.

3. **ICLDrawWidget3D** (top): 3D primitive rendering on top of the image. Links native OpenGL code into the rendering loop.

### Scene Graph (ICLGeom)
- `Scene` class: object tree with relative transformations at each node
- One or more `Camera` instances define rendering viewpoint
- Cameras can be estimated via built-in calibration tool using fiducial markers
- Scene provides OpenGL callback that links directly into `ICLDrawWidget3D`
- Mouse-based camera navigation available

## GUI Creation Pattern

Declarative, stream-operator-based syntax:
```cpp
#include <ICLQt/Common.h>

GUI gui;
GenericGrabber grabber;

void init(){
    grabber.init(pa("-i"));
    gui << Image().handle("img") << CamCfg() << Show();
}

void run(){
    gui["img"] = grabber.grab();
}

int main(int n, char **args){
    return ICLApp(n, args, "-input|-i(2)", init, run).exec();
}
```

`ICLApp` ties together:
- Program argument parsing (with typed argument specs like `"-input|-i(2)"`)
- `init()` callback (setup, run once)
- `run()` callback (main loop, called repeatedly)

## Marker Detection Toolbox (Section 2.4.4)

- `FiducialDetector`: plugin-based framework supporting multiple marker types (ARToolKit, BCH-code, Amoeba, etc.)
- Returns list of `Fiducial` instances with **deferred feature computation**:
  - Marker boundary, 6D pose, etc. computed only when the getter is called
  - Results cached for subsequent calls
- Generic `Fiducial` interface works across all backends
- Also used for camera calibration (fiducial markers on 3D calibration object)

## Physics Module (Section 2.4.5)

- Created for physical paper models needed in the thesis (robotic paper manipulation)
- Wraps Bullet physics engine, integrated with ICL's geom module
- **Unit mismatch**: ICL geom uses millimeters, Bullet uses centimeters, robotics framework uses yet another scale — internal scaling factor bridges these
- Object classes derive from `geom::SceneObject` for seamless visualization
- `PhysicsScene` multiply-inherits from `Scene` + `PhysicsWorld`
- `SoftObject` class wraps Bullet's soft-body module — basis for paper models
- Interactive paper model editor with drag-and-drop fold creation (Figure 2.6)
- Later additions: physical constraints, motors, collision callbacks

## Module Contents Summary (Table 2.3)

| Module | Description | Key Contents |
|--------|-------------|--------------|
| **utils** | general purpose utilities | Basic types, program argument evaluation, `Configurable` interface, time classes, exception types, config files, threading tools, string manipulation, generic `Function` class, random number generators |
| **math** | mathematics and ML | Dynamic/fixed matrix and vector classes, Levenberg-Marquardt, FFT, RANSAC, simplex/stochastic optimization, vector quantisation, SOM, LLM networks, quad/octree, least-squares model fitting, polynomial regression |
| **core** | basic image processing | `ImgBase`/`Img` classes, data handling, shallow copies, pixel access, ROI handling, channel management, copying, converting, scaling, minimal statistics, color format conversion |
| **filter** | image filtering | Unary: affine, warping, convolution, morphology, wiener, gabor, median, canny, chamfering, FFT, integral image, threshold, gradient, rectification. Binary: arithmetic, logical, pixel-wise comparison, proximity |
| **io** | image I/O framework | Grabber framework with backends for: FireWire, V4L, Kinect, Gig-E, shared-memory, RSB, OpenNI, OpenCV. Output framework for: files, video, shared-memory, RSB. I/O support classes for compression |
| **cv** | intermediate-level CV | Connected-component framework, SURF features, blob searching/tracking, flood-filling, Hough-line detector, generic tracking framework, mean-shift tracking, template matching |
| **qt** | GUI/visualization | GUI creation framework (all common UI components), image visualization, 2D/3D annotation, function/data plotting, mouse/keyboard handlers |
| **geom** | 3D geometry | Camera class, calibration, single/multi-view geometry, 3D scene graph, OpenGL visualization, point-cloud processing (including PCL compatibility), RGBD grabber framework, RGBD mapping, automatic 3D segmentation |
| **markers** | fiducial markers | Generic plugin-based framework for: ARToolKit, BCH-code, Amoeba, others. Single/multi-view 2D and 6D marker pose estimation |
| **physics** | Bullet wrapper | Rigid objects, soft-body objects, constraints, motors, physics world, seamless integration with 3D visualization, paper modeling |

## Positioning vs. Other Libraries

ICL differentiates itself from alternatives (as of thesis writing ~2015-2020):
- **vs. OpenCV**: ICL has no legacy C interface, simpler template usage, built-in GUI framework, self-contained. OpenCV has larger community and function volume.
- **vs. PCL**: ICL provides its own point-cloud processing but also offers PCL compatibility layer
- **vs. CImg**: ICL avoids single-header approach and excessive preprocessor macros
- **vs. VxL/RAVL**: ICL has simpler, less template-heavy interfaces
- ICL provides fast conversion functions to/from OpenCV image types for seamless interop
