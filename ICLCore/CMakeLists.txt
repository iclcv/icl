file(GLOB SOURCES src/ICLCore/*.cpp)
file(GLOB HEADERS src/ICLCore/*.h)

# Convention: *_<Backend>.cpp excluded when the backend library is not found.
if(NOT IPP_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_Ipp\\.cpp$")
endif()
if(NOT ACCELERATE_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_Accelerate\\.cpp$")
endif()

if(NOT OpenCV_FOUND)
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCore/OpenCV.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCore/OpenCV.h)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set_source_files_properties(src/ICLCore/OpenCV.cpp PROPERTIES COMPILE_FLAGS -Wno-narrowing)
endif()

icl_add_module(ICLCore
  SOURCES ${SOURCES}
  HEADERS ${HEADERS}
  ICL_DEPS ICLMath
  3RDPARTY_LIBS ${ICLCore_3RDPARTY_LIBRARIES}
  PKGCONFIG_DEPS ICLMath-${SO_VERSION}
  HAS_EXAMPLES HAS_DEMOS
  PCH_HEADERS
    <vector> <string> <map> <algorithm> <functional>
    <iostream> <sstream> <memory> <cmath> <type_traits>
    <ICLUtils/CompatMacros.h>
    <ICLUtils/BasicTypes.h>
    <ICLUtils/Exception.h>
    <ICLUtils/StringUtils.h>
    <ICLUtils/Macros.h>

    <ICLMath/DynMatrix.h>
    <ICLCore/Img.h>
    <ICLCore/ImgBase.h>
    <ICLCore/Types.h>
    <ICLCore/Color.h>)
