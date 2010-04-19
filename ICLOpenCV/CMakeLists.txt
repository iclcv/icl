project(ICLOpenCV)

#sources for lib
set(LIB_SOURCES 
	"src/OpenCV.cpp"
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
target_link_libraries(${PROJECT_NAME} ICLCore)
if(HAVE_OPENCV_COND)
target_link_libraries(${PROJECT_NAME} cxcore cv highgui cvaux ml)
endif()
target_link_libraries(${PROJECT_NAME} ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")

#add_executable(icl-dummy-app examples/dummy-app.cpp)
#target_link_libraries(icl-dummy-app ${PROJECT_NAME})

set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME} #icl-dummy-app
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
