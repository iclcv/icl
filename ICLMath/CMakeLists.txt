file(GLOB SOURCES src/ICLMath/*.cpp)
file(GLOB HEADERS src/ICLMath/*.h)

# Convention: *_<Backend>.cpp files are excluded when their library is not found
if(NOT IPP_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_Ipp\\.cpp$")
endif()
if(NOT MKL_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_Mkl\\.cpp$")
endif()
if(NOT FFTW_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_FFTW\\.cpp$")
endif()
if(NOT ACCELERATE_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_Accelerate\\.cpp$")
endif()
if(NOT OPENBLAS_FOUND)
  list(FILTER SOURCES EXCLUDE REGEX "_OpenBlas\\.cpp$")
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
