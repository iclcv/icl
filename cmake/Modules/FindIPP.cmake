#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindIPP.cmake                            **
#** Module : FindIPP                                                **
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

INCLUDE(FindPackageHandleStandardArgs)

SET(IPP_ROOT "" CACHE PATH "Define IPP root directory if not default")

# ---- Detect IPP version ----
function(get_ipp_version _ROOT_DIR)
  set(_VERSION_STR)
  set(_MAJOR)
  set(_MINOR)
  set(_BUILD)

  # read IPP version info from file
  file(STRINGS ${_ROOT_DIR}/ipp/include/ippversion.h STR1 REGEX "IPP_VERSION_MAJOR")
  file(STRINGS ${_ROOT_DIR}/ipp/include/ippversion.h STR2 REGEX "IPP_VERSION_MINOR")
  file(STRINGS ${_ROOT_DIR}/ipp/include/ippversion.h STR3 REGEX "IPP_VERSION_BUILD")
  file(STRINGS ${_ROOT_DIR}/ipp/include/ippversion.h STR4 REGEX "IPP_VERSION_STR")
  
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


# ---------------------------------------------------------------------
# Start main part here
# ---------------------------------------------------------------------

# Search IPP_ROOT first if it is set.
IF(IPP_ROOT)
  SET(_IPP_SEARCH_ROOT PATHS ${IPP_ROOT} ${IPP_ROOT}/lib ${IPP_ROOT}/ipp NO_DEFAULT_PATH)
  LIST(APPEND _IPP_SEARCHES _IPP_SEARCH_ROOT)
ENDIF()

# Normal search.
SET(_IPP_SEARCH_NORMAL
     PATHS "/opt/IPP"
           "/opt/IPP/lib"
           "/opt/IPP/ipp"
   )
LIST(APPEND _IPP_SEARCHES _IPP_SEARCH_NORMAL)
LIST(APPEND _IPP_LIBRARIES ippcore ippi ipps ippcv ippm ippcc iomp5)

# Set search path suffix
IF (ICL_64BIT)
  set (_LIB_SEARCH_PATH_SUFFIXES "/lib/intel64")
ELSE()
  set (_LIB_SEARCH_PATH_SUFFIXES "/lib/ia32")
ENDIF()

# Try each search configuration
FOREACH(_PATH ${_IPP_SEARCHES})
  FIND_PATH(IPP_INCLUDE_DIR 
            NAMES ipp.h        
	    PATHS ${${_PATH}}
	    PATH_SUFFIXES "include" 	  
	    DOC "The path to Intel(R) IPP header files"
	    NO_DEFAULT_PATH)
  
    FOREACH(_lib ${_IPP_LIBRARIES})
      FIND_LIBRARY(${_lib}_LIBRARY  
               NAMES ${_lib}
	       PATHS ${${_PATH}}
	       PATH_SUFFIXES ${_LIB_SEARCH_PATH_SUFFIXES}
	       NO_DEFAULT_PATH)
    ENDFOREACH()
ENDFOREACH()
	   
# Handle the QUIETLY and REQUIRED arguments and set IPP_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(IPP REQUIRED_VARS 
				  ippcore_LIBRARY
				  ippi_LIBRARY 
				  ipps_LIBRARY 
				  ippcv_LIBRARY 
				  ippm_LIBRARY 
				  ippcc_LIBRARY
				  iomp5_LIBRARY
				  IPP_INCLUDE_DIR)

IF(IPP_FOUND)
  # HACK: Until FIND_LIBRARY could handle multiple libraries
  FOREACH(_lib ${_IPP_LIBRARIES})
    LIST(APPEND _IPP_LIBRARIES_LIST ${${_lib}_LIBRARY})
  ENDFOREACH()

  IF(EXISTS "${IPP_INCLUDE_DIR}/ippversion.h")
    get_ipp_version(${IPP_ROOT})
  ENDIF()
  
  LIST(REMOVE_DUPLICATES _IPP_LIBRARIES_LIST)
  SET(IPP_INCLUDE_DIRS ${IPP_INCLUDE_DIR})
  SET(IPP_LIBRARIES ${_IPP_LIBRARIES_LIST})

  STRING(REGEX REPLACE "[^/]*\\.so$" "" IPP_LIB_DIR ${ippcore_LIBRARY})
  STRING(REGEX REPLACE "[^/]*\\.so$" "" IOMP_LIB_DIR ${iomp5_LIBRARY})

  #  MESSAGE(STATUS "########################## ${_dir}")
  # for all IPP_LIBRARIES
  #   find library directory (by removing libname.so)
  #   add dir to list
  # remove duplicates
  #FOREACH(_lib ${_IPP_LIBRARIES})
  #  MESSAGE(STATUS "############ lib is ${_lib}")
  #  STRING(REGEX REPLACE "(.*)/[^/]*\\.so$" "${CMAKE_MATCH_0}" _dir ${_lib})
  #  MESSAGE(STATUS "########################## ${_dir}")
  #ENDFOREACH()

ENDIF()

MARK_AS_ADVANCED(IPP_INCLUDE_DIR IPP_ROOT)
