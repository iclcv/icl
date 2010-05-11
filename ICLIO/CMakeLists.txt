project(ICLIO)

#sources for lib
set(LIB_SOURCES 
	"src/DemoImageFlowers.cpp"
	"src/DemoImageMacaw.cpp"
	"src/DemoImageWindows.cpp"
	"src/DemoImageLena.cpp"
	"src/DemoImageCameraMan.cpp"
	"src/DemoImageMandril.cpp"
	"src/File.cpp"
	"src/FileGrabber.cpp"
	"src/CreateGrabber.cpp"
	"src/FileGrabberPluginCSV.cpp"
	"src/FileGrabberPluginPNM.cpp"
	"src/FileList.cpp"
	"src/FilenameGenerator.cpp"
	"src/FileWriter.cpp"
	"src/FileWriterPluginCSV.cpp"
	"src/FileWriterPluginPNM.cpp"
	"src/GenericGrabber.cpp"
	"src/Grabber.cpp"
	"src/IOFunctions.cpp"
	"src/IOUtils.cpp"
	"src/TestImages.cpp"
	"src/DemoGrabber.cpp"
	"src/DistFromProgArgUtil.cpp"
)

if (HAVE_LIBMESASR_COND)
set(LIB_SOURCES_MESASR
	"src/SwissRangerGrabber.cpp"
)
endif()

if (HAVE_LIBJPEG_COND)
set(LIB_SOURCES_JPEG
	"src/JPEGHandle.cpp"
	"src/FileGrabberPluginJPEG.cpp"
	"src/FileWriterPluginJPEG.cpp"
	"src/JPEGDecoder.cpp"
)
endif()

if (HAVE_IMAGEMAGICK_COND)
set(LIB_SOURCES_MAGICK
	"src/FileWriterPluginImageMagick.cpp"
	"src/FileGrabberPluginImageMagick.cpp"
)
endif()

if (SYSTEM_LINUX_COND)
if (HAVE_VIDEODEV_COND)
set(LIB_SOURCES_VIDEODEV
	"src/PWCGrabber.cpp"
)
endif()
endif()

if (HAVE_LIBDC_COND)
set(LIB_SOURCES_DC
	"src/DC.cpp"
	"src/DCDevice.cpp"
	"src/DCDeviceFeatures.cpp"
	"src/DCFrameQueue.cpp"
	"src/DCGrabber.cpp"
	"src/DCGrabberThread.cpp"
)
endif()

if (HAVE_UNICAP_COND)
if (SYSTEM_LINUX_COND)
set(LIB_SOURCES_UNICAP
	"src/UnicapConvertEngine.cpp"
	"src/DefaultConvertEngine.cpp"
	"src/DefaultGrabEngine.cpp"
	"src/UnicapDevice.cpp"
	"src/UnicapFormat.cpp"
	"src/UnicapGrabber.cpp"
	"src/UnicapProperty.cpp"
)
endif()
endif()

if (HAVE_XINE_COND)
set(LIB_SOURCES_XINE
	"src/VideoGrabber.cpp"
)
endif()

if (HAVE_XCF_COND)
set(LIB_SOURCES_XCF
	"src/XCFGrabberBase.cpp"
	"src/XCFMemoryGrabber.cpp"
	"src/XCFPublisher.cpp"
	"src/XCFPublisherGrabber.cpp"
	"src/XCFServerGrabber.cpp"
	"src/XCFUtils.cpp"
	"src/XCFMemoryListener.cpp"
)
endif()
if (HAVE_OPENCV_COND)
set(LIB_SOURCES_OPENCV
	"src/OpenCVCamGrabber.cpp"
	"src/OpenCVVideoWriter.cpp"
	"src/OpenCVVideoGrabber.cpp"
)
endif()
set(LIB_SOURCES "${LIB_SOURCES};${LIB_SOURCES_MESASR};${LIB_SOURCES_JPEG};${LIB_SOURCES_MAGICK};${LIB_SOURCES_VIDEODEV};${LIB_SOURCES_DC};${LIB_SOURCES_UNICAP};${LIB_SOURCES_XINE};${LIB_SOURCES_XCF};${LIB_SOURCES_OPENCV}")
if(BUILD_EXAMPLES)
add_executable(icl-write-file-example examples/write-file-example.cpp)
target_link_libraries(icl-write-file-example ${PROJECT_NAME} ICLQuick)

add_executable(icl-testimages-example examples/testimages-example.cpp)
target_link_libraries(icl-testimages-example ${PROJECT_NAME} ICLQuick)

add_executable(icl-jpg2cpp examples/jpg2cpp.cpp)
target_link_libraries(icl-jpg2cpp ${PROJECT_NAME})

add_executable(icl-convert examples/convert.cpp)
target_link_libraries(icl-convert ${PROJECT_NAME})

add_executable(icl-create examples/create.cpp)
target_link_libraries(icl-create ${PROJECT_NAME} ICLQuick)

if (HAVE_LIBDC_COND)
add_executable(icl-dcclearisochannels examples/dcclearisochannels.cpp)
target_link_libraries(icl-dcclearisochannels ${PROJECT_NAME})

add_executable(icl-dcdeviceinfo examples/dcdeviceinfo.cpp)
target_link_libraries(icl-dcdeviceinfo ${PROJECT_NAME})

add_executable(icl-camera-param-io examples/camera-param-io.cpp)
target_link_libraries(icl-camera-param-io ${PROJECT_NAME})

add_executable(icl-reset-dc-bus examples/reset-dc-bus.cpp)
target_link_libraries(icl-reset-dc-bus ${PROJECT_NAME})

if (HAVE_Qt_COND)
add_executable(icl-multi-cam-example examples/multi-cam-example.cpp)
target_link_libraries(icl-multi-cam-example ${PROJECT_NAME} ICLQuick)
endif()
endif()

if (HAVE_XCF_COND)
add_executable(icl-xcf-grabber-benchmark examples/xcf-grabber-benchmark.cpp)
target_link_libraries(icl-xcf-grabber-benchmark ${PROJECT_NAME})

add_executable(icl-memory-listener examples/memory-listener.cpp)
target_link_libraries(icl-memory-listener ${PROJECT_NAME})

if (HAVE_Qt_COND)
add_executable(icl-xcf-publisher examples/xcf-publisher.cpp)
target_link_libraries(icl-xcf-publisher ${PROJECT_NAME} ICLQuick ICLUtils ICLFilter)
endif()
endif()

if (HAVE_UNICAP_COND)
add_executable(icl-unicap-grab-example examples/unicap-grab-example.cpp)
target_link_libraries(icl-unicap-grab-example ${PROJECT_NAME})
endif()

if (HAVE_Qt_COND)
add_executable(icl-generic-grab-example examples/generic-grab-example.cpp)
target_link_libraries(icl-generic-grab-example ${PROJECT_NAME} ICLQuick)
if(HAVE_OPENCV_COND)
add_executable(icl-opencv-camgrabber-example examples/opencv-camgrabber-example.cpp)
target_link_libraries(icl-opencv-camgrabber-example ${PROJECT_NAME} ICLQuick ICLUtils)

add_executable(icl-opencv-videowriter-example examples/opencv-videowriter-example.cpp)
target_link_libraries(icl-opencv-videowriter-example ${PROJECT_NAME} ICLQuick ICLUtils)

add_executable(icl-opencv-videograbber-example examples/opencv-videograbber-example.cpp)
target_link_libraries(icl-opencv-videograbber-example ${PROJECT_NAME} ICLQuick ICLIO)
endif()
endif()

if (HAVE_XINE_COND)
add_executable(icl-video-player examples/video-player.cpp)
target_link_libraries(icl-video-player ${PROJECT_NAME} ICLQuick)
endif()
endif()
#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
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
#already include in linuxkernel
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
if(HAVE_OPENCV_COND)
target_link_libraries(${PROJECT_NAME} ICLOpenCV)
endif()
target_link_libraries(${PROJECT_NAME} ICLFilter ICLCC ICLCore ICLUtils)
if(BUILD_STATIC_LIBS)
add_library("${PROJECT_NAME}_static" STATIC ${LIB_SOURCES})
set_target_properties("${PROJECT_NAME}_static" PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
if(HAVE_LIBZ_COND)
target_link_libraries("${PROJECT_NAME}_static" z)
endif()
if(HAVE_LIBJPEG_COND)
target_link_libraries(${PROJECT_NAME} jpeg)
endif()
if(HAVE_LIBMESASR_COND)
target_link_libraries("${PROJECT_NAME}_static" mesasr)
endif()
if(HAVE_IMAGEMAGICK_COND)
target_link_libraries("${PROJECT_NAME}_static" Magick++)
endif()
if(HAVE_VIDEODEV_COND)
#already include in linuxkernel
#target_link_libraries(${PROJECT_NAME} )
endif()
if(HAVE_LIBDC_COND)
target_link_libraries("${PROJECT_NAME}_static" dc1394)
endif()
if(HAVE_XINE_COND)
target_link_libraries("${PROJECT_NAME}_static" xine)
endif()
if(HAVE_UNICAP_COND)
target_link_libraries("${PROJECT_NAME}_static" unicap rt)
endif()
if(HAVE_XCF_COND)
target_link_libraries("${PROJECT_NAME}_static" xcf xqilla log4cxx Memory xmltio Ice IceUtil xerces-c)
endif()
if(HAVE_OPENCV_COND)
target_link_libraries("${PROJECT_NAME}_static" ICLOpenCV)
endif()
target_link_libraries("${PROJECT_NAME}_static" ICLFilter ICLCC ICLCore ICLUtils)
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
install(TARGETS icl-write-file-example
		icl-testimages-example icl-jpg2cpp icl-convert icl-create
	 RUNTIME DESTINATION bin
)
if (HAVE_LIBDC_COND)
install (TARGETS icl-dcclearisochannels icl-dcdeviceinfo
		icl-camera-param-io icl-reset-dc-bus
	RUNTIME DESTINATION bin
)
if (HAVE_Qt_COND)
install (TARGETS icl-multi-cam-example
	RUNTIME DESTINATION bin
)
endif()
endif()

if (HAVE_XCF_COND)
install (TARGETS icl-xcf-grabber-benchmark icl-memory-listener
	RUNTIME DESTINATION bin
)

if (HAVE_Qt_COND)
install (TARGETS icl-xcf-publisher
	RUNTIME DESTINATION bin
)
endif()
endif()

if (HAVE_UNICAP_COND)
install (TARGETS icl-unicap-grab-example
	RUNTIME DESTINATION bin
)
endif()

if (HAVE_Qt_COND)
install (TARGETS icl-generic-grab-example
	RUNTIME DESTINATION bin
)
if (HAVE_OPENCV_COND)
install (TARGETS icl-opencv-camgrabber-example icl-opencv-videowriter-example 
	icl-opencv-videograbber-example
	RUNTIME DESTINATION bin
)
endif()
endif()

if (HAVE_XINE_COND)
install (TARGETS icl-video-player
	RUNTIME DESTINATION bin
)
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
