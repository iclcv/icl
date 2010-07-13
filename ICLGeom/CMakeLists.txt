project(ICLGeom)

set(CONDITIONLIST "")
#sources for lib
set(LIB_SOURCES "")
add_libsource (${PROJECT_NAME} "src/Camera.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ConvexHullMonotoneChain.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Scene.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/SceneObject.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/PlaneEquation.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ViewRay.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST "BUILD_EXAMPLES HAVE_Qt_COND")
set(ICLLibsToLinkAgainst ${PROJECT_NAME} ICLQt)
add_example (${PROJECT_NAME} "geom-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "geom-demo-simple" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "cam-calib" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "show-extrinsic-calibration-grid" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "swiss-ranger-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

if(BUILD_EXAMPLES)
if (HAVE_IPP_COND)
set(CONDITIONLIST "BUILD_EXAMPLES HAVE_Qt_COND HAVE_IPP_COND")
#TODO liste von sourcefiles
#add_example (${PROJECT_NAME} "quick-benchmark" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_executable(icl-intrinsic-camera-calibration examples/intrinsic-camera-calibration.cpp 
		examples/intrinsic-camera-calibration-tools.cpp examples/intrinsic-camera-calibration-tools.h
)
target_link_libraries(icl-intrinsic-camera-calibration ${PROJECT_NAME} ICLAlgorithms ICLQuick  ICLBlob ICLFilter ICLCC ICLCore ICLUtils)
endif()
endif()

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#linker
target_link_libraries(${PROJECT_NAME} ${ICLGeom_dependencies})

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
