project(ICLAlgorithms)

#sources for lib
set(LIB_SOURCES 
	"src/SOM.cpp"
	"src/HoughLine.cpp"
)
if (HAVE_IPP_COND)
set (LIB_SOURCES ${LIBSOURCES}
	"src/UsefulFunctions.cpp"
	"src/ViewBasedTemplateMatcher.cpp"
	"src/HoughLineDetector.cpp"
)
endif()
if (HAVE_SVS_COND)	
set(LIB_SOURCES ${LIB_SOURCES}
	"src/SVS.cpp" 
)
endif()


if (HAVE_IPP_COND)
#add_executable(icl-template-matching-test examples/template-matching-test.cpp)
#target_link_libraries(icl-template-matching-test ${PROJECT_NAME})

if (HAVE_Qt_COND)
#add_executable(icl-interactive-template-matching-demo examples/interactive-template-matching-demo.cpp)
#target_link_libraries(icl-interactive-template-matching-demo ${PROJECT_NAME})

#add_executable(icl-interactive-hough-line-demo examples/interactive-hough-line-demo.cpp)
#target_link_libraries(icl-interactive-hough-line-demo ${PROJECT_NAME})
if (HAVE_SVS_COND)
#add_executable(icl-svs-online-demo examples/svs-online-demo.cpp)
#target_link_libraries(icl-svs-online-demo ${PROJECT_NAME})
endif()
endif()
endif()

if (HAVE_SVS_COND)
#add_executable(icl-svs-test examples/svs-test.cpp)
#target_link_libraries(icl-svs-test ${PROJECT_NAME})
endif()


#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
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
if(HAVE_SVS_COND)
target_link_libraries(${PROJECT_NAME} svs svscalc)
endif()
target_link_libraries(${PROJECT_NAME} ICLOpenCV ICLGeom ICLQuick ICLQt ICLBlob ICLIO ICLFilter ICLCC ICLCore ICLUtils)
if(HAVE_OPENCV_COND)
target_link_libraries(${PROJECT_NAME} cxcore cv highgui cvaux ml)
endif()

link_directories("${LIBRARY_OUTPUT_PATH}")

set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME}  
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)

if(HAVE_IPP_COND)
install (TARGETS ${PROJECT_NAME}  
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
if(HAVE_Qt_COND)
install (TARGETS #icl-interactive-template-matching-demo icl-interactive-hough-line-demo
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
if(HAVE_SVS_COND)
install (TARGETS #icl-svs-online-demo
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
endif()
endif()
endif()

if(HAVE_SVS_COND)
install (TARGETS   
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
endif()

