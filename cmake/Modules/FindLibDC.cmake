#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindLibDC.cmake                          **
#** Module : FindLibDC                                              **
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

# Search LIBDC_ROOT first if it is set.
IF(LIBDC_ROOT)
  SET(_LIBDC_SEARCH_ROOT PATHS ${LIBDC_ROOT} ${LIBDC_ROOT}/lib ${LIBDC_ROOT}/ipp NO_DEFAULT_PATH)
  LIST(APPEND _LIBDC_SEARCHES _LIBDC_SEARCH_ROOT)
ENDIF()

# Normal search.
SET(_LIBDC_SEARCH_NORMAL
     PATHS "/usr"
   )
LIST(APPEND _LIBDC_SEARCHES _LIBDC_SEARCH_NORMAL)
LIST(APPEND _LIBDC_LIBRARIES dc1394)

# Set search path suffix
IF (ICL_64BIT)
  set (_LIB_SEARCH_PATH_SUFFIXES "/lib/x86_64-linux-gnu" "/lib64")
ELSE()
  set (_LIB_SEARCH_PATH_SUFFIXES "/lib/i386-linux-gnu" "/lib")
ENDIF()

# Try each search configuration
FOREACH(_PATH ${_LIBDC_SEARCHES})
  FIND_PATH(LIBDC_INCLUDE_DIR 
            NAMES dc1394.h        
	    PATHS ${${_PATH}}
	    PATH_SUFFIXES "include/dc1394" 	  
	    DOC "The path to LIBDC header files"
	    NO_DEFAULT_PATH)
  
    FOREACH(_lib ${_LIBDC_LIBRARIES})
      FIND_LIBRARY(${_lib}_LIBRARY  
               NAMES ${_lib}
	       PATHS ${${_PATH}}
	       PATH_SUFFIXES ${_LIB_SEARCH_PATH_SUFFIXES}
	       NO_DEFAULT_PATH)
    ENDFOREACH()
ENDFOREACH()
	   
# Handle the QUIETLY and REQUIRED arguments and set LIBDC_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBDC REQUIRED_VARS 
				  dc1394_LIBRARY
				  LIBDC_INCLUDE_DIR)

IF(LIBDC_FOUND)
  # HACK: Until FIND_LIBRARY could handle multiple libraries
  FOREACH(_lib ${_LIBDC_LIBRARIES})
    LIST(APPEND _LIBDC_LIBRARIES_LIST ${${_lib}_LIBRARY})
  ENDFOREACH()

  LIST(REMOVE_DUPLICATES _LIBDC_LIBRARIES_LIST)
  SET(LIBDC_INCLUDE_DIRS ${LIBDC_INCLUDE_DIR})
  SET(LIBDC_LIBRARIES ${_LIBDC_LIBRARIES_LIST})
ENDIF()

MARK_AS_ADVANCED(LIBDC_INCLUDE_DIR)
