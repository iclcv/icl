<h1 align="center">
  <img src="doc/icl-api/images/icl-logo-50.png" height="50px"/>
  <br>
  Image Component Library
</h1>

[![Version](https://img.shields.io/badge/version-10.0.2-brightgreen.svg)](https://github.com/iclcv/icl)
[![License](https://img.shields.io/github/license/iclcv/icl.svg)](LICENSE.LGPL)

## Table of Contents
- [About ICL](#about-icl)
- [Installation](#installation)
  - [Building from source](#building-from-source)
  - [Dependencies](#dependencies)

## About ICL

ICL is a C++ computer vision framework developed at the Neuroinformatics Group, University of Bielefeld. It provides a large set of classes and functions for image processing, computer vision, 3D geometry, GUI creation, and physics simulation — all designed to work together seamlessly.

### Key Design Principles

- **Optimal Performance** — optional Intel IPP acceleration, SIMD, OpenMP, and OpenCL support. Images use planar channel storage with zero-copy shallow wrapping of existing data.
- **Simple C++ Interface** — readable, low-boilerplate code. A complete image acquisition and visualization application fits in ~15 lines of C++. PIMPL pattern hides complexity.
- **Self-Contained Framework** — write applications *in* ICL rather than gluing libraries together. Includes GUI creation, program argument parsing, multi-threaded application lifecycle, and build tools.
- **No Compulsory Dependencies** — all external dependencies are optional. Build a slim ICL for algorithm development, then link against a full build for deployment.

## Installation

### Building from Source

**Required tools:**
- CMake 3.16+
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)

**Required libraries:**
- `libjpeg` (or `libjpeg-turbo`)
- `libpng`

#### Minimal build

```bash
git clone https://github.com/iclcv/icl.git
mkdir icl/build && cd icl/build
cmake ..
make -j$(nproc)
```

#### Recommended build (with Qt6, OpenCV, and common features)

```bash
cmake .. \
  -DBUILD_WITH_QT=ON \
  -DBUILD_WITH_OPENCV=ON \
  -DBUILD_WITH_EIGEN3=ON \
  -DBUILD_WITH_IMAGEMAGICK=ON \
  -DBUILD_WITH_LIBAV=ON \
  -DBUILD_WITH_OPENCL=ON \
  -DBUILD_APPS=ON \
  -DBUILD_DEMOS=ON \
  -DBUILD_EXAMPLES=ON
make -j$(nproc)
```

### Dependencies

All dependencies except libjpeg and libpng are **optional**. Enable them with `-DBUILD_WITH_<NAME>=ON`.

#### Core (recommended)

| Feature | CMake Flag | Description | Install (apt) | Install (brew) |
|---------|-----------|-------------|---------------|----------------|
| **Qt6** | `BUILD_WITH_QT` | GUI framework, visualization, image display | `qt6-base-dev qt6-multimedia-dev libqt6opengl6-dev libglew-dev` | `qt glew` |
| **OpenCV** | `BUILD_WITH_OPENCV` | Image type conversion, ORB features | `libopencv-dev` | `opencv` |
| **Eigen3** | `BUILD_WITH_EIGEN3` | Faster matrix operations | `libeigen3-dev` | `eigen` |
| **OpenCL** | `BUILD_WITH_OPENCL` | GPU-accelerated processing | `ocl-icd-opencl-dev opencl-headers` | *(built-in on macOS)* |
| **ImageMagick** | `BUILD_WITH_IMAGEMAGICK` | Extended image format I/O | `libmagick++-dev` | `imagemagick` |
| **FFmpeg** | `BUILD_WITH_LIBAV` | Video file I/O | `libavcodec-dev libavformat-dev libswscale-dev` | `ffmpeg` |

#### Camera backends

| Feature | CMake Flag | Description | Install (apt) | Install (brew) |
|---------|-----------|-------------|---------------|----------------|
| **V4L2** | `BUILD_WITH_V4L` | USB cameras/webcams (Linux) | *(usually in kernel headers)* | n/a |
| **libdc1394** | `BUILD_WITH_LIBDC` | FireWire cameras | `libdc1394-dev` | `libdc1394` |
| **libfreenect** | `BUILD_WITH_LIBFREENECT` | Kinect v1 | `libfreenect-dev` | `libfreenect` |
| **libfreenect2** | `BUILD_WITH_LIBFREENECT2` | Kinect v2 | build from source | build from source |
| **Pylon** | `BUILD_WITH_PYLON` | Basler GigE cameras | Basler SDK | Basler SDK |

#### Advanced

| Feature | CMake Flag | Description | Install (apt) | Install (brew) |
|---------|-----------|-------------|---------------|----------------|
| **PCL** | `BUILD_WITH_PCL` | Point Cloud Library compatibility | `libpcl-dev` | `pcl` |
| **Bullet3** | `BUILD_WITH_BULLET` | Physics simulation (ICLPhysics) | `libbullet-dev` | `bullet` |
| **ZeroMQ** | `BUILD_WITH_ZMQ` | Network image streaming | `libzmq3-dev` | `zeromq` |
| **libusb** | `BUILD_WITH_LIBUSB` | USB device support | `libusb-dev` | `libusb` |
| **Intel IPP** | `BUILD_WITH_IPP` | Intel Performance Primitives (Linux x86 only) | Intel oneAPI | n/a |
| **Intel MKL** | `BUILD_WITH_MKL` | Intel Math Kernel Library (Linux x86 only) | Intel oneAPI | n/a |

#### Build options

| Flag | Default | Description |
|------|---------|-------------|
| `BUILD_APPS` | OFF | Build end-user applications (installed as `icl-*`) |
| `BUILD_DEMOS` | OFF | Build demo programs |
| `BUILD_EXAMPLES` | OFF | Build documentation examples |
| `BUILD_TESTS` | OFF | Build test suite (downloads Google Test) |

### Running Tests

```bash
cmake .. -DBUILD_TESTS=ON
make tests    # build test executables
ctest         # run all tests

# or run individually:
./ICLMath/tests_iclmath
```

## Questions, Feedback, or Issues?

Please [open an issue](https://github.com/iclcv/icl/issues) on GitHub.
