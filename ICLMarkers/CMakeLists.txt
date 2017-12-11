#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLGeom/CMakeLists.txt                                 **
#** Module : ICLGeom                                                **
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

SET(SOURCES src/ICLMarkers/BCHCode.cpp
            src/ICLMarkers/Fiducial.cpp
	    src/ICLMarkers/FiducialDetector.cpp
	    src/ICLMarkers/FiducialDetectorPlugin.cpp
	    src/ICLMarkers/FiducialDetectorPluginART.cpp
	    src/ICLMarkers/FiducialDetectorPluginAmoeba.cpp
	    src/ICLMarkers/FiducialDetectorPluginBCH.cpp
	    src/ICLMarkers/FiducialDetectorPluginForQuads.cpp
	    src/ICLMarkers/FiducialDetectorPluginHierarchical.cpp
	    src/ICLMarkers/FiducialDetectorPluginICL1.cpp
	    src/ICLMarkers/FiducialImpl.cpp
	    src/ICLMarkers/MarkerCodeICL1.cpp
	    src/ICLMarkers/MarkerMetricsICL1.cpp
	    src/ICLMarkers/MultiCamFiducial.cpp
	    src/ICLMarkers/MultiCamFiducialDetector.cpp
	    src/ICLMarkers/MultiCamFiducialImpl.cpp
	    src/ICLMarkers/QuadDetector.cpp
	    src/ICLMarkers/TwoLevelRegionStructure.cpp
            src/ICLMarkers/AdvancedMarkerGridDetector.cpp
            src/ICLMarkers/MarkerGridBasedUndistortionOptimizer.cpp
            src/ICLMarkers/InverseUndistortionProcessor.cpp
            src/ICLMarkers/MarkerGridDetector.cpp
            src/ICLMarkers/MarkerGridEvaluater.cpp
            src/ICLMarkers/MarkerGridPoseEstimator.cpp)

SET(HEADERS src/ICLMarkers/BCHCode.h
            src/ICLMarkers/Fiducial.h
	    src/ICLMarkers/FiducialDetector.h
	    src/ICLMarkers/FiducialDetectorPlugin.h
	    src/ICLMarkers/FiducialDetectorPluginART.h
	    src/ICLMarkers/FiducialDetectorPluginAmoeba.h
	    src/ICLMarkers/FiducialDetectorPluginBCH.h
	    src/ICLMarkers/FiducialDetectorPluginForQuads.h
	    src/ICLMarkers/FiducialDetectorPluginHierarchical.h
	    src/ICLMarkers/FiducialDetectorPluginICL1.h
	    src/ICLMarkers/FiducialImpl.h
	    src/ICLMarkers/MarkerCodeICL1.h
	    src/ICLMarkers/MarkerMetricsICL1.h
	    src/ICLMarkers/Markers.h
	    src/ICLMarkers/MultiCamFiducial.h
	    src/ICLMarkers/MultiCamFiducialDetector.h
	    src/ICLMarkers/MultiCamFiducialImpl.h
	    src/ICLMarkers/QuadDetector.h
	    src/ICLMarkers/RegionStructure.h
	    src/ICLMarkers/TiltedQuad.h
	    src/ICLMarkers/TwoLevelRegionStructure.h
            src/ICLMarkers/InverseUndistortionProcessor.h
            src/ICLMarkers/AdvancedMarkerGridDetector.h
            src/ICLMarkers/MarkerGridBasedUndistortionOptimizer.h
            src/ICLMarkers/MarkerGridEvaluater.h
            src/ICLMarkers/MarkerGridDetector.h
            src/ICLMarkers/MarkerGridPoseEstimator.h)


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
                           ${ICL_SOURCE_DIR}/ICLQt/src
                           ${ICL_SOURCE_DIR}/ICLCV/src
                           ${ICL_SOURCE_DIR}/ICLGeom/src)
ELSE()
INCLUDE_DIRECTORIES(BEFORE src
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src
                           ${ICL_SOURCE_DIR}/ICLFilter/src
                           ${ICL_SOURCE_DIR}/ICLIO/src
                           ${ICL_SOURCE_DIR}/ICLQt/src
                           ${ICL_SOURCE_DIR}/ICLCV/src
                           ${ICL_SOURCE_DIR}/ICLGeom/src)
ENDIF()

ADD_LIBRARY(ICLMarkers SHARED ${SOURCES} ${HEADERS})

# ---- 3rd party library handling ----
TARGET_LINK_LIBRARIES(ICLMarkers ICLGeom)

SET_TARGET_PROPERTIES(ICLMarkers
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
INSTALL(TARGETS ICLMarkers
        COMPONENT libraries
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS)

# ---- PKG-config ----
CREATE_PKGCONFIG(NAME ICLMarkers
                 PKGCONFIG_DEPS ICLGeom-${SO_VERSION})

IF(BUILD_TESTS)
  CONFIGURE_GTEST(ICLMarkers ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(BUILD_TESTS)
