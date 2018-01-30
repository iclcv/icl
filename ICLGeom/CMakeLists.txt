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

SET(SOURCES src/ICLGeom/Camera.cpp
            src/ICLGeom/CoplanarPointPoseEstimator.cpp
            src/ICLGeom/ICP.cpp
            src/ICLGeom/ICP3D.cpp
            src/ICLGeom/PlaneEquation.cpp
            src/ICLGeom/PointCloudNormalEstimator.cpp
            src/ICLGeom/PoseEstimator.cpp
            src/ICLGeom/Posit.cpp
            src/ICLGeom/SoftPosit.cpp
            src/ICLGeom/RansacBasedPoseEstimator.cpp
            src/ICLGeom/ViewRay.cpp
            src/ICLGeom/ObjectEdgeDetector.cpp
            src/ICLGeom/ObjectEdgeDetectorData.cpp
            src/ICLGeom/ObjectEdgeDetectorCPU.cpp
            src/ICLGeom/ObjectEdgeDetectorGPU.cpp
            src/ICLGeom/PlanarRansacEstimator.cpp
            src/ICLGeom/SegmenterUtils.cpp
            src/ICLGeom/SurfaceFeatureExtractor.cpp
            src/ICLGeom/CutfreeAdjacencyFeatureExtractor.cpp
            src/ICLGeom/CoPlanarityFeatureExtractor.cpp
            src/ICLGeom/CurvatureFeatureExtractor.cpp
            src/ICLGeom/RemainingPointsFeatureExtractor.cpp)

SET(HEADERS src/ICLGeom/Camera.h
            src/ICLGeom/PointCloudNormalEstimator.h
            src/ICLGeom/SoftPosit.h
            src/ICLGeom/GeomDefs.h
            src/ICLGeom/PoseEstimator.h
            src/ICLGeom/ViewRay.h
            src/ICLGeom/Geom.h
            src/ICLGeom/PCLIncludes.h
            src/ICLGeom/Posit.h
            src/ICLGeom/CoordinateFrameSceneObject.h
            src/ICLGeom/CoplanarPointPoseEstimator.h
            src/ICLGeom/Hit.h
            src/ICLGeom/PlaneEquation.h
            src/ICLGeom/PrimitiveType.h
            src/ICLGeom/SceneObjectBase.h
            src/ICLGeom/ICP.h
            src/ICLGeom/ICP3D.h
            src/ICLGeom/RGBDMapping.h
            src/ICLGeom/Plot3D.h
            src/ICLGeom/PlotHandle3D.h
            src/ICLGeom/RansacBasedPoseEstimator.h
            src/ICLGeom/PointCloudOutput.h
            src/ICLGeom/ObjectEdgeDetector.h
            src/ICLGeom/ObjectEdgeDetectorData.h
            src/ICLGeom/ObjectEdgeDetectorCPU.h
            src/ICLGeom/ObjectEdgeDetectorGPU.h
            src/ICLGeom/ObjectEdgeDetectorPlugin.h
            src/ICLGeom/PlanarRansacEstimator.h
            src/ICLGeom/SegmenterUtils.h
            src/ICLGeom/SurfaceFeatureExtractor.h
            src/ICLGeom/CutfreeAdjacencyFeatureExtractor.h
            src/ICLGeom/CoPlanarityFeatureExtractor.h
            src/ICLGeom/CurvatureFeatureExtractor.h
            src/ICLGeom/RemainingPointsFeatureExtractor.h)


IF(QT_FOUND)
  LIST (APPEND SOURCES src/ICLGeom/Scene.cpp
                       src/ICLGeom/SceneLight.cpp
                       src/ICLGeom/SceneLightObject.cpp
                       src/ICLGeom/SceneMouseHandler.cpp
                       src/ICLGeom/SceneObject.cpp
                       src/ICLGeom/Primitive.cpp
                       src/ICLGeom/GenericPointCloudGrabber.cpp
                       src/ICLGeom/GenericPointCloudOutput.cpp
                       src/ICLGeom/GridSceneObject.cpp
                       src/ICLGeom/ComplexCoordinateFrameSceneObject.cpp
                       src/ICLGeom/Segmentation3D.cpp
                       src/ICLGeom/DepthCameraPointCloudGrabber.cpp
                       src/ICLGeom/PointCloudCreator.cpp
                       src/ICLGeom/PointCloudObjectBase.cpp
                       src/ICLGeom/PointCloudObject.cpp
                       src/ICLGeom/PointCloudSerializer.cpp
                       src/ICLGeom/OctreeObject.cpp
                       src/ICLGeom/RayCastOctreeObject.cpp
                       src/ICLGeom/SQFitter.cpp
                       src/ICLGeom/PlotWidget3D.cpp
                       src/ICLGeom/ShaderUtil.cpp
                       src/ICLGeom/EuclideanBlobSegmenter.cpp
                       src/ICLGeom/FeatureGraphSegmenter.cpp
                       src/ICLGeom/ConfigurableDepthImageSegmenter.cpp
                       src/ICLGeom/PointCloudSegment.cpp
                       src/ICLGeom/PCDFileGrabber.cpp
                       src/ICLGeom/Primitive3DFilter.cpp)

  LIST (APPEND HEADERS src/ICLGeom/Scene.h
                       src/ICLGeom/SceneLight.h
                       src/ICLGeom/SceneLightObject.h
                       src/ICLGeom/SceneMouseHandler.h
                       src/ICLGeom/SceneObject.h
                       src/ICLGeom/Primitive.h
                       src/ICLGeom/GenericPointCloudGrabber.h
                       src/ICLGeom/GenericPointCloudOutput.h
                       src/ICLGeom/GridSceneObject.h
                       src/ICLGeom/ComplexCoordinateFrameSceneObject.h
                       src/ICLGeom/Segmentation3D.h
                       src/ICLGeom/DepthCameraPointCloudGrabber.h
                       src/ICLGeom/PointCloudCreator.h
                       src/ICLGeom/PointCloudGrabber.h
                       src/ICLGeom/PointCloudObject.h
                       src/ICLGeom/PointCloudObjectBase.h
                       src/ICLGeom/PointCloudSerializer.h
                       src/ICLGeom/OctreeObject.h
                       src/ICLGeom/RayCastOctreeObject.h
                       src/ICLGeom/SQFitter.h
                       src/ICLGeom/PlotWidget3D.h
                       src/ICLGeom/ShaderUtil.h
                       src/ICLGeom/EuclideanBlobSegmenter.h
                       src/ICLGeom/FeatureGraphSegmenter.h
                       src/ICLGeom/ConfigurableDepthImageSegmenter.h
                       src/ICLGeom/PointCloudSegment.h
                       src/ICLGeom/PCDFileGrabber.h
                       src/ICLGeom/Primitive3DFilter.h)
ENDIF()

IF(PCL_FOUND AND QT_FOUND)
  LIST (APPEND SOURCES src/ICLGeom/PCLPointCloudObject.cpp)
  LIST (APPEND HEADERS src/ICLGeom/PCLPointCloudObject.h)
  # requires pcl_io lib that depends on openni!!
ENDIF()

IF(OPENCL_FOUND AND QT_FOUND)
    LIST (APPEND SOURCES src/ICLGeom/PointCloudCreatorCL.cpp
                         src/ICLGeom/IterativeClosestPoint.cpp
	                 src/ICLGeom/IterativeClosestPointCLCode.cl)
    LIST (APPEND HEADERS src/ICLGeom/PointCloudCreatorCL.h
                         src/ICLGeom/IterativeClosestPoint.h
	                 src/ICLGeom/IterativeClosestPointCLCode.h)
ENDIF()

IF(RSB_FOUND AND QT_FOUND)
  # note the pointcloud-grabber needs the PointCloudObjectBase, which in turn needs qt
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
    SET(ICL_GEOM_PROTO_DEFS src/ICLGeom/proto/RSBPointCloud.proto)

    IF(RST_FOUND)
      SET(PROTOBUF_IMPORT_DIRS ${RST_STABLE_PROTO_ROOT} ${RST_SANDBOX_PROTO_ROOT})
      LIST(APPEND ICL_GEOM_PROTO_DEFS src/ICLGeom/proto/Primitive3DFloat.proto
                                      src/ICLGeom/proto/Primitive3DFloatSet.proto)
    ENDIF(RST_FOUND)

    PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${ICL_GEOM_PROTO_DEFS})
    INCLUDE_DIRECTORIES(BEFORE ${CMAKE_BINARY_DIR})

    LIST (APPEND SOURCES src/ICLGeom/RSBPointCloudGrabber.cpp
                         src/ICLGeom/ProtoBufSerializationDevice.cpp
                         src/ICLGeom/RSBPointCloudSender.cpp
                         ${PROTO_SRCS})

    LIST (APPEND HEADERS src/ICLGeom/RSBPointCloudGrabber.h
                         src/ICLGeom/ProtoBufSerializationDevice.h
                         src/ICLGeom/RSBPointCloudSender.h
                         ${PROTO_HDRS})
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
                           ${ICL_SOURCE_DIR}/ICLQt/src
                           ${ICL_SOURCE_DIR}/ICLCV/src)
ELSE()
INCLUDE_DIRECTORIES(BEFORE src
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src
                           ${ICL_SOURCE_DIR}/ICLFilter/src
                           ${ICL_SOURCE_DIR}/ICLIO/src
                           ${ICL_SOURCE_DIR}/ICLQt/src
                           ${ICL_SOURCE_DIR}/ICLCV/src)
ENDIF()

ADD_LIBRARY(ICLGeom SHARED ${SOURCES} ${HEADERS})

# ---- 3rd party library handling ----
IF(QT_FOUND)
  TARGET_LINK_LIBRARIES(ICLGeom ICLQt
                        ${ICLGeom_3RDPARTY_LIBRARIES})
ELSE()
  TARGET_LINK_LIBRARIES(ICLGeom ICLCV
                        ${ICLGeom_3RDPARTY_LIBRARIES})
ENDIF()

SET_TARGET_PROPERTIES(ICLGeom
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
INSTALL(TARGETS ICLGeom
        COMPONENT libraries
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS)

# ---- PKG-config ----
IF(QT_FOUND)
  CREATE_PKGCONFIG(NAME ICLGeom
                   LIBRARY_DEPS ${ICLGeom_3RDPARTY_LIBRARIES}
                   PKGCONFIG_DEPS ICLQt-${SO_VERSION})
ELSE()
  CREATE_PKGCONFIG(NAME ICLGeom
                   LIBRARY_DEPS ${ICLGeom_3RDPARTY_LIBRARIES}
                   PKGCONFIG_DEPS ICLCV-${SO_VERSION})
ENDIF()

IF(BUILD_TESTS)
  CONFIGURE_GTEST(ICLGeom ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(BUILD_TESTS)
