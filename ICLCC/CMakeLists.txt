project(ICLCC)

#sources for lib
set(LIB_SOURCES 
	"src/Bayer.cpp"
	"src/CCFunctions.cpp"
	"src/CCLUT.cpp"
	"src/Color.cpp"
	"src/Converter.cpp"
	"src/FixedConverter.cpp"

)
if(BUILD_EXAMPLES)
add_executable(icl-bayer-benchmark examples/bayer-benchmark.cpp)
target_link_libraries(icl-bayer-benchmark ${PROJECT_NAME} ICLCore ICLUtils ICLIO) 

add_executable(icl-converter-test examples/converter-test.cpp)
target_link_libraries(icl-converter-test ${PROJECT_NAME} ICLQuick)

if (HAVE_Qt_COND)
add_executable(icl-colorspace-online-demo examples/colorspace-online-demo.cpp)
target_link_libraries(icl-colorspace-online-demo ${PROJECT_NAME} ICLQuick)
endif()
endif()

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})

set(LIBRARY_OUTPUT_PATH lib CACHE PATH "Output directory for libraries" )

#linker
target_link_libraries(${PROJECT_NAME} ICLCore ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")

set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION lib
)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ICL
)
if(BUILD_EXAMPLES)
install(TARGETS icl-converter-test icl-bayer-benchmark
	RUNTIME DESTINATION bin
)
if(HAVE-Qt_COND)
install (TARGETS icl-colorspace-online-demo  
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
