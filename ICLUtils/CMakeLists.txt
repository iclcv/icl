file(GLOB SOURCES src/ICLUtils/*.cpp)
file(GLOB HEADERS src/ICLUtils/*.h)

if(NOT DL_FOUND)
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLUtils/IppInterface.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLUtils/IppInterface.h)
endif()

icl_add_module(ICLUtils
  SOURCES ${SOURCES}
  HEADERS ${HEADERS}
  3RDPARTY_LIBS ${ICLUtils_3RDPARTY_LIBRARIES}
  HAS_EXAMPLES HAS_APPS
  PCH_HEADERS
    <vector> <string> <map> <algorithm> <functional>
    <iostream> <sstream> <memory> <cmath> <type_traits>
    <ICLUtils/CompatMacros.h>
    <ICLUtils/BasicTypes.h>
    <ICLUtils/Exception.h>
    <ICLUtils/StringUtils.h>
    <ICLUtils/Macros.h>
    <ICLUtils/SmartPtr.h>)
