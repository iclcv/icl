project(ICLAlgorithms)

#sources for lib
set(LIB_SOURCES 
	"src/SOM.cpp"
	"src/HoughLine.cpp"
)
if (HAVE_IPP_COND)
set (LIB_SOURCES_IPP
	"src/UsefulFunctions.cpp"
	"src/ViewBasedTemplateMatcher.cpp"
	"src/HoughLineDetector.cpp"
)
endif()
if (HAVE_SVS_COND)	
set(LIB_SOURCES_SVS
	"src/SVS.cpp" 
)
endif()

if (HAVE_OPENCV_COND)
if (HAVE_OPENSURF_COND)
set (LIB_SOURCES_SURF
	"src/OpenCVSurfDetector.cpp"
	"src/OpenSurfDetector.cpp"
	"src/GenericSurfDetector.cpp"
)
endif()
endif()
set(LIB_SOURCES "${LIB_SOURCES};${LIB_SOURCES_IPP};${LIB_SOURCES_SVS};${LIB_SOURCES_SURF}")

if(BUILD_EXAMPLES)
if (HAVE_IPP_COND)
add_executable(icl-template-matching-test examples/template-matching-test.cpp)
target_link_libraries(icl-template-matching-test ${PROJECT_NAME})

if (HAVE_Qt_COND)
add_executable(icl-interactive-template-matching-demo examples/interactive-template-matching-demo.cpp)
target_link_libraries(icl-interactive-template-matching-demo ${PROJECT_NAME})

add_executable(icl-interactive-hough-line-demo examples/interactive-hough-line-demo.cpp)
target_link_libraries(icl-interactive-hough-line-demo ${PROJECT_NAME})
if (HAVE_SVS_COND)
add_executable(icl-svs-online-demo examples/svs-online-demo.cpp)
target_link_libraries(icl-svs-online-demo ${PROJECT_NAME})
endif()
endif()
endif()

if (HAVE_SVS_COND)
add_executable(icl-svs-test examples/svs-test.cpp)
target_link_libraries(icl-svs-test ${PROJECT_NAME})
endif()
endif()

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#linker
if(HAVE_SVS_COND)
target_link_libraries(${PROJECT_NAME} svs svscalc)
endif()
if(HAVE_OPENCV_COND)
target_link_libraries(${PROJECT_NAME} ICLOpenCV)
endif()
if(HAVE_OPENSURF_COND)
target_link_libraries(${PROJECT_NAME} opensurf)
endif()
target_link_libraries(${PROJECT_NAME} ICLGeom ICLQuick  ICLBlob ICLIO ICLFilter ICLCC ICLCore ICLUtils)

if(BUILD_STATIC_LIBS)
add_library("${PROJECT_NAME}_static" STATIC ${LIB_SOURCES})
set_target_properties("${PROJECT_NAME}_static" PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
target_link_libraries("${PROJECT_NAME}_static" ICLGeom ICLQuick ICLBlob ICLIO ICLFilter ICLCC ICLCore ICLUtils)
if(HAVE_OPENCV_COND)
target_link_libraries(${PROJECT_NAME} ICLOpenCV)
endif()
if(HAVE_OPENSURF_COND)
target_link_libraries(${PROJECT_NAME} opensurf)
endif()
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
if(HAVE_IPP_COND)
install (TARGETS icl-template-matching-test
	 RUNTIME DESTINATION bin)
if(HAVE_Qt_COND)
install (TARGETS  icl-interactive-hough-line-demo icl-interactive-template-matching-demo
	 RUNTIME DESTINATION bin)
if(HAVE_SVS_COND)
install (TARGETS icl-svs-online-demo
	 RUNTIME DESTINATION bin)
endif()
endif()
endif()

if(HAVE_SVS_COND)
install (TARGETS icl-svs-test  
	 RUNTIME DESTINATION bin
)
endif()
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
