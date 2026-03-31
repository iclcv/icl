file(GLOB SOURCES src/ICLMath/*.cpp)
file(GLOB HEADERS src/ICLMath/*.h)

# Convention: *_Ipp.cpp = IPP-only, *_Mkl.cpp = MKL-only
if(NOT IPP_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_Ipp\\.cpp$")
endif()
if(NOT MKL_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_Mkl\\.cpp$")
endif()

icl_add_module(ICLMath
  SOURCES ${SOURCES}
  HEADERS ${HEADERS}
  ICL_DEPS ICLUtils
  3RDPARTY_LIBS ${ICLMath_3RDPARTY_LIBRARIES}
  PKGCONFIG_DEPS ICLUtils-${SO_VERSION}
  HAS_EXAMPLES HAS_DEMOS
  PCH_HEADERS
    <vector> <string> <map> <algorithm> <functional>
    <iostream> <sstream> <memory> <cmath> <type_traits>
    <ICLUtils/CompatMacros.h>
    <ICLUtils/BasicTypes.h>
    <ICLUtils/Exception.h>
    <ICLUtils/StringUtils.h>
    <ICLUtils/Macros.h>

    <ICLMath/DynMatrix.h>)
