#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/ICLFindPackage.cmake                     **
#** Module : ICLFindPackage                                         **
#** Authors: Sergius Gaulik                                         **
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

# Input:
#   NAME                 - name of the package
#   HEADERS              - will try to find include directory for these headers
#   LIBS                 - will try to find these libraries
#   PATHS                - these paths will be used to find the include directory and the libraries
#   HEADER_PATH_SUFFIXES - path suffixes will be used for the include directory search
#   LIB_PATH_SUFFIXES    - path suffixex will be used for the libraries search
#   OPTIONAL             - if set the package is optional and there will be no error message
# Output:
#   NAME_FOUND           - shows if the package was found
#   NAME_INCLUDE_DIRS    - include directory
#   NAME_LIBRARIES       - all found libraries
#   ICL_HAVE_NAME        - this variable is added to the icl definitions
# Global input:
#  PKG_SEARCH_PATHS         - contains directories, which should be searched
#  ARCH_DEPENDENT_LIB_PATHS - suffixes should be used for the libraries search
MACRO(ICL_FIND_PACKAGE)

  # Get all arguments
  SET(options OPTIONAL)
  SET(oneValueArgs NAME)
  SET(multiValueArgs HEADERS LIBS PATHS HEADER_PATH_SUFFIXES LIB_PATH_SUFFIXES)
  CMAKE_PARSE_ARGUMENTS(_PKG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  STRING(TOUPPER ${_PKG_NAME} _PKG_NAME_UPPER)

  IF(BUILD_WITH_${_PKG_NAME_UPPER}_OPTIONAL)
    SET(_PKG_OPTIONAL TRUE)
  ENDIF()

  SET(${_PKG_NAME_UPPER}_FOUND TRUE)

  # Set all search paths
  SET(_PKG_SEARCH_PATHS "")
  IF(${_PKG_NAME_UPPER}_ROOT)
    LIST(APPEND _PKG_SEARCH_PATHS ${${_PKG_NAME_UPPER}_ROOT})
  ENDIF()
  FOREACH(_PATH ${PKG_SEARCH_PATHS})
    LIST(APPEND _PKG_SEARCH_PATHS ${_PATH})
  ENDFOREACH()
  FOREACH(_PATH ${_PKG_PATHS})
    LIST(APPEND _PKG_SEARCH_PATHS ${_PATH})
  ENDFOREACH()

  # Search for a CMake script
  #FOREACH(_PATH ${_PKG_SEARCH_PATHS})
    #MESSAGE(STATUS "searching in ${_PATH}")

  #  IF(EXISTS "${_PATH}/${_PKG_NAME}Config.cmake")
      # Include the CMake script of the lib
  #    INCLUDE("${_PATH}/${_PKG_NAME}Config.cmake")

      # TODO: set variables if not set
      #SET(${_PKG_NAME}_INCLUDE_DIRS bla)
      #SET(${_PKG_NAME}_LIBRARIES bla)
      # found variable
      # break
  #  ENDIF()
  #ENDFOREACH()

  SET(_PKG_MISSING "")

  # Search for the include directory
  IF(_PKG_HEADERS)
    #MESSAGE(STATUS "TARGET: ${_PKG_NAME_UPPER}_INCLUDE_DIR")
    #MESSAGE(STATUS "NAMES: ${_PKG_HEADERS}")
    #MESSAGE(STATUS "PATHS: ${_PKG_SEARCH_PATHS}")
    #MESSAGE(STATUS "PATH_SUFFIXES include ${_PKG_HEADER_PATH_SUFFIXES} ${ARCH_DEPENDENT_INCLUDE_PATHS}")
    
    FIND_PATH(${_PKG_NAME_UPPER}_INCLUDE_DIR
      NAMES ${_PKG_HEADERS}
      PATHS ${_PKG_SEARCH_PATHS}
      PATH_SUFFIXES "include" ${_PKG_HEADER_PATH_SUFFIXES} ${ARCH_DEPENDENT_INCLUDE_PATHS}
      DOC "The path to ${_PKG_NAME} header files"
      NO_DEFAULT_PATH)

    IF(NOT ${_PKG_NAME_UPPER}_INCLUDE_DIR)
      LIST(APPEND _PKG_MISSING ${_PKG_NAME_UPPER}_INCLUDE_DIR)
    ELSE()
      SET(${_PKG_NAME_UPPER}_INCLUDE_DIRS ${${_PKG_NAME_UPPER}_INCLUDE_DIR})
    ENDIF()
  ENDIF()

  # nice for debugging ...
  #MESSAGE(STATUS "searching headers for -${_PKG_NAME_UPPER}- NAMES:  -${_PKG_HEADERS}- PATHS: -${_PKG_SEARCH_PATHS}- PATH_SUFFIXES -${_PKG_HEADER_PATH_SUFFIXES}-")

  # Search for all listed libraries
  FOREACH(_LIB ${_PKG_LIBS})
    FIND_LIBRARY(${_LIB}_LIBRARY
      NAMES ${_LIB}
      PATHS ${_PKG_SEARCH_PATHS}
      PATH_SUFFIXES "lib" ${ARCH_DEPENDENT_LIB_PATHS} ${_PKG_LIB_PATH_SUFFIXES}
      NO_DEFAULT_PATH)
    #message(STATUS "\nLIB: ${_LIB}\nSEARCH_PATH: ${_PKG_SEARCH_PATHS}\nARCH: ${ARCH_DEPENDENT_LIB_PATHS}\n FOUND: ${${_LIB}_LIBRARY}")

    IF(NOT ${_LIB}_LIBRARY)
	  #check for debug versions
	  FIND_LIBRARY(${_LIB}_LIBRARY
	    NAMES ${_LIB}_Debug
	    PATHS ${_PKG_SEARCH_PATHS}
	    PATH_SUFFIXES "lib" ${ARCH_DEPENDENT_LIB_PATHS} ${_PKG_LIB_PATH_SUFFIXES}
	    NO_DEFAULT_PATH)
	  IF(NOT ${_LIB}_LIBRARY)
		LIST(APPEND _PKG_MISSING ${_LIB}_LIBRARY)
      ELSE()
        LIST(APPEND ${_PKG_NAME_UPPER}_LIBRARIES ${${_LIB}_LIBRARY})
      ENDIF()
    ELSE()
      LIST(APPEND ${_PKG_NAME_UPPER}_LIBRARIES ${${_LIB}_LIBRARY})
    ENDIF()
  ENDFOREACH()

  IF(_PKG_MISSING)
    SET(${_PKG_NAME_UPPER}_FOUND FALSE)
    IF(_PKG_OPTIONAL)
      MESSAGE(STATUS "Could NOT find OPTIONAL ${_PKG_NAME} (missing: ${_PKG_MISSING}, continuing...)")
    ELSE(_PKG_OPTIONAL)
      MESSAGE(SEND_ERROR "Could NOT find ${_PKG_NAME} (missing: ${_PKG_MISSING})")
    ENDIF(_PKG_OPTIONAL)
    # Ask the root directory of the package
    SET(${_PKG_NAME_UPPER}_ROOT ${${_PKG_NAME_UPPER}_ROOT} CACHE PATH "Root directory of ${_PKG_NAME}")
  ELSE(_PKG_MISSING)
    ADD_ICL_DEFINITIONS(-DICL_HAVE_${_PKG_NAME_UPPER})
    MESSAGE(STATUS "Found ${_PKG_NAME}: ${${_PKG_NAME_UPPER}_INCLUDE_DIRS} ${${_PKG_NAME_UPPER}_LIBRARIES}")
  ENDIF(_PKG_MISSING)

ENDMACRO()
