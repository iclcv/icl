project(ICLQt)

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

SET( HWQ_Qt4_SRC
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
SET( HWQ_Qt4_UI)
SET( HWQ_Qt4_RES)

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

SET( HWQ_LIB ${QT_LIBRARIES})
#message(STATUS "qtlibs: ${QT_LIBRARIES}")
add_library(${PROJECT_NAME} SHARED ${HWQ_SRC})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${HWQ_LIB} )
#create lib
#add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})

set(LIBRARY_OUTPUT_PATH lib CACHE PATH "Output directory for libraries" )

#linker
if(HAVE_LIBZ_COND)
target_link_libraries(${PROJECT_NAME} z)
endif()
if(HAVE_LIBJPEG_COND)
target_link_libraries(${PROJECT_NAME} jpeg)
endif()
if(HAVE_LIBMESASR_COND)
target_link_libraries(${PROJECT_NAME} mesasr)
endif()
if(HAVE_IMAGEMAGICK_COND)
target_link_libraries(${PROJECT_NAME} Magick++)
endif()
if(HAVE_VIDEODEV_COND)
#already inside linuxkernel
#target_link_libraries(${PROJECT_NAME} )
endif()
if(HAVE_LIBDC_COND)
target_link_libraries(${PROJECT_NAME} dc1394)
endif()
if(HAVE_XINE_COND)
target_link_libraries(${PROJECT_NAME} xine)
endif()
if(HAVE_UNICAP_COND)
target_link_libraries(${PROJECT_NAME} unicap rt)
endif()
if(HAVE_XCF_COND)
target_link_libraries(${PROJECT_NAME} xcf xqilla log4cxx Memory xmltio Ice IceUtil xerces-c)
endif()
if(HAVE_IPP_COND)
target_link_libraries(${PROJECT_NAME} ippcore ippi ipps ippsr ippcv ippm ippcc guide iomp5)
endif()
target_link_libraries(${PROJECT_NAME} pthread)
if(HAVE_MKL_COND)
target_link_libraries(${PROJECT_NAME} mkl_intel mkl_intel_thread mkl_core iomp5)
endif()
if(HAVE_OPENGL_COND)
target_link_libraries(${PROJECT_NAME} GL GLU)
endif()
if(HAVE_Qt_COND)
target_link_libraries(${PROJECT_NAME} QtCore QtGui QtOpenGL)
endif()

target_link_libraries(${PROJECT_NAME} ICLIO ICLOpenCV ICLFilter ICLCC ICLCore ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")

set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME} #icl-dummy-app
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib
)
