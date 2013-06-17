#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLIO/CMakeLists.txt                                   **
#** Module : ICLIO                                                  **
#** Authors: Christian Groszewski                                   **
#**                                                                 **
#**                                                                 **
#** GNU LESSER GENERAL PUBLIC LICENSE                               **
#** This file may be used under the terms of the GNU Lesser General **
#** Public License version 3.0 as published by the                  **
#**                                                                 **
#** Free Software Foundation and appearing in the file LICENSE.LGPL **
#** included in the packaging of this file.  Please review the      **
#** following information to ensure the license requirements will   **
#** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
#**                                                                 **
#** The development of this software was supported by the           **
#** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
#** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
#** Forschungsgemeinschaft (DFG) in the context of the German       **
#** Excellence Initiative.                                          **
#**                                                                 **
#*********************************************************************

SET(SOURCES src/ICLIO/DemoImageFlowers.cpp
            src/ICLIO/DemoImageMacaw.cpp
	    src/ICLIO/DemoImageWindows.cpp
	    src/ICLIO/DemoImageLena.cpp
	    src/ICLIO/DemoImageCameraMan.cpp
	    src/ICLIO/DemoImageMandril.cpp
	    src/ICLIO/FileGrabber.cpp
	    src/ICLIO/CreateGrabber.cpp
	    src/ICLIO/FileGrabberPluginCSV.cpp
	    src/ICLIO/FileGrabberPluginBICL.cpp
	    src/ICLIO/FileGrabberPluginPNM.cpp
	    src/ICLIO/FileList.cpp
	    src/ICLIO/FilenameGenerator.cpp
	    src/ICLIO/FileWriter.cpp
	    src/ICLIO/FileWriterPluginCSV.cpp
	    src/ICLIO/FileWriterPluginPNM.cpp
	    src/ICLIO/FileWriterPluginBICL.cpp
	    src/ICLIO/GenericGrabber.cpp
	    src/ICLIO/Grabber.cpp
	    src/ICLIO/ImageUndistortion.cpp
	    src/ICLIO/IOFunctions.cpp
	    src/ICLIO/TestImages.cpp
	    src/ICLIO/DemoGrabber.cpp
	    src/ICLIO/GenericImageOutput.cpp
	    src/ICLIO/IntrinsicCalibrator.cpp
	    src/ICLIO/ColorFormatDecoder.cpp
	    src/ICLIO/ImageCompressor.cpp
	    src/ICLIO/MyrmexDecoder.cpp
	    src/ICLIO/JPEGHandle.cpp
	    src/ICLIO/FileGrabberPluginJPEG.cpp
	    src/ICLIO/FileWriterPluginJPEG.cpp
	    src/ICLIO/JPEGDecoder.cpp
	    src/ICLIO/JPEGEncoder.cpp
	    src/ICLIO/FileGrabberPluginPNG.cpp
	    src/ICLIO/FileWriterPluginPNG.cpp)

SET(HEADERS src/ICLIO/FileGrabber.h
	    src/ICLIO/CreateGrabber.h
	    src/ICLIO/FileGrabberPlugin.h
	    src/ICLIO/FileGrabberPluginCSV.h
	    src/ICLIO/FileGrabberPluginBICL.h
	    src/ICLIO/FileGrabberPluginPNM.h
	    src/ICLIO/FileList.h
	    src/ICLIO/FilenameGenerator.h
	    src/ICLIO/FileWriter.h
	    src/ICLIO/FileWriterPlugin.h
	    src/ICLIO/FileWriterPluginCSV.h
	    src/ICLIO/FileWriterPluginPNM.h
	    src/ICLIO/FileWriterPluginBICL.h
	    src/ICLIO/GenericGrabber.h
	    src/ICLIO/Grabber.h
	    src/ICLIO/GrabberDeviceDescription.h
	    src/ICLIO/ImageOutput.h
	    src/ICLIO/ImageUndistortion.h
	    src/ICLIO/IO.h
	    src/ICLIO/IOFunctions.h
	    src/ICLIO/TestImages.h
	    src/ICLIO/DemoGrabber.h
	    src/ICLIO/GenericImageOutput.h
	    src/ICLIO/IntrinsicCalibrator.h
	    src/ICLIO/ColorFormatDecoder.h
	    src/ICLIO/ImageCompressor.h
	    src/ICLIO/MyrmexDecoder.h
	    src/ICLIO/JPEGHandle.h
	    src/ICLIO/FileGrabberPluginJPEG.h
	    src/ICLIO/FileWriterPluginJPEG.h
	    src/ICLIO/JPEGDecoder.h
	    src/ICLIO/JPEGEncoder.h
	    src/ICLIO/FourCC.h
	    src/ICLIO/FileGrabberPluginPNG.h
	    src/ICLIO/FileWriterPluginPNG.h)

IF(MESASR_FOUND)
  LIST(APPEND SOURCES src/ICLIO/SwissRangerGrabber.cpp)
  LIST(APPEND HEADERS src/ICLIO/SwissRangerGrabber.h)
ENDIF()

IF(ImageMagick_FOUND)
  LIST(APPEND SOURCES src/ICLIO/FileWriterPluginImageMagick.cpp
    src/ICLIO/FileGrabberPluginImageMagick.cpp)
  LIST(APPEND HEADERS src/ICLIO/FileWriterPluginImageMagick.h
    src/ICLIO/FileGrabberPluginImageMagick.h)
ENDIF()

IF(LIBDC_FOUND)
  LIST(APPEND SOURCES src/ICLIO/DC.cpp
    src/ICLIO/DCDevice.cpp
    src/ICLIO/DCDeviceFeatures.cpp
    src/ICLIO/DCFrameQueue.cpp
    src/ICLIO/DCGrabber.cpp
    src/ICLIO/DCGrabberThread.cpp)

  LIST(APPEND HEADERS src/ICLIO/DC.h
    src/ICLIO/DCDevice.h
    src/ICLIO/DCDeviceFeatures.h
    src/ICLIO/DCDeviceOptions.h
    src/ICLIO/DCFrameQueue.h
    src/ICLIO/DCGrabber.h
    src/ICLIO/DCGrabberThread.h)
ENDIF()

IF(V4L_FOUND)
  LIST(APPEND SOURCES src/ICLIO/V4L2Grabber.cpp)
  LIST(APPEND HEADERS src/ICLIO/V4L2Grabber.h)
ENDIF()

IF(OpenCV_FOUND)
  LIST(APPEND SOURCES src/ICLIO/OpenCVCamGrabber.cpp
                      src/ICLIO/OpenCVVideoWriter.cpp
		      src/ICLIO/OpenCVVideoGrabber.cpp)
  LIST(APPEND HEADERS src/ICLIO/OpenCVCamGrabber.h
                      src/ICLIO/OpenCVVideoWriter.h
		      src/ICLIO/OpenCVVideoGrabber.h)
ENDIF()
 
IF(QT4_FOUND)
  LIST(APPEND SOURCES src/ICLIO/SharedMemoryGrabber.cpp
                      src/ICLIO/SharedMemoryPublisher.cpp
                      src/ICLIO/SharedMemorySegment.cpp)
                      
  LIST(APPEND HEADERS src/ICLIO/SharedMemoryGrabber.h
                      src/ICLIO/SharedMemoryPublisher.h
                      src/ICLIO/SharedMemorySegment.h)
ENDIF()

IF(XINE_FOUND)
  LIST(APPEND SOURCES src/ICLIO/VideoGrabber.cpp)
  LIST(APPEND HEADERS src/ICLIO/VideoGrabber.h)
ENDIF()

IF(ZMQ_FOUND)
  LIST(APPEND SOURCES src/ICLIO/ZmqGrabber.cpp
                      src/ICLIO/ZmqImageOutput.cpp)
  LIST(APPEND HEADERS src/ICLIO/ZmqGrabber.h
                      src/ICLIO/ZmqImageOutput.h)
ENDIF()

IF(LIBFREENECT_FOUND)
  LIST(APPEND SOURCES src/ICLIO/KinectGrabber.cpp)
  LIST(APPEND HEADERS src/ICLIO/KinectGrabber.h)
ENDIF()

IF(PYLON_FOUND)
  LIST(APPEND SOURCES src/ICLIO/PylonGrabber.cpp
                      src/ICLIO/PylonGrabberThread.cpp
		      src/ICLIO/PylonCameraOptions.cpp
		      src/ICLIO/PylonColorConverter.cpp
		      src/ICLIO/PylonUtils.cpp)

  LIST(APPEND HEADERS src/ICLIO/PylonGrabber.h
                      src/ICLIO/PylonGrabberThread.h
		      src/ICLIO/PylonCameraOptions.h
		      src/ICLIO/PylonColorConverter.h
		      src/ICLIO/PylonUtils.h
		      src/ICLIO/PylonIncludes.h)
ENDIF()

IF(OPENNI_FOUND)
  LIST(APPEND SOURCES src/ICLIO/OpenNIGrabber.cpp
                      src/ICLIO/OpenNIUtils.cpp)
		      
  LIST(APPEND HEADERS src/ICLIO/OpenNIGrabber.h
                      src/ICLIO/OpenNIIncludes.h
		      src/ICLIO/OpenNIUtils.h)

ENDIF()

IF(RSB_FOUND)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
  PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS src/ICLIO/proto/RSBImage.proto)

  # the generated files happen to be in build/ICLIO directly, but here, the header file
  # cannot be found by dependent files such as RSBGrabber.cpp. Therefore, we copy the
  # RSBImage.pb.h to a new folder build/ICLIO/protobuf/ICLIO and add a -Ibuild/ICLIO/protobuf
  # which allows RSBGrabber.cpp to find ICLIO/RSBImage.bp.h
  # Update:
  # The former copy and create-dir command did not work, because it is executed during
  # CMake's configure step and not during the compile step. The new fix is to add
  # build directly so that ICLIO/RSBImage.pb.h can be found as well

  INCLUDE_DIRECTORIES(BEFORE ${CMAKE_BINARY_DIR})

  LIST(APPEND SOURCES src/ICLIO/RSBGrabber.cpp
                      src/ICLIO/RSBImageOutput.cpp
		      ${PROTO_SRCS})

  LIST(APPEND HEADERS src/ICLIO/RSBGrabber.h
                      src/ICLIO/RSBImageOutput.h
		      ${PROTO_HDRS})

ENDIF()

# ---- Library build instructions ----
INCLUDE_DIRECTORIES(BEFORE src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src
                           ${ICL_SOURCE_DIR}/ICLFilter/src)

IF(QT_FOUND)
  ADD_LIBRARY(ICLIO SHARED ${SOURCES} ${HEADERS})
ELSE()
  ADD_LIBRARY(ICLIO SHARED ${SOURCES} ${HEADERS})
ENDIF()


# ---- 3rd party library handling ----
TARGET_LINK_LIBRARIES(ICLIO ICLFilter
                            ${MESASR_LIBRARIES}
			    ${ImageMagick_LIBRARIES}
			    ${LIBDC_LIBRARIES}
			    ${V4L_LIBRARIES}
			    ${XINE_LIBRARIES}
			    ${OPENGL_LIBRARIES}
			    ${LIBFREENECT_LIBRARIES}
			    ${PYLON_LIBRARIES}
			    ${OPENNI_LIBRARIES}
			    ${QT_LIBRARIES}
			    ${RSB_LIBRARIES}
			    ${ZMQ_LIBRARIES}
			    ${PROTOBUF_LIBRARIES}
			    ${OpenCV_LIBS})

# Wo sind die hier :-? (UNICAP XCF QT GENICAM OPENNI RSB PROTOBUF)


SET_TARGET_PROPERTIES(ICLIO
                      PROPERTIES 
		      VERSION ${SO_VERSION})

# ---- Build examples/ demos/ apps ----
IF(BUILD_DEMOS)
  ADD_SUBDIRECTORY(demos)
ENDIF()

IF(BUILD_APPS)
  ADD_SUBDIRECTORY(apps)
ENDIF()

# ---- Install specifications ----
INSTALL(TARGETS ICLIO
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS)

# ---- PKG-config ----
CREATE_PKGCONFIG(NAME ICLIO
                 LIBRARY_DEPS ${MESASR_LIBRARIES}
			      ${ImageMagick_LIBRARIES}
			      ${LIBDC_LIBRARIES}
			      ${V4L_LIBRARIES}
			      ${XINE_LIBRARIES}
			      ${OPENGL_LIBRARIES}
			      ${LIBFREENECT_LIBRARIES}
			      ${PYLON_LIBRARIES}
			      ${OPENNI_LIBRARIES}
			      ${QT_LIBRARIES}
			      ${ZMQ_LIBRARIES}
			      ${RSB_RSC_AND_PROTOBUF_LIBRARIES_PC}
                 PKGCONFIG_DEPS ICLFilter-${SO_VERSION})
