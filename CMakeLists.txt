#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : CMakeLists.txt                                         **
#** Authors: Michael Goetting, Christof Elbrechter,                 **
#**          Sergius Gaulik                                         **
#**                                                                 **
#** GNU LESSER GENERAL PUBLIC LICENSE                               **
#** This file may be used under the terms of the GNU Lesser General **
#** Public License version 3.0 as published by the                  **
#**                                                                 **
#** Free Software Foundation and appearing in the file LICENSE.LGPL **
#** included in the packaging of this file.  Please review the      **
#** following information to ensure the license requirements will   **
#** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
#**                                                                 **
#*********************************************************************

cmake_minimum_required(VERSION 3.16)

project(ICL VERSION 26.0.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

# ---- CMake settings ----
include(ICLHelperMacros)
include(InstallFilesRecursive)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Put all binaries and libraries in a single output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ---- Build type ----
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  if(BUILD_TESTS)
    message(STATUS "No build type specified: Using Debug for tests")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
  else()
    message(STATUS "No build type specified: Using Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
  endif()
endif()

# ---- Compiler flags ----
if(MSVC)
  set(CMAKE_CONFIGURATION_TYPES Debug Release)
  add_compile_options(
    /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE
    /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS
    /wd4201 /wd4244 /wd4251 /wd4275 /wd4290 /wd4510 /wd4514
    /wd4610 /wd4710 /wd4711 /wd4800 /wd4820
    /fp:fast /Oi /Gy /MP /bigobj)
  string(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
  add_compile_options(-Wall)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# ---- Build options ----
option(ENABLE_PCH "Use precompiled headers to speed up compilation" ON)
option(ENABLE_NATIVE_BUILD_OPTION "Use -march=native optimization" OFF)
option(ENABLE_FASTMATH_BUILD_OPTION "Use -ffast-math optimization" OFF)
option(ENABLE_SSEFPMATH_BUILD_OPTION "Use -mfpmath=sse optimization" OFF)
option(ENABLE_OPENMP_BUILD_OPTION "Use OpenMP" OFF)

if(NOT MSVC)
  if(ENABLE_OPENMP_BUILD_OPTION)
    add_compile_options(-fopenmp)
    add_compile_definitions(USE_OPENMP)
  endif()
  if(ENABLE_NATIVE_BUILD_OPTION)
    add_compile_options(-march=native)
  endif()
  if(ENABLE_FASTMATH_BUILD_OPTION)
    add_compile_options(-ffast-math)
  endif()
  if(ENABLE_SSEFPMATH_BUILD_OPTION)
    add_compile_options(-mfpmath=sse)
  endif()
else()
  if(ENABLE_OPENMP_BUILD_OPTION)
    add_compile_options(/openmp)
    add_compile_definitions(USE_OPENMP)
  endif()
endif()

# ---- Naming and versioning ----
set(ICL_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(ICL_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(ICL_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(SO_VERSION "${ICL_VERSION_MAJOR}.${ICL_VERSION_MINOR}")
set(INSTALL_PATH_PREFIX "icl-${SO_VERSION}")
set(BINARY_PREFIX "icl-")
set(RESOURCE_DIR "etc/${INSTALL_PATH_PREFIX}")
set(ICL_CMAKE_MODULE_PATH "share/${INSTALL_PATH_PREFIX}/cmake/Modules")
set(ICL_FILE_NAME_COMPONENT_DIR PATH)

configure_file(
  ${PROJECT_SOURCE_DIR}/ICLUtils/src/ICLVersion.h.in
  ${CMAKE_BINARY_DIR}/src/ICLUtils/ICLVersion.h)

# ---- Architecture (64-bit only) ----
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "ICL requires a 64-bit platform. 32-bit is no longer supported.")
endif()
set(ICL_64BIT TRUE)
set(ARCH "x64")

# ---- Platform definitions ----
if(APPLE)
  ADD_ICL_DEFINITIONS(-DICL_SYSTEM_APPLE)
elseif(UNIX)
  ADD_ICL_DEFINITIONS(-DICL_SYSTEM_LINUX)
elseif(WIN32)
  ADD_ICL_DEFINITIONS(-DICL_SYSTEM_WINDOWS -DHAVE_STRUCT_TIMESPEC)
endif()

# ---- SIMD detection ----
set(HAVE_SIMDS "")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|AMD64|i[3-6]86)")
  include(CheckCXXCompilerFlag)
  option(DISABLE_SSE2 "Disable SSE2 even if available" OFF)
  option(DISABLE_SSE3 "Disable SSE3 even if available" OFF)
  option(DISABLE_SSSE3 "Disable SSSE3 even if available" OFF)

  foreach(EXT SSE2 SSE3 SSSE3)
    string(TOLOWER "${EXT}" ext_lower)
    check_cxx_compiler_flag("-m${ext_lower}" HAS_${EXT})
    if(HAS_${EXT} AND NOT DISABLE_${EXT})
      message(STATUS "${EXT} found and enabled")
      ADD_ICL_DEFINITIONS(-DICL_USE_${EXT})
      list(APPEND HAVE_SIMDS ${EXT})
    elseif(HAS_${EXT})
      message(STATUS "${EXT} found but disabled")
    else()
      message(STATUS "${EXT} not found")
    endif()
  endforeach()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(aarch64|arm64|ARM64)")
  # ARM NEON: use sse2neon to translate SSE intrinsics to NEON
  option(ENABLE_SSE2NEON "Use sse2neon for SIMD on ARM (SSE→NEON translation)" ON)
  if(ENABLE_SSE2NEON)
    message(STATUS "ARM NEON: enabling SSE2/SSE3/SSSE3 via sse2neon")
    ADD_ICL_DEFINITIONS(-DICL_USE_SSE2 -DICL_USE_SSE3 -DICL_USE_SSSE3)
    include_directories(${ICL_SOURCE_DIR}/3rdparty/sse2neon)
    list(APPEND HAVE_SIMDS SSE2 SSE3 SSSE3)
  else()
    message(STATUS "ARM NEON: sse2neon disabled")
  endif()
else()
  message(STATUS "SIMD: not applicable on ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# ---- Optional dependency options ----
option(BUILD_WITH_OPENCV "OpenCV image conversion functions" OFF)
option(BUILD_WITH_QT "GUI and visualization framework (needs OpenGL)" OFF)
option(BUILD_WITH_EIGEN3 "Eigen3 for speedup" OFF)
if(NOT APPLE)
  option(BUILD_WITH_IPP "Intel IPP for speedup (x86 Linux only)" OFF)
  option(BUILD_WITH_MKL "Intel MKL for math speedup (x86 Linux only)" OFF)
endif()
option(BUILD_WITH_OPENCL "OpenCL acceleration" OFF)
option(BUILD_WITH_IMAGEMAGICK "ImageMagick for image I/O" ON)
option(BUILD_WITH_LIBAV "libav/FFmpeg for video I/O" ON)
option(BUILD_WITH_LIBDC "FireWire camera support" OFF)
option(BUILD_WITH_V4L "V4L2 USB camera support" OFF)
option(BUILD_WITH_LIBFREENECT "Kinect 1 support" OFF)
option(BUILD_WITH_LIBFREENECT2 "Kinect 2 support" OFF)
option(BUILD_WITH_ZMQ "ZeroMQ network I/O" OFF)
option(BUILD_WITH_PYLON "Basler Pylon GigE cameras" OFF)
option(BUILD_WITH_PCL "PCL point cloud compatibility" OFF)

# Cycles: auto-detect if built in 3rdparty/cycles/
set(_CYCLES_SESSION "${CMAKE_SOURCE_DIR}/3rdparty/cycles/build/lib/libcycles_session.a")
if(EXISTS ${_CYCLES_SESSION})
  option(BUILD_WITH_CYCLES "Blender Cycles raytracing (requires manual build in 3rdparty/cycles)" ON)
else()
  option(BUILD_WITH_CYCLES "Blender Cycles raytracing (requires manual build in 3rdparty/cycles)" OFF)
endif()
option(BUILD_WITH_BULLET "Bullet physics simulation" OFF)
option(BUILD_WITH_LIBUSB "USB device support" OFF)
if(NOT WIN32 AND NOT APPLE)
  option(BUILD_WITH_DL "Dynamic linking support" ON)
endif()

# ---- Build targets ----
option(BUILD_EXAMPLES "Build example programs" OFF)
option(BUILD_DEMOS "Build demo programs" OFF)
option(BUILD_APPS "Build applications" OFF)
option(BUILD_TESTS "Build test suite" OFF)
set(BUILD_REDIST OFF CACHE STRING "Enable redistribution (DEB or WIX)")
set_property(CACHE BUILD_REDIST PROPERTY STRINGS OFF "DEB" "WIX")

# ---- Packaging ----
if(NOT BUILD_REDIST STREQUAL OFF)
  add_subdirectory(packaging)
endif()

# ---- Testing ----
if(BUILD_TESTS)
  enable_testing()
endif()

# ---- Dependency handling ----
include(ICLFindPackage)
include(CheckFunctionExists)

if(WIN32)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .lib)
  set(ARCH_DEPENDENT_LIB_PATHS "lib/x64;lib/msvc_x64")
elseif(APPLE)
  # Don't override CMAKE_FIND_LIBRARY_SUFFIXES — macOS uses .dylib and .tbd stubs
  # Ensure Homebrew paths are searched
  list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
else()
  set(CMAKE_FIND_LIBRARY_SUFFIXES .so)
  if(ICL_64BIT)
    set(ARCH_DEPENDENT_LIB_PATHS "lib/x86_64-linux-gnu;lib/intel64")
    set(ARCH_DEPENDENT_INCLUDE_PATHS "include/x86_64-linux-gnu")
  else()
    set(ARCH_DEPENDENT_LIB_PATHS "lib/i386-linux-gnu;lib/ia32")
    set(ARCH_DEPENDENT_INCLUDE_PATHS "include/i386-linux-gnu")
  endif()
endif()

# ---- Required dependencies ----
if(WIN32)
  ICL_FIND_PACKAGE(NAME Pthreads HEADERS "pthread.h" LIBS "pthreadVC2")
  if(PTHREADS_FOUND)
    list(APPEND REQUIRED_LIBRARIES_LIST ${PTHREADS_LIBRARIES})
  endif()
  list(APPEND REQUIRED_LIBRARIES_LIST regex_icl zlib libjpeg libpng)
  ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBZ -DICL_HAVE_LIBJPEG -DICL_HAVE_LIBPNG)
else()
  find_package(Threads REQUIRED)
  list(APPEND REQUIRED_LIBRARIES_LIST ${CMAKE_THREAD_LIBS_INIT})

  find_package(ZLIB REQUIRED)
  if(ZLIB_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBZ)
    list(APPEND REQUIRED_LIBRARIES_LIST ${ZLIB_LIBRARIES})
  endif()

  find_package(JPEG REQUIRED)
  if(JPEG_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBJPEG)
    list(APPEND REQUIRED_LIBRARIES_LIST ${JPEG_LIBRARIES})
  endif()

  find_package(PNG REQUIRED)
  if(PNG_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBPNG)
    list(APPEND REQUIRED_LIBRARIES_LIST ${PNG_LIBRARIES})
  endif()
endif()

if(NOT WIN32 AND NOT APPLE AND BUILD_WITH_DL)
  ICL_FIND_PACKAGE(NAME DL HEADERS "dlfcn.h" LIBS "dl" OPTIONAL)
  if(DL_FOUND)
    list(APPEND REQUIRED_LIBRARIES_LIST ${DL_LIBRARIES})
  else()
    # glibc 2.34+ integrates dl functions into libc — no separate libdl needed
    check_function_exists(dlopen HAVE_DLOPEN_IN_LIBC)
    if(HAVE_DLOPEN_IN_LIBC)
      set(DL_FOUND TRUE)
    endif()
  endif()
endif()

# ---- Optional dependencies ----

if(BUILD_WITH_IPP)
  if(ICL_64BIT)
    set(_IPP_LIB_SUF "ipp/lib/intel64")
  else()
    set(_IPP_LIB_SUF "ipp/lib/ia32")
  endif()
  set(IPP_REQUESTED_LIBS "ippcore;ippi;ipps;ippcv;ippcc")
  ICL_FIND_PACKAGE(NAME IPP HEADERS "ipp.h"
    LIBS ${IPP_REQUESTED_LIBS}
    PATHS "/opt/intel;/opt/IPP;/opt/intel/oneapi/ipp/latest"
    HEADER_PATH_SUFFIXES "ipp/include" "include"
    LIB_PATH_SUFFIXES ${_IPP_LIB_SUF} "lib")
  if(IPP_FOUND)
    if(EXISTS "${IPP_INCLUDE_DIRS}/ippversion.h")
      get_ipp_version(${IPP_INCLUDE_DIRS})
    endif()
    # oneAPI IPP puts individual headers (ipps.h, ippi.h, ...) in an ipp/ subdirectory
    if(IS_DIRECTORY "${IPP_INCLUDE_DIRS}/ipp")
      list(APPEND IPP_INCLUDE_DIRS "${IPP_INCLUDE_DIRS}/ipp")
    endif()
    ADD_ICL_DEFINITIONS(-DICL_HAVE_IPP)
  endif()
endif()

if(BUILD_WITH_MKL)
  if(ICL_64BIT)
    set(_MKL_LIB "mkl_intel_lp64")
    set(_MKL_LIB_SUF "mkl/lib/intel64")
  else()
    set(_MKL_LIB $<IF:$<BOOL:${WIN32}>,mkl_intel_c,mkl_intel>)
    set(_MKL_LIB_SUF "mkl/lib/ia32")
  endif()
  ICL_FIND_PACKAGE(NAME MKL HEADERS "mkl_types.h"
    LIBS "${_MKL_LIB};mkl_sequential;mkl_core"
    PATHS "/opt/intel;/opt/MKL;/opt/MKL/mkl;/opt/intel/oneapi/mkl/latest"
    HEADER_PATH_SUFFIXES "mkl/include" "include"
    LIB_PATH_SUFFIXES "${_MKL_LIB_SUF}" "lib/intel64" "lib")
endif()

if(BUILD_WITH_OPENCL)
  if(BUILD_WITH_OPENCL_OPTIONAL)
    find_package(OpenCL QUIET)
  else()
    find_package(OpenCL REQUIRED)
  endif()
  if(OpenCL_FOUND)
    message(STATUS "Found: OpenCL ${OpenCL_LIBRARIES}")
    set(OPENCL_FOUND TRUE)
    set(OPENCL_LIBRARIES ${OpenCL_LIBRARIES})
    ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENCL)
  endif()
endif()

if(BUILD_WITH_EIGEN3)
  ICL_FIND_PACKAGE(NAME Eigen3 HEADERS "Eigen/Eigen"
    HEADER_PATH_SUFFIXES "include/eigen3")
endif()

# Apple Accelerate framework (BLAS/LAPACK/vDSP — always available on macOS)
if(APPLE)
  find_library(ACCELERATE_LIBRARY Accelerate)
  if(ACCELERATE_LIBRARY)
    set(ACCELERATE_FOUND TRUE)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_ACCELERATE -DACCELERATE_NEW_LAPACK)
    message(STATUS "Found: Apple Accelerate framework")
  endif()
endif()

if(BUILD_WITH_OPENCV)
  if(OPENCV_ROOT)
    set(OpenCV_DIR ${OPENCV_ROOT})
  endif()
  find_package(OpenCV REQUIRED)
  if(OpenCV_FOUND)
    set(OPENCV_FOUND TRUE)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENCV)
    ADD_ICL_DEFINITIONS(-DICL_OPENCV_INSTALL_PATH="${OpenCV_INSTALL_PATH}")
    if(NOT OpenCV_VERSION VERSION_LESS "2.4")
      ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENCV_FEATURES_2D)
      set(OPENCV_FEATURES_2D_FOUND TRUE)
    endif()
  endif()
endif()

if(BUILD_WITH_IMAGEMAGICK)
  find_package(ImageMagick REQUIRED COMPONENTS Magick++)
  if(ImageMagick_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_IMAGEMAGICK)
    set(IMAGEMAGICK_LIBRARIES ${ImageMagick_LIBRARIES})
  endif()
endif()

if(BUILD_WITH_LIBDC)
  find_package(LibDC REQUIRED)
  if(LIBDC_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBDC)
  endif()
endif()

if(BUILD_WITH_LIBUSB)
  if(WIN32)
    ICL_FIND_PACKAGE(NAME LIBUSB HEADERS "usb.h;lusb0_usb.h" LIBS "libusb")
  else()
    ICL_FIND_PACKAGE(NAME LIBUSB HEADERS "usb.h;lusb0_usb.h" LIBS "usb")
  endif()
endif()

if(BUILD_WITH_V4L)
  find_package(V4L REQUIRED)
  if(V4L_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_VIDEODEV)
  endif()
endif()

if(BUILD_WITH_ZMQ)
  ICL_FIND_PACKAGE(NAME ZMQ HEADERS "zmq.hpp" LIBS "zmq")
endif()

if(BUILD_WITH_QT)
  set(QT_ROOT QT_ROOT CACHE PATH "Root directory of Qt6")

  if(WIN32)
    find_package(WindowsSDK REQUIRED)
    if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
      set(WINSDK_LIB_PATH "${WINDOWSSDK_PREFERRED_DIR}/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/${ARCH}")
      set(OPENGL_gl_LIBRARY "${WINSDK_LIB_PATH}/OpenGL32.lib" CACHE STRING "")
      set(OPENGL_glu_LIBRARY "${WINSDK_LIB_PATH}/GlU32.lib" CACHE STRING "")
    endif()
    if(ICL_64BIT)
      set(_GLEW_LIB_SUF "lib/Release/x64")
    else()
      set(_GLEW_LIB_SUF "lib/Release/Win32")
    endif()
    ICL_FIND_PACKAGE(NAME GLEW HEADERS "GL/glew.h" LIBS "glew32"
      LIB_PATH_SUFFIXES ${_GLEW_LIB_SUF})
  else()
    ICL_FIND_PACKAGE(NAME GLEW HEADERS "GL/glew.h" LIBS "GLEW")
  endif()

  cmake_policy(SET CMP0072 NEW) # Prefer GLVND
  find_package(OpenGL REQUIRED)

  set(ICL_QT_MODULES Core Gui Widgets OpenGL OpenGLWidgets)
  # Multimedia is optional (only needed for camera grabbers, not available on all systems)
  find_package(Qt6Multimedia QUIET HINTS ${QT_ROOT})
  set(ICL_ALL_QT_LIBS_FOUND TRUE)
  foreach(M ${ICL_QT_MODULES})
    find_package(Qt6${M} HINTS ${QT_ROOT})
    if(NOT Qt6${M}_FOUND)
      set(ICL_ALL_QT_LIBS_FOUND FALSE)
      message(STATUS "Not Found: Qt-Module ${M}")
    else()
      message(STATUS "Found Qt-Module ${M}")
    endif()
  endforeach()

  if(ICL_ALL_QT_LIBS_FOUND)
    message(STATUS "Found: All Qt Modules")
    set(QT_FOUND TRUE)
    set(QT_LIBRARIES "")
    foreach(M ${ICL_QT_MODULES})
      list(APPEND QT_LIBRARIES Qt6::${M})
    endforeach()
    ADD_ICL_DEFINITIONS(-DICL_HAVE_QT)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENGL)
  endif()
endif()

if(BUILD_WITH_LIBFREENECT)
  ICL_FIND_PACKAGE(NAME LibFreenect HEADERS "libfreenect.h"
    LIBS "freenect;freenect_sync"
    HEADER_PATH_SUFFIXES "include/libfreenect")
endif()

if(BUILD_WITH_LIBFREENECT2)
  if(LIBFREENECT2_ROOT)
    set(LIBFREENECT2_GLFW3_ROOT ${LIBFREENECT2_ROOT})
    set(LIBFREENECT2_LIBUSB_ROOT ${LIBFREENECT2_ROOT})
  endif()
  ICL_FIND_PACKAGE(NAME LibFreenect2
    HEADERS "libfreenect2/opengl.h;libfreenect2/libfreenect2.hpp;libfreenect2/frame_listener_impl.h"
    LIBS "freenect2"
    HEADER_PATH_SUFFIXES "examples/protonect/include"
    LIB_PATH_SUFFIXES "examples/protonect/lib"
    OPTIONAL)
  ICL_FIND_PACKAGE(NAME LibFreenect2_GLFW3 HEADERS "GLFW/glfw3.h" LIBS "glfw"
    HEADER_PATH_SUFFIXES "depends/glfw/include" LIB_PATH_SUFFIXES "depends/glfw/lib" OPTIONAL)
  ICL_FIND_PACKAGE(NAME LibFreenect2_LIBUSB HEADERS "libusb-1.0/libusb.h" LIBS "usb-1.0"
    HEADER_PATH_SUFFIXES "depends/libusb/include" LIB_PATH_SUFFIXES "depends/libusb/lib" OPTIONAL)
  if(LIBFREENECT2_FOUND AND LIBFREENECT2_GLFW3_FOUND AND LIBFREENECT2_LIBUSB_FOUND)
    list(APPEND LIBFREENECT2_LIBRARIES ${LIBFREENECT2_GLFW3_LIBRARIES} ${LIBFREENECT2_LIBUSB_LIBRARIES})
    list(APPEND LIBFREENECT2_INCLUDE_DIRS ${LIBFREENECT2_GLFW3_INCLUDE_DIRS} ${LIBFREENECT2_LIBUSB_INCLUDE_DIRS})
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBFREENECT2)
  elseif(NOT BUILD_WITH_LIBFREENECT2_OPTIONAL)
    message(SEND_ERROR "Could NOT find all dependencies for LIBFREENECT2")
  endif()
endif()

if(BUILD_WITH_PYLON)
  find_package(Pylon REQUIRED)
  if(PYLON_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_PYLON)
  endif()
endif()

if(BUILD_WITH_PCL)
  if(BUILD_WITH_PCL_OPTIONAL)
    find_package(PCL 1.6 COMPONENTS kdtree common octree search QUIET)
  else()
    find_package(PCL 1.6 REQUIRED COMPONENTS kdtree common octree search QUIET)
  endif()
  if(PCL_FOUND)
    message(STATUS "Found PCL: ${PCL_INCLUDE_DIRS}")
    set(PCL_LIBRARIES_PC ${PCL_LIBRARIES})
    list(REMOVE_DUPLICATES PCL_LIBRARIES_PC)
    list(REMOVE_ITEM PCL_LIBRARIES_PC debug optimized pthread)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_PCL)
  elseif(NOT BUILD_WITH_PCL_OPTIONAL)
    message(SEND_ERROR "Could NOT find PCL")
  endif()
endif()

if(BUILD_WITH_BULLET)
  if(BUILD_WITH_BULLET_OPTIONAL)
    find_package(Bullet3 QUIET)
  else()
    find_package(Bullet3 REQUIRED)
  endif()
  set(BULLET_FOUND ${BULLET3_FOUND})
  if(BULLET_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_BULLET)
    # Resolve library paths
    set(BULLET_LIBS_NEW "")
    foreach(L ${BULLET_LIBRARIES})
      if(NOT EXISTS ${L})
        list(APPEND BULLET_LIBS_NEW "${BULLET_ROOT_DIR}/${BULLET_LIBRARY_DIRS}/lib${L}${CMAKE_FIND_LIBRARY_SUFFIXES}")
      else()
        list(APPEND BULLET_LIBS_NEW "${L}")
      endif()
    endforeach()
    set(BULLET_LIBRARIES ${BULLET_LIBS_NEW})
    message(STATUS "Found Bullet: ${BULLET_INCLUDE_DIRS}")
  elseif(NOT BUILD_WITH_BULLET_OPTIONAL)
    message(SEND_ERROR "Could NOT find Bullet")
  endif()
endif()

if(BUILD_WITH_LIBAV)
  ICL_FIND_PACKAGE(NAME libav
    HEADERS "libavcodec/avcodec.h;libavformat/avformat.h;libswscale/swscale.h"
    LIBS "avcodec;avformat;avutil;swresample;swscale;avdevice"
    HEADER_PATH_SUFFIXES "include/avcodec;include/avformat;include/avdevice;include/avfilter;include/avutil;include/swresample;include/swscale")
  if(LIBAV_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBAV)
  endif()
endif()

# ---- Generate ICLConfig.h ----
configure_file(
  ${PROJECT_SOURCE_DIR}/ICLUtils/src/ICLConfig.h.in
  ${CMAKE_BINARY_DIR}/src/ICLUtils/ICLConfig.h)

# ---- Global include directories (for 3rd-party headers) ----
# These are needed globally because the old module pattern expects them.
# TODO: migrate to per-target target_include_directories
set(_ALL_3RDPARTY_INCLUDES
  ${ZLIB_INCLUDE_DIRS} ${JPEG_INCLUDE_DIR} ${PNG_INCLUDE_DIRS}
  ${ImageMagick_INCLUDE_DIRS} ${IPP_INCLUDE_DIRS} ${MKL_INCLUDE_DIRS}
  ${OpenCL_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS}
  ${LIBAV_INCLUDE_DIRS} ${LIBDC_INCLUDE_DIRS} ${V4L_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS}
  ${LIBFREENECT_INCLUDE_DIRS} ${LIBFREENECT2_INCLUDE_DIRS}
  ${PYLON_INCLUDE_DIRS} ${PCL_INCLUDE_DIRS}
  ${ZMQ_INCLUDE_DIRS} ${LIBUSB_INCLUDE_DIRS} ${BULLET_INCLUDE_DIRS})
if(WIN32)
  list(APPEND _ALL_3RDPARTY_INCLUDES ${REGEX_ICL_INCLUDE_DIRS} ${PTHREADS_INCLUDE_DIRS})
endif()
# Remove empty entries
list(FILTER _ALL_3RDPARTY_INCLUDES EXCLUDE REGEX "^$")
if(_ALL_3RDPARTY_INCLUDES)
  include_directories(SYSTEM ${_ALL_3RDPARTY_INCLUDES})
endif()

# ---- Initialize module include dir accumulator ----
set(ICL_MODULE_INCLUDE_DIRS "" CACHE INTERNAL "Accumulated ICL module include directories")

# ---- Per-module 3rd-party library lists ----
list(REMOVE_DUPLICATES REQUIRED_LIBRARIES_LIST)

list(APPEND ICLUtils_3RDPARTY_LIBRARIES ${REQUIRED_LIBRARIES_LIST} ${IPP_LIBRARIES} ${OpenCL_LIBRARIES} ${DL_LIBRARIES})
list(APPEND ICLMath_3RDPARTY_LIBRARIES ${MKL_LIBRARIES} ${ACCELERATE_LIBRARY})
list(APPEND ICLFilter_3RDPARTY_LIBRARIES ${ACCELERATE_LIBRARY})
list(APPEND ICLCore_3RDPARTY_LIBRARIES ${OpenCV_LIBRARIES})

list(APPEND ICLIO_3RDPARTY_LIBRARIES
  ${LIBAV_LIBRARIES} ${ImageMagick_LIBRARIES} ${LIBDC_LIBRARIES}
  ${V4L_LIBRARIES} ${LIBUSB_LIBRARIES} ${OPENGL_LIBRARIES}
  ${LIBFREENECT_LIBRARIES} ${LIBFREENECT2_LIBRARIES}
  ${PYLON_LIBRARIES} ${ZMQ_LIBRARIES} ${QT_LIBRARIES})

list(APPEND ICLQt_3RDPARTY_LIBRARIES ${GLEW_LIBRARIES} ${QT_LIBRARIES})

list(APPEND ICLGeom_3RDPARTY_LIBRARIES ${PCL_LIBRARIES_PC})

list(APPEND ICLPhysics_3RDPARTY_LIBRARIES ${BULLET_LIBRARIES})

# ---- Add module subdirectories ----
if(WIN32)
  add_subdirectory(3rdparty)
endif()
add_subdirectory(ICLUtils)
add_subdirectory(ICLMath)
add_subdirectory(ICLCore)
add_subdirectory(ICLFilter)
add_subdirectory(ICLIO)
add_subdirectory(ICLCV)
if(QT_FOUND)
  add_subdirectory(ICLQt)
endif()
add_subdirectory(ICLGeom)
add_subdirectory(ICLMarkers)
if(BULLET_FOUND AND QT_FOUND)
  add_subdirectory(ICLPhysics)
endif()

# ---- Build examples, demos, and apps (after all modules, so all include dirs are available) ----
macro(_icl_add_deferred_dirs PROPERTY_NAME)
  get_property(_dirs GLOBAL PROPERTY ${PROPERTY_NAME})
  foreach(_dir ${_dirs})
    file(RELATIVE_PATH _reldir ${CMAKE_SOURCE_DIR} ${_dir})
    add_subdirectory(${_dir} ${CMAKE_BINARY_DIR}/${_reldir})
  endforeach()
endmacro()

if(BUILD_EXAMPLES)
  _icl_add_deferred_dirs(ICL_EXAMPLE_DIRS)
endif()
if(BUILD_DEMOS)
  _icl_add_deferred_dirs(ICL_DEMO_DIRS)
endif()
if(BUILD_APPS)
  _icl_add_deferred_dirs(ICL_APP_DIRS)
endif()

# ---- Experimental projects (after all modules) ----
option(BUILD_EXPERIMENTAL "Build experimental projects" OFF)
if(BUILD_EXPERIMENTAL AND QT_FOUND)
  add_subdirectory(ICLExperimental)
endif()

# ---- Benchmark tool (after all modules) ----
add_subdirectory(benchmarks)

# ---- Test tool (after all modules) ----
if(BUILD_TESTS)
  add_subdirectory(tests)
endif()

add_subdirectory(doc)
add_subdirectory(cmake)

# ---- Install config and scripts ----
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/src/ICLUtils/ICLConfig.h
  DESTINATION include/${INSTALL_PATH_PREFIX}/ICLUtils
  COMPONENT development)

if(NOT WIN32)
  foreach(FILE icl-make icl-create-project icl-create-cmake-project icl-version)
    configure_file(scripts/${FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/scripts/${FILE} @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/${FILE}
      PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
      DESTINATION bin/ COMPONENT development)
  endforeach()
endif()

file(COPY scripts/icl-create-video.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/scripts)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-video.sh
  PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  DESTINATION bin/ COMPONENT development)

set(PKG_CONFIG_FILE_NAME_NO_PC "ICL-${SO_VERSION}")

configure_file(scripts/icl-create-project-data/Makefile.custom.in
  ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data/Makefile.custom @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data/Makefile.custom
  DESTINATION ${RESOURCE_DIR}/ RENAME Makefile.custom.template COMPONENT development)

foreach(FILE Makefile doxyfile sample-test.cpp)
  file(COPY scripts/icl-create-project-data/${FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data/${FILE}
    DESTINATION ${RESOURCE_DIR} RENAME ${FILE}.template)
endforeach()

foreach(FILE app.CMakeLists.txt CMakeLists.txt latex.clear.sh
             latex.report.plots.sizevarplot.tex latex.talk.talk.tex
             src.HelloWorld.cpp app.main.cpp Doxyfile.in latex.library.bib
             latex.report.report.tex LICENSE.LGPL src.HelloWorld.h
             latex.build.sh latex.quick.sh
             latex.report.sections.introduction.tex src.CMakeLists.txt)
  file(COPY scripts/icl-create-cmake-project-data/${FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-cmake-project-data)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-cmake-project-data/${FILE}
    DESTINATION ${RESOURCE_DIR} RENAME ${FILE}.template COMPONENT development)
endforeach()

# ---- Meta PKG-config ----
if(BULLET_FOUND)
  CREATE_PKGCONFIG(NAME ICL PKGCONFIG_DEPS ICLPhysics-${SO_VERSION} ICLMarkers-${SO_VERSION})
else()
  CREATE_PKGCONFIG(NAME ICL PKGCONFIG_DEPS ICLMarkers-${SO_VERSION})
endif()

# ---- Build summary ----
set(OPTS OPENCV QT EIGEN3 IMAGEMAGICK LIBAV LIBDC LIBFREENECT LIBFREENECT2
  V4L LIBUSB ZMQ PYLON OPENCL PCL BULLET)
if(NOT APPLE)
  list(APPEND OPTS IPP MKL)
endif()
if(NOT WIN32)
  list(APPEND OPTS DL)
endif()

if(NOT HAVE_SIMDS)
  set(HAVE_SIMDS "(none)")
endif()
set(DO_BUILD "LIBS")
foreach(B APPS DEMOS EXAMPLES)
  if(BUILD_${B})
    list(APPEND DO_BUILD ${B})
  endif()
endforeach()

set(BUILD_MODULES Utils Math Core Filter IO CV Geom Markers)
if(QT_FOUND)
  list(APPEND BUILD_MODULES Qt)
endif()
if(BULLET_FOUND AND QT_FOUND)
  list(APPEND BUILD_MODULES Physics)
endif()

message(" ------------ Build Options: --------------------------------------")
message(" Build Type: .............. ${CMAKE_BUILD_TYPE}")
message(" C++ Standard: ........... C++${CMAKE_CXX_STANDARD}")
message(" SIMD Extensions: ......... ${HAVE_SIMDS}")
message(" Build Parts: ............. ${DO_BUILD}")
message(" Build Modules: ........... ${BUILD_MODULES}")
message(" ------------ Dependencies: ---------------------------------------")
foreach(O ${OPTS})
  string(LENGTH ${O} L)
  math(EXPR L "17-${L}")
  set(FILL "")
  while(${L} GREATER 0)
    set(FILL ".${FILL}")
    math(EXPR L "${L}-1")
  endwhile()
  if(${O}_FOUND)
    set(WHERE "--")
    if(${O}_DIR)
      set(WHERE "${${O}_DIR}")
    elseif(${O}_LIBRARIES)
      list(GET ${O}_LIBRARIES 0 E1)
      if(EXISTS "${E1}")
        get_filename_component(WHERE ${E1} ${ICL_FILE_NAME_COMPONENT_DIR})
      else()
        set(WHERE "${E1}")
      endif()
    elseif(${O}_INCLUDE_DIRS)
      list(GET ${O}_INCLUDE_DIRS 0 WHERE)
    endif()
    message(" ${O} ${FILL} yes ... ${WHERE}")
  else()
    if(BUILD_WITH_${O})
      if(BUILD_WITH_${O}_OPTIONAL)
        message(" ${O} ${FILL} no ....  - optional -   ")
      else()
        message(" ${O} ${FILL} no .... - missing! -   ")
      endif()
    else()
      message(" ${O} ${FILL} no .... - unselected - ")
    endif()
  endif()
endforeach()
message(" ------------ Grabber Backends: -----------------------------------")
foreach(G Generic_cameras|OPENCV|QT
          Video_files|OPENCV|LIBAV|QT
          Shared_memory|QT
          Network|ZMQ
          Kinect_1|LIBFREENECT
          Kinect_2|LIBFREENECT2
          Gig-E_cameras|PYLON
          Fire-Wire_cameras|LIBDC
          USB_cameras|V4L
          Image_files|IMAGEMAGICK|PNG|JPEG)
  string(REPLACE "|" ";" G ${G})
  list(GET G 0 WHAT)
  list(REMOVE_AT G 0)
  string(REPLACE "_" " " WHAT ${WHAT})
  string(LENGTH ${WHAT} L)
  math(EXPR L "25-${L}")
  set(FILL "")
  while(${L} GREATER 0)
    set(FILL ".${FILL}")
    math(EXPR L "${L}-1")
  endwhile()
  set(HAVE "")
  foreach(D ${G})
    if(${D}_FOUND)
      list(APPEND HAVE ${D})
    endif()
  endforeach()
  if(NOT HAVE)
    set(HAVE "(none)")
  endif()
  message(" ${WHAT} ${FILL} ${HAVE}")
endforeach()
