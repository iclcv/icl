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
set(LIB_SOURCES ${LIB_SOURCES}
	"src/SwissRangerGrabber.cpp"
)
endif()

if (HAVE_LIBJPEG_COND)
set(LIB_SOURCES ${LIB_SOURCES}
	"src/JPEGHandle.cpp"
	"src/FileGrabberPluginJPEG.cpp"
	"src/FileWriterPluginJPEG.cpp"
	"src/JPEGDecoder.cpp"
)
endif()

if (HAVE_IMAGEMAGICK_COND)
set(LIB_SOURCES ${LIB_SOURCES}
	"src/FileWriterPluginImageMagick.cpp"
	"src/FileGrabberPluginImageMagick.cpp"
)
endif()

#if (SYSTEM_LINUX_COND)
if (HAVE_VIDEODEV_COND)
set(LIB_SOURCES ${LIB_SOURCES}
	"src/PWCGrabber.cpp"
)
endif()
#endif()

if (HAVE_LIBDC_COND)
set(LIB_SOURCES ${LIB_SOURCES}
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
set(LIB_SOURCES ${LIB_SOURCES}
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
set(LIB_SOURCES ${LIB_SOURCES}
	"src/VideoGrabber.cpp"
)
endif()

if (HAVE_XCF_COND)
set(LIB_SOURCES ${LIB_SOURCES}
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
set(LIB_SOURCES ${LIB_SOURCES}
	"src/OpenCVCamGrabber.cpp"
	"src/OpenCVVideoWriter.cpp"
	"src/OpenCVVideoGrabber.cpp"
)
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
if(HAVE_IPP_COND)
target_link_libraries(${PROJECT_NAME} ippcore ippi ipps ippsr ippcv ippm ippcc guide iomp5)
endif()
target_link_libraries(${PROJECT_NAME} pthread)
if(HAVE_MKL_COND)
target_link_libraries(${PROJECT_NAME} mkl_intel mkl_intel_thread mkl_core iomp5)
endif()
target_link_libraries(${PROJECT_NAME} ICLFilter ICLCC ICLCore ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")

#add_executable(icl-dummy-app examples/dummy-app.cpp)
#target_link_libraries(icl-dummy-app ${PROJECT_NAME})

set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME} #icl-dummy-app
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib
)
