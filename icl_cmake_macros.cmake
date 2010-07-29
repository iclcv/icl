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
include(CheckIncludeFile)
message(STATUS "macros found")

macro(icl_check_external_package ID FFILE REL_LIB_DIR REL_INC_DIR DEFAULT_PATH DEFINE_COND)
  set(${DEFINE_COND} FALSE)
  message(STATUS "-- checking package ${ID} --")
  message(STATUS "searching for ${FFILE}")# in ${DEFAULT_PATH}/${REL_INC_DIR}")# and ${${ID}_PATH}/${REL_INC_DIR}")
  find_path(ICL_XDEP_${ID}_PATH "${REL_INC_DIR}" PATHS "${DEFAULT_PATH}" "${ICL_XDEP_${ID}_PATH}" DOC "The path to ${ID}" NO_DEFAULT_PATH)
if(EXISTS ${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR})
	if(EXISTS ${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}/${FFILE})
		message(STATUS "found: ${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}/${FFILE}")
		message(STATUS "${ID} detected: TRUE")
		if(${ICL_XDEP_${ID}_ON} OR ${ICL_XDEP_ALL_ON})
			
        	set(${ID}_LIB_PATH "${ICL_XDEP_${ID}_PATH}/${REL_LIB_DIR}")
        	set(${ID}_INCLUDE_PATH "${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}")		
			set(ICL_XDEP_${ID}_ON ON CACHE BOOL "Use ${ID} when available" FORCE)
        	set(${DEFINE_COND} TRUE)
        	add_definitions( -DHAVE_${ID})
			message(STATUS "${ID} include path:${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}")
			message(STATUS "${ID} lib path:${ICL_XDEP_${ID}_PATH}/${REL_LIB_DIR}")			
			include_directories(${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR})
        	link_directories(${ICL_XDEP_${ID}_PATH}/${REL_LIB_DIR})
      	else()
        	set(${DEFINE_COND} FALSE)
      	endif()
	else()
		message(STATUS "File ${FFILE} not found in ${ICL_XDEP_${ID}_PATH}")
    endif()

else()
	find_path(ICL_XDEP_${ID}_PATH "${REL_INC_DIR}" PATHS "ICL_XDEP_${ID}_PATH-NOTFOUND" 
    DOC "The path to ${ID}" NO_DEFAULT_PATH )
	message(STATUS "Path ${ICL_XDEP_${ID}_PATH} not found")
	set(${DEFINE_COND} FALSE)
    set(ICL_XDEP_${ID}_ON OFF CACHE BOOL "Use ${ID} when available" FORCE)
	message(STATUS "${ID} detected: FALSE")
endif()
endmacro()

macro(icl_check_external_package2 ID FFILE REL_LIB_DIR REL_INC_DIR DEFAULT_PATH DEFINE_COND PKG_NAME)
  set(${DEFINE_COND} FALSE)
  message(STATUS "-- checking package ${ID} --")
  #if(ICL_VAR_USE_PKG_CONFIG)
  pkg_check_modules(${ID}_PKG ${PKG_NAME})
		if(NOT ${ID}_PKG_FOUND)
    		message(STATUS "package config file ${PKG_NAME}.pc not found")# -> ICLPackage ${PACKAGE} disabled")
			set(SPATH "${DEFAULT_PATH}")	
			message(STATUS "bla:${SPATH}")	
		else()
			set(PKG_${ID}_LIBS_l "${${ID}_PKG_LIBRARIES}")
message(STATUS "LIBS_l:${PKG_${ID}_LIBS_l}")
            set(PKG_${ID}_LIBS_L "${${ID}_PKG_LIBRARY_DIRS}")
message(STATUS "LIBS_L:${PKG_${ID}_LIBS_L}")
            set(PKG_${ID}_LIBS_OTHER "${${ID}_PKG_LDFLAGS_OTHER}")
message(STATUS "LIBS_OTHER:${PKG_${ID}_LIBS_OTHER}")
			set(PKG_${ID}_CFLAGS_I "${${ID}_PKG_INCLUDE_DIRS}")
message(STATUS "CFLAGS_I:${PKG_${ID}_CFLAGS_I}")
			set(PKG_${ID}_CFLAGS_OTHER "${${ID}_PKG_CFLAGS_OTHER}")
message(STATUS "CFLAGS_OTHER:${PKG_${ID}_CFLAGS_OTHER}")
			set(SPATH "")
			set(pfound "")
			execute_process(
   COMMAND pkg-config --variable prefix QtCore 
   OUTPUT_VARIABLE SPATH
   RESULT_VARIABLE p_found)
string(STRIP "${SPATH}" SPATH)
message(STATUS "spath:${SPATH}")
		endif()
message(STATUS "ssspath:${SPATH}")
  #endif()
	
  message(STATUS "searching for ${FFILE}")# in ${DEFAULT_PATH}/${REL_INC_DIR}")# and ${${ID}_PATH}/${REL_INC_DIR}")
  find_path(ICL_XDEP_${ID}_PATH "${REL_INC_DIR}" PATHS "${SPATH}" "${${ID}_PATH}" DOC "The path to ${ID}" NO_DEFAULT_PATH)
if(EXISTS ${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR})
	if(EXISTS ${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}/${FFILE})
		message(STATUS "found: ${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}/${FFILE}")
		message(STATUS "${ID} detected: TRUE")
		if(${ICL_XDEP_${ID}_ON} OR ${ICL_XDEP_ALL_ON})
			
        	set(${ID}_LIB_PATH "${ICL_XDEP_${ID}_PATH}/${REL_LIB_DIR}")
        	set(${ID}_INCLUDE_PATH "${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}")		
			set(ICL_XDEP_${ID}_ON ON CACHE BOOL "Use ${ID} when available" FORCE)
        	set(${DEFINE_COND} TRUE)
        	add_definitions( -DHAVE_${ID})
			message(STATUS "${ID} include path:${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}")
			message(STATUS "${ID} lib path:${ICL_XDEP_${ID}_PATH}/${REL_LIB_DIR}")			
			include_directories(${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR})
        	link_directories(${ICL_XDEP_${ID}_PATH}/${REL_LIB_DIR})
      	else()
        	set(${DEFINE_COND} FALSE)
      	endif()
	else()
		message(STATUS "File ${FFILE} not found in ${ICL_XDEP_${ID}_PATH}")
    endif()

else()
	find_path(ICL_XDEP_${ID}_PATH "${FFILE}" PATHS "ICL_XDEP_${ID}_PATH-NOTFOUND" #"${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}"
    DOC "The path to ${ID}" NO_DEFAULT_PATH)
	message(STATUS "Path ${ICL_XDEP_${ID}_PATH} not found")
	set(${DEFINE_COND} FALSE)
    set(ICL_XDEP_${ID}_ON OFF CACHE BOOL "Use ${ID} when available" FORCE)
	message(STATUS "${ID} detected: FALSE")
endif()
endmacro()


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

macro(add_external_dependencies DEPLIST dependencies)
  foreach(DEPENDENCY ${${DEPLIST}})
    if(HAVE_${DEPENDENCY}_COND)
      set(${dependencies} "${${DEPENDENCY}_LIBS_l};${${dependencies}}")
    endif()
  endforeach()
endmacro()

macro(add_libsource PROJECT_NAME FILE CONDITIONLIST LIBSOURCES)
  set(COND TRUE)	
  foreach(CONDITION ${CONDITIONLIST})	
    if(NOT ${CONDITION})
      set(COND FALSE)
    endif()
  endforeach()
  if(${COND})
    set(${LIBSOURCES} "${${LIBSOURCES}};${FILE}")	
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
  if(${COND})
    add_executable(icl-${FILE} examples/${FILE}.cpp)
    target_link_libraries(icl-${FILE} ${${ICLLibsToLinkAgainst}})
    install (TARGETS icl-${FILE} RUNTIME DESTINATION bin)
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
  if(${COND})
    add_executable(icl-${EXAMPLE_NAME} ${FILES})
    target_link_libraries(icl-${EXAMPLE_NAME} ${${ICLLibsToLinkAgainst}})
    install (TARGETS icl-${EXAMPLE_NAME} RUNTIME DESTINATION bin)
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
  set(lLIBDIRS -L${GTEST_LIB_PATH} -Wl,-rpath=${GTEST_LIB_PATH})
  foreach(l ${${ICLLibsToLinkAgainst}})
	set(lLIBS ${lLIBS} -l${l})
    set(lLIBDIRS ${lLIBDIRS} -L${CMAKE_BINARY_DIR}/${l}/lib -Wl,-rpath=${CMAKE_BINARY_DIR}/${l}/lib)
  endforeach()
  add_custom_target(check 
		COMMAND 
		g++ -O0 -DHAVE_GTEST ${lLIBDIRS} ${lLIBS} -pthread ${lINCLUDES} "${FILE}.cpp" runner.cpp -o icl-${FILE}
		COMMAND ./icl-${FILE}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${PROJECT_N}/test 
		VERBATIM
	)
  endif()
endmacro()

macro(add_doc_gen PROJECT_NAME)
  if(DOXYGEN_FOUND)
    add_custom_target(doc doxygen doc/doxyfile)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html/
      DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/${PROJECT_NAME}
      )
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/
      DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/${PROJECT_NAME}
      PATTERN "doxyfile" EXCLUDE
	REGEX .svn* EXCLUDE
      )
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES doc/html/.)
  endif()
endmacro()


macro(icl_create_pkg_config_file2 ICL_SUB_PACKAGE_PLACEHOLDER REQUIRE_INTERNAL REQUIRE_EXTERNAL)
  set(ICL_SUB_PACKAGE "${ICL_SUB_PACKAGE_PLACEHOLDER}")
  set(REQUIRE "")
  set(OPTIONAL_INCLUDES "") 
  set(OPTIONAL_LIBS "")

  foreach(TEMP ${REQUIRE_INTERNAL})
    set(REQUIRE "${REQUIRE} ${TEMP}")
  endforeach()
  #message(STATUS "reqin: ${REQUIRE_INTERNAL}")
  foreach(REQ ${REQUIRE_EXTERNAL})
	    
	if(${HAVE_${REQ}_COND})
      #message(STATUS "have ${REQ}")
      set(OPTIONAL_INCLUDES "${OPTIONAL_INCLUDES} -I${${REQ}_INCLUDE_PATH} -DHAVE_${REQ}")
      set(OPTIONAL_LIBS "${OPTIONAL_LIBS} -L${${REQ}_LIB_PATH}")
	  #message(STATUS ${${REQ}_LIB_PATH})
      foreach(LIBRARY ${${REQ}_LIBS_l})
	set(OPTIONAL_LIBS "${OPTIONAL_LIBS} -l${LIBRARY}")
      endforeach()
      set(OPTIONAL_LIBS "${OPTIONAL_LIBS} -Wl,-rpath -Wl,${${REQ}_LIB_PATH}")
    endif()
  endforeach()
  set(ICL_PACKAGE_DESCRIPTION "ICL's ${ICL_SUB_PACKAGE_PLACEHOLDER} package")
  set(LIBS "-L\${libdir} -l${ICL_SUB_PACKAGE_PLACEHOLDER} '-Wl,-rpath -Wl,\${libdir}' ${OPTIONAL_LIBS} -pthread")
  set(INCLUDES "-I\${includedir}/ICL ${OPTIONAL_INCLUDES}")
  configure_file(pkg.in ${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}.pc @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}.pc" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/)
  set(${OPTIONAL_INCLUDES} "")
  set(${OPTIONAL_LIBS} "")
endmacro()


macro(icl_create_pkg_config_file_if PACKAGE CONDITION)
  if("${CONDITION}" STREQUAL  "TRUE")
    message(STATUS "creating ${PACKAGE}.pc")
    icl_create_pkg_config_file2("${PACKAGE}" "${${PACKAGE}_internal_dependencies}" "${${PACKAGE}_external_dependencies}")
  else()
    message(STATUS "skipped creation of ${PACKAGE}.pc")
  endif()
endmacro()

macro(icl_check_external_package_from_pkg_conf ID PKG_CONFIG_LIST DEFINE_COND)
set(${DEFINE_COND} FALSE)
message(STATUS "-- checking package ${ID} --")
pkg_check_modules(${ID}_PKG ${PKG_CONFIG_LIST})
if(NOT ${ID}_PKG_FOUND)
    message(STATUS "package config file ${PKG_NAME}.pc not found")	
else()
	set(SPATH "")
	set(pfound "")
	execute_process(COMMAND pkg-config --variable prefix ${PKG_CONFIG_LIST} 
   				OUTPUT_VARIABLE SPATH RESULT_VARIABLE p_found)
	string(STRIP "${SPATH}" SPATH)
	find_path(ICL_XDEP_${ID}_PATH "" PATHS "${SPATH}" "${ICL_XDEP_${ID}_PATH}" DOC "The path to ${ID}" NO_DEFAULT_PATH)
endif()
if(EXISTS ${SPATH})
		message(STATUS "found: ${ICL_XDEP_${ID}_PATH}")
		message(STATUS "${ID} detected: TRUE")
		if(${ICL_XDEP_${ID}_ON} OR ${ICL_XDEP_ALL_ON})
			string(STRIP "${${ID}_PKG_LIBRARY_DIRS}" ${ID}_PKG_LIBRARY_DIRS)
			string(STRIP "${${ID}_PKG_INCLUDE_DIRS}" ${ID}_PKG_INCLUDE_DIRS)
			if(NOT "${${ID}_PKG_LIBRARY_DIRS}" STREQUAL "")
        		set(${ID}_LIB_PATH "${${ID}_PKG_LIBRARY_DIRS}")
			else()
				set(${ID}_LIB_PATH "/usr/lib")
			endif()
			if(NOT "${${ID}_PKG_INCLUDE_DIRS}" STREQUAL "")
        		set(${ID}_INCLUDE_PATH "${${ID}_PKG_INCLUDE_DIRS}")
			else()
				set(${ID}_INCLUDE_PATH "/usr/include")
			endif()			
			set(${ID}_LIBS_l "${${ID}_PKG_LIBRARIES}")
			
			set(ICL_XDEP_${ID}_ON ON CACHE BOOL "Use ${ID} when available" FORCE)
        	set(${DEFINE_COND} TRUE)
        	add_definitions( -DHAVE_${ID})
			message(STATUS "${ID} include path:${${ID}_INCLUDE_PATH}")
			message(STATUS "${ID} lib path:${${ID}_LIB_PATH}")	
					
			include_directories(${${ID}_INCLUDE_PATH})
        	link_directories(${${ID}_LIB_PATH})
      	else()
        	set(${DEFINE_COND} FALSE)
      	endif()
else()
	find_path(ICL_XDEP_${ID}_PATH "${FFILE}" PATHS "ICL_XDEP_${ID}_PATH-NOTFOUND" #"${ICL_XDEP_${ID}_PATH}/${REL_INC_DIR}"
    DOC "The path to ${ID}" NO_DEFAULT_PATH)
	message(STATUS "Path ${ICL_XDEP_${ID}_PATH} not found")
	set(${DEFINE_COND} FALSE)
    set(ICL_XDEP_${ID}_ON OFF CACHE BOOL "Use ${ID} when available" FORCE)
	message(STATUS "${ID} detected: FALSE")
endif()
endmacro()
######################
macro(icl_check_external_package_from_pkg_conf_mult ID DEFINE_COND)
set(${DEFINE_COND} FALSE)
set(${ID}_ALL_FOUND TRUE)
message(STATUS "-- checking package ${ID} --")

set(SPATH "")
set(pfound "")
set(${ID}_LIB_PATH_TMP "")
set(${ID}_INCLUDE_PATH_TMP "")
set(${ID}_LIBS_TMP "")
foreach(item ${ARGN})
	pkg_check_modules(${item}_PKG ${item})
	if(NOT ${item}_PKG_FOUND)
		message(STATUS "package config file ${item}.pc not found")
		set(${ID}_ALL_FOUND FALSE)
        break()
	else()
		string(STRIP "${${item}_PKG_LIBRARY_DIRS}" ${item}_PKG_LIBRARY_DIRS)
		string(STRIP "${${item}_PKG_INCLUDE_DIRS}" ${item}_PKG_INCLUDE_DIRS)
			execute_process(COMMAND pkg-config --variable prefix ${item} 
   				OUTPUT_VARIABLE SPATH RESULT_VARIABLE p_found)
			if(NOT "${${item}_PKG_LIBRARY_DIRS}" STREQUAL "")
        		set(${ID}_LIB_PATH_TMP ${${ID}_LIB_PATH_TMP} ${${item}_PKG_LIBRARY_DIRS})
			else()
				set(${item}_LIB_PATH_TMP ${${ID}_LIB_PATH_TMP} /usr/lib)
			endif()

			if(NOT "${${item}_PKG_INCLUDE_DIRS}" STREQUAL "")
        	set(${ID}_INCLUDE_PATH_TMP ${${ID}_INCLUDE_PATH_TMP} ${${item}_PKG_INCLUDE_DIRS})
			else()
				set(${ID}_INCLUDE_PATH_TMP ${${ID}_INCLUDE_PATH_TMP} /usr/include)
			endif()
		set(${ID}_LIBS_TMP ${${ID}_LIBS_TMP} ${${item}_PKG_LIBRARIES})
	endif()
endforeach()
#remove duplicates from temporary lists
list(REMOVE_DUPLICATES ${ID}_LIB_PATH_TMP)
list(REMOVE_DUPLICATES ${ID}_INCLUDE_PATH_TMP)
list(REMOVE_DUPLICATES ${ID}_LIBS_TMP)
#remove pthread from libs
list(REMOVE_ITEM  ${ID}_LIBS_TMP pthread)			
#searchpath for gui
string(STRIP "${SPATH}" SPATH)

find_path(ICL_XDEP_${ID}_PATH "" PATHS "${SPATH}" "${ICL_XDEP_${ID}_PATH}" DOC "The path to ${ID}" NO_DEFAULT_PATH)
if(${ID}_ALL_FOUND)
if(EXISTS ${SPATH})
		message(STATUS "found: ${ICL_XDEP_${ID}_PATH}")
		message(STATUS "${ID} detected: TRUE")
		set(${ID}_PKG_FOUND TRUE)
		if(${ICL_XDEP_${ID}_ON} OR ${ICL_XDEP_ALL_ON})
        	set(${ID}_LIB_PATH "${${ID}_LIB_PATH_TMP}")
			set(${ID}_INCLUDE_PATH "${${ID}_INCLUDE_PATH_TMP}")
			set(${ID}_LIBS_l "${${ID}_LIBS_TMP}")
			
			set(ICL_XDEP_${ID}_ON ON CACHE BOOL "Use ${ID} when available" FORCE)
        	set(${DEFINE_COND} TRUE)
        	add_definitions( -DHAVE_${ID})
			message(STATUS "${ID} include path:${${ID}_INCLUDE_PATH}")
			message(STATUS "${ID} lib path:${${ID}_LIB_PATH}")	
					
			include_directories(${${ID}_INCLUDE_PATH})
        	link_directories(${${ID}_LIB_PATH})
      	else()
        	set(${DEFINE_COND} FALSE)
      	endif()
else()
	find_path(ICL_XDEP_${ID}_PATH "include" PATHS "ICL_XDEP_${ID}_PATH-NOTFOUND"
    DOC "The path to ${ID}" NO_DEFAULT_PATH)
	message(STATUS "Path ${ICL_XDEP_${ID}_PATH} not found")
	set(${DEFINE_COND} FALSE)
    set(ICL_XDEP_${ID}_ON OFF CACHE BOOL "Use ${ID} when available" FORCE)
	message(STATUS "${ID} detected: FALSE")
endif()
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
