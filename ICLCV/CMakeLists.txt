#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLCV/CMakeLists.txt                                   **
#** Module : ICLCV                                                  **
#** Authors: Michael Goetting                                       **
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

SET(SOURCES src/ICLCV/CornerDetectorCSS.cpp
            src/ICLCV/CV.cpp
            src/ICLCV/Extrapolator.cpp
            src/ICLCV/FloodFiller.cpp
            src/ICLCV/HoughLine.cpp
            src/ICLCV/HoughLineDetector.cpp
            src/ICLCV/HungarianAlgorithm.cpp
            src/ICLCV/ImageRegion.cpp
            src/ICLCV/ImageRegionData.cpp
            src/ICLCV/MeanShiftTracker.cpp
            src/ICLCV/PositionTracker.cpp
            src/ICLCV/RegionDetector.cpp
            src/ICLCV/RegionPCAInfo.cpp
            src/ICLCV/RunLengthEncoder.cpp
            src/ICLCV/SimpleBlobSearcher.cpp
            src/ICLCV/SurfFeature.cpp
            src/ICLCV/SurfFeatureDetector.cpp
            src/ICLCV/VectorTracker.cpp
            src/ICLCV/ContourDetector.cpp
            src/ICLCV/CurvatureExtractor.cpp
            src/ICLCV/RDPApproximation.cpp)




SET(HEADERS src/ICLCV/CornerDetectorCSS.h
            src/ICLCV/CV.h
            src/ICLCV/Extrapolator.h
            src/ICLCV/FloodFiller.h
            src/ICLCV/HoughLine.h
            src/ICLCV/HoughLineDetector.h
            src/ICLCV/HungarianAlgorithm.h
            src/ICLCV/ImageRegionData.h
            src/ICLCV/ImageRegion.h
            src/ICLCV/ImageRegionPart.h
            src/ICLCV/LineSegment.h
            src/ICLCV/MeanShiftTracker.h
            src/ICLCV/PositionTracker.h
            src/ICLCV/QuickDocumentation.h
            src/ICLCV/RegionGrower.h
            src/ICLCV/RegionDetector.h
            src/ICLCV/RegionDetectorTools.h
            src/ICLCV/RegionPCAInfo.h
            src/ICLCV/RunLengthEncoder.h
            src/ICLCV/SimpleBlobSearcher.h
            src/ICLCV/VectorTracker.h
            src/ICLCV/SurfFeature.h
            src/ICLCV/SurfFeatureDetector.h
            src/ICLCV/WorkingLineSegment.h
            src/ICLCV/ContourDetector.h
            src/ICLCV/CurvatureExtractor.h
            src/ICLCV/RDPApproximation.h)

IF (QT_FOUND)
  LIST(APPEND HEADERS src/ICLCV/HeartrateDetector.h)
  LIST(APPEND SOURCES src/ICLCV/HeartrateDetector.cpp)
ENDIF()

IF(IPP_FOUND)
  LIST(APPEND SOURCES src/ICLCV/ViewBasedTemplateMatcher.cpp
                      src/ICLCV/TemplateTracker.cpp)

  LIST(APPEND HEADERS src/ICLCV/ViewBasedTemplateMatcher.h
                      src/ICLCV/TemplateTracker.h)
ENDIF()

IF(OPENCV_FOUND)
  LIST(APPEND SOURCES src/ICLCV/OpenSurfLib.cpp
                      src/ICLCV/LensUndistortionCalibrator.cpp
                      src/ICLCV/OpenCVCamCalib.cpp
                      src/ICLCV/CheckerboardDetector.cpp)

  LIST(APPEND HEADERS src/ICLCV/OpenSurfLib.h
                      src/ICLCV/LensUndistortionCalibrator.h
                      src/ICLCV/OpenCVCamCalib.h
                      src/ICLCV/CheckerboardDetector.h)

  IF(OPENCV_FEATURES_2D_FOUND)
      LIST(APPEND SOURCES src/ICLCV/ORBFeatureDetector.cpp)
      LIST(APPEND HEADERS src/ICLCV/ORBFeatureDetector.h)
  ENDIF()

ENDIF()


IF(OPENCL_FOUND)
  LIST(APPEND SOURCES src/ICLCV/CLSurfLib.cpp
                      src/ICLCV/CLSurfLibKernels.cpp)
  LIST(APPEND HEADERS src/ICLCV/CLSurfLib.h
                      src/ICLCV/CLSurfLibKernels.h)
ENDIF()

# ---- Library build instructions ----
IF(WIN32)
INCLUDE_DIRECTORIES(BEFORE src
                           ${ICL_SOURCE_DIR}/3rdparty/zlib
                           ${ICL_SOURCE_DIR}/3rdparty/libpng
                           ${ICL_SOURCE_DIR}/3rdparty/libjpeg
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src
                           ${ICL_SOURCE_DIR}/ICLFilter/src
                           ${ICL_SOURCE_DIR}/ICLIO/src
                           ${ICL_SOURCE_DIR}/ICLQt/src)
ELSE()
INCLUDE_DIRECTORIES(BEFORE src
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src
                           ${ICL_SOURCE_DIR}/ICLFilter/src
                           ${ICL_SOURCE_DIR}/ICLIO/src
                           ${ICL_SOURCE_DIR}/ICLQt/src)
ENDIF()

ADD_LIBRARY(ICLCV SHARED ${SOURCES} ${HEADERS})

# ---- 3rd party library handling ----
TARGET_LINK_LIBRARIES(ICLCV ICLIO)

SET_TARGET_PROPERTIES(ICLCV
                      PROPERTIES
                      VERSION ${SO_VERSION})

# ---- Build examples/ demos/ apps ----
#IF(BUILD_EXAMPLES) no examples yet
#  ADD_SUBDIRECTORY(examples)
#ENDIF()

IF(BUILD_DEMOS)
  ADD_SUBDIRECTORY(demos)
ENDIF()

IF(BUILD_APPS)
  ADD_SUBDIRECTORY(apps)
ENDIF()

# ---- Install specifications ----
INSTALL(TARGETS ICLCV
        COMPONENT libraries
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS)

# ---- PKG-config ----
CREATE_PKGCONFIG(NAME ICLCV
                 PKGCONFIG_DEPS ICLIO-${SO_VERSION})

IF(BUILD_TESTS)
  CONFIGURE_GTEST(ICLCV ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(BUILD_TESTS)
