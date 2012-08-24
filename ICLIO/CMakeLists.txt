#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLIO/CMakeLists.txt                                   **
#** Module : ICLIO                                                  **
#** Authors: Christian Groszewski                                   **
#**                                                                 **
#**                                                                 **
#** Commercial License                                              **
#** ICL can be used commercially, please refer to our website       **
#** www.iclcv.org for more details.                                 **
#**                                                                 **
#** GNU General Public License Usage                                **
#** Alternatively, this file may be used under the terms of the     **
#** GNU General Public License version 3.0 as published by the      **
#** Free Software Foundation and appearing in the file LICENSE.GPL  **
#** included in the packaging of this file.  Please review the      **
#** following information to ensure the GNU General Public License  **
#** version 3.0 requirements will be met:                           **
#** http://www.gnu.org/copyleft/gpl.html.                           **
#**                                                                 **
#** The development of this software was supported by the           **
#** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
#** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
#** Forschungsgemeinschaft (DFG) in the context of the German       **
#** Excellence Initiative.                                          **
#**                                                                 **
#*********************************************************************
project(ICLIO)

set(CONDITIONLIST "")
#sources for lib
set(LIB_SOURCES "")
add_libsource (${PROJECT_NAME} "src/DemoImageFlowers.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DemoImageMacaw.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DemoImageWindows.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DemoImageLena.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DemoImageCameraMan.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DemoImageMandril.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/File.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/CreateGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileGrabberPluginCSV.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileGrabberPluginBICL.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileGrabberPluginPNM.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileList.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FilenameGenerator.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileWriter.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileWriterPluginCSV.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileWriterPluginPNM.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileWriterPluginBICL.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GenericGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Grabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ImageUndistortion.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/IOFunctions.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/IOUtils.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/TestImages.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DemoGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GenericImageOutput.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/IntrinsicCalibrator.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ColorFormatDecoder.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ImageCompressor.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/MyrmexDecoder.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_LIBMESASR_COND)
add_libsource (${PROJECT_NAME} "src/SwissRangerGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_LIBJPEG_COND)
add_libsource (${PROJECT_NAME} "src/JPEGHandle.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileGrabberPluginJPEG.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileWriterPluginJPEG.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/JPEGDecoder.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/JPEGEncoder.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_LIBPNG_COND)
add_libsource (${PROJECT_NAME} "src/FileGrabberPluginPNG.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileWriterPluginPNG.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_IMAGEMAGICK_COND)
add_libsource (${PROJECT_NAME} "src/FileWriterPluginImageMagick.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FileGrabberPluginImageMagick.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST SYSTEM_LINUX_COND HAVE_VIDEODEV_COND)
add_libsource (${PROJECT_NAME} "src/V4L2Grabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_LIBDC_COND)
add_libsource (${PROJECT_NAME} "src/DC.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DCDevice.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DCDeviceFeatures.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DCFrameQueue.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DCGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DCGrabberThread.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST HAVE_XINE_COND)
add_libsource (${PROJECT_NAME} "src/VideoGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_OPENCV_COND)
add_libsource (${PROJECT_NAME} "src/OpenCVCamGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/OpenCVVideoWriter.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_OPENCV_COND)
add_libsource (${PROJECT_NAME} "src/OpenCVVideoGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_QT_COND)
add_libsource (${PROJECT_NAME} "src/SharedMemoryGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/SharedMemoryPublisher.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_LIBFREENECT_COND)
add_libsource (${PROJECT_NAME} "src/KinectGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_PYLON_COND)

add_libsource (${PROJECT_NAME} "src/PylonGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/PylonGrabberThread.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/PylonCameraOptions.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/PylonColorConverter.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/PylonUtils.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_OPENNI_COND)
add_libsource (${PROJECT_NAME} "src/OpenNIGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/OpenNIUtils.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_RSB_COND HAVE_PROTOBUF_COND)
#add_libsource (${PROJECT_NAME} "src/RSBImage.pb.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/RSBImageOutput.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/RSBGrabber.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES)
set(ICLLibsToLinkAgainst ${PROJECT_NAME})
add_example (${PROJECT_NAME} "jpg2cpp" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "convert" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(ICLLibsToLinkAgainst ${PROJECT_NAME} ICLQuick)
add_example (${PROJECT_NAME} "create" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
set(CONDITIONLIST HAVE_RSB_COND HAVE_PROTOBUF_COND)
add_example (${PROJECT_NAME} "rsb-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_LIBDC_COND)
set(ICLLibsToLinkAgainst ${PROJECT_NAME})
add_example (${PROJECT_NAME} "dcclearisochannels" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "dcdeviceinfo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "camera-param-io" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "reset-dc-bus" "${CONDITIONLIST}" ICLLibsToLinkAgainst)


set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_LIBDC_COND HAVE_QT_COND)
set(ICLLibsToLinkAgainst ${PROJECT_NAME} ICLQuick)
add_example (${PROJECT_NAME} "multi-cam-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

#set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_QT_COND) should be possible even without GUI
# Christian: How to make icl-pipe not be built but not link against Qt if this is not available
set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES)
set(ICLLibsToLinkAgainst ${PROJECT_NAME} ICLQuick ICLUtils ICLFilter)
add_example (${PROJECT_NAME} "pipe" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_QT_COND)
set(ICLLibsToLinkAgainst ${PROJECT_NAME} ICLQt ICLFilter ICLUtils)
add_example (${PROJECT_NAME} "reset-sm-bus" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "undistortion-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_QT_COND HAVE_XINE_COND)
set(ICLLibsToLinkAgainst ${PROJECT_NAME} ICLQuick)
add_example (${PROJECT_NAME} "video-player" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST HAVE_GTEST_COND)
set(ICLLibsToLinkAgainst "${GTEST_LIBS_l}")
#add_gtest (${PROJECT_NAME} "test-image.cpp" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

if(UNIX AND HAVE_PROTOBUF_COND)
  set(P ${CMAKE_SOURCE_DIR}/ICLIO/src/RSBImage.proto)
  set(I ${CMAKE_SOURCE_DIR}/include/ICLIO/RSBImage.pb.h)
  set(C ${CMAKE_SOURCE_DIR}/ICLIO/src/RSBImage.pb.cpp)
  ADD_CUSTOM_COMMAND(OUTPUT ${C} ${I}
    COMMAND ${PROTOC_CMD} RSBImage.proto --cpp_out=.
    && mv "./RSBImage.pb.h" ${I}
    && mv "./RSBImage.pb.cc" ${C}
    && sed -i 's|"RSBImage.pb.h"|<ICLIO/RSBImage.pb.h>|g' ${C}
    DEPENDS ${P}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/ICLIO/src")
  set_source_files_properties(${I} ${C} PROPERTIES GENERATED TRUE)
  set(LIB_SOURCES "${LIB_SOURCES};${I};${C}")

endif()

#create lib
if(WIN32)
add_library(${PROJECT_NAME} ${LIB_SOURCES})
else()
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
endif()
#linker
target_link_libraries(${PROJECT_NAME} ${ICLIO_dependencies})

link_directories("${LIBRARY_OUTPUT_PATH}")
if(WIN32)
install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION "${LIBRARY_OUTPUT_PATH}"
	 #MINGW COMPATIBILITY 
	ARCHIVE DESTINATION "${LIBRARY_OUTPUT_PATH}"
	RUNTIME DESTINATION "${LIBRARY_OUTPUT_PATH}")
else()
install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION "${LIBRARY_OUTPUT_PATH}"
	 PERMISSIONS OWNER_WRITE OWNER_READ
	 GROUP_WRITE GROUP_READ 
	 WORLD_READ
)
endif()

install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icl-${VERSION}${DEBUG_POSTFIX}
		 FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
		 DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
		 	GROUP_EXECUTE GROUP_WRITE GROUP_READ 
		 	WORLD_READ WORLD_EXECUTE
		 FILES_MATCHING PATTERN "*.h"
		 REGEX .svn* EXCLUDE
)
add_doc_gen(${PROJECT_NAME})
