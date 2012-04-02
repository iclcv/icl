#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : icl_cmake_macros.cmake                                 **
#** Module :                                                        **
#** Authors: Christian Groszewski, Christof Elbrechter              **
#**                                                                 **
#**                                                                 **
#** Commercial License                                              **
#** ICL can be used commercially, please refer to our website       **
#** www.iclcv.org for more details.                                 **
#**                                                                 **
#** GNU General Public License Usage                                **
#** Alternatively, this file may be used under the terms of the     **
#** GNU General Public License version 3.0 as published by the      **
#** Free Software Foundation and appearing in the file LICENSE.GPL  **
#** included in the packaging of this file.  Please review the      **
#** following information to ensure the GNU General Public License  **
#** version 3.0 requirements will be met:                           **
#** http://www.gnu.org/copyleft/gpl.html.                           **
#**                                                                 **
#** The development of this software was supported by the           **
#** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
#** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
#** Forschungsgemeinschaft (DFG) in the context of the German       **
#** Excellence Initiative.                                          **
#**                                                                 **
#*********************************************************************

message(STATUS "macros found")

macro(icl_check_single_path ID FILE WHAT RESULT)
  set(RESULT_TEXT "found")
  if(NOT EXISTS "${FILE}")
    set(${RESULT} FALSE)
    set(RESULT_TEXT "not found")
  endif()
  message(STATUS "${RESULT_TEXT} (${WHAT}): ${FILE} ... ")
endmacro()

macro(icl_check_lib ID LIB DIR RESULT)
  set(RESULT_TEXT "found")
  if(NOT EXISTS "${DIR}/lib${LIB}.so")
    if(NOT EXISTS "${DIR}/${ICL_REL_LIB_DIR_SUFFIX}/lib${LIB}.so")
      set(${RESULT} FALSE)
      set(RESULT_TEXT "not found")
    endif()
  endif()
  message(STATUS "${RESULT_TEXT} (library): lib${LIB}.so")
endmacro()

#standard check for external dependency
macro(icl_check_external_package ID INCFILES LIBFILES REL_LIB_DIR REL_INC_DIR VERBOSE_NOT_FOUND VERBOSE_FOUND)
  message(STATUS " ############### checking for ${ID} ####################")
  if(NOT ICL_XDEP_${ID}_ON)
    message(STATUS "${ID} detected: DEACTIVATED")
    set(HAVE_${ID}_COND FALSE)
  else()
    set(P "${ICL_XDEP_${ID}_PATH}")
    set(I "${P}/${REL_INC_DIR}")
    set(L "${P}/${REL_LIB_DIR}")

    set(FOUND_ALL TRUE)
    icl_check_single_path(${ID} "${P}" "root dir" FOUND_ALL)
    icl_check_single_path(${ID} "${I}" "include dir" FOUND_ALL)
    icl_check_single_path(${ID} "${L}" "lib dir" FOUND_ALL)
    
    foreach(lib ${LIBFILES})
      icl_check_lib(${ID} "${lib}" "${L}" FOUND_ALL) 
    endforeach()
    
    foreach(inc ${INCFILES})
      icl_check_single_path(${ID} "${I}/${inc}" "header" FOUND_ALL)
    endforeach()
    
    if(${FOUND_ALL})
      set(${ID}_LIB_PATH "${ICL_XDEP_${ID}_PATH}/${REL_LIB_DIR}")
      set(${ID}_INCLUDE_PATH "${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}")		
      set(ICL_XDEP_${ID}_ON ON CACHE BOOL "Use ${ID} when available" FORCE)
      set(HAVE_${ID}_COND TRUE)
      add_definitions( -DHAVE_${ID})
      include_directories(${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR})
      link_directories(${ICL_XDEP_${ID}_PATH}/${REL_LIB_DIR})
      if(${VERBOSE_FOUND})
        message(STATUS "${ID} detected: TRUE")
      endif()
      set(${ID}_LIBS_l ${LIBFILES})
    else()
      set(ICL_XDEP_${ID}_ON OFF CACHE BOOL "Use ${ID} when available" FORCE)
      if(${VERBOSE_NOT_FOUND})
        message(STATUS "${ID} detected: FALSE")
      endif()
    endif()
  endif()
endmacro()

macro(icl_simple_check_external_package ID INCFILES LIBFILES)
  icl_check_external_package(${ID} "${INCFILES}" "${LIBFILES}" lib include TRUE TRUE)
endmacro()

#dissolves internal dependencies and add them to new list, removes duplicates
macro(add_internal_dependencies DEPLIST dependencies)
  foreach(DEPENDENCY ${${DEPLIST}})
    add_internal_dependencies(${DEPENDENCY}_internal_dependencies ${dependencies})	
  endforeach()
  foreach(DEPENDENCY ${${DEPLIST}})
    set(${dependencies} "${DEPENDENCY};${${dependencies}}")	
  endforeach()
  list(LENGTH ${dependencies} listsize)
  if(${listsize} GREATER 1)
    list(REMOVE_DUPLICATES ${dependencies})
  endif()
endmacro()

#checks if external dependency is given and adds it to new list
macro(add_external_dependencies DEPLIST dependencies)
  foreach(DEPENDENCY ${${DEPLIST}})
    if(HAVE_${DEPENDENCY}_COND)
      set(${dependencies} "${${DEPENDENCY}_LIBS_l};${${dependencies}}")
    endif()
  endforeach()
endmacro()

#adds sourcefile to library sourcefile list if all conditions are met
macro(add_libsource PROJECT_NAME FILE CONDITIONLIST LIBSOURCES)
  set(COND TRUE)	
  foreach(CONDITION ${CONDITIONLIST})	
    if(NOT ${CONDITION})
      set(COND FALSE)
    endif()
  endforeach()
  if(${COND})
    #message(STATUS "adding ${FILE} to ${PROJECT_NAME}")
    set(${LIBSOURCES} "${${LIBSOURCES}};${FILE}")	
  endif()
endmacro()

#to build only examples specified in config file
macro(is_example_to_bulid EXAMPLE_NAME)
  if(NOT ${COND} OR ("${ICL_VAR_EXAMPLES_TO_BUILD}" STREQUAL "all"))
    #nothing to do here
  else()
    set(COND FALSE)
    foreach(T ${ICL_VAR_EXAMPLES_TO_BUILD})
      if(${EXAMPLE_NAME} STREQUAL ${T})
        set(COND TRUE)
      endif()
    endforeach()
  endif()
endmacro()

#adds example to current build
macro(add_example PROJECT_N FILE CONDITIONLIST ICLLibsToLinkAgainst)
  set(COND TRUE)	
  foreach(CONDITION ${CONDITIONLIST})
    if(NOT ${CONDITION})
      set(COND FALSE)
    endif()
  endforeach()
  is_example_to_bulid(${FILE})
  if(${COND})
    add_executable(icl-${FILE} examples/${FILE}.cpp)
    target_link_libraries(icl-${FILE} ${${ICLLibsToLinkAgainst}})
    install (TARGETS icl-${FILE} 
      PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
      GROUP_EXECUTE GROUP_WRITE GROUP_READ 
      WORLD_READ WORLD_EXECUTE 
      RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_OUTPUT_PATH}
      COMPONENT applications)
  endif()
endmacro()

#adds example consisting of more than one file to current build
macro(add_example_mult PROJECT_N EXAMPLE_NAME FILES CONDITIONLIST ICLLibsToLinkAgainst)
  set(COND TRUE)	
  foreach(CONDITION ${CONDITIONLIST})
    if(NOT ${CONDITION})
      set(COND FALSE)
    endif()
  endforeach()
  is_example_to_bulid(${EXAMPLE_NAME})
  if(${COND})
    add_executable(icl-${EXAMPLE_NAME} ${FILES})
    target_link_libraries(icl-${EXAMPLE_NAME} ${${ICLLibsToLinkAgainst}})
    install (TARGETS icl-${EXAMPLE_NAME} 
      PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
      GROUP_EXECUTE GROUP_WRITE GROUP_READ 
      WORLD_READ WORLD_EXECUTE 
      RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_OUTPUT_PATH}
      COMPONENT applications)
  endif()
endmacro()

#adds gtest to current build
macro(add_gtest PROJECT_N FILE CONDITIONLIST ICLLibsToLinkAgainst)
  set(COND TRUE)	
  foreach(CONDITION ${CONDITIONLIST})
    if(NOT ${CONDITION})
      set(COND FALSE)
    endif()
  endforeach()
  if(${COND})
    set(lINCLUDES -I${GTEST_INCLUDE_PATH} -I${CMAKE_SOURCE_DIR}/include)
    set(lLIBS -l${GTEST_LIBS_l})
    #set(lLIBDIRS -L${GTEST_LIB_PATH} -Wl,-rpath -Wl,${GTEST_LIB_PATH})
    set(lLIBDIRS -L${GTEST_LIB_PATH} -Wl,-rpath,${GTEST_LIB_PATH})
    foreach(l ${${ICLLibsToLinkAgainst}})
      set(lLIBS ${lLIBS} -l${l})
      #set(lLIBDIRS ${lLIBDIRS} -L${CMAKE_BINARY_DIR}/${l}/lib -Wl,-rpath -Wl,${CMAKE_BINARY_DIR}/${l}/lib)
      set(lLIBDIRS ${lLIBDIRS} -L${CMAKE_BINARY_DIR}/${l}/lib -Wl,-rpath,${CMAKE_BINARY_DIR}/${l}/lib)
    endforeach()
    add_custom_target(check 
      COMMAND 
      g++ -O0 -DHAVE_GTEST ${lLIBDIRS} ${lLIBS} -pthread ${lINCLUDES} "${FILE}.cpp" runner.cpp -o icl-${FILE}
      COMMAND ./icl-${FILE}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${PROJECT_N}/test 
      VERBATIM)
  endif()
endmacro()

#generates docs and cleans them if necessary
macro(add_doc_gen PROJECT_NAME)
  if(DOXYGEN_FOUND)
    add_custom_target(doc doxygen doc/doxyfile)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html/
      DESTINATION ${CMAKE_INSTALL_PREFIX}/${DOC_DIR}/${PROJECT_NAME}
      FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
      GROUP_EXECUTE GROUP_WRITE GROUP_READ 
      WORLD_READ WORLD_EXECUTE
      DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
      GROUP_EXECUTE GROUP_WRITE GROUP_READ 
      WORLD_READ WORLD_EXECUTE
      COMPONENT docu)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/
      DESTINATION ${CMAKE_INSTALL_PREFIX}/${DOC_DIR}/${PROJECT_NAME}
      FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
      GROUP_EXECUTE GROUP_WRITE GROUP_READ 
      WORLD_READ WORLD_EXECUTE
      DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
      GROUP_EXECUTE GROUP_WRITE GROUP_READ 
      WORLD_READ WORLD_EXECUTE
      COMPONENT docu
      PATTERN "doxyfile" EXCLUDE
      REGEX .svn* EXCLUDE
      )
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES doc/html/.)
  endif()
endmacro()


#generates the pkg-config file for ICL_SUB_PACKAGE
macro(icl_create_pkg_config_file2 PACKAGE_NAME ICL_SUB_PACKAGE_PLACEHOLDER REQUIRE_INTERNAL REQUIRE_EXTERNAL)
  set(ICL_SUB_PACKAGE "${ICL_SUB_PACKAGE_PLACEHOLDER}")
  set(REQUIRE "")
  set(OPTIONAL_INCLUDES "") 
  set(OPTIONAL_LIBS "")

  foreach(TEMP ${REQUIRE_INTERNAL})
    set(REQUIRE "${REQUIRE} ${TEMP}-${VERSION}${DEBUG_POSTFIX}")
  endforeach()
  foreach(REQ ${REQUIRE_EXTERNAL})
    if(${HAVE_${REQ}_COND})
      foreach(inc ${${REQ}_INCLUDE_PATH})
	set(OPTIONAL_INCLUDES "${OPTIONAL_INCLUDES} -I${inc}")
      endforeach()
      set(OPTIONAL_INCLUDES "${OPTIONAL_INCLUDES} -DHAVE_${REQ}")
      foreach(libp2 ${${REQ}_LIB_PATH})
      	set(OPTIONAL_LIBS "${OPTIONAL_LIBS} -L${libp2}")
      endforeach()
      foreach(LIBRARY ${${REQ}_LIBS_l})
	set(OPTIONAL_LIBS "${OPTIONAL_LIBS} -l${LIBRARY}")
      endforeach()
      foreach(libp ${${REQ}_LIB_PATH})
	if(NOT "/usr/lib" STREQUAL ${libp})
      	  #set(OPTIONAL_LIBS "${OPTIONAL_LIBS} '-Wl,-rpath -Wl,${libp}'")
	  set(OPTIONAL_LIBS "${OPTIONAL_LIBS} -Wl,-rpath,${libp}")
	endif()
      endforeach()
    endif()
  endforeach()

  set(ICL_PACKAGE_DESCRIPTION "ICL's ${ICL_SUB_PACKAGE_PLACEHOLDER} package")
  #set(LIBS "-L\${libdir} -l${PACKAGE_NAME} '-Wl,-rpath -Wl,\${libdir}' ${OPTIONAL_LIBS} -pthread")
  set(LIBS "-L\${libdir} -l${PACKAGE_NAME} -Wl,-rpath,\${libdir} ${OPTIONAL_LIBS} -pthread")
  set(INCLUDES "-I\${includedir} ${OPTIONAL_INCLUDES}")
  configure_file(pkg.in ${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}.pc @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}.pc" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_WRITE GROUP_READ 
    WORLD_READ WORLD_EXECUTE)
  set(${OPTIONAL_INCLUDES} "")
  set(${OPTIONAL_LIBS} "")
endmacro()

#shorter call for icl_create_pkg_config_file2
macro(icl_create_pkg_config_file_if PACKAGE CONDITION)
  #if("${CONDITION}" STREQUAL  "TRUE")
  if(${CONDITION})
    message(STATUS "creating ${PACKAGE}-${VERSION}${DEBUG_POSTFIX}.pc")
    icl_create_pkg_config_file2("${PACKAGE}" "${PACKAGE}-${VERSION}${DEBUG_POSTFIX}" "${${PACKAGE}_internal_dependencies}" "${${PACKAGE}_external_dependencies}")
  else()
    message(STATUS "skipped creation of ${PACKAGE}-${VERSION}${DEBUG_POSTFIX}.pc")
  endif()
endmacro()

###QT MACROS####
MACRO (QT4_EXTRACT_OPTIONS _qt4_files _qt4_options)
  SET(${_qt4_files})
  SET(${_qt4_options})
  SET(_QT4_DOING_OPTIONS FALSE)
  FOREACH(_currentArg ${ARGN})
    IF ("${_currentArg}" STREQUAL "OPTIONS")
      SET(_QT4_DOING_OPTIONS TRUE)
    ELSE ("${_currentArg}" STREQUAL "OPTIONS")
      IF(_QT4_DOING_OPTIONS) 
        LIST(APPEND ${_qt4_options} "${_currentArg}")
      ELSE(_QT4_DOING_OPTIONS)
        LIST(APPEND ${_qt4_files} "${_currentArg}")
      ENDIF(_QT4_DOING_OPTIONS)
    ENDIF ("${_currentArg}" STREQUAL "OPTIONS")
  ENDFOREACH(_currentArg) 
ENDMACRO (QT4_EXTRACT_OPTIONS)

MACRO (QT4_MAKE_OUTPUT_FILE infile prefix ext outfile )
  STRING(LENGTH ${CMAKE_CURRENT_BINARY_DIR} _binlength)
  STRING(LENGTH ${infile} _infileLength)
  SET(_checkinfile ${CMAKE_CURRENT_SOURCE_DIR})
  IF(_infileLength GREATER _binlength)
    STRING(SUBSTRING "${infile}" 0 ${_binlength} _checkinfile)
    IF(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
      FILE(RELATIVE_PATH rel ${CMAKE_CURRENT_BINARY_DIR} ${infile})
    ELSE(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
      FILE(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
    ENDIF(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  ELSE(_infileLength GREATER _binlength)
    FILE(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
  ENDIF(_infileLength GREATER _binlength)
  IF(WIN32 AND rel MATCHES "^[a-zA-Z]:") # absolute path 
    STRING(REGEX REPLACE "^([a-zA-Z]):(.*)$" "\\1_\\2" rel "${rel}")
  ENDIF(WIN32 AND rel MATCHES "^[a-zA-Z]:") 
  SET(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${rel}")
  STRING(REPLACE ".." "__" _outfile ${_outfile})
  GET_FILENAME_COMPONENT(outpath ${_outfile} PATH)
  GET_FILENAME_COMPONENT(_outfile ${_outfile} NAME_WE)
  FILE(MAKE_DIRECTORY ${outpath})
  SET(${outfile} ${outpath}/${prefix}${_outfile}.${ext})
ENDMACRO (QT4_MAKE_OUTPUT_FILE )


#not finished yet
macro(process_moc_files FILES outfiles)
  set(moc_found "")
  set(moc_path_ex "")
  set(QT_MOC_EXECUTABLE "")
  execute_process(
    COMMAND pkg-config --variable moc_location QtCore 
    OUTPUT_VARIABLE QT_MOC_EXECUTABLE
    RESULT_VARIABLE moc_found)
  string(STRIP "${QT_MOC_EXECUTABLE}" QT_MOC_EXECUTABLE)

  if(NOT ${QT_MOC_EXECUTABLE} STREQUAL "")
    message(STATUS "moc_exe:${QT_MOC_EXECUTABLE}")

    #if using apple we need to do this
    # QT4_GET_MOC_FLAGS(moc_flags)
    SET(_moc_flags "")
    GET_DIRECTORY_PROPERTY(_inc_DIRS INCLUDE_DIRECTORIES)
    set(framework_path ${QT_INCLUDE_PATH})
    FOREACH(_current ${_inc_DIRS})
      IF("${_current}" MATCHES ".framework/?$")
        STRING(REGEX REPLACE "/[^/]+.framework" "" framework_path "${_current}")
        SET(_moc_flags ${_moc_flags} "-F${framework_path}")
      ELSE("${_current}" MATCHES ".framework/?$")
        SET(_moc_flags ${_moc_flags} "-I${_current}")
      ENDIF()
    ENDFOREACH(_current ${_inc_DIRS})

    GET_DIRECTORY_PROPERTY(_defines COMPILE_DEFINITIONS)
    FOREACH(_current ${_defines})
      SET(_moc_flags ${_moc_flags} "-D${_current}")
    ENDFOREACH(_current ${_defines})
    
    set(moc_options "")
    QT4_EXTRACT_OPTIONS(moc_files moc_options ${${FILES}})
    #message(STATUS "mf: ${moc_files}")
    foreach(it ${moc_files})
      GET_FILENAME_COMPONENT(it ${it} ABSOLUTE)
      QT4_MAKE_OUTPUT_FILE(${it} moc_ cxx outfile)
      ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
        COMMAND ${QT_MOC_EXECUTABLE}
        ARGS ${_moc_flags} ${moc_options} -o ${outfile} ${it}
        DEPENDS ${it})
      set(${outfiles} ${${outfiles}} ${outfile})	
    endforeach()
  endif()
endmacro()

MACRO (TODAY RESULT)
    IF (WIN32)
        EXECUTE_PROCESS(COMMAND "date" "/T" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "20\\3/\\2/\\1" ${RESULT} ${${RESULT}})
    ELSEIF(UNIX)
        EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "20\\3/\\2/\\1" ${RESULT} ${${RESULT}})
    ELSE (WIN32)
        MESSAGE(SEND_ERROR "date not implemented")
        SET(${RESULT} 000000)
    ENDIF (WIN32)
ENDMACRO (TODAY)
