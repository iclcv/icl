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
#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})

set(LIBRARY_OUTPUT_PATH lib CACHE PATH "Output directory for libraries" )

#linker
if(HAVE_IPP_COND)
target_link_libraries(${PROJECT_NAME} ippcore ippi ipps ippsr ippcv ippm ippcc guide iomp5)
endif()
target_link_libraries(${PROJECT_NAME} pthread)
if(HAVE_MKL_COND)
target_link_libraries(${PROJECT_NAME} mkl_intel mkl_intel_thread mkl_core iomp5)
endif()
target_link_libraries(${PROJECT_NAME} ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")

#add_executable(icl-img-example examples/img-example.cpp)
#target_link_libraries(icl-img-example ${PROJECT_NAME})

#add_executable(icl-copy-channel-roi-example examples/copy-channel-roi-example.cpp)
#target_link_libraries(icl-copy-channel-roi-example ${PROJECT_NAME})

#add_executable(icl-img-iterator-benchmark examples/img-iterator-benchmark.cpp)
#target_link_libraries(icl-img-iterator-benchmark ${PROJECT_NAME})

#add_executable(icl-reduce-channels-test examples/reduce-channels-test.cpp)
#target_link_libraries(icl-reduce-channels-test ${PROJECT_NAME})

#add_executable(icl-sampled-line-test examples/sampled-line-test.cpp)
#target_link_libraries(icl-sampled-line-test ${PROJECT_NAME})

set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME} #icl-img-example icl-copy-channel-roi-example icl-img-iterator-benchmark icl-reduce-channels-test 
	#icl-sampled-line-test
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
