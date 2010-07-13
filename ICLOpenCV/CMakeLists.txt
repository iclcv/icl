project(ICLOpenCV)

set(CONDITIONLIST "")
#sources for lib
set(LIB_SOURCES "")
add_libsource (${PROJECT_NAME} "src/OpenCV.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST "BUILD_EXAMPLES")
#do not link dummy-apps!!!
set(ICLLibsToLinkAgainst "")
add_example (${PROJECT_NAME} "dummy-app" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#linker
target_link_libraries(${PROJECT_NAME} ${ICLOpenCV_dependencies})

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
