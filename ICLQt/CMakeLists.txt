project(ICLQt)
if(BUILD_EXAMPLES)
add_executable(icl-3D-demo examples/3D-demo.cpp)
target_link_libraries(icl-3D-demo ${PROJECT_NAME})

add_executable(icl-camviewer examples/camviewer.cpp)
target_link_libraries(icl-camviewer ${PROJECT_NAME})

add_executable(icl-chroma-demo examples/chroma-demo.cpp)
target_link_libraries(icl-chroma-demo ${PROJECT_NAME})

add_executable(icl-colorpicker examples/colorpicker.cpp)
target_link_libraries(icl-colorpicker ${PROJECT_NAME})

add_executable(icl-gui-test examples/gui-test.cpp)
target_link_libraries(icl-gui-test ${PROJECT_NAME} ICLQuick)

add_executable(icl-gui-test-2 examples/gui-test-2.cpp)
target_link_libraries(icl-gui-test-2 ${PROJECT_NAME} ICLQuick)

add_executable(icl-gui-callback-test examples/gui-callback-test.cpp)
target_link_libraries(icl-gui-callback-test ${PROJECT_NAME})

add_executable(icl-interactive-filter-demo examples/interactive-filter-demo.cpp)
target_link_libraries(icl-interactive-filter-demo ${PROJECT_NAME})

add_executable(icl-mouse-interaction-demo examples/mouse-interaction-demo.cpp)
target_link_libraries(icl-mouse-interaction-demo ${PROJECT_NAME})

add_executable(icl-visualization-demo examples/visualization-demo.cpp)
target_link_libraries(icl-visualization-demo ${PROJECT_NAME})

add_executable(icl-xv examples/xv.cpp)
target_link_libraries(icl-xv ${PROJECT_NAME} ICLQuick)

add_executable(icl-camcfg examples/camcfg.cpp)
target_link_libraries(icl-camcfg ${PROJECT_NAME})

add_executable(icl-config-gui-example examples/config-gui-example.cpp)
target_link_libraries(icl-config-gui-example ${PROJECT_NAME})

add_executable(icl-datastore-test examples/datastore-test.cpp)
target_link_libraries(icl-datastore-test ${PROJECT_NAME})

add_executable(icl-special-button-demo examples/special-button-demo.cpp)
target_link_libraries(icl-special-button-demo ${PROJECT_NAME} ICLQuick)

add_executable(icl-complex-gui-callback-test examples/complex-gui-callback-test.cpp)
target_link_libraries(icl-complex-gui-callback-test ${PROJECT_NAME})

add_executable(icl-image-compare examples/image-compare.cpp)
target_link_libraries(icl-image-compare ${PROJECT_NAME})
endif()
#sources for lib
set(LIB_SOURCES 
	"src/BorderHandle.cpp"
	"src/ButtonGroupHandle.cpp"
	"src/ButtonHandle.cpp"
	"src/CamThread.cpp"
	"src/ChromaGUI.cpp"
	"src/ChromaClassifierIO.cpp"
	"src/ComboHandle.cpp"
	"src/CompabilityLabel.cpp"
	"src/DoubleSlider.cpp"
	"src/Dragger.cpp"
	"src/DrawHandle3D.cpp"
	"src/CheckBoxHandle.cpp"
	"src/DrawHandle.cpp"
	"src/DrawWidget3D.cpp"
	"src/StateHandle.cpp"
	"src/DrawWidget.cpp"
	"src/FloatHandle.cpp"
	"src/FSliderHandle.cpp"
	"src/GLPaintEngine.cpp"
	"src/GLTextureMapBaseImage.cpp"
	"src/GLTextureMapImage.cpp"
	"src/GLTextureMapPaintEngine.cpp"
	"src/GUI.cpp"
	"src/GUIDefinition.cpp"
	"src/GUIWidget.cpp"
	"src/ImageHandle.cpp"
	"src/ImgParamWidget.cpp"
	"src/IntHandle.cpp"
	"src/LabelHandle.cpp"
	"src/MouseHandler.cpp"
	"src/MouseEvent.cpp"
	"src/MultiDrawHandle.cpp"
	"src/QImageConverter.cpp"
	"src/SliderHandle.cpp"
	"src/SpinnerHandle.cpp"
	"src/StringHandle.cpp"
	"src/StringSignalButton.cpp"
	"src/Widget.cpp"
	"src/CamCfgWidget.cpp"
	"src/ToggleButton.cpp"
	"src/ConfigFileGUI.cpp"
	"src/Application.cpp"
	"src/IconFactory.cpp"
	"src/ThreadedUpdatableTextView.cpp"
	"src/DataStore.cpp"

)

SET(HWQ_Qt4_SRC
	"../include/ICLQt/Widget.h"
	"../include/ICLQt/MouseHandler.h"
	"../include/ICLQt/DoubleSlider.h"
	"../include/ICLQt/ImgParamWidget.h"
	"../include/ICLQt/StringSignalButton.h"
	"../include/ICLQt/CamThread.h"
	"../include/ICLQt/GUIWidget.h"
	"../include/ICLQt/ChromaGUI.h"
	"../include/ICLQt/MultiDrawHandle.h"
	"../include/ICLQt/CamCfgWidget.h"
	"../include/ICLQt/ToggleButton.h"
	"../include/ICLQt/ConfigFileGUI.h"
)
SET(HWQ_Qt4_UI "")
SET(HWQ_Qt4_RES "")

QT4_WRAP_CPP(HWQ_MOC_CPP ${HWQ_Qt4_SRC})
QT4_WRAP_UI(HWQ_UI_CPP ${HWQ_Qt4_UI})
QT4_ADD_RESOURCES(HWQ_RES_H ${HWQ_Qt4_RES})

# General
SET( HWQ_SRC
	"${LIB_SOURCES}"
	${HWQ_MOC_CPP}
	${HWQ_UI_CPP}
	${HWQ_RES_H}
)
#message(STATUS "src: ${HWQ_SRC}")
SET( HWQ_LIB ${QT_LIBRARIES})
#message(STATUS "qtlibs: ${QT_LIBRARIES}")
#create lib
add_library(${PROJECT_NAME} SHARED ${HWQ_SRC})
target_link_libraries(${PROJECT_NAME} ${HWQ_LIB} )
#linker
if(HAVE_OPENGL_COND)
target_link_libraries(${PROJECT_NAME} GL GLU)
endif()
if(HAVE_Qt_COND)
target_link_libraries(${PROJECT_NAME} QtCore QtGui QtOpenGL)
endif()
if(HAVE_OPENCV_COND)
#target_link_libraries(${PROJECT_NAME} ICLOpenCV)
endif()
target_link_libraries(${PROJECT_NAME} ICLIO ICLFilter ICLCC ICLCore ICLUtils)
if(BUILD_STATIC_LIBS)
add_library("${PROJECT_NAME}_static" STATIC ${HWQ_SRC})
target_link_libraries("${PROJECT_NAME}_static" ${HWQ_LIB} )
set_target_properties("${PROJECT_NAME}_static" PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
if(HAVE_OPENGL_COND)
target_link_libraries("${PROJECT_NAME}_static" GL GLU)
endif()
if(HAVE_Qt_COND)
target_link_libraries("${PROJECT_NAME}_static" QtCore QtGui QtOpenGL)
endif()
target_link_libraries("${PROJECT_NAME}_static" ICLIO ICLFilter ICLCC ICLCore ICLUtils)
endif()

link_directories("${LIBRARY_OUTPUT_PATH}")

install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION "${LIBRARY_OUTPUT_PATH}"
)
if(BUILD_STATIC_LIBS)
install(TARGETS "${PROJECT_NAME}_static"
	ARCHIVE DESTINATION "${LIBRARY_OUTPUT_PATH}"
)
endif()
install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ICL
)
if(BUILD_EXAMPLES)
install(TARGETS icl-3D-demo icl-camviewer icl-chroma-demo icl-colorpicker
		icl-gui-test icl-gui-test-2 icl-gui-callback-test icl-interactive-filter-demo
		icl-mouse-interaction-demo icl-visualization-demo icl-xv icl-camcfg icl-config-gui-example
		icl-datastore-test icl-special-button-demo icl-complex-gui-callback-test icl-image-compare
	 RUNTIME DESTINATION bin)
endif()
if(DOXYGEN_FOUND)
add_custom_target(doc doxygen doc/doxyfile)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html/
         DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/${PROJECT_NAME}
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/${PROJECT_NAME}
	PATTERN "doxyfile" EXCLUDE
)
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES doc/html)
endif()
