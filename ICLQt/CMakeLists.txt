file(GLOB SOURCES src/ICLQt/*.cpp)
file(GLOB HEADERS src/ICLQt/*.h)

# Remove dead RSB backend
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLQt/RSBRemoteGUI.cpp)
list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLQt/RSBRemoteGUI.h)

# Remove Qt Multimedia dependent files if not available
if(NOT Qt6Multimedia_FOUND)
  list(REMOVE_ITEM SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLQt/ICLVideoSurface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLQt/QtCameraGrabber.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLQt/QtVideoGrabber.cpp)
  list(REMOVE_ITEM HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLQt/ICLVideoSurface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLQt/QtCameraGrabber.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLQt/QtVideoGrabber.h)
endif()

set(ICL_MOC_HEADERS src/ICLQt/Widget.h
                    src/ICLQt/MouseHandler.h
                    src/ICLQt/DoubleSlider.h
                    src/ICLQt/ImgParamWidget.h
                    src/ICLQt/StringSignalButton.h
                    src/ICLQt/GUIWidget.h
                    src/ICLQt/ChromaGUI.h
                    src/ICLQt/ThreadedUpdatableSlider.h
                    src/ICLQt/MultiDrawHandle.h
                    src/ICLQt/ToggleButton.h
                    src/ICLQt/Application.h)
if(Qt6Multimedia_FOUND)
  list(APPEND ICL_MOC_HEADERS src/ICLQt/ICLVideoSurface.h)
endif()

icl_add_module(ICLQt
  SOURCES ${SOURCES}
  HEADERS ${HEADERS}
  ICL_DEPS ICLCV
  3RDPARTY_LIBS ${ICLQt_3RDPARTY_LIBRARIES}
  PKGCONFIG_DEPS ICLCV-${SO_VERSION}
  MOC_HEADERS ${ICL_MOC_HEADERS}
  HAS_EXAMPLES HAS_DEMOS HAS_APPS
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
    <QWidget>
    <QString>
    <QObject>
    <QPainter>
    <QOpenGLWidget>)

# GLEW is included before Qt's OpenGL headers (via CompatMacros.h).
# Qt detects this and emits a #warning — suppress it since ICL uses
# GLEW directly and does not use QOpenGLFunctions.
if(TARGET ICLQt)
  target_compile_options(ICLQt PRIVATE -Wno-\#warnings)
endif()
