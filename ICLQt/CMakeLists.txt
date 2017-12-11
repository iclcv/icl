#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLQt/CMakeLists.txt                                   **
#** Module : ICLQt                                                  **
#** Authors: Michael Goetting                                       **
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

SET(SOURCES src/ICLQt/AbstractPlotWidget.cpp
            src/ICLQt/Application.cpp
	    src/ICLQt/BorderHandle.cpp
	    src/ICLQt/ButtonGroupHandle.cpp
	    src/ICLQt/ButtonHandle.cpp
	    src/ICLQt/CamCfgWidget.cpp
	    src/ICLQt/CheckBoxHandle.cpp
	    src/ICLQt/ChromaClassifierIO.cpp
	    src/ICLQt/ChromaGUI.cpp
	    src/ICLQt/ColorLabel.cpp
	    src/ICLQt/ComboHandle.cpp
	    src/ICLQt/CompabilityLabel.cpp
	    src/ICLQt/DataStore.cpp
	    src/ICLQt/DefineRectanglesMouseHandler.cpp
	    src/ICLQt/DefineQuadrangleMouseHandler.cpp
	    src/ICLQt/DoubleSlider.cpp
	    src/ICLQt/Dragger.cpp
	    src/ICLQt/DragRectangleMouseHandler.cpp
	    src/ICLQt/DrawHandle.cpp
	    src/ICLQt/DrawHandle3D.cpp
	    src/ICLQt/DrawWidget.cpp
	    src/ICLQt/DrawWidget3D.cpp
	    src/ICLQt/FloatHandle.cpp
	    src/ICLQt/FSliderHandle.cpp
	    src/ICLQt/GLPaintEngine.cpp
            src/ICLQt/GLImg.cpp
	    src/ICLQt/GLFragmentShader.cpp
	    src/ICLQt/GUI.cpp
	    src/ICLQt/DynamicGUI.cpp
	    src/ICLQt/GUIDefinition.cpp
	    src/ICLQt/GUIWidget.cpp
	    src/ICLQt/HistogrammWidget.cpp
	    src/ICLQt/IconFactory.cpp
	    src/ICLQt/ImageHandle.cpp
	    src/ICLQt/ImgParamWidget.cpp
	    src/ICLQt/IntHandle.cpp
	    src/ICLQt/LabelHandle.cpp
	    src/ICLQt/LowLevelPlotWidget.cpp
	    src/ICLQt/MouseEvent.cpp
	    src/ICLQt/MouseHandler.cpp
	    src/ICLQt/MultiDrawHandle.cpp
	    src/ICLQt/PlotHandle.cpp
	    src/ICLQt/PlotWidget.cpp
	    src/ICLQt/QImageConverter.cpp
	    src/ICLQt/Quick.cpp
	    src/ICLQt/SliderHandle.cpp
	    src/ICLQt/SpinnerHandle.cpp
	    src/ICLQt/StateHandle.cpp
	    src/ICLQt/StringHandle.cpp
	    src/ICLQt/StringSignalButton.cpp
	    src/ICLQt/ThreadedUpdatableSlider.cpp
	    src/ICLQt/ThreadedUpdatableTextView.cpp
	    src/ICLQt/ToggleButton.cpp
            src/ICLQt/Widget.cpp
            src/ICLQt/ICLVideoSurface.cpp
            src/ICLQt/QtVideoGrabber.cpp
            src/ICLQt/QtCameraGrabber.cpp
            src/ICLQt/AdjustGridMouseHandler.cpp)

SET(HEADERS src/ICLQt/AbstractPlotWidget.h
	    src/ICLQt/BorderBox.h
	    src/ICLQt/BorderHandle.h
	    src/ICLQt/BoxHandle.h
	    src/ICLQt/ButtonGroupHandle.h
	    src/ICLQt/ButtonHandle.h
	    src/ICLQt/CamCfgWidget.h
	    src/ICLQt/CheckBoxHandle.h
	    src/ICLQt/ChromaClassifierIO.h
	    src/ICLQt/ColorHandle.h
	    src/ICLQt/ColorLabel.h
	    src/ICLQt/ComboHandle.h
	    src/ICLQt/Common.h
	    src/ICLQt/CompabilityLabel.h
	    src/ICLQt/ContainerGUIComponent.h
	    src/ICLQt/ContainerGUIComponents.h
	    src/ICLQt/ContainerHandle.h
	    src/ICLQt/DataStore.h
	    src/ICLQt/DefineRectanglesMouseHandler.h
	    src/ICLQt/DefineQuadrangleMouseHandler.h
	    src/ICLQt/DispHandle.h
	    src/ICLQt/Dragger.h
	    src/ICLQt/DragRectangleMouseHandler.h
	    src/ICLQt/DrawHandle3D.h
	    src/ICLQt/DrawHandle.h
	    src/ICLQt/DrawWidget3D.h
	    src/ICLQt/DrawWidget.h
	    src/ICLQt/FloatHandle.h
	    src/ICLQt/FPSHandle.h
	    src/ICLQt/FSliderHandle.h
	    src/ICLQt/GLFragmentShader.h
	    src/ICLQt/GLImg.h
	    src/ICLQt/GLPaintEngine.h
	    src/ICLQt/GUIComponent.h
	    src/ICLQt/GUIComponents.h
	    src/ICLQt/GUIComponentWithOutput.h
	    src/ICLQt/GUIDefinition.h
	    src/ICLQt/GUI.h
	    src/ICLQt/DynamicGUI.h
	    src/ICLQt/GUIHandleBase.h
	    src/ICLQt/GUIHandle.h
	    src/ICLQt/GUISyntaxErrorException.h
	    src/ICLQt/HistogrammWidget.h
	    src/ICLQt/IconFactory.h
	    src/ICLQt/ImageHandle.h
	    src/ICLQt/ImageStatistics.h
	    src/ICLQt/IntHandle.h
	    src/ICLQt/LabelHandle.h
	    src/ICLQt/LowLevelPlotWidget.h
	    src/ICLQt/MouseEvent.h
	    src/ICLQt/PaintEngine.h
	    src/ICLQt/PlotHandle.h
	    src/ICLQt/PlotWidget.h
	    src/ICLQt/ProxyLayout.h
	    src/ICLQt/QImageConverter.h
	    src/ICLQt/Qt.h
	    src/ICLQt/Quick.h
	    src/ICLQt/SliderHandle.h
	    src/ICLQt/SliderUpdateEvent.h
	    src/ICLQt/SpinnerHandle.h
	    src/ICLQt/SplitterHandle.h
	    src/ICLQt/StateHandle.h
	    src/ICLQt/StringHandle.h
	    src/ICLQt/TabHandle.h
	    src/ICLQt/ThreadedUpdatableTextView.h
	    src/ICLQt/ThreadedUpdatableWidget.h
	    src/ICLQt/WidgetCaptureMode.h
	    src/ICLQt/GLPaintEngine.h
            src/ICLQt/GLImg.h
	    src/ICLQt/DrawWidget3D.h
	    src/ICLQt/DrawHandle3D.h
	    src/ICLQt/GLFragmentShader.h
            src/ICLQt/Quick.h
            src/ICLQt/ICLVideoSurface.h
            src/ICLQt/QtVideoGrabber.h
            src/ICLQt/QtCameraGrabber.h
            src/ICLQt/AdjustGridMouseHandler.h)

IF(RSB_FOUND)
 LIST(APPEND SOURCES src/ICLQt/RSBRemoteGUI.cpp)
 LIST(APPEND HEADERS src/ICLQt/RSBRemoteGUI.h)
ENDIF()

SET(ICL_MOC_HEADERS src/ICLQt/Widget.h
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

# ---- Library build instructions ----
IF(WIN32)
INCLUDE_DIRECTORIES(BEFORE src
                           ${ICL_SOURCE_DIR}/3rdparty/zlib
                           ${ICL_SOURCE_DIR}/3rdparty/libpng
                           ${ICL_SOURCE_DIR}/3rdparty/libjpeg
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src
                           ${ICL_SOURCE_DIR}/ICLFilter/src
                           ${ICL_SOURCE_DIR}/ICLIO/src)
ELSE()
INCLUDE_DIRECTORIES(BEFORE src
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src
                           ${ICL_SOURCE_DIR}/ICLFilter/src
                           ${ICL_SOURCE_DIR}/ICLIO/src)
 ENDIF()

IF(Qt5Widgets_FOUND)
  QT5_WRAP_CPP(MOC_FILES ${ICL_MOC_HEADERS})
ELSE()
  QT4_WRAP_CPP(MOC_FILES ${ICL_MOC_HEADERS})
ENDIF()

ADD_LIBRARY(ICLQt SHARED ${SOURCES} ${HEADERS} ${MOC_FILES})

# ---- 3rd party library handling ----
TARGET_LINK_LIBRARIES(ICLQt ICLCV
                            ${ICLQt_3RDPARTY_LIBRARIES})

SET_TARGET_PROPERTIES(ICLQt
                      PROPERTIES
		      VERSION ${SO_VERSION})

# ---- Build examples/ demos/ apps ----
IF(BUILD_EXAMPLES)
  ADD_SUBDIRECTORY(examples)
ENDIF()

IF(BUILD_DEMOS)
  ADD_SUBDIRECTORY(demos)
ENDIF()

IF(BUILD_APPS)
  ADD_SUBDIRECTORY(apps)
ENDIF()

# ---- Install specifications ----
INSTALL(TARGETS ICLQt
        COMPONENT libraries
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

#get_target_property(ICLQt5CoreLibAbs Qt5::Core IMPORTED_LOCATION_RELEASE)
#get_target_property(ICLQt5GuiLibAbs Qt5::Gui IMPORTED_LOCATION_RELEASE)
#get_target_property(ICLQt5WidgetsLibAbs Qt5::Widgets IMPORTED_LOCATION_RELEASE)
#get_target_property(ICLQt5OpenGLLibAbs Qt5::OpenGL IMPORTED_LOCATION_RELEASE)


#MESSAGE(STATUS "libs: ${ICLQt5CoreLibAbs}")
#MESSAGE(STATUS "libs: ${ICLQt5GuiLibAbs}")
#MESSAGE(STATUS "libs: ${ICLQt5OpenGLLibAbs}")
#MESSAGE(STATUS "libs: ${ICLQt5WidgetsLibAbs}")

# temporary test here
#SET(Qt5_LIBRARIES_X "${ICLQt5CoreLibAbs} ${ICLQt5OpenGLLibAbs} ${ICLQtWidgetsLibAbs} ${ICLQt5GuiLibAbs}")
#MESSAGE(STATUS "qt5libs: ${Qt5_LIBRARIES_X}")

#MESSAGE(STATUS "libs: ${ICLQt5CoreLibAbs}")
#MESSAGE(STATUS "libs: ${ICLQt5GuiLibAbs}")
#MESSAGE(STATUS "libs: ${ICLQt5OpenGLLibAbs}")
#MESSAGE(STATUS "libs: ${ICLQt5WidgetsLibAbs}")

INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS )
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" ICL_MOC_HEADERS )

# ---- PKG-config ----
CREATE_PKGCONFIG(NAME ICLQt
                 LIBRARY_DEPS ${ICLQt_3RDPARTY_LIBRARIES}
                 PKGCONFIG_DEPS ICLCV-${SO_VERSION})

IF(BUILD_TESTS)
  CONFIGURE_GTEST(ICLQt ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(BUILD_TESTS)
