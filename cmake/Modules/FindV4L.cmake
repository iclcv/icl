#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindV4L.cmake                            **
#** Module : FindV4L                                                **
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

INCLUDE(FindPackageHandleStandardArgs)

# check old-style v4l/v4l2 within linux kernel
# here, there is no extra library -> functions are part of the kernel
FIND_PATH(V4L_INCLUDE_DIR 
          NAMES linux/videodev.h linux/videodev2.h sys/ioctl.h sys/mman.h fcntl.h
          PATHS /usr ${V4L_ROOT}
          PATH_SUFFIXES "include" 	  
          DOC "The path to Old-Style Video4Linux header file")
          
IF(NOT ${V4L_INCLUDE_DIR} MATCHES "V4L_INCLUDE_DIR_NOT_FOUND")
  SET(V4L_INCLUDE_DIRS ${V4L_INCLUDE_DIR})
  SET(V4L_LIBRARIES "")
  SET(V4L_FOUND TRUE)
  MESSAGE(STATUS "Found old-style V4L headers in ${V4L_INCLUDE_DIR}")
  ADD_DEFINITIONS( -DUSE_VIDEODEV2_HEADER)
ELSE()
  MESSAGE(STATUS "Not Found: old-style V4L headers -> searching for v4l library")
  # Search V4L_ROOT first if it is set.
  IF(V4L_ROOT)
    SET(_V4L_SEARCH_ROOT PATHS ${V4L_ROOT} NO_DEFAULT_PATH)
    LIST(APPEND _V4L_SEARCHES _V4L_SEARCH_ROOT)
  ENDIF()
  
  # Normal search
  SET(_V4L_SEARCH_NORMAL PATHS "/usr")
  
  # Set search paths and libraries
  IF (ICL_64BIT)
    set (_LIB_SEARCH_PATH_SUFFIXES "/lib/x86_64-linux-gnu" "/lib64")
  ELSE()
    set (_LIB_SEARCH_PATH_SUFFIXES "/lib/i386-linux-gnu" "/lib")
  ENDIF()
  
  LIST(APPEND _V4L_SEARCHES _V4L_SEARCH_NORMAL)
  LIST(APPEND _V4L_LIBRARIES v4l2 v4l1 v4lconvert)
  
  # Try each search configuration
  FOREACH(_PATH ${_V4L_SEARCHES})
    FIND_PATH(V4L_INCLUDE_DIR 
      NAMES libv4l2.h libv4l1.h        
      PATHS ${${_PATH}}
      PATH_SUFFIXES "include" 	  
      DOC "The path to Video4Linux header files"
      NO_DEFAULT_PATH)
    
    FOREACH(_lib ${_V4L_LIBRARIES})
      FIND_LIBRARY(${_lib}_LIBRARY  
        NAMES ${_lib}
	PATHS ${${_PATH}}
	PATH_SUFFIXES ${_LIB_SEARCH_PATH_SUFFIXES}
	NO_DEFAULT_PATH)
    ENDFOREACH()
  ENDFOREACH()
  
  # Handle the QUIETLY and REQUIRED arguments and set V4L_FOUND to TRUE if 
  # all listed variables are TRUE
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(V4L REQUIRED_VARS 
    v4l1_LIBRARY 
    V4L_INCLUDE_DIR)
  
  IF(V4L_FOUND)
    # HACK: Until FIND_LIBRARY could handle multiple libraries
    SET(_V4L_LIBRARIES_LIST)
    FOREACH(_lib ${_V4L_LIBRARIES})
      LIST(APPEND _V4L_LIBRARIES_LIST "${${_lib}_LIBRARY}")
    ENDFOREACH()
    
    SET(V4L_INCLUDE_DIRS ${V4L_INCLUDE_DIR})
    SET(V4L_LIBRARIES ${_V4L_LIBRARIES_LIST})
  ENDIF()
  
ENDIF()

MARK_AS_ADVANCED(V4L_INCLUDE_DIR)
  