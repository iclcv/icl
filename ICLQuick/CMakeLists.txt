project(ICLQuick)

set(CONDITIONLIST "")
#sources for lib
set(LIB_SOURCES "")
add_libsource (${PROJECT_NAME} "src/Quick.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/QuickRegions.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST BUILD_EXAMPLES HAVE_Qt_COND)
set(ICLLibsToLinkAgainst ${PROJECT_NAME})
add_example (${PROJECT_NAME} "quick-test" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "quick-benchmark" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#linker
#if(HAVE_Qt_COND)
#target_link_libraries(${PROJECT_NAME} ICLQt)
#endif()
target_link_libraries(${PROJECT_NAME} ${ICLQuick_dependencies})

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
