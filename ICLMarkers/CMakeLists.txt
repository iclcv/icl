file(GLOB SOURCES src/ICLMarkers/*.cpp)
file(GLOB HEADERS src/ICLMarkers/*.h)

icl_add_module(ICLMarkers
  SOURCES ${SOURCES}
  HEADERS ${HEADERS}
  ICL_DEPS ICLGeom
  PKGCONFIG_DEPS ICLGeom-${SO_VERSION}
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
    <ICLCore/ImgBase.h>)
