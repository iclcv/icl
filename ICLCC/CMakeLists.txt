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

#add_executable(icl-bayer-benchmark examples/bayer-benchmark.cpp)
#target_link_libraries(icl-bayer-benchmark ICLIO ICLOpenCV ICLFilter ${PROJECT_NAME} ICLCore ICLUtils)#ICLQuick ICLQt  

#add_executable(icl-converter-test examples/converter-test.cpp)
#target_link_libraries(icl-converter-test  ICLQuick ICLQt ICLIO ${PROJECT_NAME} ICLFilter ICLUtils)

#add_executable(icl-colorspace-online-demo examples/colorspace-online-demo.cpp)
#target_link_libraries(icl-colorspace-online-demo ICLQuick ICLQt ICLIO ICLOpenCV ${PROJECT_NAME} ICLCore ICLUtils)

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
target_link_libraries(${PROJECT_NAME} ICLCore ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")



set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME}  #icl-colorspace-online-demo #icl-converter-test icl-bayer-benchmark  
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
