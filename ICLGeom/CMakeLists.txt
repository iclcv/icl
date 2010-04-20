project(ICLGeom)

#sources for lib
set(LIB_SOURCES
	"src/Camera.cpp"
	"src/ConvexHullMonotoneChain.cpp"
	"src/Scene.cpp"
	"src/SceneObject.cpp"
	"src/PlaneEquation.cpp"
	"src/ViewRay.cpp"
)

if (HAVE_Qt_COND)
#add_executable(icl-geom-demo examples/geom-demo.cpp)
#target_link_libraries(icl-geom-demo ${PROJECT_NAME} ICLQt)

#add_executable(icl-geom-demo-simple examples/geom-demo-simple.cpp)
#target_link_libraries(icl-geom-demo-simple ${PROJECT_NAME} ICLQt)

#add_executable(icl-cam-calib examples/cam-calib.cpp)
#target_link_libraries(icl-cam-calib ${PROJECT_NAME} ICLQt)

#add_executable(icl-show-extrinsic-calibration-grid examples/show-extrinsic-calibration-grid.cpp)
#target_link_libraries(icl-show-extrinsic-calibration-grid ${PROJECT_NAME} ICLQt)

#add_executable(icl-swiss-ranger-demo examples/swiss-ranger-demo.cpp)
#target_link_libraries(icl-swiss-ranger-demo ${PROJECT_NAME} ICLQt)

if (HAVE_IPP_COND)
#add_executable(icl-intrinsic-camera-calibration examples/intrinsic-camera-calibration.cpp)
#target_link_libraries(icl-intrinsic-camera-calibration ${PROJECT_NAME} ICLQt)
endif()
endif()


#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})

set(LIBRARY_OUTPUT_PATH lib CACHE PATH "Output directory for libraries" )

#linker
if(HAVE_LIBZ_COND)
target_link_libraries(${PROJECT_NAME} z)
endif()
if(HAVE_LIBJPEG_COND)
target_link_libraries(${PROJECT_NAME} jpeg)
endif()
if(HAVE_LIBMESASR_COND)
target_link_libraries(${PROJECT_NAME} mesasr)
endif()
if(HAVE_IMAGEMAGICK_COND)
target_link_libraries(${PROJECT_NAME} Magick++)
endif()
if(HAVE_VIDEODEV_COND)
#already inside linuxkernel
#target_link_libraries(${PROJECT_NAME} )
endif()
if(HAVE_LIBDC_COND)
target_link_libraries(${PROJECT_NAME} dc1394)
endif()
if(HAVE_XINE_COND)
target_link_libraries(${PROJECT_NAME} xine)
endif()
if(HAVE_UNICAP_COND)
target_link_libraries(${PROJECT_NAME} unicap rt)
endif()
if(HAVE_XCF_COND)
target_link_libraries(${PROJECT_NAME} xcf xqilla log4cxx Memory xmltio Ice IceUtil xerces-c)
endif()
if(HAVE_IPP_COND)
target_link_libraries(${PROJECT_NAME} ippcore ippi ipps ippsr ippcv ippm ippcc guide iomp5)
endif()
target_link_libraries(${PROJECT_NAME} pthread)
if(HAVE_MKL_COND)
target_link_libraries(${PROJECT_NAME} mkl_intel mkl_intel_thread mkl_core iomp5)
endif()
if(HAVE_OPENGL_COND)
target_link_libraries(${PROJECT_NAME} GL GLU)
endif()
if(HAVE_Qt_COND)
target_link_libraries(${PROJECT_NAME} QtCore QtGui QtOpenGL)
endif()

target_link_libraries(${PROJECT_NAME} ICLQuick ICLQt ICLBlob ICLIO ICLFilter ICLCC ICLCore ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")

set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME}  
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
if(HAVE_Qt_COND)
install (TARGETS  #icl-geom-demo 
#icl-geom-demo-simple 
#icl-cam-calib 
	#icl-show-extrinsic-calibration-grid 
#icl-swiss-ranger-demo
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
if(HAVE_IPP_COND)
install (TARGETS #icl-intrinsic-camera-calibration 
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
endif()
endif()
