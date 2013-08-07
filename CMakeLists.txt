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
#** Authors: Michael Goetting, Christof Elbrechter                  **
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
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

PROJECT(ICL)

# ---- cmake settings ----
INCLUDE(ICLHelperMacros)
INCLUDE(CheckArchitecture)
INCLUDE(InstallFilesRecursive)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# ---- build type (debug/release) and opptions
OPTION(ENABLE_NATIVE_BUILD_OPTION "Decide whether to use -march=native optimization flag" OFF)
OPTION(ENABLE_SSEFPMATH_BUILD_OPTION "Decide whether to use -mfpmath=sse optimization flag" OFF)
OPTION(ENABLE_OPENMP_BUILD_OPTION "Decide whether to use -fopenmp optimization flag" OFF)
OPTION(ENABLE_FASTMATH_BUILD_OPTION "Decide whether to use -ffast-math optimization flag" OFF)

if(CMAKE_CXX_COMPILER MATCHES ".*clang\\+\\+$") # little ugly here
  set(ICL_DEACTIVATED_WARNING -Wno-internal-linkage-in-inline 
                              -Werror -Wno-\\#warnings 
                              -Wno-deprecated-declarations 
                              -Wno-uninitialized 
                              -Wno-overloaded-virtual)
else()
  set(ICL_DEACTIVATED_WARNING "")
endif()

IF(NOT CMAKE_BUILD_TYPE)
  MESSAGE(STATUS "No Specific build type specified: using Release")
  SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

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

# ---- global definitions ----
# todo: add "-debug" postfix here and there!
SET(ICL_VERSION_MAJOR "8" CACHE STRING "Major project version part")
SET(ICL_VERSION_MINOR "4" CACHE STRING "Minor project version part")
SET(ICL_VERSION_PATCH "0" CACHE STRING "Patch project version part")


SET(ICL_NAME                   icl)
SET(PROJECT_VERSION            "${ICL_VERSION_MAJOR}.${ICL_VERSION_MINOR}.${ICL_VERSION_PATCH}")
SET(SO_VERSION                 "${ICL_VERSION_MAJOR}.${ICL_VERSION_MINOR}")
SET(INSTALL_PATH_PREFIX        "${ICL_NAME}-${SO_VERSION}")
SET(BINARY_PREFIX              "${ICL_NAME}-")
SET(RESOURCE_DIR               "etc/${INSTALL_PATH_PREFIX}")

SET(ICL_CMAKE_MODULE_PATH "share/${INSTALL_PATH_PREFIX}/cmake/Modules")

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/ICLUtils/src/ICLVersion.h.in ${CMAKE_BINARY_DIR}/src/ICLUtils/ICLVersion.h)

# ---- Check Architecture ----
CURRENT_ARCHITECTURE(ICL_BUILD_ARCH)

IF(${ICL_BUILD_ARCH} EQUAL "64_BIT_FOUND")
  SET(ICL_64BIT TRUE)
  ADD_ICL_DEFINITIONS(-DICL_64BIT)
  #SET_PROPERTY(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)
ELSE()
  SET(ICL_32BIT TRUE)
  ADD_ICL_DEFINITIONS(-DICL_32BIT)
ENDIF()

# --- global compiler flags ---
IF(UNIX)
  ADD_ICL_DEFINITIONS(-DICL_SYSTEM_LINUX)
ELSEIF(APPLE)
  ADD_ICL_DEFINITIONS(-DICL_SYSTEM_APPLE)
ELSEIF(WIN32)
  ADD_ICL_DEFINITIONS(-DICL_SYSTEM_WINDOWS)
ENDIF()

# ---- User options ----
OPTION(BUILD_WITH_IPP "Link against Intel IPP for general speedup" OFF)
OPTION(BUILD_WITH_MKL "Link against Intel MKL to speed up several mathematic fuctions" OFF)
OPTION(BUILD_WITH_OPENCV "Link against OpenCL and provide OpenCV image convertion functions" OFF)
OPTION(BUILD_WITH_MESASR "Link against libmesasr to provide SwissRanger camera support" OFF)
OPTION(BUILD_WITH_IMAGEMAGICK "Use ImageMagick to provide I/O for many common image formats" OFF)
OPTION(BUILD_WITH_LIBDC "Use libdc to provide FireWire-Camera support" OFF)
OPTION(BUILD_WITH_LIBFREENECT "Use libfreenect to grab from Kinect Devices" OFF)
OPTION(BUILD_WITH_XINE "Provide a libxine base Video-Grabber backend" OFF)
OPTION(BUILD_WITH_QT "Provide GUI and image visualization framework (needs OPENGL and GLX)" OFF)
OPTION(BUILD_WITH_OPENGL "Linked to BUILD_WITH_QT" OFF)
OPTION(BUILD_WITH_GLX "Linked to BUILD_WITH_QT" OFF)
OPTION(BUILD_WITH_V4L "Provide V4L2 grabber backend for USB-Cameras" OFF)
OPTION(BUILD_WITH_ZMQ "Provide Zero-MQ-based network grabber and image output" OFF)
OPTION(BUILD_WITH_PYLON "Link against the basler Pylon package to support GIG-E cameras" OFF)
OPTION(BUILD_WITH_OPENCL "Uses OpenCL to speed up several algorithms" OFF)
OPTION(BUILD_WITH_OPENNI "Links against OpenNI to grab from Kinect devices" OFF)
OPTION(BUILD_WITH_PCL "Link against PCL to provide a compatibility layer to PCL point clouds" OFF)
OPTION(BUILD_WITH_RSB "Link against the RSB framework for network-based image transfer (needs PROTOBUF)" OFF)
OPTION(BUILD_WITH_PROTOBUF "Needed for RSB-based image transfer" OFF)
OPTION(BUILD_WITH_SSE "Decide if SSE intrinsics shall be used to speed up many support functions" ON)

OPTION(BUILD_EXAMPLES "Decide if examples shall be build or not" OFF)
OPTION(BUILD_DEMOS "Decide if demos shall be build or not" OFF)
OPTION(BUILD_APPS "Decide if applications shall be build or not" OFF)

# ---- Dependency handling ----
FIND_PACKAGE(Threads REQUIRED)
IF(Threads_FOUND)
  LIST(APPEND REQUIRED_LIBRARIES_LIST ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

FIND_PACKAGE(ZLIB REQUIRED)
IF(ZLIB_FOUND)
  ADD_ICL_DEFINITIONS(-DHAVE_LIBZ)
  LIST(APPEND REQUIRED_LIBRARIES_LIST ${ZLIB_LIBRARIES})
ENDIF()

FIND_PACKAGE(JPEG REQUIRED)
IF(JPEG_FOUND)
  ADD_ICL_DEFINITIONS(-DHAVE_LIBJPEG)
  LIST(APPEND REQUIRED_LIBRARIES_LIST ${JPEG_LIBRARIES})
ENDIF()

FIND_PACKAGE(PNG REQUIRED)
IF(PNG_FOUND)
  ADD_ICL_DEFINITIONS(-DHAVE_LIBPNG)
  LIST(APPEND REQUIRED_LIBRARIES_LIST ${PNG_LIBRARIES})
ENDIF()

IF(BUILD_WITH_IPP)
  FIND_PACKAGE(IPP REQUIRED)
  IF(IPP_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_IPP)
  ENDIF()
ENDIF()

IF(BUILD_WITH_MKL)
  FIND_PACKAGE(MKL REQUIRED)
  IF(MKL_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_MKL)
  ENDIF()
ENDIF()

IF(BUILD_WITH_OPENCL)
  FIND_PACKAGE(OpenCL REQUIRED)
  IF(OPENCL_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_OPENCL)# -DCL_USE_DEPRECATED_OPENCL_1_1_APIS)
  ENDIF()
ENDIF()

IF(BUILD_WITH_EIGEN3)
  FIND_PACKAGE(Eigen3 REQUIRED)
  IF(EIGEN3_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_EIGEN3)
  ENDIF()
ENDIF()

IF(BUILD_WITH_OPENNI)
  FIND_PACKAGE(OpenNI)
  IF(OpenNI_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_OPENNI)
  ENDIF()
ENDIF()

IF(BUILD_WITH_OPENCV)
  FIND_PACKAGE(OpenCV)
  IF(OPENCV_FOUND)
    SET(OpenCV_FOUND TRUE)
    ADD_ICL_DEFINITIONS(-DHAVE_OPENCV)
  ENDIF()
ENDIF()

IF(BUILD_WITH_IMAGEMAGICK)
  FIND_PACKAGE(ImageMagick REQUIRED COMPONENTS Magick++)
  IF(ImageMagick_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_IMAGEMAGICK)
  ENDIF()
ENDIF()

IF(BUILD_WITH_MESASR)
  FIND_PACKAGE(MesaSR REQUIRED)
  IF(MesaSR_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_LIBMESAR)
  ENDIF()
ENDIF()

IF(BUILD_WITH_LIBDC)
  FIND_PACKAGE(LibDC REQUIRED)
  IF(LIBDC_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_LIBDC)
  ENDIF()
ENDIF()

IF(BUILD_WITH_V4L)
  FIND_PACKAGE(V4L REQUIRED)
  IF(V4L_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_VIDEODEV)
  ENDIF()
ENDIF()

IF(BUILD_WITH_ZMQ)
  FIND_PACKAGE(ZMQ REQUIRED)
  IF(ZMQ_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_ZMQ)
  ENDIF()
ENDIF()


IF(BUILD_WITH_XINE)
  FIND_PACKAGE(Xine REQUIRED)
  IF(XINE_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_XINE)
  ENDIF()
ENDIF()

IF(BUILD_WITH_QT)
  FIND_PACKAGE(OpenGL REQUIRED)
  FIND_PACKAGE(GLEW REQUIRED)
  FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtOpenGL QtNetwork)
  IF(QT4_FOUND)
    INCLUDE(${QT_USE_FILE})
    ADD_ICL_DEFINITIONS(-DHAVE_QT)
    ADD_ICL_DEFINITIONS(-DHAVE_OPENGL)
    ADD_ICL_DEFINITIONS(-DHAVE_GLX)
  ENDIF()
ENDIF()

IF(BUILD_WITH_LIBFREENECT)
  FIND_PACKAGE(LibFreenect REQUIRED)
  IF(LIBFREENECT_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_LIBFREENECT)
  ENDIF()  
ENDIF()

IF(BUILD_WITH_PYLON)
  FIND_PACKAGE(Pylon REQUIRED)
  IF(PYLON_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_PYLON)
  ENDIF()  
ENDIF()

IF(BUILD_WITH_PCL)
  FIND_PACKAGE(PCL 1.6 REQUIRED COMPONENTS kdtree common octree search)
  # io requires openni aswell !!
  # FIND_PACKAGE(PCL REQUIRED) ?? 
  IF(PCL_FOUND)
    # workaround aroud issue that PCL_LIBRARIES contains
    # tokens optimized pthread and debug
    SET(PCL_LIBRARIES_PC ${PCL_LIBRARIES})
    LIST(REMOVE_DUPLICATES PCL_LIBRARIES_PC)
    LIST(REMOVE_ITEM PCL_LIBRARIES_PC debug)
    LIST(REMOVE_ITEM PCL_LIBRARIES_PC optimized)
    LIST(REMOVE_ITEM PCL_LIBRARIES_PC pthread)
    ADD_ICL_DEFINITIONS(-DHAVE_PCL)
  ENDIF()  
ENDIF()

IF(BUILD_WITH_RSB)
  IF(NOT BUILD_WITH_PROTOBUF)
    MESSAGE(FATAL_ERROR "Option BUILD_WITH_RSB was defined, but BUILD_WITH_PROTOBUF was not defined (these options belong together, so either specify both or none of them)")
  ENDIF()
  FIND_PACKAGE(RSB REQUIRED)
  IF(PROTOBUF_ROOT)
    FIND_PACKAGE(Protobuf REQUIRED HINTS ${PROTOBUF_ROOT})
  ELSE()
    FIND_PACKAGE(Protobuf REQUIRED)
  ENDIF()

  IF(RSB_FOUND AND PROTOBUF_FOUND)
    ADD_ICL_DEFINITIONS(-DHAVE_RSB)
    ADD_ICL_DEFINITIONS(-DHAVE_PROTOBUF)
    LIST(APPEND RSB_RSC_AND_PROTOBUF_LIBRARIES_PC ${RSB_LIBRARIES})
    LIST(APPEND RSB_RSC_AND_PROTOBUF_LIBRARIES_PC ${PROTOBUF_LIBRARIES})
    LIST(REMOVE_DUPLICATES RSB_RSC_AND_PROTOBUF_LIBRARIES_PC)
    LIST(REMOVE_ITEM RSB_RSC_AND_PROTOBUF_LIBRARIES_PC debug)
    LIST(REMOVE_ITEM RSB_RSC_AND_PROTOBUF_LIBRARIES_PC optimized)
    LIST(REMOVE_ITEM RSB_RSC_AND_PROTOBUF_LIBRARIES_PC pthread)
  ENDIF()  
ENDIF()


IF(BUILD_WITH_SSE)
  ADD_ICL_DEFINITIONS(-DUSE_SSE)
ENDIF()

# ---- OS system include directories ----
INCLUDE_DIRECTORIES(BEFORE SYSTEM ${ZLIB_INCLUDE_DIRS}
                                  ${JPEG_INCLUDE_DIR}
				  ${PNG_INCLUDE_DIRS}
                                  ${ImageMagick_INCLUDE_DIRS})

# ---- 3rd party include directories ----
INCLUDE_DIRECTORIES(${IPP_INCLUDE_DIRS}
                    ${MKL_INCLUDE_DIRS}
		    ${OPENCL_INCLUDE_DIRS}
		    ${EIGEN3_INCLUDE_DIRS}
		    ${OpenCV_INCLUDE_DIRS}
		    ${MESASR_INCLUDE_DIRS}
		    ${LIBDC_INCLUDE_DIRS}
		    ${V4L_INCLUDE_DIRS}
		    ${XINE_INCLUDE_DIRS}
		    ${OPENGL_INCLUDE_DIR}
		    ${GLEW_INCLUDE_DIR}
		    ${LIBFREENECT_INCLUDE_DIRS}
		    ${PYLON_INCLUDE_DIRS}
		    ${PCL_INCLUDE_DIRS}
		    ${OPENNI_INCLUDE_DIRS}
		    ${RSB_INCLUDE_DIRS}
		    ${ZMQ_INCLUDE_DIRS}
		    ${PROTOBUF_INCLUDE_DIRS}
		    )

#---- Add src sub directories ----
ADD_SUBDIRECTORY(ICLUtils)
ADD_SUBDIRECTORY(ICLMath)
ADD_SUBDIRECTORY(ICLCore)
ADD_SUBDIRECTORY(ICLFilter)
ADD_SUBDIRECTORY(ICLIO)
ADD_SUBDIRECTORY(ICLCV)
IF(QT4_FOUND)
  ADD_SUBDIRECTORY(ICLQt)
ENDIF()
ADD_SUBDIRECTORY(ICLGeom)
ADD_SUBDIRECTORY(ICLMarkers)

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

FOREACH(FILE icl-make icl-create-project icl-version)
  SET(SRC scripts/${FILE}.in)
  SET(DST ${CMAKE_CURRENT_BINARY_DIR}/scripts/${FILE})
  CONFIGURE_FILE(${SRC} ${DST} @ONLY)
  INSTALL(FILES ${DST}
          PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
          DESTINATION bin/)
ENDFOREACH()

CONFIGURE_FILE(scripts/icl-create-project-data/Makefile.custom.in
               ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data/Makefile.custom @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data/Makefile.custom
        DESTINATION ${RESOURCE_DIR}/ RENAME Makefile.custom.template)

FOREACH(FILE Makefile doxyfile sample-test.cpp)
  SET(SRC scripts/icl-create-project-data/${FILE})
  SET(DST ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data)
  FILE(COPY ${SRC} DESTINATION ${DST})
  INSTALL(FILES ${DST}/${FILE} DESTINATION ${RESOURCE_DIR} RENAME ${FILE}.template)
ENDFOREACH()

# ---- PKG-config ----
CREATE_PKGCONFIG(NAME ICL
                 PKGCONFIG_DEPS ICLMarkers-${SO_VERSION})
