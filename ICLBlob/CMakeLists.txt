project(ICLBlob)

#sources for lib
set(LIB_SOURCES 
	"src/ColorBlobSearcher.cpp"
	"src/DefaultColorBlobSearcher.cpp"
	"src/Extrapolator.cpp"
	"src/FMCreator.cpp"
	"src/HungarianAlgorithm.cpp"
	"src/PositionTracker.cpp"
	"src/VectorTracker.cpp"
	"src/RegionBasedBlobSearcher.cpp"
	"src/Region.cpp"
	"src/RegionDetector.cpp"
	"src/RegionFilter.cpp"
	"src/RegionPCAInfo.cpp"
	"src/VQ2D.cpp"
	"src/SimpleBlobSearcher.cpp"
	"src/MeanShiftTracker.cpp"
)

if(BUILD_EXAMPLES)
add_executable(icl-blobtracker-benchmark examples/blobtracker-benchmark.cpp)
target_link_libraries(icl-blobtracker-benchmark ${PROJECT_NAME})

add_executable(icl-region-based-blob-finder-test examples/region-based-blob-finder-test.cpp)
target_link_libraries(icl-region-based-blob-finder-test ${PROJECT_NAME} ICLQuick)

add_executable(icl-region-detector-test examples/region-detector-test.cpp)
target_link_libraries(icl-region-detector-test ${PROJECT_NAME} ICLQuick)

if (HAVE_QT_COND)
add_executable(icl-region-inspector-demo examples/region-inspector-demo.cpp)
target_link_libraries(icl-region-inspector-demo ${PROJECT_NAME})

add_executable(icl-vector-tracker-demo examples/vector-tracker-demo.cpp)
target_link_libraries(icl-vector-tracker-demo ${PROJECT_NAME})

add_executable(icl-interactive-vector-tracker-benchmark examples/interactive-vector-tracker-benchmark.cpp)
target_link_libraries(icl-interactive-vector-tracker-benchmark ${PROJECT_NAME})

add_executable(icl-simple-blob-searcher-test examples/simple-blob-searcher-test.cpp)
target_link_libraries(icl-simple-blob-searcher-test ${PROJECT_NAME})

add_executable(icl-mean-shift-online-demo examples/mean-shift-online-demo.cpp)
target_link_libraries(icl-mean-shift-online-demo ${PROJECT_NAME})

add_executable(icl-corner-detection-css-demo examples/corner-detection-css-demo.cpp)
target_link_libraries(icl-corner-detection-css-demo ${PROJECT_NAME})

add_executable(icl-online-region-detection-demo examples/online-region-detection-demo.cpp)
target_link_libraries(icl-online-region-detection-demo ${PROJECT_NAME})
endif()
endif()

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})

set(LIBRARY_OUTPUT_PATH lib CACHE PATH "Output directory for libraries" )

#linker
target_link_libraries(${PROJECT_NAME} ICLCC ICLCore ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")

set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION lib
)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ICL
)
if(BUILD_EXAMPLES)
install (TARGETS icl-blobtracker-benchmark icl-region-based-blob-finder-test 
		icl-region-detector-test
	RUNTIME DESTINATION bin)
if(HAVE_QT_COND)
install (TARGETS icl-region-inspector-demo icl-vector-tracker-demo 
		icl-interactive-vector-tracker-benchmark icl-simple-blob-searcher-test 
		icl-mean-shift-online-demo icl-corner-detection-css-demo 
		icl-online-region-detection-demo
	 RUNTIME DESTINATION bin)
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
