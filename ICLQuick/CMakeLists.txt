project(ICLQuick)

#sources for lib
set(LIB_SOURCES
	"src/Quick.cpp"
	"src/QuickRegions.cpp"
)
if(BUILD_EXAMPLES)
if(HAVE_Qt_COND)
add_executable(icl-quick-test examples/quick-test.cpp)
target_link_libraries(icl-quick-test ${PROJECT_NAME})

add_executable(icl-quick-benchmark examples/quick-benchmark.cpp)
target_link_libraries(icl-quick-benchmark ${PROJECT_NAME}) 
endif()
endif()

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#linker
if(HAVE_Qt_COND)
target_link_libraries(${PROJECT_NAME} ICLQt)
endif()
target_link_libraries(${PROJECT_NAME} ICLBlob ICLIO ICLFilter ICLCC ICLCore ICLUtils)
if(BUILD_STATIC_LIBS)
add_library("${PROJECT_NAME}_static" STATIC ${LIB_SOURCES})
set_target_properties("${PROJECT_NAME}_static" PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
target_link_libraries("${PROJECT_NAME}_static" ICLBlob ICLIO ICLFilter ICLCC ICLCore ICLUtils)
if(HAVE_Qt_COND)
target_link_libraries(${PROJECT_NAME} ICLQt)
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
if(HAVE_Qt_COND)
	install (TARGETS icl-quick-test icl-quick-benchmark 
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
