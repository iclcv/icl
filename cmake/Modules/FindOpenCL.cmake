#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindOpenCL.cmake                         **
#** Module : FindOpenCL                                             **
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

# ---------------------------------------------------------------------
# Start main part here
# ---------------------------------------------------------------------

IF(WIN32)
  # Ask the root directory of OpenCL.
  IF(NOT OPENCL_ROOT)
    SET(OPENCL_ROOT $ENV{CUDA_PATH} CACHE PATH "Root directory of OpenCL")
  ELSE()
    SET(OPENCL_ROOT OPENCL_ROOT CACHE PATH "Root directory of OpenCL")
  ENDIF()

  # Look for the header files
  FIND_PATH(OPENCL_INCLUDE_DIR 
            NAMES CL/cl.hpp CL/cl.h CL/opencl.h      
            PATHS ${OPENCL_ROOT}
            PATH_SUFFIXES "include"
            DOC "The path to OPENCL header files"
            NO_DEFAULT_PATH)

  IF(ICL_64BIT)
    FIND_LIBRARY(OPENCL_LIBRARY  
                 NAMES OpenCL
                 PATHS ${OPENCL_ROOT}
                 PATH_SUFFIXES "/lib" "/lib/x64"
                 NO_DEFAULT_PATH)
  ELSE()
    FIND_LIBRARY(OPENCL_LIBRARY  
                 NAMES OpenCL
                 PATHS ${OPENCL_ROOT}
                 PATH_SUFFIXES "/lib" "/lib/Win32"
                 NO_DEFAULT_PATH)
  ENDIF()
ELSE(WIN32)
  # Search OPENCL_ROOT first if it is set.
  IF(OPENCL_ROOT)
    SET(_OPENCL_SEARCH_ROOT PATHS ${OPENCL_ROOT} ${OPENCL_ROOT}/lib NO_DEFAULT_PATH)
    LIST(APPEND _OPENCL_SEARCHES _OPENCL_SEARCH_ROOT)
  ENDIF()

  # Normal search.
  SET(_OPENCL_SEARCH_NORMAL
       PATHS "/usr"
     )
  LIST(APPEND _OPENCL_SEARCHES _OPENCL_SEARCH_NORMAL)

  # Set search path suffix
  SET (_LIB_SEARCH_PATH_SUFFIXES "/lib")

  # Try each search configuration
  FOREACH(_PATH ${_OPENCL_SEARCHES})
    FIND_PATH(OPENCL_INCLUDE_DIR 
              NAMES cl.hpp cl.h opencl.h      
        PATHS ${${_PATH}}
        PATH_SUFFIXES "include/CL" 	  
        DOC "The path to OPENCL header files"
        NO_DEFAULT_PATH)
    
    FIND_LIBRARY(OPENCL_LIBRARY  
                 NAMES OpenCL
                 PATHS ${${_PATH}}
                 PATH_SUFFIXES ${_LIB_SEARCH_PATH_SUFFIXES}
                 NO_DEFAULT_PATH)
  ENDFOREACH()
ENDIF(WIN32)
       
# Handle the QUIETLY and REQUIRED arguments and set OPENCL_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENCL REQUIRED_VARS 
                                  OPENCL_LIBRARY
                                  OPENCL_INCLUDE_DIR)

IF(OPENCL_FOUND)
  IF(WIN32)
    FILE(STRINGS ${OPENCL_INCLUDE_DIR}/CL/cl.h HAVE_CL_1_0 REGEX "CL_VERSION_1_0")
    FILE(STRINGS ${OPENCL_INCLUDE_DIR}/CL/cl.h HAVE_CL_1_1 REGEX "CL_VERSION_1_1")
    FILE(STRINGS ${OPENCL_INCLUDE_DIR}/CL/cl.h HAVE_CL_1_2 REGEX "CL_VERSION_1_2")
  ELSE(WIN32)
    FILE(STRINGS ${OPENCL_INCLUDE_DIR}/cl.h HAVE_CL_1_0 REGEX "CL_VERSION_1_0")
    FILE(STRINGS ${OPENCL_INCLUDE_DIR}/cl.h HAVE_CL_1_1 REGEX "CL_VERSION_1_1")
    FILE(STRINGS ${OPENCL_INCLUDE_DIR}/cl.h HAVE_CL_1_2 REGEX "CL_VERSION_1_2")
  ENDIF(WIN32)

  
  IF("${HAVE_CL_1_2}" STREQUAL "")
    IF("${HAVE_CL_1_1}" STREQUAL "")
      message(STATUS "Found OpenCL Version: 1.0")
    ELSE()
      message(STATUS "Found OpenCL Version: 1.1")
    ENDIF()
  ELSE()
    message(STATUS "Found OpenCL Version: 1.2 (adding compatibility definition for 1.1)")
    ADD_DEFINITIONS( -DCL_USE_DEPRECATED_OPENCL_1_1_APIS)
  ENDIF()

  
  IF(NOT WIN32)
    # HACK: Until FIND_LIBRARY could handle multiple libraries
    FOREACH(_lib ${_OPENCL_LIBRARIES})
      LIST(APPEND _OPENCL_LIBRARIES_LIST ${${_lib}_LIBRARY})
    ENDFOREACH()

    LIST(REMOVE_DUPLICATES _OPENCL_LIBRARIES_LIST)
    SET(OPENCL_LIBRARIES ${_OPENCL_LIBRARIES_LIST})
  ENDIF(NOT WIN32)
  
  SET(OPENCL_INCLUDE_DIRS ${OPENCL_INCLUDE_DIR})
ENDIF()

#MARK_AS_ADVANCED(OPENCL_INCLUDE_DIR)
