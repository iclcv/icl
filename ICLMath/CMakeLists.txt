#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLMath/CMakeLists.txt                                 **
#** Module : ICLMath                                                **
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

SET(SOURCES src/ICLMath/DynMatrix.cpp
            src/ICLMath/DynMatrixUtils.cpp
            src/ICLMath/FFTUtils.cpp
            src/ICLMath/FixedMatrix.cpp
            src/ICLMath/GraphCutter.cpp
            src/ICLMath/Homography2D.cpp
            src/ICLMath/KDTree.cpp
            src/ICLMath/LevenbergMarquardtFitter.cpp
            src/ICLMath/LLM.cpp
            src/ICLMath/PolynomialRegression.cpp
            src/ICLMath/SimplexOptimizer.cpp
            src/ICLMath/SOM.cpp
            src/ICLMath/StochasticOptimizer.cpp
            src/ICLMath/StraightLine2D.cpp
            src/ICLMath/EigenICLConverter.cpp
            src/ICLMath/HomogeneousMath.cpp
            src/ICLMath/Projective4PointTransform.cpp)

SET(HEADERS src/ICLMath/DynMatrix.h
            src/ICLMath/DynMatrixUtils.h
            src/ICLMath/DynVector.h
            src/ICLMath/FFTException.h
            src/ICLMath/FFTUtils.h
            src/ICLMath/FixedMatrix.h
            src/ICLMath/GraphCutter.h
            src/ICLMath/FixedVector.h
            src/ICLMath/Homography2D.h
            src/ICLMath/KDTree.h
            src/ICLMath/KMeans.h
            src/ICLMath/LeastSquareModelFitting2D.h
            src/ICLMath/LeastSquareModelFitting.h
            src/ICLMath/LevenbergMarquardtFitter.h
            src/ICLMath/LinearTransform1D.h
            src/ICLMath/LLM.h
            src/ICLMath/MathFunctions.h
            src/ICLMath/Math.h
            src/ICLMath/MatrixSubRectIterator.h
            src/ICLMath/Octree.h
            src/ICLMath/PCLKdtree.h
            src/ICLMath/PCLOctree.h
            src/ICLMath/PolynomialRegression.h
            src/ICLMath/PolynomialSolver.h
            src/ICLMath/QuadTree.h
            src/ICLMath/RansacFitter.h
            src/ICLMath/SimplexOptimizer.h
            src/ICLMath/SOM2D.h
            src/ICLMath/SOM.h
            src/ICLMath/StochasticOptimizer.h
            src/ICLMath/StraightLine2D.h
            src/ICLMath/EigenICLConverter.h
            src/ICLMath/HomogeneousMath.h
            src/ICLMath/Projective4PointTransform.h)

IF(WIN32)
INCLUDE_DIRECTORIES(BEFORE src
                           ${ICL_SOURCE_DIR}/3rdparty/zlib
                           ${ICL_SOURCE_DIR}/3rdparty/libpng
                           ${ICL_SOURCE_DIR}/3rdparty/libjpeg
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src)
ELSE()
INCLUDE_DIRECTORIES(BEFORE src
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src)
ENDIF()

ADD_LIBRARY(ICLMath SHARED ${SOURCES} ${HEADERS})

# ---- Linker commands ----
TARGET_LINK_LIBRARIES(ICLMath ICLUtils
                              ${ICLMath_3RDPARTY_LIBRARIES})

SET_TARGET_PROPERTIES(ICLMath
                      PROPERTIES
		      VERSION ${SO_VERSION})

# ---- Build examples ----
IF(BUILD_EXAMPLES)
  ADD_SUBDIRECTORY(examples)
ENDIF()

IF(BUILD_DEMOS)
  ADD_SUBDIRECTORY(demos)
ENDIF()

# ---- Install specifications ----
INSTALL(TARGETS ICLMath
        COMPONENT libraries
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS)

# --- pkgconfig file ---
LIST(APPEND PROCESSED_ICL_LIBS "ICLMath")

# ---- PKG-config ----
CREATE_PKGCONFIG(NAME ICLMath
                 LIBRARY_DEPS ${ICLMath_3RDPARTY_LIBRARIES}
                 PKGCONFIG_DEPS ICLUtils-${SO_VERSION})

IF(BUILD_TESTS)
  CONFIGURE_GTEST(ICLMath ${CMAKE_CURRENT_SOURCE_DIR})
  # In case we want to upload to coveralls later
  # FILE(GLOB TEST_FILES src/ICLMath/*.[cpp,h])
  # message(STATUS "##*()&**()FUYIYFH ${TEST_FILES}")
  # coveralls_setup(${TEST_FILES} OFF)
ENDIF(BUILD_TESTS)
