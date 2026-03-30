file(GLOB SOURCES src/ICLFilter/*.cpp)
file(GLOB HEADERS src/ICLFilter/*.h)

# OpenCL kernel integration
file(GLOB KERNEL src/ICLFilter/OpenCL/*.cl)
foreach(kernel_file ${KERNEL})
  get_filename_component(kernel_source_name ${kernel_file} NAME_WE)
  string(REGEX REPLACE ".cl$" "Kernel.h" kernel_header_file "${kernel_file}")
  CREATE_CL_HEADER("${kernel_file}" "${kernel_header_file}" "${kernel_source_name}KernelSource" filter)
  list(APPEND HEADERS ${kernel_header_file})
endforeach()

# Removed: UnaryOpWork (replaced by std::async in UnaryOp/NeighborhoodOp)
list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLFilter/UnaryOpWork.h)

# Conditionally exclude backend files when their dependency is unavailable.
# Convention: *_Ipp.cpp = IPP-only, *_OpenCL.cpp = OpenCL-only, *_Simd.cpp = always built.
if(NOT IPP_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_Ipp\\.cpp$")
  # ProximityOp is IPP-only (no C++ fallback):
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLFilter/ProximityOp.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLFilter/ProximityOp.h)
endif()
if(NOT OPENCL_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_OpenCL\\.cpp$")
endif()

icl_add_module(ICLFilter
  SOURCES ${SOURCES}
  HEADERS ${HEADERS}
  ICL_DEPS ICLCore
  EXTRA_SOURCES ${KERNEL}
  PKGCONFIG_DEPS ICLCore-${SO_VERSION}
  HAS_DEMOS HAS_APPS
  PCH_HEADERS
    <vector> <string> <map> <algorithm> <functional>
    <iostream> <sstream> <memory> <cmath> <type_traits>
    <ICLUtils/CompatMacros.h>
    <ICLUtils/BasicTypes.h>
    <ICLUtils/Exception.h>
    <ICLUtils/StringUtils.h>
    <ICLUtils/Macros.h>
    <ICLCore/Img.h>
    <ICLCore/ImgBase.h>
    <ICLFilter/UnaryOp.h>)
