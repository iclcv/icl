project(ICLCore)

#sources for lib
set(LIB_SOURCES 
	"src/CoreFunctions.cpp"
	"src/ImgBase.cpp"
	"src/ImgBorder.cpp"
	"src/Img.cpp"
	"src/ImgParams.cpp"
	"src/Line32f.cpp"
	"src/Line.cpp"
	"src/Mathematics.cpp"
	"src/Random.cpp"
	"src/CornerDetectorCSS.cpp"
	"src/SampledLine.cpp"
	"src/LineSampler.cpp"
)

if(BUILD_EXAMPLES)
add_executable(icl-img-example examples/img-example.cpp)
target_link_libraries(icl-img-example ${PROJECT_NAME})

add_executable(icl-copy-channel-roi-example examples/copy-channel-roi-example.cpp)
target_link_libraries(icl-copy-channel-roi-example ${PROJECT_NAME})

add_executable(icl-img-iterator-benchmark examples/img-iterator-benchmark.cpp)
target_link_libraries(icl-img-iterator-benchmark ${PROJECT_NAME})

add_executable(icl-reduce-channels-test examples/reduce-channels-test.cpp)
target_link_libraries(icl-reduce-channels-test ${PROJECT_NAME})

add_executable(icl-sampled-line-test examples/sampled-line-test.cpp)
target_link_libraries(icl-sampled-line-test ${PROJECT_NAME})
endif()

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#linker
target_link_libraries(${PROJECT_NAME} ICLUtils)

if(BUILD_STATIC_LIBS)
add_library("${PROJECT_NAME}_static" STATIC ${LIB_SOURCES})
set_target_properties("${PROJECT_NAME}_static" PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
target_link_libraries("${PROJECT_NAME}_static" ICLUtils)
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
install(TARGETS icl-img-example icl-copy-channel-roi-example 
	icl-img-iterator-benchmark icl-reduce-channels-test icl-sampled-line-test
	 RUNTIME DESTINATION bin
)
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
