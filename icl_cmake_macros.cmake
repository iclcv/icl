include(CheckIncludeFile)

message(STATUS "macros found")
macro(icl_check_external_package ID FFILE REL_LIB_DIR REL_INC_DIR DEFAULT_PATH DEFINE_COND)
  set(${DEFINE_COND} FALSE)
  message(STATUS "-- checking package ${ID} --")
  message(STATUS "searching for ${FFILE}")# in ${DEFAULT_PATH}/${REL_INC_DIR}")# and ${${ID}_PATH}/${REL_INC_DIR}")
  find_path(${ID}_PATH "${REL_INC_DIR}" PATHS "${DEFAULT_PATH}" "${${ID}_PATH}" #PATH_SUFFIXES "${REL_INC_DIR}" 
     DOC "The path to ${ID}" NO_DEFAULT_PATH)
#message(STATUS "path: ${${ID}_PATH}")
if(EXISTS ${${ID}_PATH}/${REL_INC_DIR})
	message(STATUS "found path: ${${ID}_PATH}")    
	if(EXISTS ${${ID}_PATH}/${REL_INC_DIR}/${FFILE})
		message(STATUS "${ID} detected: TRUE")
		if(${USE_${ID}} OR ${ALL_ON})
        	set(${ID}_LIB_PATH "${${ID}_PATH}/${REL_LIB_DIR}")
        	set(${ID}_INCLUDE_PATH "${${ID}_PATH}/${REL_INC_DIR}")		
        	set(USE_${ID} ON CACHE BOOL "Use ${ID} when available" FORCE)
        	set(${DEFINE_COND} TRUE)
        	add_definitions( -DHAVE_${ID})
			include_directories(${${ID}_PATH}/${REL_INC_DIR})
        	link_directories(${${ID}_PATH}/${REL_LIB_DIR})
        	#include_directories(${DEFAULT_PATH}/${REL_INC_DIR})
        	#link_directories(${DEFAULT_PATH}/${REL_LIB_DIR})
      	else()
        	set(${DEFINE_COND} FALSE)
      	endif()
	else()
		message(STATUS "File ${FFILE} not found in ${${ID}_PATH}")
    endif()

else()
	find_path(${ID}_PATH "${FFILE}" PATHS "${ID}_PATH-NOTFOUND" "${${ID}_PATH}/${REL_INC_DIR}"
    DOC "The path to ${ID}" NO_DEFAULT_PATH)
	message(STATUS "Path ${${ID}_PATH} not found")
	set(${DEFINE_COND} FALSE)
    set(USE_${ID} OFF CACHE BOOL "Use ${ID} when available" FORCE)
	message(STATUS "${ID} detected: FALSE")
endif()
  
endmacro()


macro(add_internal_dependencies DEPLIST dependencies)
  #message(STATUS "depp: ${${DEPLIST}}")
  foreach(DEPENDENCY ${${DEPLIST}})
    #message(STATUS "look for ${DEPENDENCY}_internal_dependencies")		
    add_internal_dependencies(${DEPENDENCY}_internal_dependencies ${dependencies})	
  endforeach()
  #message(STATUS "de: ${${dependencies}}")	
  foreach(DEPENDENCY ${${DEPLIST}})
    set(${dependencies} "${DEPENDENCY};${${dependencies}}")	
  endforeach()
  #set(templist ${${dependencies}})
  list(LENGTH ${dependencies} listsize)
  if(${listsize} GREATER 1)
    #message(STATUS "removing")	
    list(REMOVE_DUPLICATES ${dependencies})
  endif()
endmacro()

macro(add_external_dependencies DEPLIST dependencies)
	#message(STATUS "${DEPLIST}")
  foreach(DEPENDENCY ${${DEPLIST}})
    if(HAVE_${DEPENDENCY}_COND)
		#message(STATUS "adding dependeny ${DEPENDENCY}")
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
    #message(STATUS "added ${FILE} to ${PROJECT_NAME} sources.")		
  endif() 
  #message(STATUS "ddd:${${LIBSOURCES}}")  
endmacro()

macro(add_example PROJECT_N FILE CONDITIONLIST ICLLibsToLinkAgainst)
  set(COND TRUE)	
  foreach(CONDITION ${CONDITIONLIST})
    if(NOT ${CONDITION})
      set(COND FALSE)
    endif()
  endforeach()
  if(${COND})
	#message(STATUS "${PROJECT_N} ${FILE}")
    add_executable(icl-${FILE} examples/${FILE}.cpp)
    target_link_libraries(icl-${FILE} ${${ICLLibsToLinkAgainst}})
    install (TARGETS icl-${FILE} RUNTIME DESTINATION bin)
    #message(STATUS "added ${FILE} to ${PROJECT_NAME} examples.")
  endif()
endmacro()

macro(add_gtest PROJECT_N FILE CONDITIONLIST ICLLibsToLinkAgainst)
  set(COND TRUE)	
  foreach(CONDITION ${CONDITIONLIST})
    if(NOT ${CONDITION})
      set(COND FALSE)
    endif()
  endforeach()
  if(${COND})
message(STATUS "hoho: ${FILE}")
#SET (LATEX_COMPILE latex)

#SET(DOC_ROOT ${Test_SOURCE_DIR}/Documentation)

ADD_CUSTOM_TARGET (check COMMAND 

add_executable(icl-test-${PRJECT_N} ${FILE})
target_link_libraries(icl-test-${PRJECT_N} ${ICLLibsToLinkAgainst})

					WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PRJECT_N}/test
                    
#                    SOURCES ${FILE}
)
#ADD_CUSTOM_COMMAND(
 #   SOURCE    ${DOC_ROOT}/junk.tex
  #  COMMAND   ${LATEX_COMPILE}
   # ARGS      ${DOC_ROOT}/junk.tex
    #TARGET    LaTeXDocument
    #OUTPUTS   ${Test_BINARY_DIR}/junk.dvi
#)
#ADD_CUSTOM_COMMAND(
 #   SOURCE    LaTeXDocument
  #  TARGET    LaTeXDocument
   # DEPENDS   ${Test_BINARY_DIR}/junk.dvi
#)

	#message(STATUS "${PROJECT_N} ${FILE}")
    add_executable(icl-${FILE} test/${FILE}.cpp)
    target_link_libraries(icl-${FILE} ${${ICLLibsToLinkAgainst}})
    #install (TARGETS icl-${FILE} RUNTIME DESTINATION bin)
    #message(STATUS "added ${FILE} to ${PROJECT_NAME} examples.")
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
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES doc/html)
  endif()
endmacro()

#write pkg config files
macro(icl_create_pkg_config_file ICL_SUB_PACKAGE_PLACEHOLDER REQUIRE OPTIONAL_INCLUDES OPTIONAL_LIBS)
  set(ICL_PACKAGE_DESCRIPTION "ICL's ${ICL_SUB_PACKAGE_PLACEHOLDER} package")
  set(LIBS "-L\${exec_prefix}/lib -l${ICL_SUB_PACKAGE_PLACEHOLDER} '-Wl,-rpath -Wl,\${exec_prefix}/lib ${${OPTIONAL_LIBS}}'")
  set(INCLUDES "-I\${prefix}/include/ICL ${${OPTIONAL_INCLUDES}}")
  configure_file(pkg.in ${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}.pc @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}.pc" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/)
  set(${OPTIONAL_INCLUDES} "")
  set(${OPTIONAL_LIBS} "")
endmacro()
#

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
      set(OPTIONAL_INCLUDES "${OPTIONAL_INCLUDES} -I${${REQ}_INCLUDE_PATH} -DHAVE_${REQ}")
      set(OPTIONAL_LIBS "${OPTIONAL_LIBS} -L${${REQ}_LIB_PATH}")
      foreach(LIBRARY ${${REQ}_LIBS_l})
	set(OPTIONAL_LIBS "${OPTIONAL_LIBS} -l${LIBRARY}")
      endforeach()
      set(OPTIONAL_LIBS "${OPTIONAL_LIBS} -Wl,-rpath -Wl,${${REQ}_LIB_PATH}")
    endif()
  endforeach()
  set(ICL_PACKAGE_DESCRIPTION "ICL's ${ICL_SUB_PACKAGE_PLACEHOLDER} package")
  set(LIBS "-L\${libdir} -l${ICL_SUB_PACKAGE_PLACEHOLDER} '-Wl,-rpath -Wl,\${libdir} ${OPTIONAL_LIBS}'")
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
