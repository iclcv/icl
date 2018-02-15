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
          COMPONENT examples
          RUNTIME DESTINATION share/${INSTALL_PATH_PREFIX}/examples)
ENDFUNCTION()

#*********************************************************************
# ---- BUILD_DEMO ----
#*********************************************************************
FUNCTION(BUILD_DEMO)
  SET(oneValueArgs NAME)
  SET(multiValueArgs SOURCES LIBRARIES)
  CMAKE_PARSE_ARGUMENTS(DEMO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  LIST(GET DEMO_SOURCES 0 FIRST_SOURCE)
  get_filename_component(SOURCE_ABSOLUTE ${FIRST_SOURCE} ABSOLUTE)
  get_filename_component(SOURCE_FOLDER ${SOURCE_ABSOLUTE} PATH)

  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ICLUtils/src
                      ${CMAKE_SOURCE_DIR}/ICLMath/src
                      ${CMAKE_SOURCE_DIR}/ICLCore/src
                      ${CMAKE_SOURCE_DIR}/ICLFilter/src
                      ${CMAKE_SOURCE_DIR}/ICLIO/src
                      ${CMAKE_SOURCE_DIR}/ICLQt/src
                      ${CMAKE_SOURCE_DIR}/ICLCV/src
                      ${CMAKE_SOURCE_DIR}/ICLGeom/src
                      ${CMAKE_SOURCE_DIR}/ICLMarkers/src
                      ${CMAKE_SOURCE_DIR}/ICLPhysics/src
                      ${SOURCE_FOLDER})

  SET(BINARY "${DEMO_NAME}-demo")
  ADD_EXECUTABLE(${BINARY} ${DEMO_SOURCES})
  TARGET_LINK_LIBRARIES(${BINARY} ${DEMO_LIBRARIES})
  INSTALL(TARGETS ${BINARY}
          COMPONENT demos
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
                      ${CMAKE_SOURCE_DIR}/ICLMarkers/src
                      ${CMAKE_SOURCE_DIR}/ICLPhysics/src)

  SET(BINARY "icl-${APP_NAME}")
  ADD_EXECUTABLE(${BINARY} ${APP_SOURCES})
  TARGET_LINK_LIBRARIES(${BINARY} ${APP_LIBRARIES})
  INSTALL(TARGETS ${BINARY}
          COMPONENT applications
          RUNTIME DESTINATION bin)
ENDFUNCTION()

#*********************************************************************
# ---- split paths into lib and link path ----
#*********************************************************************
FUNCTION(SPLIT_LIB_PATHS LIST_OUT LIB_LIST)
    SET( ${LIST_OUT} "" )
    SET( COMPONENT_DIRS "" )
    SET( COMPONENT_DEPS "" )
    IF(NOT ${LIB_LIST})
        RETURN()
    ENDIF()
    FOREACH(_LIB ${${LIB_LIST}})
        IF(CMAKE_VERSION VERSION_LESS 2.8.12)
            get_filename_component(_DIR ${_LIB} PATH)
        ELSE()
            get_filename_component(_DIR ${_LIB} DIRECTORY)
        ENDIF()
        IF(NOT _DIR)
          STRING(SUBSTRING "${_LIB}" 0 1 _SUB)
          IF(NOT ${_SUB} STREQUAL "-")
            GET_PROPERTY(_LIB TARGET ${_LIB} PROPERTY LOCATION)
            IF(CMAKE_VERSION VERSION_LESS 2.8.12)
              get_filename_component(_DIR ${_LIB} PATH)
            ELSE()
              get_filename_component(_DIR ${_LIB} DIRECTORY)
            ENDIF()
          ENDIF()
        ELSE()
            LIST(APPEND COMPONENT_DEPS "${_LIB}")
            LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${_DIR}" isSystemDir)
            STRING(REGEX MATCH "^/usr/lib/*" isSystemDir ${_DIR})
            IF(NOT isSystemDir)
                LIST(APPEND COMPONENT_DIRS "-Wl,-rpath,${_DIR}")
            ENDIF()
        ENDIF()
    ENDFOREACH()
    IF(COMPONENT_DIRS)
        LIST(REMOVE_DUPLICATES COMPONENT_DIRS)
    ENDIF()

    SET(LIB_DEPENDS_ON "${COMPONENT_DIRS} ${COMPONENT_DEPS} ${CONFIG_RPATH_DEPS}" CACHE INTERNAL "Library dependencies for this pkg-config")
    # removing /usr/lib and also the architecture dependent default library paths for the list
	#MESSAGE(STATUS "before filtering: ;${LIB_DEPENDS_ON};")

    # tokenize the list properly: i.e. replace " " by ";" so that each
    # token actually becomes a cmake list token
    SET(LIB_DEPENDS_ON_NEW "")
    FOREACH(T ${LIB_DEPENDS_ON})
    #   MESSAGE(STATUS "---- ${T}")
    STRING(REGEX REPLACE " " ";" T ${T})
    FOREACH(T2 ${T})
    #      MESSAGE(STATUS "      ${T2}")
      LIST(APPEND LIB_DEPENDS_ON_NEW ${T2})
    ENDFOREACH()
    ENDFOREACH()

	# update actual list and remove silly rpaths
	SET(LIB_DEPENDS_ON "${LIB_DEPENDS_ON_NEW}")
	LIST(REMOVE_DUPLICATES LIB_DEPENDS_ON)
	LIST(REMOVE_ITEM LIB_DEPENDS_ON "-Wl,-rpath,")
	LIST(REMOVE_ITEM LIB_DEPENDS_ON "-Wl,-rpath,:")
    SET(LIB_DEPENDS_ON_NEW "")

    # loop over the list and replace full paths with -lLIBNAME
    # for all libs that are in default directories (i.e. located
    # in /usr/lib/somewhere
    FOREACH(T  ${LIB_DEPENDS_ON})
    STRING(REGEX MATCH "^/usr/lib/.*\\.so.*" M ${T})
    IF(NOT M)
    #    MESSAGE(STATUS "keeping token ${T}")
      LIST(APPEND LIB_DEPENDS_ON_NEW ${T})
    ELSE()
      # magic begins here!
      STRING(REGEX REPLACE "/" ";" M ${T})
      LIST(REVERSE M)
      SET(M0 "")
      LIST(GET M 0 M)
      STRING(REGEX REPLACE "^lib" "" M ${M})
      STRING(REGEX REPLACE "\\.so[\\.0-9]*$" "" M ${M})
      SET(REP ${M})
      LIST(APPEND LIB_DEPENDS_ON_NEW ${M})
    #   MESSAGE(STATUS "replaced token by -l${M}")
    ENDIF()
	ENDFOREACH()
    SET(${LIST_OUT} ${LIB_DEPENDS_ON_NEW} PARENT_SCOPE)

ENDFUNCTION()

#*********************************************************************
# ---- Create pkg-config ----
#*********************************************************************
FUNCTION(CREATE_PKGCONFIG)
IF(NOT WIN32)
  SET(oneValueArgs NAME)
  SET(multiValueArgs PKGCONFIG_DEPS LIBRARY_DEPS RPATH_DEPS)
  CMAKE_PARSE_ARGUMENTS(CONFIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Set appropriate variables
  SET(PKG_NAME ${CONFIG_NAME} CACHE INTERNAL "Name of the pkg-config file")

  MESSAGE(STATUS "Creatig Package Config file for ${PKG_NAME}")

  # Link against specific library (if not ICL-meta package)
  IF(${CONFIG_NAME} STREQUAL "ICL")
    SET(PKG_LIB "")
  ELSE()
    IF(APPLE)
      SET(PKG_LIB "${CMAKE_INSTALL_PREFIX}/lib/lib${PKG_NAME}.${SO_VERSION}.dylib")
    ELSE(APPLE)
      SET(PKG_LIB "${CMAKE_INSTALL_PREFIX}/lib/lib${PKG_NAME}.so.${SO_VERSION}")
    ENDIF(APPLE)
  ENDIF()

  # Prepare include paths
  GET_PROPERTY(INCLUDE_DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
  LIST(REMOVE_DUPLICATES INCLUDE_DIRS)

  # remove local include dirs
  FOREACH(M Utils Math Core Filter IO CV Qt Geom Markers Physics)
    LIST(REMOVE_ITEM INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/ICL${M}/src)
  ENDFOREACH()
  # remove build-dir added in case of RSB-Support
  LIST(REMOVE_ITEM INCLUDE_DIRS ${CMAKE_BINARY_DIR})
  LIST(REMOVE_ITEM INCLUDE_DIRS ${CMAKE_BINARY_DIR}/src)
  LIST(REMOVE_ITEM INCLUDE_DIRS ${CMAKE_BINARY_DIR}/ICLIO)
  LIST(REMOVE_ITEM INCLUDE_DIRS ${CMAKE_BINARY_DIR}/ICLGeom)

  STRING(REPLACE ";" " -I" # first -I is in icl.pc.in
         INCLUDE_DIRS
         "${INCLUDE_DIRS}")

  # Prepare pkg-config dependencies
  SET(PKG_DEPENDS_ON ${CONFIG_PKGCONFIG_DEPS} CACHE INTERNAL "Dependencies for this pkg-config")
  STRING(REPLACE ";" " "
         PKG_DEPENDS_ON
         "${PKG_DEPENDS_ON}")

  # Prepare libraries list
  SET(_LIBRARY_DEPS "")
  SET(_LIBRARY_DIRS "")
  FOREACH(_LIB ${CONFIG_LIBRARY_DEPS})
    IF(CMAKE_VERSION VERSION_LESS 2.8.12)
      get_filename_component(_DIR ${_LIB} PATH)
    ELSE()
      get_filename_component(_DIR ${_LIB} DIRECTORY)
    ENDIF()
    IF(NOT _DIR)
      STRING(SUBSTRING "${_LIB}" 0 1 _SUB)
      IF(NOT ${_SUB} STREQUAL "-")
        GET_PROPERTY(_LIB TARGET ${_LIB} PROPERTY LOCATION)
        IF(CMAKE_VERSION VERSION_LESS 2.8.12)
          get_filename_component(_DIR ${_LIB} PATH)
        ELSE()
          get_filename_component(_DIR ${_LIB} DIRECTORY)
        ENDIF()
      ENDIF()
    ENDIF()
    LIST(APPEND _LIBRARY_DEPS "${_LIB}")
    LIST(APPEND _LIBRARY_DIRS "-Wl,-rpath,${_DIR}")
  ENDFOREACH()
  IF(_LIBRARY_DIRS)
    LIST(REMOVE_DUPLICATES _LIBRARY_DIRS)
  ENDIF()

  SET(LIB_DEPENDS_ON "${_LIBRARY_DIRS} ${_LIBRARY_DEPS} ${CONFIG_RPATH_DEPS}" CACHE INTERNAL "Library dependencies for this pkg-config")
  # removing /usr/lib and also the architecture dependent default library paths for the list
  #MESSAGE(STATUS "before filtering: ${LIB_DEPENDS_ON}")

  # tokenize the list properly: i.e. replace " " by ";" so that each
  # token actually becomes a cmake list token
  SET(LIB_DEPENDS_ON_NEW "")
  FOREACH(T ${LIB_DEPENDS_ON})
 #   MESSAGE(STATUS "---- ${T}")
    STRING(REGEX REPLACE " " ";" T ${T})
    FOREACH(T2 ${T})
#      MESSAGE(STATUS "      ${T2}")
      LIST(APPEND LIB_DEPENDS_ON_NEW ${T2})
    ENDFOREACH()
  ENDFOREACH()

  # update actual list and remove silly rpaths
  SET(LIB_DEPENDS_ON ${LIB_DEPENDS_ON_NEW})
  LIST(REMOVE_DUPLICATES LIB_DEPENDS_ON)
  LIST(REMOVE_ITEM LIB_DEPENDS_ON "-Wl,-rpath,")
  LIST(REMOVE_ITEM LIB_DEPENDS_ON "-Wl,-rpath,:")
  SET(LIB_DEPENDS_ON_NEW "")

  # loop over the list and replace full paths with -lLIBNAME
  # for all libs that are in default directories (i.e. located
  # in /usr/lib/somewhere
  FOREACH(T  ${LIB_DEPENDS_ON})
    SET(NNNN "")
    #STRING(REGEX MATCH ".*rpath.*" NNNN ${T}) # new idea: we dont want any rpath stuff in the pkg-config files
    IF(NOT NNNN)
      STRING(REGEX MATCH "^/usr/lib/.*\\.so.*" M ${T})
      IF(NOT M)
        #    MESSAGE(STATUS "keeping token ${T}")
        LIST(APPEND LIB_DEPENDS_ON_NEW ${T})
      ELSE()
        # magic begins here!
        STRING(REGEX REPLACE "/" ";" M ${T})
        LIST(REVERSE M)
        SET(M0 "")
        LIST(GET M 0 M)
        STRING(REGEX REPLACE "^lib" "" M ${M})
        STRING(REGEX REPLACE "\\.so[\\.0-9]*$" "" M ${M})
        SET(REP -l${M})
        LIST(APPEND LIB_DEPENDS_ON_NEW -l${M})
        #   MESSAGE(STATUS "replaced token by -l${M}")
      ENDIF()
    ENDIF()
  ENDFOREACH()
  SET(LIB_DEPENDS_ON ${LIB_DEPENDS_ON_NEW})

  FOREACH(A ${ARCH_DEPENDENT_LIB_PATHS} lib)
    #MESSAGE(STATUS "   removing: /usr/${A}")
    LIST(REMOVE_ITEM LIB_DEPENDS_ON "-Wl,-rpath,/usr/${A}")
  ENDFOREACH()

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
          RENAME      "${PKG_NAME}-${SO_VERSION}.pc"
          COMPONENT development)
ENDIF()
ENDFUNCTION()

#*********************************************************************
# ---- Add definitions ----
#*********************************************************************
FUNCTION(ADD_ICL_DEFINITIONS)
  # iterate over list of arguments and define variable values in outer scope (for ICLConfig.h)
  foreach(var ${ARGV})
    string(REPLACE "-D" "" var "${var}")
    if(${var} MATCHES "=") # var_name=arg_value
      string(REPLACE "=" ";" split "${var}")
      list(GET split 0 var_name)
      list(GET split 1 arg_value)
      set(${var_name} ${arg_value} PARENT_SCOPE)
    else() # var_name only
      SET(${var} 1 PARENT_SCOPE)
    endif()
  endforeach()

  ADD_DEFINITIONS(${ARGV})
  SET(ICL_DEFINITIONS ${ICL_DEFINITIONS} ${ARGV} PARENT_SCOPE)
ENDFUNCTION()


function(get_ipp_version _INCLUDE_DIR)
  set(_VERSION_STR)
  set(_MAJOR)
  set(_MINOR)
  set(_BUILD)

  # read IPP version info from file
  file(STRINGS ${_INCLUDE_DIR}/ippversion.h STR1 REGEX "IPP_VERSION_MAJOR")
  file(STRINGS ${_INCLUDE_DIR}/ippversion.h STR2 REGEX "IPP_VERSION_MINOR")
  file(STRINGS ${_INCLUDE_DIR}/ippversion.h STR3 REGEX "IPP_VERSION_BUILD")
  file(STRINGS ${_INCLUDE_DIR}/ippversion.h STR4 REGEX "IPP_VERSION_STR")

  if (NOT STR3)
    file(STRINGS ${_INCLUDE_DIR}/ippversion.h STR3 REGEX "IPP_VERSION_UPDATE")
  endif(NOT STR3)

  # extract info and assign to variables
  string(REGEX MATCHALL "[0-9]+" _MAJOR ${STR1})
  string(REGEX MATCHALL "[0-9]+" _MINOR ${STR2})
  string(REGEX MATCHALL "[0-9]+" _BUILD ${STR3})
  string(REGEX MATCHALL "[0-9]+[.]+[0-9]+[^\"]+|[0-9]+[.]+[0-9]+" _VERSION_STR ${STR4})

  # export info to parent scope
  set(IPP_VERSION_STR   ${_VERSION_STR} PARENT_SCOPE)
  set(IPP_VERSION_MAJOR ${_MAJOR}       PARENT_SCOPE)
  set(IPP_VERSION_MINOR ${_MINOR}       PARENT_SCOPE)
  set(IPP_VERSION_BUILD ${_BUILD}       PARENT_SCOPE)

  MESSAGE(STATUS "           IPP Version: ${_MAJOR}.${_MINOR}.${_BUILD} [${_VERSION_STR}]")

  return()
endfunction()

#*********************************************************************
# ---- create opencl header files from source code ----
#*********************************************************************
function(CREATE_CL_HEADER file_in file_out kernel_source_name namespace_name)
	file(READ "${file_in}" file_content)
	# make program-internal semicolons invisible for cmake
	string(REGEX REPLACE ";" "\\\\;" file_content "${file_content}")
	# split the "string" into rows by replacing newline by E;
	string(REGEX REPLACE "\n" "E;" file_content "${file_content}")
	file(WRITE "${file_out}" "namespace icl {\n namespace ${namespace_name} {\n  const char ${kernel_source_name}[] = \n\"")
	foreach(lineE ${file_content})
		string(REGEX REPLACE "^(.*)E$" "\\1" line "${lineE}")
		file(APPEND "${file_out}" "  ${line}\\\n")
	endforeach()
	file(APPEND "${file_out}" "\";\n }\n}\n")
	return()
endfunction()

function(CONFIGURE_GTEST library_name library_root)
  string(TOLOWER ${library_name} library_lower)
  set(TEST_TARGET_NAME tests_${library_lower})
  file(GLOB TEST_FILES "${library_root}/test/test-*.cpp")
  if(TEST_FILES)
    message(STATUS "${TEST_TARGET_NAME}: ${TEST_FILES}")
    add_executable(${TEST_TARGET_NAME} ${TEST_FILES})
    target_link_libraries(${TEST_TARGET_NAME} gtest_main ${library_name})
    gtest_discover_tests(${TEST_TARGET_NAME} TEST_PREFIX ${library_lower})
    add_dependencies(tests ${TEST_TARGET_NAME})
    SETUP_TARGET_FOR_COVERAGE(NAME coverage_${library_lower}
                              EXECUTABLE ${CMAKE_BINARY_DIR}/${library_name}/${TEST_TARGET_NAME})
    add_dependencies(coverage coverage_${library_lower})
    add_test(NAME ${TEST_TARGET_NAME} COMMAND ${CMAKE_BINARY_DIR}/${library_name}/${TEST_TARGET_NAME})
  endif()
endfunction()
