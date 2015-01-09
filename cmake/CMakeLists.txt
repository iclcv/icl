#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/CMakeLists.txt                                   **
#** Module :                                                        **
#** Authors: Michael Goetting, Sergius Gaulik                       **
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

ADD_SUBDIRECTORY(Modules)

SET(ICL_COMPONENTS "Utils;Core;Math;Filter;IO;CV;Geom;Markers")
SET(ICLUtils_COMPONENTS "")
SET(ICLCore_COMPONENTS "Utils")
SET(ICLMath_COMPONENTS "Utils;Core")
SET(ICLFilter_COMPONENTS "Utils;Core;Math")
SET(ICLIO_COMPONENTS "Utils;Core;Math;Filter")
SET(ICLCV_COMPONENTS "Utils;Core;Math;Filter;IO")
SET(ICLQt_COMPONENTS "Utils;Core;Math;Filter;IO;CV")
SET(ICLGeom_COMPONENTS "Utils;Core;Math;Filter;IO;CV")
SET(ICLMarkers_COMPONENTS "Utils;Core;Math;Filter;IO;CV;Geom")
SET(ICLPhysics_COMPONENTS "Utils;Core;Math;Filter;IO;CV;Geom;Physics")

GET_PROPERTY(ICL_3RDPARTY_INCLUDE_DIRS DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
LIST(REMOVE_DUPLICATES ICL_3RDPARTY_INCLUDE_DIRS)

IF(QT_FOUND)
  LIST(APPEND ICL_COMPONENTS "Qt")
  LIST(APPEND ICLGeom_COMPONENTS "Qt")
  LIST(APPEND ICLMarkers_COMPONENTS "Qt")
ENDIF(QT_FOUND)

IF(BULLET_FOUND)
  LIST(APPEND ICL_COMPONENTS "Physics")
  LIST(APPEND ICLPhysics_COMPONENTS "Physics")
ENDIF(BULLET_FOUND)

IF(WIN32)
  set(LIBRARY_PREFIX "ICL")
  set(LIBRARY_SUFFIX ".lib")
ELSE(WIN32)
  set(LIBRARY_PREFIX "libICL")
  set(LIBRARY_SUFFIX ".so")
ENDIF(WIN32)

# remove absolut paths from dependencies
SPLIT_LIB_PATHS(ICLUtils_3RDPARTY_DEPS_SLIT ICLUtils_3RDPARTY_LIBRARIES)
SPLIT_LIB_PATHS(ICLCore_3RDPARTY_DEPS_SLIT ICLCore_3RDPARTY_LIBRARIES)
SPLIT_LIB_PATHS(ICLMath_3RDPARTY_DEPS_SLIT ICLMath_3RDPARTY_LIBRARIES)
SPLIT_LIB_PATHS(ICLFilter_3RDPARTY_DEPS_SLIT ICLFilter_3RDPARTY_LIBRARIES)
MESSAGE( STATUS "ICLFilter_3RDPARTY_LIBRARIES: ${ICLFilter_3RDPARTY_LIBRARIES}")
MESSAGE( STATUS "ICLFilter_3RDPARTY_DEPS_SLIT: ${ICLFilter_3RDPARTY_DEPS_SLIT}")
SPLIT_LIB_PATHS(ICLIO_3RDPARTY_DEPS_SLIT ICLIO_3RDPARTY_LIBRARIES)
SPLIT_LIB_PATHS(ICLCV_3RDPARTY_DEPS_SLIT ICLCV_3RDPARTY_LIBRARIES)
SPLIT_LIB_PATHS(ICLQt_3RDPARTY_DEPS_SLIT ICLQt_3RDPARTY_LIBRARIES)
SPLIT_LIB_PATHS(ICLGeom_3RDPARTY_DEPS_SLIT ICLGeom_3RDPARTY_LIBRARIES)
SPLIT_LIB_PATHS(ICLMarkers_3RDPARTY_DEPS_SLIT ICLMarkers_3RDPARTY_LIBRARIES)
SPLIT_LIB_PATHS(ICLPhysics_3RDPARTY_DEPS_SLIT ICLPhysics_3RDPARTY_LIBRARIES)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cmake/ICLConfig.cmake.in
               ${CMAKE_BINARY_DIR}/cmake/ICLConfig.cmake
               @ONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cmake/ICLConfigVersion.cmake.in
               ${CMAKE_BINARY_DIR}/cmake/ICLConfigVersion.cmake
               @ONLY)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/ICLConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/ICLConfigVersion.cmake
        DESTINATION lib/cmake/ICL)
