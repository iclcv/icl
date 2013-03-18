#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/ICLHelperMacros.cmake                    **
#** Module : ICLHelperMacros                                        **
#** Authors: Michael Goetting                                       **
#**                                                                 **
#**                                                                 **
#** GNU LESSER GENERAL PUBLIC LICENSE                               **
#** This file may be used under the terms of the GNU Lesser General **
#** Public License version 3.0 as published by the                  **
#**                                                                 **
#** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

INCLUDE(CMakeParseArguments)

#*********************************************************************
# ---- BUILD_EXAMPLE ----
#*********************************************************************
FUNCTION(BUILD_EXAMPLE)
  SET(oneValueArgs NAME)
  SET(multiValueArgs SOURCES LIBRARIES)
  CMAKE_PARSE_ARGUMENTS(EXAMPLE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  SET(BINARY "${EXAMPLE_NAME}-example")
  ADD_EXECUTABLE(${BINARY} ${EXAMPLE_SOURCES})
  TARGET_LINK_LIBRARIES(${BINARY} ${EXAMPLE_LIBRARIES})
  INSTALL(TARGETS ${BINARY}
          RUNTIME DESTINATION share/${INSTALL_PATH_PREFIX}/examples)
ENDFUNCTION()

#*********************************************************************
# ---- BUILD_DEMO ----
#*********************************************************************
FUNCTION(BUILD_DEMO)
  SET(oneValueArgs NAME)
  SET(multiValueArgs SOURCES LIBRARIES)
  CMAKE_PARSE_ARGUMENTS(DEMO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ICLUtils/src
                      ${CMAKE_SOURCE_DIR}/ICLMath/src
                      ${CMAKE_SOURCE_DIR}/ICLCore/src
                      ${CMAKE_SOURCE_DIR}/ICLFilter/src
                      ${CMAKE_SOURCE_DIR}/ICLIO/src
                      ${CMAKE_SOURCE_DIR}/ICLQt/src
                      ${CMAKE_SOURCE_DIR}/ICLCV/src
                      ${CMAKE_SOURCE_DIR}/ICLGeom/src
                      ${CMAKE_SOURCE_DIR}/ICLMarkers/src)

  SET(BINARY "${DEMO_NAME}-demo")
  ADD_EXECUTABLE(${BINARY} ${DEMO_SOURCES})
  TARGET_LINK_LIBRARIES(${BINARY} ${DEMO_LIBRARIES})
  INSTALL(TARGETS ${BINARY}
          RUNTIME DESTINATION share/${INSTALL_PATH_PREFIX}/demos)
ENDFUNCTION()

#*********************************************************************
# ---- BUILD_APP ----
#*********************************************************************
FUNCTION(BUILD_APP)
  SET(oneValueArgs NAME)
  SET(multiValueArgs SOURCES LIBRARIES)
  CMAKE_PARSE_ARGUMENTS(APP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ICLUtils/src
                      ${CMAKE_SOURCE_DIR}/ICLMath/src
                      ${CMAKE_SOURCE_DIR}/ICLCore/src
                      ${CMAKE_SOURCE_DIR}/ICLFilter/src
                      ${CMAKE_SOURCE_DIR}/ICLIO/src
                      ${CMAKE_SOURCE_DIR}/ICLQt/src
                      ${CMAKE_SOURCE_DIR}/ICLCV/src
                      ${CMAKE_SOURCE_DIR}/ICLGeom/src
                      ${CMAKE_SOURCE_DIR}/ICLMarkers/src)

  SET(BINARY "icl-${APP_NAME}")
  ADD_EXECUTABLE(${BINARY} ${APP_SOURCES})
  TARGET_LINK_LIBRARIES(${BINARY} ${APP_LIBRARIES})
  INSTALL(TARGETS ${BINARY}
          RUNTIME DESTINATION bin)
ENDFUNCTION()

#*********************************************************************
# ---- Create pkg-config ----
#*********************************************************************
FUNCTION(CREATE_PKGCONFIG)
  SET(oneValueArgs NAME)
  SET(multiValueArgs PKGCONFIG_DEPS LIBRARY_DEPS RPATH_DEPS)
  CMAKE_PARSE_ARGUMENTS(CONFIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  
  # Set appropriate variables
  SET(PKG_NAME ${CONFIG_NAME} CACHE INTERNAL "Name of the pkg-config file")
  
  # Link against specific library (if not ICL-meta package)
  IF(${CONFIG_NAME} STREQUAL "ICL")
    SET(PKG_LIB "")
  ELSE()
    SET(PKG_LIB "-l${PKG_NAME}")
  ENDIF()
  
  # Prepare include paths
  GET_PROPERTY(INCLUDE_DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
  LIST(REMOVE_DUPLICATES INCLUDE_DIRS)
  STRING(REPLACE ";" " -I" # first -I is in icl.pc.in
         INCLUDE_DIRS
         "${INCLUDE_DIRS}")
       
  # Prepare pkg-config dependencies
  SET(PKG_DEPENDS_ON ${CONFIG_PKGCONFIG_DEPS} CACHE INTERNAL "Dependencies for this pkg-config")
  STRING(REPLACE ";" " "
         PKG_DEPENDS_ON
         "${PKG_DEPENDS_ON}")
       
  # Prepare libraries list
  SET(LIB_DEPENDS_ON "${CONFIG_LIBRARY_DEPS} ${CONFIG_RPATH_DEPS}" CACHE INTERNAL "Library dependencies for this pkg-config")
  STRING(REPLACE ";" " "
         LIB_DEPENDS_ON
         "${LIB_DEPENDS_ON}")

  # Prepare ICL definition list
  STRING(REPLACE ";" " "
         ICL_DEFINITIONS
         "${ICL_DEFINITIONS}")
  
  # Prepare pkg-config file
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/icl.pc.in icl.pc @ONLY)
  
  # Install pkg-config
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/icl.pc
          DESTINATION lib/pkgconfig
          RENAME      "${PKG_NAME}-${SO_VERSION}.pc")
ENDFUNCTION()

#*********************************************************************
# ---- Add definitions ----
#*********************************************************************
MACRO(ADD_ICL_DEFINITIONS)
  ADD_DEFINITIONS(${ARGV})
  LIST(APPEND ICL_DEFINITIONS ${ARGV})
ENDMACRO()
