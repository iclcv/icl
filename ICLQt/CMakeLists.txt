file(GLOB SOURCES src/ICLQt/*.cpp)
file(GLOB HEADERS src/ICLQt/*.h)

# Remove dead RSB backend
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLQt/RSBRemoteGUI.cpp)
list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLQt/RSBRemoteGUI.h)

set(ICL_MOC_HEADERS src/ICLQt/ICLVideoSurface.h
                    src/ICLQt/Widget.h
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
