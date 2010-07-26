project(ICLAlgorithms)

set(CONDITIONLIST "")
#sources for lib
set(LIB_SOURCES "")
add_libsource (${PROJECT_NAME} "src/SOM.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/HoughLine.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST "HAVE_IPP_COND")
add_libsource (${PROJECT_NAME} "src/UsefulFunctions.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ViewBasedTemplateMatcher.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/HoughLineDetector.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST "HAVE_SVS_COND")
add_libsource (${PROJECT_NAME} "src/SVS.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST "HAVE_OPENCV_COND")
add_libsource (${PROJECT_NAME} "src/OpenCVSurfDetector.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST "HAVE_OPENSURF_COND")
add_libsource (${PROJECT_NAME} "src/OpenSurfDetector.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GenericSurfDetector.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_IPP_COND)
set(ICLLibsToLinkAgainst ${PROJECT_NAME})
add_example (${PROJECT_NAME} "template-matching-test" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_IPP_COND HAVE_QT_COND)
add_example (${PROJECT_NAME} "interactive-template-matching-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "interactive-hough-line-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_IPP_COND HAVE_QT_COND HAVE_SVS_COND)
add_example (${PROJECT_NAME} "svs-online-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_QT_COND HAVE_OPENCV_COND HAVE_OPENSURF_COND)
add_example (${PROJECT_NAME} "generic-surf-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_SVS_COND)
add_example (${PROJECT_NAME} "svs-test" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST "HAVE_GTEST_COND")
set(ICLLibsToLinkAgainst "${GTEST_LIBS_l}")
#add_gtest (${PROJECT_NAME} "test-image.cpp" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#linker
target_link_libraries(${PROJECT_NAME} ${ICLAlgorithms_dependencies})

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
