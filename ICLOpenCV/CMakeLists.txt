project(ICLOpenCV)

#sources for lib
set(LIB_SOURCES 
	"src/OpenCV.cpp"
)
if(BUILD_EXAMPLES)
add_executable(icl-dummy-app examples/dummy-app.cpp)
#do not link dummy-apps!!!
target_link_libraries(icl-dummy-app )#${PROJECT_NAME})
endif()
#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})

set(LIBRARY_OUTPUT_PATH lib CACHE PATH "Output directory for libraries" )

#linker
if(HAVE_OPENCV_COND)
target_link_libraries(${PROJECT_NAME} cxcore cv highgui cvaux ml)
endif()
target_link_libraries(${PROJECT_NAME} ICLCore ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")

set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION lib)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ICL
)
if(BUILD_EXAMPLES)
install(TARGETS icl-dummy-app
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
