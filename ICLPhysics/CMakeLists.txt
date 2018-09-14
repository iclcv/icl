#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLPhysics/CMakeLists.txt                              **
#** Module : ICLPhysics                                             **
#** Authors: Christof Elbrechter, Matthias Esau                     **
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





SET(SOURCES src/ICLPhysics/Object2PointConstraint.cpp
            src/ICLPhysics/BallSocketConstraint.cpp
            src/ICLPhysics/BendingConstraint.cpp
            src/ICLPhysics/Constraint.cpp
            src/ICLPhysics/GeometricTools.cpp
            src/ICLPhysics/PhysicsDefs.cpp
            src/ICLPhysics/HingeConstraint.cpp
            src/ICLPhysics/ManipulatablePaper.cpp
            src/ICLPhysics/PhysicsMouseHandler.cpp
            src/ICLPhysics/PhysicsObject.cpp
            src/ICLPhysics/RigidBoxObject.cpp
            src/ICLPhysics/RigidCompoundObject.cpp
            src/ICLPhysics/RigidConvexHullObject.cpp
            src/ICLPhysics/RigidCylinderObject.cpp
            src/ICLPhysics/RigidObject.cpp
            src/ICLPhysics/RigidSphereObject.cpp
            src/ICLPhysics/PhysicsScene.cpp
            src/ICLPhysics/SliderConstraint.cpp
            src/ICLPhysics/SoftObject.cpp
            src/ICLPhysics/TriangleIntersectionEstimator.cpp
            src/ICLPhysics/PhysicsPaper.cpp
            src/ICLPhysics/PhysicsPaper3.cpp
            src/ICLPhysics/FoldMap.cpp
            src/ICLPhysics/PhysicsUtils.cpp
            src/ICLPhysics/PhysicsWorld.cpp
            src/ICLPhysics/SixDOFConstraint.cpp
            src/ICLPhysics/PhysicsPaper3MouseHandler.cpp
            src/ICLPhysics/SceneMultiCamCapturer.cpp
            src/ICLPhysics/PhysicsPaper3ContextMenu.cpp)





SET(HEADERS src/ICLPhysics/Object2PointConstraint.h
            src/ICLPhysics/SixDOFConstraint.h
            src/ICLPhysics/BallSocketConstraint.h
            src/ICLPhysics/BendingConstraint.h
            src/ICLPhysics/Constraint.h
            src/ICLPhysics/GeometricTools.h
            src/ICLPhysics/PhysicsDefs.h
            src/ICLPhysics/HingeConstraint.h
            src/ICLPhysics/ManipulatablePaper.cpp
            src/ICLPhysics/MotionState.h
            src/ICLPhysics/PhysicsMouseHandler.h
            src/ICLPhysics/PhysicsObject.h
            src/ICLPhysics/RigidBoxObject.h
            src/ICLPhysics/RigidCompoundObject.h
            src/ICLPhysics/RigidConvexHullObject.h
            src/ICLPhysics/RigidCylinderObject.h
            src/ICLPhysics/RigidObject.h
            src/ICLPhysics/RigidSphereObject.h
            src/ICLPhysics/PhysicsScene.h
            src/ICLPhysics/SliderConstraint.h
            src/ICLPhysics/SoftObject.h
            src/ICLPhysics/TriangleIntersectionEstimator.h
            src/ICLPhysics/PhysicsPaper.h
            src/ICLPhysics/PhysicsPaper3.h
            src/ICLPhysics/FoldMap.h
            src/ICLPhysics/PhysicsUtils.h
            src/ICLPhysics/PhysicsWorld.h
            src/ICLPhysics/PhysicsPaper3MouseHandler.h
            src/ICLPhysics/SceneMultiCamCapturer.h
            src/ICLPhysics/PhysicsPaper3ContextMenu.h)

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
                           ${ICL_SOURCE_DIR}/ICLGeom/src
                           ${ICL_SOURCE_DIR}/ICLMarkers/src
						   "${BULLET_ROOT_DIR}/${BULLET_INCLUDE_DIRS}")
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
                           ${ICL_SOURCE_DIR}/ICLGeom/src
                           ${ICL_SOURCE_DIR}/ICLMarkers/src
						   "${BULLET_ROOT_DIR}/${BULLET_INCLUDE_DIRS}")
ENDIF()

ADD_LIBRARY(ICLPhysics SHARED ${SOURCES} ${HEADERS})

# ---- 3rd party library handling ----
TARGET_LINK_LIBRARIES(ICLPhysics ICLMarkers ${ICLPhysics_3RDPARTY_LIBRARIES})

SET_TARGET_PROPERTIES(ICLPhysics
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
INSTALL(TARGETS ICLPhysics
        COMPONENT libraries
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS)

# ---- PKG-config ----
CREATE_PKGCONFIG(NAME ICLPhysics
                 LIBRARY_DEPS ${ICLPhysics_3RDPARTY_LIBRARIES}
                 PKGCONFIG_DEPS ICLGeom-${SO_VERSION})

IF(BUILD_TESTS)
  CONFIGURE_GTEST(ICLPhysics ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(BUILD_TESTS)
