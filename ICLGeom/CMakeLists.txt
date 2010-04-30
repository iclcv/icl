project(ICLGeom)

#sources for lib
set(LIB_SOURCES
	"src/Camera.cpp"
	"src/ConvexHullMonotoneChain.cpp"
	"src/Scene.cpp"
	"src/SceneObject.cpp"
	"src/PlaneEquation.cpp"
	"src/ViewRay.cpp"
)
if(BUILD_EXAMPLES)
if (HAVE_Qt_COND)
add_executable(icl-geom-demo examples/geom-demo.cpp)
target_link_libraries(icl-geom-demo ${PROJECT_NAME} ICLQt)

add_executable(icl-geom-demo-simple examples/geom-demo-simple.cpp)
target_link_libraries(icl-geom-demo-simple ${PROJECT_NAME} ICLQt)

add_executable(icl-cam-calib examples/cam-calib.cpp)
target_link_libraries(icl-cam-calib ${PROJECT_NAME} ICLQt)

add_executable(icl-show-extrinsic-calibration-grid examples/show-extrinsic-calibration-grid.cpp)
target_link_libraries(icl-show-extrinsic-calibration-grid ${PROJECT_NAME} ICLQt)

add_executable(icl-swiss-ranger-demo examples/swiss-ranger-demo.cpp)
target_link_libraries(icl-swiss-ranger-demo ${PROJECT_NAME} ICLQt)

if (HAVE_IPP_COND)
#add_executable(icl-intrinsic-camera-calibration examples/intrinsic-camera-calibration.cpp 
#		examples/intrinsic-camera-calibration-tools.cpp examples/intrinsic-camera-calibration-tools.h)
#target_link_libraries(icl-intrinsic-camera-calibration ${PROJECT_NAME} ICLAlgorithms ICLQuick  ICLBlob ICLFilter ICLCC ICLCore ICLUtils)
endif()
endif()
endif()

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})

set(LIBRARY_OUTPUT_PATH lib CACHE PATH "Output directory for libraries" )

#linker ICLQt
target_link_libraries(${PROJECT_NAME} ICLQuick  ICLBlob ICLIO ICLFilter ICLCC ICLCore ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")

set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION lib)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ICL
)
if(BUILD_EXAMPLES)
if(HAVE_Qt_COND)
install (TARGETS  icl-geom-demo icl-geom-demo-simple icl-cam-calib 
	icl-show-extrinsic-calibration-grid icl-swiss-ranger-demo
	 RUNTIME DESTINATION bin
)
if(HAVE_IPP_COND)
install (TARGETS #icl-intrinsic-camera-calibration 
	 RUNTIME DESTINATION bin)
endif()
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
