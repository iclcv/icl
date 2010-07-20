project(ICLCore)

set(CONDITIONLIST "")
#sources for lib
set(LIB_SOURCES "")
add_libsource (${PROJECT_NAME} "src/CoreFunctions.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ImgBase.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ImgBorder.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Img.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ImgParams.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Line32f.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Line.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Mathematics.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Random.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/CornerDetectorCSS.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/SampledLine.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/LineSampler.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST "BUILD_EXAMPLES")
set(ICLLibsToLinkAgainst ${PROJECT_NAME})
add_example (${PROJECT_NAME} "img-example" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "copy-channel-roi-example" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "img-iterator-benchmark" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "reduce-channels-test" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "sampled-line-test" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST "HAVE_GTEST_COND")
set(ICLLibsToLinkAgainst gtest)
#add_gtest (${PROJECT_NAME} "runner" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
#add_gtest (${PROJECT_NAME} "test/test-image.cpp;test/runner.cpp" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
#ADD_CUSTOM_TARGET (check)
#ADD_CUSTOM_TARGET (check
#add_executable  test/icl-test-image test/test-image.cpp test/runner.cpp     
#COMMAND target_link_libraries  icl-test-${PRJECT_N} ${ICLLibsToLinkAgainst}
#SOURCES test/test-image.cpp test/runner.cpp
#WORKING_DIRECTORY .
#)
#ADD_CUSTOM_COMMAND(TARGET    check
#add_executable  test/icl-test-image test/test-image.cpp test/runner.cpp     
#COMMAND target_link_libraries  icl-test-${PRJECT_N} ${ICLLibsToLinkAgainst}
#)
#TARGET target
 #                    PRE_BUILD | PRE_LINK | POST_BUILD
  #                   COMMAND command1 [ARGS] [args1...]
   #                  [COMMAND command2 [ARGS] [args2...] ...]
    #                 [WORKING_DIRECTORY dir]
     #                [COMMENT comment] [VERBATIM])
#create lib

add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#linker
target_link_libraries(${PROJECT_NAME} ${ICLCore_dependencies})

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
