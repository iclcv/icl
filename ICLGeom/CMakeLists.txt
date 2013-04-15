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
	    src/ICLGeom/GeomDefs.cpp
	    src/ICLGeom/ICP.cpp
	    src/ICLGeom/PlaneEquation.cpp
	    src/ICLGeom/PointCloudNormalEstimator.cpp
	    src/ICLGeom/PoseEstimator.cpp
	    src/ICLGeom/Posit.cpp
	    src/ICLGeom/SoftPosit.cpp
	    src/ICLGeom/ViewRay.cpp)

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
	    src/ICLGeom/PointCloudGrabber.h 
	    src/ICLGeom/CoplanarPointPoseEstimator.h
	    src/ICLGeom/Hit.h
            src/ICLGeom/PlaneEquation.h
	    src/ICLGeom/PrimitiveType.h
            src/ICLGeom/SceneObjectBase.h
	    src/ICLGeom/DataSegmentBase.h  
            src/ICLGeom/ICP.h
            src/ICLGeom/RGBDMapping.h
            src/ICLGeom/Plot3D.h 
	    src/ICLGeom/DataSegment.h            
            src/ICLGeom/PlotHandle3D.h)

IF(QT_FOUND)
  LIST (APPEND SOURCES src/ICLGeom/Scene.cpp
	               src/ICLGeom/SceneLight.cpp
		       src/ICLGeom/SceneLightObject.cpp
		       src/ICLGeom/SceneMouseHandler.cpp
		       src/ICLGeom/SceneObject.cpp
		       src/ICLGeom/Primitive.cpp
		       src/ICLGeom/GridSceneObject.cpp
		       src/ICLGeom/ComplexCoordinateFrameSceneObject.cpp
		       src/ICLGeom/Segmentation3D.cpp
		       src/ICLGeom/DepthCameraPointCloudGrabber.cpp
		       src/ICLGeom/PointCloudCreator.cpp
		       src/ICLGeom/PointCloudObjectBase.cpp
		       src/ICLGeom/PointCloudObject.cpp
		       src/ICLGeom/OctreeObject.cpp
		       src/ICLGeom/RayCastOctreeObject.cpp
		       src/ICLGeom/PlotWidget3D.cpp)

  LIST (APPEND HEADERS src/ICLGeom/Scene.h
                       src/ICLGeom/SceneLight.h
		       src/ICLGeom/SceneLightObject.h
		       src/ICLGeom/SceneMouseHandler.h
		       src/ICLGeom/SceneObject.h	    
		       src/ICLGeom/Primitive.h
		       src/ICLGeom/GridSceneObject.h
		       src/ICLGeom/ComplexCoordinateFrameSceneObject.h
		       src/ICLGeom/Segmentation3D.h
		       src/ICLGeom/DepthCameraPointCloudGrabber.h
		       src/ICLGeom/PointCloudCreator.h
		       src/ICLGeom/PointCloudObject.h
		       src/ICLGeom/PointCloudObjectBase.h
		       src/ICLGeom/OctreeObject.h 
		       src/ICLGeom/RayCastOctreeObject.h 
		       src/ICLGeom/PlotWidget3D.h)
ENDIF()

IF(PCL_FOUND)
  LIST (APPEND SOURCES src/ICLGeom/PCLPointCloudObject.cpp)
  LIST (APPEND HEADERS src/ICLGeom/PCLPointCloudObject.h)
  # requires pcl_io lib that depends on openni!!
  # src/ICLGeom/PCDFileGrabber.cpp
  # src/ICLGeom/PCDFileGrabber.h
ENDIF()

IF(OpenCV_FOUND)
  LIST (APPEND SOURCES src/ICLGeom/OpenCVCamCalib.cpp)
  LIST (APPEND HEADRES src/ICLGeom/OpenCVCamCalib.h)
ENDIF()

IF(OPENCL_FOUND)
    LIST (APPEND SOURCES src/ICLGeom/PointCloudCreatorCL.cpp)
    LIST (APPEND HEADERS src/ICLGeom/PointCloudCreatorCL.h)
ENDIF()


# ---- Library build instructions ----
INCLUDE_DIRECTORIES(BEFORE src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src
                           ${ICL_SOURCE_DIR}/ICLFilter/src
                           ${ICL_SOURCE_DIR}/ICLIO/src
                           ${ICL_SOURCE_DIR}/ICLQt/src
                           ${ICL_SOURCE_DIR}/ICLCV/src)

ADD_LIBRARY(ICLGeom SHARED ${SOURCES} ${HEADERS})

# ---- 3rd party library handling ----
IF(QT4_FOUND)
  TARGET_LINK_LIBRARIES(ICLGeom ICLQt
                        ${PCL_LIBRARIES})
ELSE()
  TARGET_LINK_LIBRARIES(ICLGeom ICLCV
                        ${PCL_LIBRARIES})
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
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS)

# ---- PKG-config ----

IF(QT4_FOUND)
  CREATE_PKGCONFIG(NAME ICLGeom
                   LIBRARY_DEPS ${PCL_LIBRARIES}
                   PKGCONFIG_DEPS ICLQt-${SO_VERSION})
ELSE()
  CREATE_PKGCONFIG(NAME ICLGeom
                   LIBRARY_DEPS ${PCL_LIBRARIES}
                   PKGCONFIG_DEPS ICLCV-${SO_VERSION})
ENDIF()
