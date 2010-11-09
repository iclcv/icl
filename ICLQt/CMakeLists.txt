#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLQt/CMakeLists.txt                                   **
#** Module : ICLQt                                                  **
#** Authors: Christian Groszewski                                   **
#**                                                                 **
#**                                                                 **
#** Commercial License                                              **
#** ICL can be used commercially, please refer to our website       **
#** www.iclcv.org for more details.                                 **
#**                                                                 **
#** GNU General Public License Usage                                **
#** Alternatively, this file may be used under the terms of the     **
#** GNU General Public License version 3.0 as published by the      **
#** Free Software Foundation and appearing in the file LICENSE.GPL  **
#** included in the packaging of this file.  Please review the      **
#** following information to ensure the GNU General Public License  **
#** version 3.0 requirements will be met:                           **
#** http://www.gnu.org/copyleft/gpl.html.                           **
#**                                                                 **
#** The development of this software was supported by the           **
#** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
#** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
#** Forschungsgemeinschaft (DFG) in the context of the German       **
#** Excellence Initiative.                                          **
#**                                                                 **
#*********************************************************************
project(ICLQt)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES)
set(ICLLibsToLinkAgainst ${PROJECT_NAME} )

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

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_OPENGL_COND)
add_example (${PROJECT_NAME} "3D-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES)
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
add_libsource (${PROJECT_NAME} "src/CheckBoxHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DrawHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
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
add_libsource (${PROJECT_NAME} "src/ColorLabel.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST HAVE_OPENGL_COND)
add_libsource (${PROJECT_NAME} "src/DrawWidget3D.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DrawHandle3D.cpp" "${CONDITIONLIST}" LIB_SOURCES)

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
	"../include/ICLQt/ToggleButton.h"
	"../include/ICLQt/ConfigFileGUI.h"
)

#pkg-config --variable moc_location QtCore /usr/bin/moc-qt4
set(moc_found "")
set(moc_path_ex "")
set(QT_MOC_EXECUTABLE "")#${ICL_XDEP_QT_PATH}/bin/moc)
execute_process(
   COMMAND pkg-config --variable moc_location QtCore 
   OUTPUT_VARIABLE QT_MOC_EXECUTABLE
   RESULT_VARIABLE moc_found)
string(STRIP "${QT_MOC_EXECUTABLE}" QT_MOC_EXECUTABLE)
#message(STATUS "moc_exe:${QT_MOC_EXECUTABLE}")

#set(HWQ_SRC ${LIB_SOURCES})
if(WIN32)
set(QT_MOC_EXECUTABLE "${ICL_XDEP_QT_PATH}/bin/moc.exe")
endif()

if(NOT ${QT_MOC_EXECUTABLE} STREQUAL "")
message(STATUS "moc_exe:${QT_MOC_EXECUTABLE}")
# QT4_GET_MOC_FLAGS(moc_flags)
SET(_moc_flags "")
  GET_DIRECTORY_PROPERTY(_inc_DIRS INCLUDE_DIRECTORIES)
	set(framework_path ${QT_INCLUDE_PATH})
  FOREACH(_current ${_inc_DIRS})
    IF("${_current}" MATCHES ".framework/?$")
      STRING(REGEX REPLACE "/[^/]+.framework" "" framework_path "${_current}")
      SET(_moc_flags ${_moc_flags} "-F${framework_path}")
    ELSE("${_current}" MATCHES ".framework/?$")
      SET(_moc_flags ${_moc_flags} "-I${_current}")
    ENDIF()
  ENDFOREACH(_current ${_inc_DIRS})

  GET_DIRECTORY_PROPERTY(_defines COMPILE_DEFINITIONS)
  FOREACH(_current ${_defines})
    SET(_moc_flags ${_moc_flags} "-D${_current}")
  ENDFOREACH(_current ${_defines})

set(outfiles "")
set(moc_options "")

QT4_EXTRACT_OPTIONS(moc_files moc_options ${HWQ_Qt4_SRC})
#set(moc_files ${HWQ_Qt4_SRC})
#message(STATUS "add ${moc_files}")
foreach(it ${moc_files})
  GET_FILENAME_COMPONENT(it ${it} ABSOLUTE)
  QT4_MAKE_OUTPUT_FILE(${it} moc_ cxx outfile)
  #SET(outfiles ${outfiles} ${outfile})
  ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
                      COMMAND ${QT_MOC_EXECUTABLE}
                      ARGS ${_moc_flags} ${moc_options} -o ${outfile} ${it}
                      DEPENDS ${it})
  set(outfiles ${outfiles} ${outfile})	
endforeach()
endif()

#old code
#QT4_WRAP_CPP(HWQ_MOC_CPP ${HWQ_Qt4_SRC})
#SET( HWQ_SRC ${LIB_SOURCES} ${HWQ_MOC_CPP})

set(HWQ_SRC ${LIB_SOURCES} ${outfiles})
set(CONDITIONLIST "HAVE_GTEST_COND")
set(ICLLibsToLinkAgainst "${GTEST_LIBS_l}")
#add_gtest (${PROJECT_NAME} "test-image.cpp" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

#create lib
if(WIN32)
add_library(${PROJECT_NAME} ${HWQ_SRC})
else()
add_library(${PROJECT_NAME} SHARED ${HWQ_SRC})
endif()
#target_link_libraries(${PROJECT_NAME} ${HWQ_LIB} )
#linker
target_link_libraries(${PROJECT_NAME} ${ICLQt_dependencies})

link_directories("${LIBRARY_OUTPUT_PATH}")

if(WIN32)
install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION "${LIBRARY_OUTPUT_PATH}"
	 #MINGW COMPATIBILITY 
	ARCHIVE DESTINATION "${LIBRARY_OUTPUT_PATH}"
	RUNTIME DESTINATION "${LIBRARY_OUTPUT_PATH}")
else()
install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION "${LIBRARY_OUTPUT_PATH}")
endif()
install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icl-${VERSION}${DEBUG_POSTFIX}
	FILES_MATCHING PATTERN "*.h"
	REGEX .svn* EXCLUDE
)
add_doc_gen(${PROJECT_NAME})
