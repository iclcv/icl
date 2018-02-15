#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : CMakeLists.txt                                         **
#** Module :                                                        **
#** Authors: Michael Goetting, Christof Elbrechter,                 **
#**          Sergius Gaulik                                         **
#**                                                                 **
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
#** The development of this software was supported by the           **
#** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
#** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
#** Forschungsgemeinschaft (DFG) in the context of the German       **
#** Excellence Initiative.                                          **
#**                                                                 **
#*********************************************************************

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

# ---- global definitions ----
# todo: add "-debug" postfix here and there!

# Version information
SET(ICL_VERSION_MAJOR "10" CACHE STRING "Major project version part")
SET(ICL_VERSION_MINOR "0" CACHE STRING "Minor project version part")
SET(ICL_VERSION_PATCH "1" CACHE STRING "Patch project version part")
SET(PROJECT_VERSION "${ICL_VERSION_MAJOR}.${ICL_VERSION_MINOR}.${ICL_VERSION_PATCH}")

# Since version 3.0.2 CMake warns about CMP0048 policy which should be
# set to NEW to allow PROJECT() to handle version and language information
IF(CMAKE_VERSION VERSION_LESS 3.0.0)
  PROJECT(ICL)
  IF(CMAKE_VERSION VERSION_LESS 2.8.12)
    SET(ICL_FILE_NAME_COMPONENT_DIR DIRECTORY)
  ENDIF()
ELSE()
  cmake_policy(SET CMP0048 NEW)
  PROJECT(ICL VERSION ${PROJECT_VERSION} LANGUAGES C CXX)
ENDIF()

# ---- cmake settings ----
INCLUDE(ICLHelperMacros)
INCLUDE(CheckArchitecture)
INCLUDE(InstallFilesRecursive)
INCLUDE(FindSSE)

SET(ICL_FILE_NAME_COMPONENT_DIR PATH)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# ---- build type (debug/release) and opptions
IF(WIN32)
  IF(MSVC)
    SET(ICL_MSC_VER ${MSVC_VERSION})
  ENDIF(MSVC)
ENDIF(WIN32)

IF(MSVC)
  set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)
ENDIF(MSVC)

IF(NOT WIN32)
  OPTION(ENABLE_NATIVE_BUILD_OPTION "Decide whether to use -march=native optimization flag" OFF)
  OPTION(ENABLE_SSEFPMATH_BUILD_OPTION "Decide whether to use -mfpmath=sse optimization flag" OFF)
  OPTION(ENABLE_FASTMATH_BUILD_OPTION "Decide whether to use -ffast-math optimization flag" OFF)
ENDIF()
OPTION(ENABLE_OPENMP_BUILD_OPTION "Decide whether to use -fopenmp optimization flag" OFF)

#if(CMAKE_CXX_COMPILER MATCHES ".*clang\\+\\+$") # little ugly here
#  set(ICL_DEACTIVATED_WARNING -Wno-internal-linkage-in-inline
#                              -Werror -Wno-\\#warnings
#                              -Wno-deprecated-declarations
#                              -Wno-uninitialized
#                              -Wno-overloaded-virtual)
#else()
  set(ICL_DEACTIVATED_WARNING "")
#endif()

IF(NOT CMAKE_BUILD_TYPE)
  IF(BUILD_TESTS)
    MESSAGE(STATUS "No build type specified: Using Debug for tests")
    SET(CMAKE_BUILD_TYPE "Debug")
  ELSE()
    MESSAGE(STATUS "No build type specified: Using Release")
    SET(CMAKE_BUILD_TYPE "Release")
  ENDIF()
ENDIF()

IF(MSVC)
  # Disable some warnings
  SET(ICL_COMPILER_FLAGS "/D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS")
  SET(ICL_COMPILER_FLAGS "${ICL_COMPILER_FLAGS} /wd4201 /wd4244 /wd4251 /wd4275 /wd4290 /wd4510 /wd4514 /wd4610 /wd4710 /wd4711 /wd4800 /wd4820")

  # Floating point model
  SET(ICL_COMPILER_FLAGS "${ICL_COMPILER_FLAGS} /fp:fast")
  # Enable intrinsic functions
  SET(ICL_COMPILER_FLAGS "${ICL_COMPILER_FLAGS} /Oi")
  # Enable function-level linking
  SET(ICL_COMPILER_FLAGS "${ICL_COMPILER_FLAGS} /Gy")
  # Multi-processor compilation
  SET(ICL_COMPILER_FLAGS "${ICL_COMPILER_FLAGS} /MP")

  string(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  /Zi /debug")
  SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${ICL_COMPILER_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ICL_COMPILER_FLAGS}")

  IF(ENABLE_OPENMP_BUILD_OPTION)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp -DUSE_OPENMP")
  ENDIF()
ELSE(MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${ICL_DEACTIVATED_WARNING}")
  SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

  IF(ENABLE_OPENMP_BUILD_OPTION)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -DUSE_OPENMP")
  ENDIF()
  IF(ENABLE_NATIVE_BUILD_OPTION)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
  ENDIF()
  IF(ENABLE_FASTMATH_BUILD_OPTION)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
  ENDIF()
  IF(ENABLE_SSEFPMATH_BUILD_OPTION)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse")
  ENDIF()
ENDIF(MSVC)

SET(ICL_NAME                   icl)
SET(SO_VERSION                 "${ICL_VERSION_MAJOR}.${ICL_VERSION_MINOR}")
SET(INSTALL_PATH_PREFIX        "${ICL_NAME}-${SO_VERSION}")
SET(BINARY_PREFIX              "${ICL_NAME}-")
SET(RESOURCE_DIR               "etc/${INSTALL_PATH_PREFIX}")

SET(ICL_CMAKE_MODULE_PATH "share/${INSTALL_PATH_PREFIX}/cmake/Modules")

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/ICLUtils/src/ICLVersion.h.in ${CMAKE_BINARY_DIR}/src/ICLUtils/ICLVersion.h)

# ---- Check Architecture ----
CURRENT_ARCHITECTURE(ICL_BUILD_ARCH)

IF(${ICL_BUILD_ARCH} EQUAL "64_BIT_FOUND")
  ADD_ICL_DEFINITIONS(-DICL_64BIT)
  SET(ARCH "x64")
ELSE()
  ADD_ICL_DEFINITIONS(-DICL_32BIT)
  SET(ARCH "x86")
ENDIF()

# --- global compiler flags ---
IF(APPLE)
  ADD_ICL_DEFINITIONS(-DICL_SYSTEM_APPLE)
ELSEIF(UNIX)
  ADD_ICL_DEFINITIONS(-DICL_SYSTEM_LINUX)
ELSEIF(WIN32)
  ADD_ICL_DEFINITIONS(-DICL_SYSTEM_WINDOWS -DHAVE_STRUCT_TIMESPEC)
ENDIF()

# ---- User options ----
OPTION(BUILD_WITH_OPENCV "Link against OpenCV and provide OpenCV image convertion functions" OFF)
OPTION(BUILD_WITH_QT "Provides GUI and image visualization framework (needs OPENGL)" OFF)
OPTION(BUILD_WITH_OPENGL "Linked to BUILD_WITH_QT" OFF)
OPTION(BUILD_WITH_RSB "Link against the RSB framework for network-based image transfer (needs PROTOBUF)" OFF)
OPTION(BUILD_WITH_PROTOBUF "Needed for RSB-based image transfer" OFF)
OPTION(BUILD_WITH_EIGEN3 "Uses Eigen3 to speed up several algorithms" OFF)
OPTION(BUILD_WITH_IPP "Link against Intel IPP for general speedup" OFF)
OPTION(BUILD_WITH_MKL "Link against Intel MKL to speed up several mathematic fuctions" OFF)
OPTION(BUILD_WITH_MESASR "Link against libmesasr to provide SwissRanger camera support" OFF)
OPTION(BUILD_WITH_IMAGEMAGICK "Use ImageMagick to provide I/O for many common image formats" OFF)
OPTION(BUILD_WITH_LIBAV "Use libav to provide video encoding for many common video formats" OFF)
OPTION(BUILD_WITH_LIBDC "Use libdc to provide FireWire-Camera support" OFF)
OPTION(BUILD_WITH_LIBFREENECT "Use libfreenect to grab from Kinect Devices" OFF)
OPTION(BUILD_WITH_LIBFREENECT2 "Use experimental libfreenect2 dependency to grab from Kinect2 Devices" OFF)
OPTION(BUILD_WITH_XINE "Provides a libxine base Video-Grabber backend" OFF)
IF(NOT WIN32)
OPTION(BUILD_WITH_DL "Link against libdl to provide dynamic runtime linking support" ON)
ENDIF(NOT WIN32)
OPTION(BUILD_WITH_V4L "Provides V4L2 grabber backend for USB-Cameras" OFF)
OPTION(BUILD_WITH_LIBUSB "Provides grabber backend for the pixelsense device (microsoft surface)" OFF)
OPTION(BUILD_WITH_ZMQ "Provides Zero-MQ-based network grabber and image output" OFF)
OPTION(BUILD_WITH_PYLON "Link against the basler Pylon package to support GIG-E cameras" OFF)
OPTION(BUILD_WITH_OPENCL "Uses OpenCL to speed up several algorithms" OFF)
OPTION(BUILD_WITH_OPENNI "Links against OpenNI to grab from Kinect devices" OFF)
OPTION(BUILD_WITH_PCL "Link against PCL to provide a compatibility layer to PCL point clouds" OFF)
OPTION(BUILD_WITH_BULLET "Provides physics simulation. Needs the bullet physics engine" OFF)
OPTION(BUILD_WITH_RST "Link against the RST framework for message definitions (needs PROTOBUF)" OFF)
OPTION(BUILD_WITH_RSC "Link against the RSC framework for message definitions (needs RST)" OFF)
OPTION(BUILD_WITH_XIAPI "Link against the libm3api, e.g. used for Ximea Cameras" OFF)
OPTION(BUILD_WITH_LIBIRIMAGER "Link against the libirimager needed for Optris' IR cameras (requires LIBUDEV and V4L)" OFF)
OPTION(BUILD_WITH_LIBUDEV "Link against the libudev (needed the LIBIRIMAGER dependency)" OFF)
OPTION(DISABLE_SSE2 "Disables SSE2 support, even if found on the current machine" OFF)
OPTION(DISABLE_SSE3 "Disables SSE3 support, even if found on the current machine" OFF)
OPTION(DISABLE_SSSE3 "Disables SSSE3 support, even if found on the current machine" OFF)

OPTION(BUILD_EXAMPLES "Decide if examples shall be build or not" OFF)
OPTION(BUILD_DEMOS "Decide if demos shall be build or not" OFF)
OPTION(BUILD_APPS "Decide if applications shall be build or not" OFF)
OPTION(BUILD_TESTS "Decide whether tests should be build" OFF)
SET(BUILD_REDIST OFF CACHE STRING "Enable redistribution workflow")
SET_PROPERTY(CACHE BUILD_REDIST PROPERTY STRINGS OFF "DEB" "WIX")

# ---- Generate package data ----

IF(NOT BUILD_REDIST STREQUAL OFF)
  ADD_SUBDIRECTORY(packaging)
ENDIF(NOT BUILD_REDIST STREQUAL OFF)

IF(BUILD_TESTS)

  # integrate local google test build as suggested in
  # https://github.com/google/googletest/tree/master/googletest#incorporating-into-an-existing-cmake-project
  IF(CMAKE_VERSION VERSION_LESS 3.10.0)
    MESSAGE(FATAL_ERROR "For testing cmake version 3.10 or higher is required")
  ENDIF(CMAKE_VERSION VERSION_LESS 3.10.0)

  configure_file(${CMAKE_SOURCE_DIR}/scripts/CMakeLists.txt.in googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()

  # Prevent overriding the parent project's compiler/linker
  # settings on Windows
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines
  # the gtest and gtest_main targets.
  add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                   ${CMAKE_BINARY_DIR}/googletest-build
                   EXCLUDE_FROM_ALL)

  # The gtest/gtest_main targets carry header search path
  # dependencies automatically when using CMake 2.8.11 or
  # later. Otherwise we have to add them here ourselves.
  if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
  endif()

  include(GoogleTest)
  enable_testing()
  add_custom_target(tests)

  include(CodeCoverage)
  APPEND_COVERAGE_COMPILER_FLAGS()
  # --coverage -fno-inline -fno-inline-small-functions -fno-default-inline
  add_definitions(-fno-inline)
  set(COVERAGE_EXCLUDES '/Applications/*' '/usr/*')
  add_custom_target(coverage)

  # run tests automatically after build
  add_custom_target(run-test
      ALL
      DEPENDS tests 
      COMMAND ${CMAKE_COMMAND} --build . --target test)

  # # In case we want to upload coverage to coveralls at some point
  # include(Coveralls)
ENDIF(BUILD_TESTS)

# ---- Dependency handling ----
INCLUDE(ICLFindPackage)

IF(WIN32)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .lib)
ELSEIF(APPLE)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .dylib)
ELSE()
  set(CMAKE_FIND_LIBRARY_SUFFIXES .so)
ENDIF(WIN32)

IF(WIN32)
  SET(PKG_SEARCH_PATHS "")
  SET(ARCH_DEPENDENT_INCLUDE_PATHS "")
  IF(ICL_64BIT)
    SET(ARCH_DEPENDENT_LIB_PATHS "lib/x64;lib/msvc_x64")
  ELSE(ICL_64BIT)
    SET(ARCH_DEPENDENT_LIB_PATHS "lib/x86;lib/msvc;lib/Win32")
  ENDIF(ICL_64BIT)
ELSEIF(APPLE)
  SET(ARCH_DEPENDENT_INCLUDE_PATHS "")
  SET(PKG_SEARCH_PATHS "/usr;/usr/local")
  LIST(APPEND CMAKE_PREFIX_PATH "/usr;/usr/local")
ELSE(WIN32)
  SET(PKG_SEARCH_PATHS "/usr;/usr/local")
  LIST(APPEND CMAKE_PREFIX_PATH "/usr;/usr/local")
  IF(ICL_64BIT)
    SET(ARCH_DEPENDENT_LIB_PATHS "lib/x86_64-linux-gnu;lib/intel64")
    SET(ARCH_DEPENDENT_INCLUDE_PATHS "include/x86_64-linux-gnu")
  ELSE(ICL_64BIT)
    SET(ARCH_DEPENDENT_LIB_PATHS "lib/i386-linux-gnu;lib/ia32")
    SET(ARCH_DEPENDENT_INCLUDE_PATHS "include/i386-linux-gnu")
  ENDIF(ICL_64BIT)
ENDIF(WIN32)

IF(WIN32)
    ICL_FIND_PACKAGE(NAME Pthreads
                     HEADERS "pthread.h"
                     LIBS "pthreadVC2")
    IF(PTHREADS_FOUND)
      LIST(APPEND REQUIRED_LIBRARIES_LIST ${PTHREADS_LIBRARIES})
    ENDIF()
ELSE()
    FIND_PACKAGE(Threads REQUIRED)
    IF(Threads_FOUND)
      LIST(APPEND REQUIRED_LIBRARIES_LIST ${CMAKE_THREAD_LIBS_INIT})
    ENDIF()
ENDIF()

# workaround for the fact that on windows, regex.h is not available
IF(WIN32)
  LIST(APPEND REQUIRED_LIBRARIES_LIST regex_icl)
ENDIF()

IF(WIN32)
  LIST(APPEND REQUIRED_LIBRARIES_LIST zlib)
  ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBZ)
ELSE()
  FIND_PACKAGE(ZLIB REQUIRED)
ENDIF()
IF(ZLIB_FOUND)
  ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBZ)
  LIST(APPEND REQUIRED_LIBRARIES_LIST ${ZLIB_LIBRARIES})
ENDIF()

IF(WIN32)
  LIST(APPEND REQUIRED_LIBRARIES_LIST libjpeg)
  ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBJPEG)
ELSE()
  FIND_PACKAGE(JPEG REQUIRED)
ENDIF()
IF(JPEG_FOUND)
  ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBJPEG)
  LIST(APPEND REQUIRED_LIBRARIES_LIST ${JPEG_LIBRARIES})
ENDIF()

IF(WIN32)
  LIST(APPEND REQUIRED_LIBRARIES_LIST libpng)
  ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBPNG)
ELSE()
  FIND_PACKAGE(PNG REQUIRED)
ENDIF()
IF(PNG_FOUND)
  ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBPNG)
  LIST(APPEND REQUIRED_LIBRARIES_LIST ${PNG_LIBRARIES})
ENDIF()

IF((NOT WIN32) AND (NOT APPLE))
  ICL_FIND_PACKAGE(NAME DL HEADERS "dlfcn.h" LIBS "dl")
  IF(DL_FOUND)
    LIST(APPEND REQUIRED_LIBRARIES_LIST ${DL_LIBRARIES})
  ENDIF()
ENDIF()

IF(BUILD_WITH_IPP)
  IF(ICL_64BIT)
    SET(_IPP_LIB_SUF "ipp/lib/intel64")
  ELSE(ICL_64BIT)
    SET(_IPP_LIB_SUF "ipp/lib/ia32")
  ENDIF(ICL_64BIT)

  SET(IPP_REQUESTED_LIBS "ippcore;ippi;ipps;ippcv;ippcc;iomp5")
  # check for the legacy version of IPP first
  # IPP 9 dropped ippm. Defining this check optional to prevent error.
  ICL_FIND_PACKAGE(NAME IPP
    HEADERS "ipp.h"
    LIBS "${IPP_REQUESTED_LIBS};ippm"
    PATHS "/opt/intel;/opt/IPP"
    HEADER_PATH_SUFFIXES "ipp/include"
    LIB_PATH_SUFFIXES ${_IPP_LIB_SUF}
    OPTIONAL)

  # This double check is a bit annoying. Feel free to improve this section.
  IF(NOT IPP_FOUND)
    MESSAGE("IPP Legacy not found. Checking for recent releases.")
    ICL_FIND_PACKAGE(NAME IPP
      HEADERS "ipp.h"
      LIBS ${IPP_REQUESTED_LIBS}
      PATHS "/opt/intel;/opt/IPP"
      HEADER_PATH_SUFFIXES "ipp/include"
      LIB_PATH_SUFFIXES ${_IPP_LIB_SUF})
  ENDIF(NOT IPP_FOUND)

  IF(IPP_FOUND)
    IF(EXISTS "${IPP_INCLUDE_DIRS}/ippversion.h")
      get_ipp_version(${IPP_INCLUDE_DIRS})
    ENDIF()
    ADD_ICL_DEFINITIONS(-DICL_HAVE_IPP)
  ENDIF()
ENDIF()


IF(BUILD_WITH_MKL)
  IF(ICL_64BIT)
    SET(_MKL_LIB "mkl_intel_lp64")
    SET(_MKL_LIB_SUF "mkl/lib/intel64")
  ELSE(ICL_64BIT)
    IF(WIN32)
      SET(_MKL_LIB "mkl_intel_c")
    ELSE(WIN32)
      SET(_MKL_LIB "mkl_intel")
    ENDIF(WIN32)
    SET(_MKL_LIB_SUF "mkl/lib/ia32")
  ENDIF(ICL_64BIT)
  ICL_FIND_PACKAGE(NAME MKL
                   HEADERS "mkl_types.h"
                   LIBS "${_MKL_LIB};mkl_intel_thread;mkl_core;iomp5"
                   PATHS "/opt/intel;/opt/MKL;/opt/MKL/mkl"
                   HEADER_PATH_SUFFIXES "mkl/include"
                   LIB_PATH_SUFFIXES "${_MKL_LIB_SUF}")
ENDIF()

IF(BUILD_WITH_OPENCL)
  IF(BUILD_WITH_OPENCL_OPTIONAL)
    FIND_PACKAGE(OpenCL)
  ELSE()
    FIND_PACKAGE(OpenCL REQUIRED)
  ENDIF()
  IF(OpenCL_FOUND)
    MESSAGE(STATUS "Found: OpenCL ${OpenCL_LIBRARIES}")
    SET(OPENCL_FOUND TRUE)
    IF(WIN32)
    SET(CMAKE_CXX_FLAGS "/arch:SSE2 ${CMAKE_CXX_FLAGS}")
    ELSE()
    SET(CMAKE_CXX_FLAGS "-msse2 ${CMAKE_CXX_FLAGS}")
    ENDIF()
   ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENCL)
   SET(OPENCL_LIBRARIES ${OpenCL_LIBRARIES})
 ELSE()
   IF(BUILD_WITH_OPENCL_OPTIONAL)
     MESSAGE(STATUS "Not Found: optional OpenCL")
   ELSE()
     MESSAGE(STATUS "Not Found: OpenCL")
   ENDIF()
 ENDIF()
 #wait what? that's not a definition!
  #add_definitions("-std=gnu++11")
ENDIF()

IF(BUILD_WITH_EIGEN3)
  ICL_FIND_PACKAGE(NAME Eigen3
                   HEADERS "Eigen/Eigen"
                   HEADER_PATH_SUFFIXES "include/eigen3")
ENDIF()

IF(BUILD_WITH_OPENNI)
  ICL_FIND_PACKAGE(NAME OpenNI
                   HEADERS "XnOpenNI.h"
                   LIBS "OpenNI"
                   HEADER_PATH_SUFFIXES "include/ni")

  IF(OpenNI_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENNI)
  ENDIF()
ENDIF()

IF(BUILD_WITH_OPENCV)
  message(STATUS "opencv root x: ${OPENCV_ROOT}")
  IF(OPENCV_ROOT)
    SET(OpenCV_DIR ${OPENCV_ROOT})
    message(STATUS "opencv root: ${OPENCV_ROOT}")
  ENDIF()
  SET(OpenCV_OLD_LIBS_NOT_FOUND "TRUE")
  SET(OPENCV_FEATURES_2D_FOUND "FALSE")
  FIND_PACKAGE(OpenCV REQUIRED)
  IF(OPENCV_FOUND)
    SET(OpenCV_FOUND "TRUE")
  ENDIF()
  IF(OpenCV_FOUND)
    SET(OPENCV_FOUND "TRUE")
    ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENCV)
    ADD_ICL_DEFINITIONS(-DQWT_DLL)
    # don't show it in the GUI
    MARK_AS_ADVANCED(OPENCV_FOUND)
    MARK_AS_ADVANCED(OpenCV_FOUND)

    message(STATUS "using opencv from : ${OpenCV_INSTALL_PATH}/share/opencv")

    ADD_ICL_DEFINITIONS(-DICL_OPENCV_INSTALL_PATH="${OpenCV_INSTALL_PATH}")

    IF(${OpenCV_OLD_LIBS_NOT_FOUND} STREQUAL "FALSE")
      ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENCV_OLD_STYLE)
    ELSE()
      ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENCV_NEW_STYLE)
      # we need at least opencv 2.4 for the ORB-feature interface we want to use
      IF(((NOT OPENCV_VERSION_MAJOR LESS 2) AND (NOT OPENCV_VERSION_MINOR LESS 4))
          OR (OPENCV_VERSION_MAJOR GREATER 2))
        MESSAGE(STATUS "OpenCV >= 2.4.x detected, adding ORB feature detector")
        ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENCV_FEATURES_2D)
        SET(OPENCV_FEATURES_2D_FOUND "TRUE")
      ELSE()
        MESSAGE(STATUS "OpenCV < 2.4.x detected, ommitting ORB feature detector")
      ENDIF()
    ENDIF()
  ENDIF()
  IF(OpenCV_DIR)
    SET(OPENCV_DIR ${OpenCV_DIR})
  ENDIF()
ENDIF()

IF(BUILD_WITH_IMAGEMAGICK)
  FIND_PACKAGE(ImageMagick REQUIRED COMPONENTS Magick++)
  IF(ImageMagick_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_IMAGEMAGICK)
    SET(IMAGEMAGICK_LIBRARIES ${ImageMagick_LIBRARIES})
  ENDIF()
ENDIF()

IF(BUILD_WITH_XIAPI)
 ICL_FIND_PACKAGE(NAME XIAPI
                  HEADERS "m3api/xiApi.h"
                  LIBS "m3api")
  IF(XIAPI_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_XIAPI)
  ENDIF()
ENDIF()

IF(BUILD_WITH_LIBIRIMAGER)
  IF(NOT BUILD_WITH_LIBUDEV)
    MESSAGE(SEND_ERROR "BUILD_WITH_LIBIRIMAGER was defined, but BUILD_WITH_LIBUDEV wasn't (udev is needed mandatorily for LIBIRIMAGER)")
  ENDIF()

  IF(NOT BUILD_WITH_V4L)
    MESSAGE(SEND_ERROR "BUILD_WITH_LIBIRIMAGER was defined, but BUILD_WITH_V4L wasn't (video for linus is needed mandatorily for LIBIRIMAGER)")
  ENDIF()

  ICL_FIND_PACKAGE(NAME LIBIRIMAGER
                  HEADERS "libirimager/IRImager.h"
                  LIBS "irimager;ircore;irimageprocessing")
                #please note that the library order seems to be mandatory here!
  ICL_FIND_PACKAGE(NAME LIBUDEV
                  LIBS "udev")

 IF(LIBIRIMAGER_FOUND AND LIBUDEV_FOUND)
   # check version:
    FILE(READ "${LIBIRIMAGER_INCLUDE_DIRS}/libirimager/IRImager.h" IRImagerH)
    #    message(SEND_ERROR ":file content: ${IRImagerH}")

    STRING(FIND "${IRImagerH}" "namespace evo" EVO_NAMESPACE_FOUND)
    IF(NOT "${EVO_NAMESPACE_FOUND}" STREQUAL "-1")
      ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBIRIMAGER_EVO)
    ENDIF()
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBIRIMAGER -DICL_HAVE_LIBUDEV)
  ELSE()
    SET(HAVE_LIBIRIMAGER FALSE)
    SET(HAVE_LIBUDEV FALSE)
  ENDIF()
ENDIF()


IF(BUILD_WITH_MESASR)
  ICL_FIND_PACKAGE(NAME MesaSR
                   HEADERS "libMesaSR.h"
                   LIBS "mesasr"
                   PATHS "/opt/MESASR")
  IF(MesaSR_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBMESAR)
  ENDIF()
ENDIF()

IF(BUILD_WITH_LIBDC)
  FIND_PACKAGE(LibDC REQUIRED)
  IF(LIBDC_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBDC)
  ENDIF()
ENDIF()

IF(BUILD_WITH_LIBUSB)
  IF(WIN32)
    ICL_FIND_PACKAGE(NAME LIBUSB
                     HEADERS "usb.h;lusb0_usb.h"
                     LIBS "libusb"
                     PATHS "/opt/MESASR")
  ELSE(WIN32)
    ICL_FIND_PACKAGE(NAME LIBUSB
                     HEADERS "usb.h;lusb0_usb.h"
                     LIBS "usb")
#    IF(NOT LIBUSB_FOUND)
#     ICL_FIND_PACKAGE(NAME LIBUSB
#        HEADERS "usb.h"
#        HEADER_PATH_SUFFIXES "include/libusb-1.0"
#        LIBS "usb")

  ENDIF(WIN32)
ENDIF()

IF(BUILD_WITH_V4L)
  FIND_PACKAGE(V4L REQUIRED)
  IF(V4L_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_VIDEODEV)
  ENDIF()
ENDIF()

IF(BUILD_WITH_ZMQ)
  ICL_FIND_PACKAGE(NAME ZMQ
                   HEADERS "zmq.hpp"
                   LIBS "zmq")
ENDIF()


IF(BUILD_WITH_XINE)
  ICL_FIND_PACKAGE(NAME Xine
                   HEADERS "xine.h"
                   LIBS "xine")
ENDIF()

IF(BUILD_WITH_QT)
  SET(QT_ROOT QT_ROOT CACHE PATH "Root directory of Qt5")
  IF(WIN32)
    FIND_PACKAGE(WindowsSDK REQUIRED)
    # Windows 10 SDK Scheme is a bit different
    # Force file names to find the right libraries later
    IF(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
      SET(WINSDK_LIB_PATH "${WINDOWSSDK_PREFERRED_DIR}/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/${ARCH}")
      SET(OPENGL_gl_LIBRARY "${WINSDK_LIB_PATH}/OpenGL32.lib" CACHE STRING "")
      SET(OPENGL_glu_LIBRARY "${WINSDK_LIB_PATH}/GlU32.lib" CACHE STRING "")
    ENDIF(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    SET(CMAKE_LIBRARY_PATH "${WINDOWSSDK_PREFERRED_DIR}/Lib")
    if(POLICY CMP0020)
      cmake_policy(SET CMP0020 NEW)
    endif()
    IF(ICL_64BIT)
      SET(_GLEW_LIB_SUF "lib/Release/x64")
    ELSE(ICL_64BIT)
      SET(_GLEW_LIB_SUF "lib/Release/Win32")
    ENDIF(ICL_64BIT)
    ICL_FIND_PACKAGE(NAME GLEW
      HEADERS "GL/glew.h"
      LIBS "glew32"
      LIB_PATH_SUFFIXES ${_GLEW_LIB_SUF})
  ELSE(WIN32)
    ICL_FIND_PACKAGE(NAME GLEW
      HEADERS "GL/glew.h"
      LIBS "GLEW")
  ENDIF(WIN32)

  FIND_PACKAGE(OpenGL REQUIRED)
  SET(ICL_QT_MODULES Core Gui Multimedia Widgets OpenGL)
  SET(ICL_ALL_QT_LIBS_FOUND TRUE)

  FOREACH(M ${ICL_QT_MODULES})
    FIND_PACKAGE(Qt5${M} REQUIRED HINTS ${QT_ROOT})
    IF(NOT Qt5${M}_FOUND)
      SET(ICL_ALL_QT_LIBS_FOUND FALSE)
      MESSAGE(STATUS "Not Found: Qt-Module ${M}")
    ELSE()
      MESSAGE(STATUS "Found Qt-Module ${M}")
    ENDIF()
  ENDFOREACH()

  IF(ICL_ALL_QT_LIBS_FOUND)
    MESSAGE(STATUS "Found: All Qt Modules")
    SET(QT_FOUND TRUE)
    #set(CMAKE_AUTOMOC ON)
    #set(CMAKE_INCLUDE_CURRENT_DIR ON)
    add_definitions(${Qt5Widgets_DEFINITIONS})
    IF(NOT WIN32)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    ENDIF(NOT WIN32)

    # This creates full library paths from Qt's colon separated paths.
    # Colon separated Qt library paths only work when Qt's find package was called.
    # This is not necessarily done in projects linking against icl so we need full
    # Paths here.
    IF(WIN32)
      SET(QWHAT IMPLIB)
      SET(QPFX Abs)
    ELSEIF(APPLE)
      SET(QWHAT LOCATION)
      SET(QPFX Abs)
    ELSE() # unix case
      SET(QWHAT SONAME)
      SET(QPFX "")
    ENDIF()

    FOREACH(M ${ICL_QT_MODULES})
      get_target_property(ICLQt5${M}Lib${QPFX} Qt5::${M} IMPORTED_${QWHAT}_RELEASE)
    ENDFOREACH()

    IF(UNIX AND NOT APPLE)
      # For some reason cmake does not add library paths to rpath when they end
      # with a number like libQt5Core.so.5. Changing the linked libraries to the
      # simple .so resolves this problem.
      get_target_property(_QT5CORE_LOCATION Qt5::Core LOCATION)
      get_filename_component(_QT5CORE_ABSOLUTE ${_QT5CORE_LOCATION} ${ICL_FILE_NAME_COMPONENT_DIR})

      FOREACH(M ${ICL_QT_MODULES})
        STRING(REGEX REPLACE "[.]so([.][0-9])+" ".so" ICLQt5${M}LibAbs "${_QT5CORE_ABSOLUTE}/${ICLQt5${M}Lib}")
      ENDFOREACH()
    ENDIF()
    SET(Qt5_LIBRARIES "")
    FOREACH(M ${ICL_QT_MODULES})
      LIST(APPEND Qt5_LIBRARIES ${ICLQt5${M}LibAbs})
    ENDFOREACH()
    SET(QT_LIBRARIES ${Qt5_LIBRARIES})
    # IF(WIN32)
    #   get_target_property(ICLQt5CoreLibAbs Qt5::Core IMPORTED_IMPLIB_RELEASE)
    #   get_target_property(ICLQt5GuiLibAbs Qt5::Gui IMPORTED_IMPLIB_RELEASE)
    #   get_target_property(ICLQt5MultimediaLibAbs Qt5::Multimedia IMPORTED_IMPLIB_RELEASE)
    #   get_target_property(ICLQt5WidgetsLibAbs Qt5::Widgets IMPORTED_IMPLIB_RELEASE)
    #   get_target_property(ICLQt5OpenGLLibAbs Qt5::OpenGL IMPORTED_IMPLIB_RELEASE)
    # ELSEIF(APPLE)
    #   get_target_property(ICLQt5CoreLibAbs Qt5::Core IMPORTED_LOCATION_RELEASE)
    #   get_target_property(ICLQt5GuiLibAbs Qt5::Gui IMPORTED_LOCATION_RELEASE)
    #   get_target_property(ICLQt5MultimediaLibAbs Qt5::Multimedia IMPORTED_LOCATION_RELEASE)
    #   get_target_property(ICLQt5WidgetsLibAbs Qt5::Widgets IMPORTED_LOCATION_RELEASE)
    #   get_target_property(ICLQt5OpenGLLibAbs Qt5::OpenGL IMPORTED_LOCATION_RELEASE)
    # ELSE(WIN32)
    #   get_target_property(ICLQt5CoreLib Qt5::Core IMPORTED_SONAME_RELEASE)
    #   get_target_property(ICLQt5GuiLib Qt5::Gui IMPORTED_SONAME_RELEASE)
    #   get_target_property(ICLQt5MultimediaLib Qt5::Multimedia IMPORTED_SONAME_RELEASE)
    #   get_target_property(ICLQt5WidgetsLib Qt5::Widgets IMPORTED_SONAME_RELEASE)
    #   get_target_property(ICLQt5OpenGLLib Qt5::OpenGL IMPORTED_SONAME_RELEASE)
    #   get_target_property(_QT5CORE_LOCATION Qt5::Core LOCATION)
    #   IF(CMAKE_VERSION VERSION_LESS 2.8.12)
    #     get_filename_component(_QT5CORE_ABSOLUTE ${_QT5CORE_LOCATION} PATH)
    #   ELSE()
    #     get_filename_component(_QT5CORE_ABSOLUTE ${_QT5CORE_LOCATION} DIRECTORY)
    #   ENDIF()
    #   MESSAGE(STATUS "ICLQt5CoreLib : ${ICLQt5CoreLib}")
    #   MESSAGE(STATUS "ICLQt5GuiLib : ${ICLQt5CoreLib}")
    #   MESSAGE(STATUS "ICLQt5MultimediaLib : ${ICLQt5MultimediaLib}")

    #   # For some reason cmake does not add library paths to rpath when they end
    #   # with a number like libQt5Core.so.5. Changing the linked libraries to the
    #   # simple .so resolves this problem.
    #   STRING(REGEX REPLACE "[.]so([.][0-9])+" ".so" ICLQt5CoreLibAbs "${_QT5CORE_ABSOLUTE}/${ICLQt5CoreLib}")
    #   STRING(REGEX REPLACE "[.]so([.][0-9])+" ".so" ICLQt5OpenGLLibAbs "${_QT5CORE_ABSOLUTE}/${ICLQt5OpenGLLib}")
    #   STRING(REGEX REPLACE "[.]so([.][0-9])+" ".so" ICLQt5WidgetsLibAbs "${_QT5CORE_ABSOLUTE}/${ICLQt5WidgetsLib}")
    #   STRING(REGEX REPLACE "[.]so([.][0-9])+" ".so" ICLQt5GuiLibAbs "${_QT5CORE_ABSOLUTE}/${ICLQt5GuiLib}")
    #   STRING(REGEX REPLACE "[.]so([.][0-9])+" ".so" ICLQt5MultimediaLibAbs "${_QT5CORE_ABSOLUTE}/${ICLQt5MultimediaLib}")
    # ENDIF(WIN32)

    # SET(Qt5_LIBRARIES ${ICLQt5CoreLibAbs})
    # LIST(APPEND Qt5_LIBRARIES ${ICLQt5OpenGLLibAbs})
    # LIST(APPEND Qt5_LIBRARIES ${ICLQt5GuiLibAbs})
    # LIST(APPEND Qt5_LIBRARIES ${ICLQt5WidgetsLibAbs})
    # LIST(APPEND Qt5_LIBRARIES ${ICLQt5MultimediaLibAbs})

    ADD_ICL_DEFINITIONS(-DICL_HAVE_QT)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_OPENGL)
    ADD_ICL_DEFINITIONS(-DQWT_DLL)
  ENDIF()
ENDIF()

IF(BUILD_WITH_LIBFREENECT)
  ICL_FIND_PACKAGE(NAME LibFreenect
                   HEADERS "libfreenect.h"
                   LIBS "freenect;freenect_sync"
                   HEADER_PATH_SUFFIXES "include/libfreenect")
ENDIF()

IF(BUILD_WITH_LIBFREENECT2)
  IF(LIBFREENECT2_ROOT)
    SET(LIBFREENECT2_GLFW3_ROOT ${LIBFREENECT2_ROOT})
    SET(LIBFREENECT2_LIBUSB_ROOT ${LIBFREENECT2_ROOT})
  ENDIF()

  ICL_FIND_PACKAGE(NAME LibFreenect2
                   HEADERS "libfreenect2/opengl.h;libfreenect2/libfreenect2.hpp;libfreenect2/frame_listener_impl.h"
                   LIBS "freenect2"
                   HEADER_PATH_SUFFIXES "examples/protonect/include"
                   LIB_PATH_SUFFIXES "examples/protonect/lib"
                   OPTIONAL)

  ICL_FIND_PACKAGE(NAME LibFreenect2_GLFW3
                   HEADERS "GLFW/glfw3.h"
                   LIBS "glfw"
                   HEADER_PATH_SUFFIXES "depends/glfw/include"
                   LIB_PATH_SUFFIXES "depends/glfw/lib"
                   OPTIONAL)

  ICL_FIND_PACKAGE(NAME LibFreenect2_LIBUSB
                   HEADERS "libusb-1.0/libusb.h"
                   LIBS "usb-1.0"
                   HEADER_PATH_SUFFIXES "depends/libusb/include"
                   LIB_PATH_SUFFIXES "depends/libusb/lib"
                   OPTIONAL)


  IF(LIBFREENECT2_FOUND AND LIBFREENECT2_GLFW3_FOUND AND LIBFREENECT2_LIBUSB_FOUND)
    LIST(APPEND LIBFREENECT2_LIBRARIES ${LIBFREENECT2_GLFW3_LIBRARIES} ${LIBFREENECT2_LIBUSB_LIBRARIES})
    LIST(APPEND LIBFREENECT2_INCLUDE_DIRS ${LIBFREENECT2_GLFW3_INCLUDE_DIRS} ${LIBFREENECT2_LIBUSB_INCLUDE_DIRS})
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBFREENECT2)
  ELSE()
     IF(BUILD_WITH_LIBFREENECT2_OPTIONAL)
       MESSAGE(STATUS "Could NOT find all dependencies for OPTIONAL LIBFREENECT2 (continuing...)")
     ELSE()
       MESSAGE(SEND_ERROR "Could NOT find all dependencies for LIBFREENECT2")
    ENDIF()
  ENDIF()
ENDIF()


IF(BUILD_WITH_PYLON)
  FIND_PACKAGE(Pylon REQUIRED)
  IF(PYLON_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_PYLON)
  ENDIF()
ENDIF()

IF(BUILD_WITH_PCL)
  IF(BUILD_WITH_PCL_OPTIONAL)
    FIND_PACKAGE(PCL 1.6 COMPONENTS kdtree common octree search QUIET)
  ELSE()
    FIND_PACKAGE(PCL 1.6 REQUIRED COMPONENTS kdtree common octree search QUIET)
  ENDIF()
  # io requires openni aswell !!
  # FIND_PACKAGE(PCL REQUIRED) ??
  IF(PCL_FOUND)
    MESSAGE(STATUS "Found PCL: ${PCL_INCLUDE_DIRS} ${PCL_LIBRARIES}")
    # workaround aroud issue that PCL_LIBRARIES contains
    # tokens optimized pthread and debug
    SET(PCL_LIBRARIES_PC ${PCL_LIBRARIES})
    LIST(REMOVE_DUPLICATES PCL_LIBRARIES_PC)
    LIST(REMOVE_ITEM PCL_LIBRARIES_PC debug)
    LIST(REMOVE_ITEM PCL_LIBRARIES_PC optimized)
    LIST(REMOVE_ITEM PCL_LIBRARIES_PC pthread)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_PCL)
  ELSE()
    IF(BUILD_WITH_PCL_OPTIONAL)
      MESSAGE(STATUS "Could NOT find OPTIONAL PCL (continuing...)")
    ELSE()
      MESSAGE(SEND_ERROR "Could NOT find PCL")
    ENDIF()
  ENDIF()
ENDIF()

# Why does it have to be before the RSB stuff?
IF(BUILD_WITH_RST)
  IF(NOT BUILD_WITH_PROTOBUF)
    MESSAGE(FATAL_ERROR "Option BUILD_WITH_RST was defined, but BUILD_WITH_PROTOBUF was not defined (these options belong together, so either specify both or none of them)")
  ENDIF()
  IF(NOT BUILD_WITH_RSC)
    MESSAGE(FATAL_ERROR "Option BUILD_WITH_RST was defined, but BUILD_WITH_RSC was not defined (these options belong together, so either specify both or none of them)")
  ENDIF()

  IF(RST_ROOT)
    FIND_PACKAGE(RST COMPONENTS sandbox PATHS ${RST_ROOT} NO_DEFAULT_PATH QUIET)
  ELSE()
    FIND_PACKAGE(RST COMPONENTS sandbox QUIET)
  ENDIF()

  IF(RST_FOUND)
    set(HAVE_ALL_PROTOS TRUE)
    foreach(proto rst/math/Vec3DFloat.proto rst/geometry/PolygonalPatch3D.proto)
      LIST(FIND STABLE_PROTOS_REL proto/stable/${proto} _HAVE_STABLE_PROTO)
      LIST(FIND SANDBOX_PROTOS_REL proto/sandbox/${proto} _HAVE_SANDBOX_PROTO)
      if((_HAVE_STABLE_PROTO LESS 0) AND (_HAVE_SANDBOX_PROTO LESS 0))
        LIST(APPEND RST_MISSING ${proto})
        set(HAVE_ALL_PROTOS FALSE)
      endif()
    endforeach()
  else()
    SET(RST_MISSING "RST package")
  endif()

  IF(RST_FOUND AND HAVE_ALL_PROTOS AND BUILD_WITH_RSC)
    MESSAGE(STATUS "Found RST: ${RST_INCLUDE_DIRS} ${RST_LIBRARIES}")

    IF(RSC_ROOT)
      FIND_PACKAGE(RSC PATHS ${RSC_ROOT} NO_DEFAULT_PATH QUIET)
    ELSE()
      FIND_PACKAGE(RSC PATHS QUIET)
    ENDIF()


    IF(RSC_FOUND)
      ADD_ICL_DEFINITIONS(-DICL_HAVE_RST ${RST_CFLAGS})
      ADD_ICL_DEFINITIONS(-DICL_HAVE_RSC ${RSC_CFLAGS})
    ELSE()
      IF(BUILD_WITH_RSC_OPTIONAL)
        MESSAGE(STATUS "Could NOT find OPTIONAL RSC (missing:  continuing...)")
      ELSE()
        MESSAGE(SEND_ERROR "Could NOT find RST")
      ENDIF()
    ENDIF()

  ELSE()
    SET(RSC_FOUND FALSE)
    SET(RST_FOUND FALSE)
    IF(BUILD_WITH_RST_OPTIONAL)
      MESSAGE(STATUS "Could NOT find OPTIONAL RST (missing: ${RST_MISSING}, continuing...)")
    ELSE()
      MESSAGE(SEND_ERROR "Could NOT find RST (missing: ${RST_MISSING})")
    ENDIF()
  ENDIF()
ENDIF()

MESSAGE(STATUS ${RST_LIBRARIES_PC})

IF(BUILD_WITH_RSB)
  IF(NOT BUILD_WITH_PROTOBUF)
    MESSAGE(FATAL_ERROR "Option BUILD_WITH_RSB was defined, but BUILD_WITH_PROTOBUF was not defined (these options belong together, so either specify both or none of them)")
  ENDIF()
  FIND_PACKAGE(RSB REQUIRED HINTS ${RSB_ROOT} QUIET)
  IF(WIN32)
    FIND_PACKAGE(WinProtobuf REQUIRED)
  ELSE()
    IF(PROTOBUF_ROOT)
      FIND_PACKAGE(Protobuf REQUIRED HINTS ${PROTOBUF_ROOT} QUIET)
    ELSE()
      FIND_PACKAGE(Protobuf REQUIRED QUIET)
    ENDIF()
  ENDIF()

  IF(RSB_FOUND AND PROTOBUF_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_RSB)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_PROTOBUF)

    LIST(APPEND RSB_RSC_AND_PROTOBUF_LIBRARIES_PC ${RSB_LIBRARIES})
    LIST(APPEND RSB_RSC_AND_PROTOBUF_LIBRARIES_PC ${PROTOBUF_LIBRARIES})
    IF(WIN32)
      LIST(APPEND RSB_RSC_AND_PROTOBUF_LIBRARIES_PC ${BOOST_LIBRARIES})
    ENDIF(WIN32)
    LIST(REMOVE_DUPLICATES RSB_RSC_AND_PROTOBUF_LIBRARIES_PC)
    LIST(REMOVE_ITEM RSB_RSC_AND_PROTOBUF_LIBRARIES_PC debug)
    LIST(REMOVE_ITEM RSB_RSC_AND_PROTOBUF_LIBRARIES_PC optimized)
    LIST(REMOVE_ITEM RSB_RSC_AND_PROTOBUF_LIBRARIES_PC pthread)
    LIST(REMOVE_ITEM RSB_LIBRARIES pthread)
    LIST(REMOVE_ITEM RSB_LIBRARIES optimized)
    LIST(REMOVE_ITEM RSB_LIBRARIES debug)
    LIST(REMOVE_ITEM PROTOBUF_LIBRARIES pthread)
    LIST(REMOVE_ITEM PROTOBUF_LIBRARIES optimized)
    LIST(REMOVE_ITEM PROTOBUF_LIBRARIES debug)
    MESSAGE(STATUS "Found RSB: ${RSB_INCLUDE_DIRS} ${RSB_LIBRARIES}")
    MESSAGE(STATUS "Found PROTOBUF: ${PROTOBUF_INCLUDE_DIRS} ${PROTOBUF_LIBRARIES}")
  ELSE()
    IF(NOT RSB_FOUND)
      IF(BUILD_WITH_RSB_OPTIONAL)
        MESSAGE(STATUS "Could NOT find OPTIONAL RSB (continuing...)")
      ELSE()
        MESSAGE(SEND_ERROR "Could NOT find RSB")
      ENDIF()
    ELSE()
      IF(NOT PROTOBUF_FOUND)
        MESSAGE(SEND_ERROR "Could NOT find PROTOBUF")
        IF(BUILD_WITH_PROTOBUF_OPTIONAL)
          IF(BUILD_WITH_RSB_OPTIONAL)
            MESSAGE(STATUS "Could NOT find OPTIONAL PROTOBUF (continuing...)")
          ELSE()
            MESSAGE(SEND_ERROR "Defining BUILD_WITH_PROTOBUF_OPTIONAL=TRUE is not possible as RSB needs PROTOBUF mandatorily (define BUILD_WITH_RSB_OPTIONAL=TRUE aswell to achieve a real optional RSB dependency)")
          ENDIF()
        ENDIF()
      ENDIF()
    ENDIF()
    SET(RSB_FOUND FALSE)
    SET(PROTOBUF_FOUND FALSE)
  ENDIF()
ENDIF()

MESSAGE(STATUS ${RSB_RSC_AND_PROTOBUF_LIBRARIES_PC})

SET(HAVE_SIMDS "")
FOREACH(EXT SSE2 SSE3 SSSE3)
  IF(${EXT}_FOUND)
    IF(DISALBE_${EXT})
      message(STATUS "${EXT} found but disabled")
    ELSE()
      message(STATUS "${EXT} found and enabled")
      ADD_ICL_DEFINITIONS(-DICL_USE_${EXT})
      LIST(APPEND HAVE_SIMDS ${EXT})
    ENDIF()
  ELSE()
    message(STATUS "${EXT} not found")
  ENDIF()
ENDFOREACH()

IF(BUILD_WITH_BULLET)
  #SET(BULLET_ROOT BULLET_ROOT CACHE PATH "Root directory BULLET")
  IF(BUILD_WITH_BULLET_OPTIONAL)
    FIND_PACKAGE(Bullet3 QUIET)
    SET(BULLET_FOUND ${BULLET3_FOUND})
    IF(BULLET_FOUND)
      ADD_ICL_DEFINITIONS(-DICL_HAVE_BULLET)
    ELSE()
      MESSAGE(STATUS "Could NOT find OPTIONAL BULLET (missing: libs and/or headers, continuing...)")
      SET(BUILD_WITH_BULLET FALSE)
      SET(BULLET_INCLUDE_DIRS "")
      SET(BULLET_LIBRARIES "")
      SET(BULLET_ROOT_DIR "")
    ENDIF()
  ELSE()
    FIND_PACKAGE(Bullet3 REQUIRED)
    SET(BULLET_FOUND ${BULLET3_FOUND})
    IF(BULLET_FOUND)
      ADD_ICL_DEFINITIONS(-DICL_HAVE_BULLET)
    ENDIF()
  ENDIF()
  IF(BULLET_FOUND)
  FOREACH(L ${BULLET_LIBRARIES})
    IF(NOT EXISTS ${L})
      LIST( APPEND BULLET_LIBS_NEW "${BULLET_ROOT_DIR}/${BULLET_LIBRARY_DIRS}/lib${L}${CMAKE_FIND_LIBRARY_SUFFIXES}" )
    ELSE()
      LIST( APPEND BULLET_LIBS_NEW "${L}")
    ENDIF()
  ENDFOREACH()
  SET(BULLET_LIBRARIES ${BULLET_LIBS_NEW})
  MESSAGE(STATUS "Found Bullet: \n INCLUDES: ${BULLET_INCLUDE_DIRS} \n LIBS: ${BULLET_LIBRARIES}")
  ENDIF(BULLET_FOUND)
ENDIF()


IF(BUILD_WITH_LIBAV)
  ICL_FIND_PACKAGE(NAME libav
                   HEADERS "libavcodec/avcodec.h;libavformat/avformat.h;libswscale/swscale.h"
                   LIBS "avcodec;avformat;avutil;avresample;swscale;avdevice;"
                   HEADER_PATH_SUFFIXES "include/avcodec;include/avformat;include/avdevice;include/avfilter;include/avutil;include/avresample;include/swscale")
  IF(LIBAV_FOUND)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_LIBAV)
  ENDIF()
ENDIF()

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/ICLUtils/src/ICLConfig.h.in ${CMAKE_BINARY_DIR}/src/ICLUtils/ICLConfig.h)

# ---- OS system include directories ----
IF(WIN32)
INCLUDE_DIRECTORIES(BEFORE SYSTEM ${ZLIB_INCLUDE_DIRS}
                                  ${REGEX_ICL_INCLUDE_DIRS}
                                  ${JPEG_INCLUDE_DIR}
                                  ${PTHREADS_INCLUDE_DIRS}
                                  ${PNG_INCLUDE_DIRS}
                                  ${ImageMagick_INCLUDE_DIRS})
ELSE()
INCLUDE_DIRECTORIES(BEFORE SYSTEM ${ZLIB_INCLUDE_DIRS}
                                  ${JPEG_INCLUDE_DIR}
                                  ${PNG_INCLUDE_DIRS}
                                  ${ImageMagick_INCLUDE_DIRS})
ENDIF()

# ---- 3rd party include directories ----
INCLUDE_DIRECTORIES(${IPP_INCLUDE_DIRS}
                    ${MKL_INCLUDE_DIRS}
                    ${OpenCL_INCLUDE_DIRS}
                    ${EIGEN3_INCLUDE_DIRS}
                    ${OpenCV_INCLUDE_DIRS}
                    ${MESASR_INCLUDE_DIRS}
                    ${LIBAV_INCLUDE_DIRS}
                    ${LIBDC_INCLUDE_DIRS}
                    ${V4L_INCLUDE_DIRS}
                    ${XINE_INCLUDE_DIRS}
                    ${OPENGL_INCLUDE_DIR}
                    ${GLEW_INCLUDE_DIRS}
                    ${LIBFREENECT_INCLUDE_DIRS}
                    ${LIBFREENECT2_INCLUDE_DIRS}
                    ${PYLON_INCLUDE_DIRS}
                    ${PCL_INCLUDE_DIRS}
                    ${OPENNI_INCLUDE_DIRS}
                    ${RSB_INCLUDE_DIRS}
                    ${RSC_INCLUDE_DIRS}
                    ${BOOST_INCLUDE_DIR}
                    ${PROTOBUF_INCLUDE_DIRS}
                    ${ZMQ_INCLUDE_DIRS}
                    ${LIBUSB_INCLUDE_DIRS}
                    ${Qt5Multimedia_INCLUDE_DIRS}
                    ${Qt5Widgets_INCLUDE_DIRS}
                    ${Qt5OpenGL_INCLUDE_DIRS}
                    ${BULLET_INCLUDE_DIRS}
                    ${RST_INCLUDE_DIRS}
                    ${XIAPI_INCLUDE_DIRS}
                    ${LIBIRIMAGER_INCLUDE_DIRS})

# ---- Variables for 3rd party libraries ----
LIST(REMOVE_DUPLICATES REQUIRED_LIBRARIES_LIST)

LIST(APPEND ICLUtils_3RDPARTY_LIBRARIES ${REQUIRED_LIBRARIES_LIST})
LIST(APPEND ICLUtils_3RDPARTY_LIBRARIES ${IPP_LIBRARIES})
LIST(APPEND ICLUtils_3RDPARTY_LIBRARIES ${OpenCL_LIBRARIES})
LIST(APPEND ICLUtils_3RDPARTY_LIBRARIES ${DL_LIBRARIES})

LIST(APPEND ICLMath_3RDPARTY_LIBRARIES ${MKL_LIBRARIES})

LIST(APPEND ICLCore_3RDPARTY_LIBRARIES ${OpenCV_LIBRARIES})

LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${MESASR_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${LIBAV_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${ImageMagick_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${LIBDC_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${V4L_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${XINE_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${LIBUSB_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${OPENGL_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${XIAPI_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${LIBIRIMAGER_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${LIBUDEV_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${LIBFREENECT_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${LIBFREENECT2_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${PYLON_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${OPENNI_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${RSB_RSC_AND_PROTOBUF_LIBRARIES_PC})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${RSC_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${RST_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${BOOST_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${ZMQ_LIBRARIES})
LIST(APPEND ICLIO_3RDPARTY_LIBRARIES ${Qt5_LIBRARIES})


LIST(APPEND ICLQt_3RDPARTY_LIBRARIES ${GLEW_LIBRARIES})
LIST(APPEND ICLQt_3RDPARTY_LIBRARIES ${Qt5_LIBRARIES})

LIST(APPEND ICLGeom_3RDPARTY_LIBRARIES ${PCL_LIBRARIES_PC})
LIST(APPEND ICLGeom_3RDPARTY_LIBRARIES ${RSB_RSC_AND_PROTOBUF_LIBRARIES_PC})
LIST(APPEND ICLGeom_3RDPARTY_LIBRARIES ${RST_LIBRARIES})

LIST(APPEND ICLPhysics_3RDPARTY_LIBRARIES ${BULLET_LIBRARIES})

#---- Add src sub directories ----
IF(WIN32)
  ADD_SUBDIRECTORY(3rdparty)
ENDIF()
ADD_SUBDIRECTORY(ICLUtils)
ADD_SUBDIRECTORY(ICLMath)
ADD_SUBDIRECTORY(ICLCore)
ADD_SUBDIRECTORY(ICLFilter)
ADD_SUBDIRECTORY(ICLIO)
ADD_SUBDIRECTORY(ICLCV)
IF(QT_FOUND)
  ADD_SUBDIRECTORY(ICLQt)
ENDIF()
ADD_SUBDIRECTORY(ICLGeom)
ADD_SUBDIRECTORY(ICLMarkers)
IF(BULLET_FOUND AND QT_FOUND)
  ADD_SUBDIRECTORY(ICLPhysics)
ENDIF()

#---- Add documentation ----
ADD_SUBDIRECTORY(doc)

#---- Add cmake modules ----
ADD_SUBDIRECTORY(cmake)

# ---- pkgconfig file ----
# GET_PROPERTY(ALL_INLUDE_DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
# GET_PROPERTY(ALL_LIBS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY CMAKE_STANDARD_LIBRARIES)
# message(STATUS "Jihaaaaaaaaaaaaaaaaaaaaaa: ${ALL_INCLUDE_DIRS}")
# message(STATUS "Jihaaaaaaaaaaaaaaaaaaaaaa: ${ALL_LIBS}")

# LIST(REMOVE_DUPLICATES ALL_INCLUDE_DIRS)
# STRING(REPLACE ";" " -I" # first -I is in icl.pc.in
#        ICL_INCLUDES_BLANK
#        "${ALL_INCLUDE_DIRS}")

# STRING(REPLACE ";" " "
#        ICL_DEFINITIONS_BLANK
#        "${ICL_DEFINITIONS}")
# CONFIGURE_FILE(icl.pc.in icl.pc @ONLY)
# MESSAGE(STATUS "Package-config file will be installed to lib/pkgconfig/${ICL_NAME}-${SO_VERSION}.pc")
# INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/icl.pc
#         DESTINATION lib/pkgconfig
#         RENAME      "${PKG_CONFIG_FILE_NAME}")

# ---- icl-make, icl-version and icl-install-project ----
SET(PKG_CONFIG_FILE_NAME_NO_PC "ICL-${SO_VERSION}")

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/src/ICLUtils/ICLConfig.h
        DESTINATION include/${INSTALL_PATH_PREFIX}/ICLUtils
        COMPONENT development)

IF(NOT WIN32)
  FOREACH(FILE icl-make icl-create-project icl-create-cmake-project icl-version)
    SET(SRC scripts/${FILE}.in)
    SET(DST ${CMAKE_CURRENT_BINARY_DIR}/scripts/${FILE})
    CONFIGURE_FILE(${SRC} ${DST} @ONLY)
    INSTALL(FILES ${DST}
            PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            DESTINATION bin/
            COMPONENT development)
  ENDFOREACH()
ENDIF()

FILE(COPY scripts/icl-create-video.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/scripts)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-video.sh
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DESTINATION bin/
        COMPONENT development)

CONFIGURE_FILE(scripts/icl-create-project-data/Makefile.custom.in
               ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data/Makefile.custom @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data/Makefile.custom
        DESTINATION ${RESOURCE_DIR}/ RENAME Makefile.custom.template
        COMPONENT development)

FOREACH(FILE Makefile doxyfile sample-test.cpp)
  SET(SRC scripts/icl-create-project-data/${FILE})
  SET(DST ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data)
  FILE(COPY ${SRC} DESTINATION ${DST})
  INSTALL(FILES ${DST}/${FILE} DESTINATION ${RESOURCE_DIR} RENAME ${FILE}.template)
ENDFOREACH()

FOREACH(FILE app.CMakeLists.txt CMakeLists.txt latex.clear.sh
             latex.report.plots.sizevarplot.tex latex.talk.talk.tex
             src.HelloWorld.cpp app.main.cpp Doxyfile.in latex.library.bib
             latex.report.report.tex LICENSE.LGPL  src.HelloWorld.h
             latex.build.sh latex.quick.sh
             latex.report.sections.introduction.tex src.CMakeLists.txt)
  SET(SRC scripts/icl-create-cmake-project-data/${FILE})
  SET(DST ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-cmake-project-data)
  FILE(COPY ${SRC} DESTINATION ${DST})
  INSTALL(FILES ${DST}/${FILE} DESTINATION ${RESOURCE_DIR} RENAME ${FILE}.template COMPONENT development)
ENDFOREACH()

# ---- PKG-config ----
IF(BULLET_FOUND)
  CREATE_PKGCONFIG(NAME ICL PKGCONFIG_DEPS ICLPhysics-${SO_VERSION} ICLMarkers-${SO_VERSION})
ELSE()
  CREATE_PKGCONFIG(NAME ICL PKGCONFIG_DEPS ICLMarkers-${SO_VERSION} )
ENDIF()


# ----- Summary of all dependencies
SET(OPTS OPENCV QT OPENGL RSB RST RSC PROTOBUF EIGEN3 IPP MKL MESASR IMAGEMAGICK LIBAV LIBDC LIBFREENECT LIBFREENECT2 XINE
  V4L LIBUSB ZMQ PYLON OPENCL OPENNI PCL BULLET XIAPI LIBIRIMAGER LIBUDEV)
IF(NOT WIN32)
  LIST(APPEND OPTS DL)
ENDIF()
SET(BUILD EXAMPLES DEMOS APPS)

IF(NOT HAVE_SIMDS)
  SET(HAVE_SIMDS "(none")
ENDIF()
SET(DO_BUILD "LIBS")
FOREACH(B APPS DEMOS EXAMPLES)
  IF(BUILD_${B})
    LIST(APPEND DO_BUILD ${B})
  ENDIF()
ENDFOREACH()

SET(BUILD_MODULES  Utils Math Core Filter IO CV Geom Markers)
IF(QT_FOUND)
  LIST(APPEND BUILD_MODULES Qt)
ENDIF()
IF(BULLET_FOUND)
  LIST(APPEND BUILD_MODULES Physics)
ENDIF()

MESSAGE(" ------------ Build Options: --------------------------------------")
MESSAGE(" Build Type: .............. ${CMAKE_BUILD_TYPE}")
MESSAGE(" SIMD Extensions: ......... ${HAVE_SIMDS}")
MESSAGE(" Build Parts: ............. ${DO_BUILD}")
MESSAGE(" Build Modules: ........... ${BUILD_MODULES}")
MESSAGE(" ------------ Dependencies: ---------------------------------------")
FOREACH(O ${OPTS})
  STRING(LENGTH ${O} L)
  MATH(EXPR L "17-${L}")
  SET(FILL "")
  WHILE(${L} GREATER 0)
    SET(FILL ".${FILL}")
    MATH(EXPR L "${L}-1")
  ENDWHILE()

  IF(${O}_FOUND)
    SET(WHERE "--")
    IF(${O}_DIR)
      SET(WHERE "${${O}_DIR}")
    ELSEIF(${O}_LIBRARIES)
      LIST(GET ${O}_LIBRARIES 0 E1)
      GET_FILENAME_COMPONENT(WHERE ${E1} ${ICL_FILE_NAME_COMPONENT_DIR})
    ELSEIF(${O}_INCLUDE_DIRS)
      LIST(GET ${O}_INCLUDE_DIRS 0 WHERE)
    ENDIF()
    MESSAGE(" ${O} ${FILL} yes ... ${WHERE}")
  ELSE()
    IF(BUILD_WITH_${O})
      IF(BUILD_WITH_${O}_OPTIONAL)
        MESSAGE(" ${O} ${FILL} no ....  - optional -   ")
      ELSE()
        MESSAGE(" ${O} ${FILL} no .... - missing! -   ")
      ENDIF()
    ELSE()
        MESSAGE(" ${O} ${FILL} no .... - unselected - ")
    ENDIF()
  ENDIF()
ENDFOREACH()
MESSAGE(" ------------ Grabber Backends: -----------------------------------")

FOREACH(G Generic_cameras|OPENCV|QT
          Video_files|OPENCV|XINE|QT
          Shared_memory|QT
          Network|RSB|ZMQ
          Kinect_1|LIBFREENECT|OPENNI
          Kinect_2|LIBFREENECT2
          Gig-E_cameras|PYLON
          Swiss-Ranger_cameras|MESASR
          Fire-Wire_cameras|LIBDC
          USB_cameras|V4L
          Ximea_cameras|XIAPI
          Optris_IR-cameras|LIBIRIMAGER
          Image_files|IMAGEMAGICK|PNG|JPEG)
  STRING(REPLACE "|" ";" G ${G})
  LIST(GET G 0 WHAT)
  LIST(REMOVE_AT G 0)
  STRING(REPLACE "_" " " WHAT ${WHAT})
  STRING(LENGTH ${WHAT} L)
  MATH(EXPR L "25-${L}")
  SET(FILL "")
  WHILE(${L} GREATER 0)
    SET(FILL ".${FILL}")
    MATH(EXPR L "${L}-1")
  ENDWHILE()
  SET(HAVE "")
  FOREACH(D ${G})
    IF(${D}_FOUND)
      LIST(APPEND HAVE ${D})
    ENDIF()
  ENDFOREACH()
  IF(NOT HAVE)
    SET(HAVE "(none)")
  ENDIF()
  MESSAGE(" ${WHAT} ${FILL} ${HAVE}")
ENDFOREACH()
