project(ICLQt)

set(CONDITIONLIST "BUILD_EXAMPLES")
set(ICLLibsToLinkAgainst ${PROJECT_NAME} )
add_example (${PROJECT_NAME} "3D-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "camviewer" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "chroma-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "colorpicker" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "gui-callback-test" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "interactive-filter-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "mouse-interaction-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "visualization-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "camcfg" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "config-gui-example" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "datastore-test" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "complex-gui-callback-test" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "image-compare" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
set(ICLLibsToLinkAgainst ${PROJECT_NAME} ICLQuick)
add_example (${PROJECT_NAME} "gui-test" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "gui-test-2" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "xv" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "special-button-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST "")
#sources for lib
set(LIB_SOURCES "")
add_libsource (${PROJECT_NAME} "src/BorderHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ButtonGroupHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ButtonHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/CamThread.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ChromaGUI.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ChromaClassifierIO.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ComboHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/CompabilityLabel.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DoubleSlider.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Dragger.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DrawHandle3D.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/CheckBoxHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DrawHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DrawWidget3D.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/StateHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DrawWidget.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FloatHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FSliderHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GLPaintEngine.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GLTextureMapBaseImage.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GLTextureMapImage.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GLTextureMapPaintEngine.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GUI.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GUIDefinition.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GUIWidget.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ImageHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ImgParamWidget.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/IntHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/LabelHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/MouseHandler.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/MouseEvent.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/MultiDrawHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/QImageConverter.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/SliderHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/SpinnerHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/StringHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/StringSignalButton.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Widget.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/CamCfgWidget.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ToggleButton.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ConfigFileGUI.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Application.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/IconFactory.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ThreadedUpdatableTextView.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DataStore.cpp" "${CONDITIONLIST}" LIB_SOURCES)


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
#SET(HWQ_Qt4_UI "")
#SET(HWQ_Qt4_RES "")

QT4_WRAP_CPP(HWQ_MOC_CPP ${HWQ_Qt4_SRC})
#QT4_WRAP_UI(HWQ_UI_CPP ${HWQ_Qt4_UI})
#QT4_ADD_RESOURCES(HWQ_RES_H ${HWQ_Qt4_RES})

# General
SET( HWQ_SRC
	"${LIB_SOURCES}"
	${HWQ_MOC_CPP}
#	${HWQ_UI_CPP}
#	${HWQ_RES_H}
)
#message(STATUS "src: ${HWQ_SRC}")
#SET( HWQ_LIB ${QT_LIBRARIES})
#message(STATUS "qtlibs: ${QT_LIBRARIES}")
#create lib
add_library(${PROJECT_NAME} SHARED ${HWQ_SRC})
#target_link_libraries(${PROJECT_NAME} ${HWQ_LIB} )
#linker
target_link_libraries(${PROJECT_NAME} ${ICLQt_dependencies})

link_directories("${LIBRARY_OUTPUT_PATH}")

install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION "${LIBRARY_OUTPUT_PATH}"
)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ICL
	FILES_MATCHING PATTERN "*.h"
	REGEX .svn* EXCLUDE
)
add_doc_gen(${PROJECT_NAME})
