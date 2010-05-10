project(ICLUtils)

#sources for lib
set(LIB_SOURCES 
	"src/DynMatrix.cpp"
	"src/Point32f.cpp"
	"src/Point.cpp"
	"src/Range.cpp"
	"src/Rect32f.cpp"
	"src/Rect.cpp"
	"src/Size32f.cpp"
	"src/Size.cpp"
	"src/Exception.cpp"
	"src/FixedMatrix.cpp"
	"src/MultiThreader.cpp"
	"src/ThreadUtils.cpp"
	"src/ProgArg.cpp"
	"src/Semaphore.cpp"
	"src/SignalHandler.cpp"
	"src/SmartPtr.cpp"
	"src/StrTok.cpp"
	"src/Thread.cpp"
	"src/Time.cpp"
	"src/Timer.cpp"
	"src/ConsoleProgress.cpp"
	"src/FPSEstimator.cpp"
	"src/FPSLimiter.cpp"
	"src/Mutex.cpp"
	"src/DynMatrixUtils.cpp"
	"src/StochasticOptimizer.cpp"
	"src/StringUtils.cpp"
	"src/MultiTypeMap.cpp"
	"src/ConfigFile.cpp"
	"src/XMLAttribRef.cpp"
	"src/XML.cpp"
	"src/XMLNodeFilter.cpp"
	"src/XMLDocument.cpp"
	"src/XMLNode.cpp"
	"src/XMLNodeIterator.cpp"
	"src/StraightLine2D.cpp"
	"src/FFTUtils.cpp"
)
if(BUILD_EXAMPLES)
add_executable(icl-progargdemo examples/progargdemo.cpp)
target_link_libraries(icl-progargdemo ${PROJECT_NAME})

add_executable(icl-dyn-matrix-test examples/dyn-matrix-test.cpp)
target_link_libraries(icl-dyn-matrix-test ${PROJECT_NAME})

add_executable(icl-dyn-matrix-utils-test examples/dyn-matrix-utils-test.cpp)
target_link_libraries(icl-dyn-matrix-utils-test ${PROJECT_NAME})

add_executable(icl-fixed-matrix-test examples/fixed-matrix-test.cpp)
target_link_libraries(icl-fixed-matrix-test ${PROJECT_NAME})

add_executable(icl-matrix-stream-operator-test examples/matrix-stream-operator-test.cpp)
target_link_libraries(icl-matrix-stream-operator-test ${PROJECT_NAME})

add_executable(icl-regular-expression-test examples/regular-expression-test.cpp)
target_link_libraries(icl-regular-expression-test ${PROJECT_NAME})

add_executable(icl-xml-parser-test examples/xml-parser-test.cpp)
target_link_libraries(icl-xml-parser-test ${PROJECT_NAME})

add_executable(icl-config-file-test examples/config-file-test.cpp)
target_link_libraries(icl-config-file-test ${PROJECT_NAME})
endif()

#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
if(BUILD_STATIC_LIBS)
add_library("${PROJECT_NAME}_static" STATIC ${LIB_SOURCES})
set_target_properties("${PROJECT_NAME}_static" PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif()

#linker
if(HAVE_IPP_COND)
target_link_libraries(${PROJECT_NAME} ippcore ippi ipps ippsr ippcv ippm ippcc guide iomp5)
endif()
target_link_libraries(${PROJECT_NAME} pthread)
if(HAVE_MKL_COND)
target_link_libraries(${PROJECT_NAME} mkl_intel mkl_intel_thread mkl_core iomp5)
endif()
if(BUILD_STATIC_LIBS)
if(HAVE_IPP_COND)
target_link_libraries("${PROJECT_NAME}_static" ippcore ippi ipps ippsr ippcv ippm ippcc guide iomp5)
endif()
target_link_libraries("${PROJECT_NAME}_static" pthread)
if(HAVE_MKL_COND)
target_link_libraries("${PROJECT_NAME}_static" mkl_intel mkl_intel_thread mkl_core iomp5)
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
install(TARGETS icl-progargdemo icl-dyn-matrix-test 
		icl-dyn-matrix-utils-test icl-fixed-matrix-test 
		icl-matrix-stream-operator-test icl-regular-expression-test 
		icl-xml-parser-test icl-config-file-test
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
